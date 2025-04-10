//
// Created by giacomo on 21/12/24.
//

#include <parser/schema/DataLoaderFromSchema.h>

#include <schema_language/schemaLexer.h>
#include <schema_language/schemaParser.h>
#include <parser/writer.h>


#include "rocksdb/convenience.h"

bool SchemaReader::readFromPath(const std::string &path) {
    std::ifstream data{path};
    antlr4::ANTLRInputStream input(data);
    schemaLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    schemaParser parser(&tokens);

    this->writer->initDatabase();
    auto outcomeForVisiting = visitLanguage(parser.language());
    bool result = std::any_cast<bool>(outcomeForVisiting);
    this->mampa.clear(); // killing all threads from rocksdb messing up the debugging

    this->writer->close();
    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// PARSER


std::any SchemaReader::visitLanguage(schemaParser::LanguageContext *context) {
    if (context) {
        for (const auto &ref: context->entity_declaration()) {
            visitEntity_declaration(ref);
        }

        globalObjectId = 0;
        for (auto &[namespac_, entity_name]: loading_with_scheme2) {
            {
                auto& entity = loading_with_scheme[namespac_][entity_name];
                switch (entity.type) {
                    case CSV:
                        if (!load_csv(entity, true))
                            return {false};
                        break;
                    case LJSON:
                        if (!load_ljson(entity, true))
                            return {false};
                        break;
                    case JSON:
                        if (!load_json(entity, true))
                            return {false};
                        break;
                    case XML:
                        if (!load_xml(entity, true))
                            return {false};
                        break;
                    default:
                        break;
                }
            }
        }

        globalObjectId = 0;
        for (auto &[namespac_, entity_name]: loading_with_scheme2) {
            {
                auto& entity = loading_with_scheme[namespac_][entity_name];
                switch (entity.type) {
                    case CSV:
                        if (!load_csv(entity, false))
                            return {false};
                        break;
                    case LJSON:
                        if (!load_ljson(entity, false))
                            return {false};
                        break;
                    case JSON:
                        if (!load_json(entity, false))
                            return {false};
                        break;
                    case XML:
                        if (!load_xml(entity, false))
                            return {false};
                        break;
                    default:
                        break;
                }
            }
        }
    }
    return {true};
}

#include <yaucl/data/json.h>

std::any SchemaReader::visitEntity_declaration(schemaParser::Entity_declarationContext *context)  {
    if (context) {
        auto file = context->file();
        if (file) {
            namespace_ = context->WORD()->getText();
            auto csv = file->csv();
            if (file->XML())
                type = XML;
            else if (file->JSON())
                type = JSON;
            else if (file->LJSON())
                type = LJSON;
            else if (csv) {
                type = CSV;
                has_csv_header = csv->WITH_HEADER();
                if (csv->SEP()) {
                    sep = UNESCAPE(csv->EscapedString()->getText());
                } else {
                    sep = ",";
                }
            } else {
                sep.clear();
                has_csv_header = false;
            }
            loading_filename = UNESCAPE(file->EscapedString()->getText());
            auto e = std::any_cast<Entity>(visitLocal_entity_declaration(context->local_entity_declaration()));
            auto& nss = loading_with_scheme[namespace_];
            if (!nss.contains(e.name)) {
                nss[e.name] = std::move(e);
            }
            loading_with_scheme2.emplace_back(namespace_,e.name);
        }
    }
    return {};
}

    std::any SchemaReader::visitLocal_entity_declaration(schemaParser::Local_entity_declarationContext *ctx)  {
        if (ctx) {
            std::string name = ctx->entity->getText();
            Entity e;
            e.name = name;
            e.namespace_ = this->namespace_;
            size_t declaration_offset = 0;
            for (auto& field : ctx->field()) {
                auto f = std::any_cast<Field>(visit(field));
                f.declaration_offset = declaration_offset;
                e.fields_order.emplace_back(f.field_name);
                f.entity = e.name;
                f.namespace_ = e.namespace_;
                e.fields.emplace(f.field_name, f);
                if (f.is_id) {
                    if (!e.id) {
                        e.id = f.field_name;
                    }
                }
                if (f.xml_property) {
                    e.attributes.emplace(f.field_name);
                }
                if (!f.map_to_actual_field_or_attribute.empty())
                    e.mapped_field_resolution[f.map_to_actual_field_or_attribute] = f.field_name;
                declaration_offset++;
            }
            e.has_csv_header = has_csv_header;
            e.loading_filename = loading_filename;
            e.type = type;
            e.has_csv_header = has_csv_header;
            e.sep = sep;
            auto& nss = loading_with_scheme[namespace_];
            if (!nss.contains(e.name))
                nss[e.name] = e;
            return e;
        }
        return {};
    }

     std::any SchemaReader::visitNative_field(schemaParser::Native_fieldContext *ctx)  {
        Field f;
        f.type = native_field;
        if (ctx) {
            f.is_id = ctx->STAR();
            f.xml_tag = ctx->TAG();
            f.xml_property = ctx->PROPERTY();
            if ((!f.xml_tag) && (!f.xml_property)) {
                f.xml_property = f.is_id;
                f.xml_tag = !f.xml_property;
            }
            f.field_name = ctx->name->getText();
            if (ctx->map) {
                f.map_to_actual_field_or_attribute = UNESCAPE(ctx->map->getText());
            }
            f.native_field_type = std::any_cast<NativeTypes>(visit(ctx->type()));
            if (ctx->SCORE()) {
                f.score= (f.native_field_type == NativeTypes::Double);
            }
        }
        return f;
    }

     std::any SchemaReader::visitExternal_reference(schemaParser::External_referenceContext *ctx)  {
        Field f;
        f.type = external_reference;
        if (ctx) {
            auto nd = ctx->name_declar();
            f.is_id = ctx->STAR();
            f.xml_tag = ctx->TAG();
            f.xml_property = ctx->PROPERTY();
            if ((!f.xml_tag) && (!f.xml_property)) {
                f.xml_tag = false;
                f.xml_property = true;
            }
            f.field_name = ctx->name->getText();
            if (ctx->map) {
                f.map_to_actual_field_or_attribute = UNESCAPE(ctx->map->getText());
            }
            if (nd) {
                f.ext_namespace = nd->namespace_->getText();
                f.ext_entity = nd->entity->getText();
            }
            f.ext_field = ctx->ext_field_name->getText();
        }
        return f;
    }

     std::any SchemaReader::visitLocal_nested_entity_list(schemaParser::Local_nested_entity_listContext *ctx)  {
        Field f;
        f.type = local_nested_entity_list;
        if (ctx) {
            f.is_id = ctx->STAR();
            f.xml_tag = true;
            f.xml_property = false;
            f.field_name = ctx->name->getText();
            if (ctx->map) {
                f.map_to_actual_field_or_attribute = UNESCAPE(ctx->map->getText());
            }
            f.collective = true;
            f.nested_decl.emplace_back(std::any_cast<Entity>(visitLocal_entity_declaration(ctx->local_entity_declaration())));
        }
        return f;
    }

     std::any SchemaReader::visitLocal_nested_entity(schemaParser::Local_nested_entityContext *ctx)  {
        Field f;
        f.type = local_nested_entity;
        if (ctx) {
            f.is_id = ctx->STAR();
            f.xml_tag = true;
            f.xml_property = false;
            f.field_name = ctx->name->getText();
            if (ctx->map) {
                f.map_to_actual_field_or_attribute = UNESCAPE(ctx->map->getText());
            }
            f.nested_decl.emplace_back(std::any_cast<Entity>(visitLocal_entity_declaration(ctx->local_entity_declaration())));
        }
        return f;
    }

     std::any SchemaReader::visitMulti_nested_entity(schemaParser::Multi_nested_entityContext *ctx)  {
        Field f;
        f.type = multi_nested_entity;
        if (ctx) {
            f.is_id = ctx->STAR();
            f.xml_tag = true;
            f.xml_property = false;
            f.field_name = ctx->name->getText();
            if (ctx->map) {
                f.map_to_actual_field_or_attribute = UNESCAPE(ctx->map->getText());
            }
            f.multi = true;
            f.nested_decl.emplace_back(std::any_cast<Entity>(visitLocal_entity_declaration(ctx->local_entity_declaration())));
        }
        return f;
    }

     std::any SchemaReader::visitT_string(schemaParser::T_stringContext *ctx)  {
        return NativeTypes::String;
    }

     std::any SchemaReader::visitT_uint(schemaParser::T_uintContext *ctx)  {
        return NativeTypes::UInt;
    }

     std::any SchemaReader::visitT_int(schemaParser::T_intContext *ctx)  {
        return NativeTypes::Int;
    }

     std::any SchemaReader::visitT_double(schemaParser::T_doubleContext *ctx)  {
        return NativeTypes::Double;
    }



#include <yaucl/data/csv/parser.hpp>
#include <yaucl/data/csv/csv.h>

bool SchemaReader::load_csv(const Entity& e, bool isFirstPass) {
    std::cerr << "Loading CSV " << e.loading_filename << " of '" << e.name << "' with pass " << isFirstPass << std::endl;
    // TODO: loader
//    gsm_object object;
//        std::vector<std::pair<std::string,union_minimal>> dr;
//        std::vector<std::pair<std::string,size_t>> cc;
//        std::vector<double> score;
    std::ifstream input_file(e.loading_filename);
    aria::csv::CsvParser parser = aria::csv::CsvParser(input_file);
    parser.delimiter(e.sep[0]);

    bool first_row = e.has_csv_header;
    table data;
//        std::vector<std::pair<std::string,union_minimal>> data_row;
//        std::vector<std::pair<std::string,size_t>> containment;
    std::vector<std::string> headers;
    std::unordered_map<std::string, size_t> S;
    std::vector<ssize_t> header_mapping;

    for (auto &row : parser) {
        if (first_row) {
            first_row = false;
            size_t offset = 0;
            for (auto &field : row) {
                S.emplace(field, offset);
                offset++;
            }
            header_mapping.resize(offset, -1);

            for (auto& declared_field_name: e.fields_order) {
                const auto f = e.fields.find(declared_field_name);
                if (f->second.map_to_actual_field_or_attribute.empty()) {
                    auto it = S.find(f->first);
                    if (it == S.end()) {
                        return false;
                    } else {
                        headers.push_back(declared_field_name);
                        header_mapping[it->second] = f->second.declaration_offset;
                    }
                } else {
                    const auto& first = f->second.map_to_actual_field_or_attribute;
                    auto it = S.find(first);
                    if (it == S.end()) {
                        return false;
                    } else {
                        headers.push_back(declared_field_name);
                        header_mapping[it->second] = f->second.declaration_offset;
                    }
                }
            }

            data.SetHeaders(headers);
            continue;
        }

        size_t idx = 0;
        gsm_object current;
        current.id = globalObjectId;
        for (auto &field : row) {
            auto offset = header_mapping.empty() ? idx : header_mapping[idx];
            if ((!field.empty()) && (offset != -1)) {
                auto it = e.find(e.fields_order[offset]);
                if (it.has_value()) {
                    auto val = it.value();
                    if (isFirstPass ) {
                        if ((val->is_id))
                            update(e.namespace_, e.name, val->field_name,field,globalObjectId);
                    } else {
                        auto type_ = val->native_field_type;
                        switch (val->type) {
                            case native_field:
                                break;
                            case external_reference:
                            {

                                auto it3 = this->resolve(val->ext_namespace,
                                                                     val->ext_entity,
                                                                     val->ext_field);
                                if (!it3)
                                    return false;

                                if ((!isFirstPass) && (it3->is_id)) {
                                    type_ = Skip;

                                    auto result = retrieve(val->ext_namespace,val->ext_entity,val->ext_field, field);
//                                    auto result = increasingIdCorrespondence[{val->ext_namespace,val->ext_entity,val->ext_field}][field];
//                                    cc.emplace_back(headers.empty() ? val->field_name : headers.at(idx), result);
                                    current.phi[headers.empty() ? val->field_name : headers.at(idx)].emplace_back(result);
                                } else
                                    type_ = it3->native_field_type;
                            }
                                break;
                            case local_nested_entity_list:
                            case local_nested_entity:
                            case multi_nested_entity:
                                return false;
                        }
                        if ((!val->score)) {
                            switch (type_) {
                                case NativeTypes::String:
                                    current.content.emplace(headers.empty() ? val->field_name : headers.at(idx), field);
                                    break;
                                case NativeTypes::UInt:
                                    current.content.emplace(headers.empty() ? val->field_name : headers.at(idx), (double)std::stoull(field));
                                    break;
                                case NativeTypes::Int:
                                    current.content.emplace(headers.empty() ? val->field_name : headers.at(idx), (double)std::stoll(field));
                                    break;
                                case NativeTypes::Double:
                                    current.content.emplace(headers.empty() ? val->field_name : headers.at(idx), std::stod(field));
                                    break;
                                default:
                                    break;
                            }
                        } else if (val->score && type_ == NativeTypes::Double) {
                            current.scores.emplace_back(std::stod(field));
                        }
                    }
                }
            }
            idx++;
        }

        if (isFirstPass) {
            // TODO: set the attributes value to the current object ids, and consider score
//            data.AddRow(dr);
        } else {
            // TODO: set the containment attributes to the current object ids
            this->writer->writeObject(current, {});
        }
//        dr.clear();
//        cc.clear();
        globalObjectId++;
    }
    return true;
}

#include <parser/xml.h>

bool SchemaReader::load_xml(const Entity& e, bool isFirstPass) {
    std::cerr << "Loading XML " << e.loading_filename << " of '" << e.name << "' with pass " << isFirstPass << std::endl;
    first_pass = true;
    state_stack.emplace_back(&e);
    _isFirstPass = isFirstPass;

    // Initialize all fields to zero
    xmlSAXHandler sh = { 0 };

    // register callback
    sh.startElement = start_element_callback;
    sh.endElement = end_element;
    sh.characters =character_callback;

    xmlParserCtxtPtr ctxt;

    // create the context
    if ((ctxt = xmlCreateFileParserCtxt(e.loading_filename.c_str() )) == NULL) {
        fprintf(stderr, "Erreur lors de la création du contexte\n");
        return EXIT_FAILURE;
    }
    // register sax handler with the context
    ctxt->sax = &sh;
    ctxt->userData = this;

    // parse the doc
    xmlParseDocument(ctxt);
    // well-formed document?
    return ctxt->wellFormed;
}

#include "rapidjson/filereadstream.h"
#include "rapidjson/reader.h"

bool SchemaReader::load_json(const Entity& e, bool isFirstPass, const std::string& json) {

    state_stack.emplace_back(&e);
    rapidjson::Reader reader;
    if (!json.empty()) {
//    std::cerr << "Loading JSON DATA with pass " << isFirstPass << std::endl;
        rapidjson::StringStream ss{json.c_str()};
        _isFirstPass = isFirstPass;
        if (!reader.Parse(ss, *this))
            return false;
    } else {
    std::cerr << "Loading JSON " << e.loading_filename << " of '" << e.name << "' with pass " << isFirstPass << std::endl;
        FILE *ff = fopen(e.loading_filename.c_str(), "r");
        char buffer[4096];
        rapidjson::FileReadStream ss{ff, buffer, sizeof(buffer)};
        _isFirstPass = isFirstPass;
        if (!reader.Parse(ss, *this))
            return false;
        fclose(ff);
    }
    state_stack.clear();
    return true;
}

    bool SchemaReader::Null() {  return true; }
    bool SchemaReader::Bool(bool val) {
        auto& toppe = *state_stack.rbegin();
        std::string val3=val ? "true" : "false";
        const auto& field_key = toppe.key;
        const auto val2F = toppe.entity_stack->find(field_key);
        DEBUG_ASSERT(val2F.has_value());
        auto val2 = val2F.value();
        if (_isFirstPass) {
            if ((val2->is_id)) {
                update(toppe.entity_stack->namespace_, toppe.entity_stack->name, val2->field_name,val3,globalObjectId);
            }
        } else {
            if (val2->asks_for_reference()) {
                auto result = retrieve(val2->ext_namespace, val2->ext_entity, val2->ext_field, val3);
                toppe.object.phi[field_key].emplace_back(result);
            } else {
                if (toppe.isThisScore) {
                    toppe.object.scores.emplace_back(val ? 1.0 : 0.0);
                } else if (toppe._keyType == NativeTypes::String) {
                    toppe.object.content.emplace(field_key, val3);
                } else {
                    toppe.object.content.emplace(field_key, val ? 1.0 : 0.0);
                }
            }
        }
        if (!toppe.key.empty())
            toppe.key.clear();
        toppe.isThisScore = false;
        toppe.key.clear();
        return true;
    }
    bool SchemaReader::Int(int val) {
        auto& toppe = *state_stack.rbegin();

        std::string val3=std::to_string(val);
        const auto& field_key = toppe.key;
        const auto val2F = toppe.entity_stack->find(field_key);
        DEBUG_ASSERT(val2F.has_value());
        auto val2 = val2F.value();
        if (_isFirstPass) {
            if ((val2->is_id)) {
                update(toppe.entity_stack->namespace_, toppe.entity_stack->name, val2->field_name,val3,globalObjectId);
            }
        } else {
            if (val2->asks_for_reference()) {
                auto result = retrieve(val2->ext_namespace, val2->ext_entity, val2->ext_field, val3);
                toppe.object.phi[field_key].emplace_back(result);
            } else {
                if (toppe.isThisScore)
                    toppe.object.scores.emplace_back(val);
                else if (toppe._keyType == NativeTypes::String) {
                    toppe.object.content.emplace(field_key, val3);
                } else {
                    toppe.object.content.emplace(field_key, (double)val);
                }
            }
        }
        if (!toppe.key.empty())
            toppe.key.clear();
        toppe.isThisScore = false;
        toppe.key.clear();
        return true;
    }
    bool SchemaReader::Uint(unsigned val) {
        auto& toppe =*state_stack.rbegin();
        std::string val3=std::to_string(val);
        const auto& field_key = toppe.key;
        const auto val2F = toppe.entity_stack->find(field_key);
        DEBUG_ASSERT(val2F.has_value());
        auto val2 = val2F.value();
        if (_isFirstPass) {
            if ((val2->is_id)) {
                update(toppe.entity_stack->namespace_, toppe.entity_stack->name, val2->field_name,val3,globalObjectId);
            }
        } else {
            if (val2->asks_for_reference()) {
                auto result = retrieve(val2->ext_namespace, val2->ext_entity, val2->ext_field, val3);
                toppe.object.phi[field_key].emplace_back(result);
            } else {
                if (toppe.isThisScore)
                    toppe.object.scores.emplace_back(val);
                else if (toppe._keyType == NativeTypes::String) {
                    toppe.object.content.emplace(field_key, val3);
                } else {
                    toppe.object.content.emplace(field_key, (double)val);
                }
            }
        }
        if (!toppe.key.empty())
            toppe.key.clear();
        toppe.isThisScore=false;
        toppe.key.clear();
        return true;
    }
    bool SchemaReader::Int64(int64_t val) {
        auto& toppe = *state_stack.rbegin();
        std::string val3=std::to_string(val);
        const auto& field_key = toppe.key;
        const auto val2F = toppe.entity_stack->find(field_key);
        DEBUG_ASSERT(val2F.has_value());
        auto val2 = val2F.value();
        if (_isFirstPass) {
            if ((val2->is_id)) {
                update(toppe.entity_stack->namespace_, toppe.entity_stack->name, val2->field_name,val3,globalObjectId);
            }
        } else {
            if (val2->asks_for_reference()) {
                auto result = retrieve(val2->ext_namespace, val2->ext_entity, val2->ext_field, val3);
                toppe.object.phi[field_key].emplace_back(result);
            } else {
                if (toppe.isThisScore)
                    toppe.object.scores.emplace_back(val);
                else if (toppe._keyType == NativeTypes::String) {
                    toppe.object.content.emplace(field_key, val3);
                } else {
                    toppe.object.content.emplace(field_key, (double)val);
                }
            }
        }
        if (!toppe.key.empty())
            toppe.key.clear();
        toppe.isThisScore = false;
        toppe.key.clear();
        return true;
    }
    bool SchemaReader::Uint64(uint64_t val) {
        auto& toppe = *state_stack.rbegin();
        std::string val3=std::to_string(val);
        const auto& field_key = toppe.key;
        const auto val2F = toppe.entity_stack->find(field_key);
        DEBUG_ASSERT(val2F.has_value());
        auto val2 = val2F.value();
        if (_isFirstPass) {
            if ((val2->is_id)) {
                update(toppe.entity_stack->namespace_, toppe.entity_stack->name, val2->field_name,val3,globalObjectId);
            }
        } else {
            if (val2->asks_for_reference()) {
                auto result = retrieve(val2->ext_namespace, val2->ext_entity, val2->ext_field, val3);
                toppe.object.phi[field_key].emplace_back(result);
            } else {
                if (toppe.isThisScore)
                    toppe.object.scores.emplace_back(val);
                else if (toppe._keyType == NativeTypes::String) {
                    toppe.object.content.emplace(field_key, val3);
                } else {
                    toppe.object.content.emplace(field_key, (double)val);
                }
            }
        }
        if (!toppe.key.empty())
            toppe.key.clear();
        toppe.isThisScore = false;
        toppe.key.clear();
        return true;
    }
    bool SchemaReader::Double(double val) {
        auto& toppe = *state_stack.rbegin();
        std::string val3=std::to_string(val);
        const auto& field_key = toppe.key;
        const auto val2F = toppe.entity_stack->find(field_key);
        DEBUG_ASSERT(val2F.has_value());
        auto val2 = val2F.value();
        if (_isFirstPass) {
            if ((val2->is_id)) {
                update(toppe.entity_stack->namespace_, toppe.entity_stack->name, val2->field_name,val3,globalObjectId);
            }
        } else {
            if (val2->asks_for_reference()) {
                auto result = retrieve(val2->ext_namespace, val2->ext_entity, val2->ext_field, val3);
                toppe.object.phi[field_key].emplace_back(result);
            } else {
                if (toppe.isThisScore)
                    toppe.object.scores.emplace_back(val);
                else if (toppe._keyType == NativeTypes::String) {
                    toppe.object.content.emplace(field_key, val3);
                } else {
                    toppe.object.content.emplace(field_key, val);
                }
            }
        }
        if (!toppe.key.empty())
            toppe.key.clear();
        toppe.isThisScore = false;
        toppe.key.clear();
        return true;
    }
    bool SchemaReader::RawNumber(const char* str, rapidjson::SizeType length, bool copy) {
        auto& toppe = *state_stack.rbegin();
        const auto val2K = toppe.entity_stack->find(toppe.key);
        DEBUG_ASSERT(val2K.has_value());
        auto key = val2K.value();
//        auto key = toppe.entity_stack->find(toppe.key).value();
        auto isId = key->is_id;
        auto isExtRef = key->type == external_reference;
        auto val3 = std::string(str, length);
        auto number = std::stod(val3);

        const auto& field_key = toppe.key;
        const auto val2F = toppe.entity_stack->find(field_key);
        DEBUG_ASSERT(val2F.has_value());
        auto val2 = val2F.value();
        if (_isFirstPass) {
            if ((val2->is_id)) {
                update(toppe.entity_stack->namespace_, toppe.entity_stack->name, val2->field_name,val3,globalObjectId);
            }
        } else {
            if (val2->asks_for_reference()) {
                auto result = retrieve(val2->ext_namespace, val2->ext_entity, val2->ext_field, val3);
                toppe.object.phi[field_key].emplace_back(result);
            } else {
                if (toppe.isThisScore)
                    toppe.object.scores.emplace_back(number);
                else if (toppe._keyType == NativeTypes::String) {
                    toppe.object.content.emplace(field_key, val3);
                } else {
                    toppe.object.content.emplace(field_key, (double)number);
                }
            }
        }
        if (!toppe.key.empty())
            toppe.key.clear();
        toppe.skipCurrentKey= false;
        toppe.key.clear();
        return true;
    }
    bool SchemaReader::String(const char* str, rapidjson::SizeType length, bool copy) {
        auto& toppe = *state_stack.rbegin();
        std::string val3(str, length);
        const auto& field_key = toppe.key;
        const auto val2F = toppe.entity_stack->find(field_key);
        DEBUG_ASSERT(val2F.has_value());
        auto val2 = val2F.value();
        if (_isFirstPass) {
            if ((val2->is_id)) {
                update(toppe.entity_stack->namespace_, toppe.entity_stack->name, val2->field_name,val3,globalObjectId);
            }
        } else {
            if (val2->asks_for_reference()) {
                auto result = retrieve(val2->ext_namespace, val2->ext_entity, val2->ext_field, val3);
                toppe.object.phi[field_key].emplace_back(result);
            } else {
                if (toppe.isThisScore)
                    toppe.object.scores.emplace_back(std::stod(val3));
                else if (toppe._keyType == NativeTypes::String) {
                    toppe.object.content.emplace(field_key, val3);
                } else {
                    toppe.object.content.emplace(field_key, std::stod(val3));
                }
            }
        }
        if (!toppe.key.empty())
            toppe.key.clear();
        toppe.skipCurrentKey = false;
        toppe.key.clear();
        return true;
    }
    bool SchemaReader::StartObject() {
        auto it = state_stack.rbegin();
        it->countAt--;
        it->object.id = globalObjectId;
        return true;
    }

    bool SchemaReader::Key(const char* str, rapidjson::SizeType length, bool copy) {
        auto& toppe = *state_stack.rbegin();
        const auto& e = toppe.entity_stack;
        toppe.key = std::string(str, length);
        toppe.skipCurrentKey = !_isFirstPass;
//        if ((toppe.key == "TotalPrice") && (!_isFirstPass))
//            std::cerr << "HERE" << std::endl;
        auto itv2 = e->find(toppe.key);
        DEBUG_ASSERT(itv2.has_value());
//        auto it = e->fields.find(toppe.key);
        if (itv2.has_value()) {
            toppe._keyType = itv2.value()->native_field_type;
            switch (itv2.value()->type) {
                case native_field:
                    toppe.isThisScore = itv2.value()->score;
                    break;
                case external_reference:
                {
                    if (!_isFirstPass) {
                        auto it1 = loading_with_scheme.find(itv2.value()->ext_namespace);
                        if (it1 == loading_with_scheme.end())
                            return false;
                        auto it2 = it1->second.find(itv2.value()->ext_entity);
                        if (it2 == it1->second.end())
                            return false;
                        auto it3 = it2->second.fields.find(itv2.value()->ext_field);
                        if (it3 == it2->second.fields.end())
                            return false;
                        toppe.skipCurrentKey = false;
                    } else {
                        toppe.skipCurrentKey = true;
                    }
                }
                    break;
                case local_nested_entity_list:
                case local_nested_entity:
                case multi_nested_entity: {
                    if ((itv2.value()->nested_decl.empty()))
                        return false;
                    state_stack.emplace_back(&itv2.value()->nested_decl[0]);
                    state_stack.rbegin()->countFor = state_stack.rbegin()->countAt = itv2.value()->type == local_nested_entity ? 1 : 2;
                }
                    break;
            }
        }
        return true;
    }
    bool SchemaReader::EndObject(rapidjson::SizeType memberCount) {
        // TODO: serialize the object in the stack
        auto it = state_stack.rbegin();
        it->countAt++;
        it->object.id = globalObjectId;
        it->object.ell = {it->entity_stack->name};
        if (!this->_isFirstPass) {
            this->writer->writeObject(it->object, {});
            it->object.clear();
        }
//        if ((it->entity_stack->name == "OrderLine") || (it->entity_stack->name == "Orderline")) {
//            std::cerr << "member" << std::endl;
//        }
//        if ((it->countAt == 0) || (it->countFor == 0)) {
//            std::cout << "BUG" << std::endl;
//        }
        if (it->countAt == it->countFor) {
            auto prev = it;
            prev++;
            // TODO: associate it->data_row (first) or it-> containment to globalObjectId;

            it->object.clear();
            if (it->countAt != 0)
                prev->object.phi[prev->key].emplace_back(globalObjectId);
            // TODO: else, we reached the end, so you can directly store this object within the database!
            state_stack.pop_back();
        } else {
            // associate it->data_row (first) or it-> containment to globalObjectId;
            it->objects.emplace_back(globalObjectId);
        }
        globalObjectId++;
        return true;
    }
    bool SchemaReader::StartArray() {
        state_stack.rbegin()->countAt--;
        return true;
    }
    bool SchemaReader::EndArray(rapidjson::SizeType elementCount) {
        auto it = state_stack.rbegin();
        it->countAt++;
        if (it->countAt == it->countFor) {
            auto prev = it;
            prev++;
            for (size_t object : it->objects)
                prev->object.phi[prev->key].emplace_back( object);
            state_stack.pop_back();
        } else {
            // associate it->data_row (first) or it-> containment to globalObjectId;
            it->objects.emplace_back(globalObjectId);
            globalObjectId++;
        }
        return true;
    }

std::any SchemaReader::visitName_declar(schemaParser::Name_declarContext *ctx) {
    return {};
}

std::any SchemaReader::visitFile(schemaParser::FileContext *ctx) {
    return {};
}

std::any SchemaReader::visitCsv(schemaParser::CsvContext *ctx) {
    return {};
}
