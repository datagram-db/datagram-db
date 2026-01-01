//
// Created by giacomo on 21/12/24.
//

#ifndef GSM2_DATALOADERFROMSCHEMA_H
#define GSM2_DATALOADERFROMSCHEMA_H

#include <schema_language/schemaBaseVisitor.h>
#include <parser/schema/NestingState.h>
#include <yaucl/data/RocksDBMap.h>
#include <yaucl/structures/any_to_uint_bimap.h>

#include "rapidjson/filereadstream.h"

#include <parser/parser.h>

// #include "simdjson/concepts.h"

#include "simdjson.h"

struct SchemaReader : public schemaBaseVisitor, public DataReader {

    std::string namespace_;
    std::string loading_filename;
    FileType type;
    bool has_csv_header;
    bool first_pass;
    std::string sep;

    virtual ~SchemaReader() {};

    // TODO: secondary memory
    std::unordered_map<std::string, std::unordered_map<std::string, Entity>> loading_with_scheme;
    std::vector<std::pair<std::string,std::string>> loading_with_scheme2;

    std::any visitName_declar(schemaParser::Name_declarContext *ctx) override;

    std::any visitFile(schemaParser::FileContext *ctx) override;

    std::any visitCsv(schemaParser::CsvContext *ctx) override;

    bool readFromPath(const std::string& path) override;

    std::any visitLanguage(schemaParser::LanguageContext *context) override;
    std::any visitEntity_declaration(schemaParser::Entity_declarationContext *context) override;
     std::any visitLocal_entity_declaration(schemaParser::Local_entity_declarationContext *ctx) override ;
     std::any visitNative_field(schemaParser::Native_fieldContext *ctx) override ;
    std::any visitExternal_reference(schemaParser::External_referenceContext *ctx) override ;
   std::any visitLocal_nested_entity_list(schemaParser::Local_nested_entity_listContext *ctx) override ;
     std::any visitLocal_nested_entity(schemaParser::Local_nested_entityContext *ctx) override ;
     std::any visitMulti_nested_entity(schemaParser::Multi_nested_entityContext *ctx) override ;
     std::any visitT_string(schemaParser::T_stringContext *ctx) override;
     std::any visitT_uint(schemaParser::T_uintContext *ctx) override ;
    std::any visitT_int(schemaParser::T_intContext *ctx) override ;
    std::any visitT_double(schemaParser::T_doubleContext *ctx) override ;

    size_t globalObjectId = 0;

    typedef yaucl::structures::RocksDBStringSizeTMap map_type;
    std::map<std::tuple<size_t,size_t,size_t>,map_type> mampa;
    yaucl::structures::any_to_uint_bimap<std::string> namespace_idx;
    yaucl::structures::any_to_uint_bimap<std::string> entities_idx;
    yaucl::structures::any_to_uint_bimap<std::string> fields_idx;

    const Field* resolve(const std::string& namespace_, const std::string& entity_, const std::string& field_) const {
        auto it1 = loading_with_scheme.find(namespace_);
        if (it1 == loading_with_scheme.end())
            return nullptr;
        auto it2 = it1->second.find(entity_);
        if (it2 == it1->second.end())
            return nullptr;
        auto it3 = it2->second.find(field_);
        if (!it3.has_value())
        return nullptr;
        return it3.value();
    }

    inline void update(const std::string& ns, const std::string& e, const std::string& f, const std::string& value, size_t set) {
        std::tuple<size_t, size_t, size_t> s{namespace_idx.put(ns).first,entities_idx.put(e).first,fields_idx.put(f).first};
        (mampa[s]).put(value,set);
    }

    inline size_t retrieve(const std::string& ns, const std::string& e, const std::string& f, const std::string& value) {
        std::tuple<size_t, size_t, size_t> s{namespace_idx.put(ns).first,entities_idx.put(e).first,fields_idx.put(f).first};
        auto result = (mampa[s]).get(value);
        DEBUG_ASSERT(result.has_value());
        return result.value();
    }

    std::vector<NestingState> state_stack;
    bool _isFirstPass;


    bool load_xml(const Entity& e, bool isFirstPass);
    bool load_csv(const Entity& e, bool isFirstPass);

    // JSON Parser
    bool load_ljson(const Entity& e, bool isFirstPass) {
        std::cerr << "Loading LJSON " << e.loading_filename << " with pass " << isFirstPass << std::endl;
        simdjson::dom::parser parser;
        auto result = parser.load_many(e.loading_filename);
        if (!result.has_value())
            return false;
        for (simdjson::dom::element doc : result.value()) {
            // simdjson::dom::parser parser;
            state_stack.emplace_back(&e);
            // rapidjson::Reader reader;
            if (!print_json(doc))
                return false;
            state_stack.clear();
            return true;
        }
        return true;
        // std::string line;
        // std::ifstream infile{e.loading_filename};
        // while (std::getline(infile, line))
        // {
            // if (!load_json(e, isFirstPass, line))
                // return false;
        // }
        // return true;
    }
    bool load_json(const Entity& e, bool isFirstPass, const std::string& json = "");


    bool print_json(simdjson::dom::element element) {
        switch (element.type()) {
            case simdjson::dom::element_type::ARRAY: {
                if (!StartArray())
                    return false;
                for (simdjson::dom::element child : simdjson::dom::array(element)) {
                    if (!print_json(child))
                        return false;
                }
                if (!EndArray(0))
                    return false;
                return true;
            }
                break;
            case simdjson::dom::element_type::OBJECT: {
                if (!StartObject())
                    return false;
                for (simdjson::dom::key_value_pair field : simdjson::dom::object(element)) {
                    if (!Key(field.key.data(), field.key.length(), false))
                        return false;
                    // cout << "\"" << field.key << "\": ";
                    if (!print_json(field.value))
                        return false;
                }
                // cout << "}";
                if (!EndObject(0))
                    return false;
                return true;
                break;
            }

            case simdjson::dom::element_type::INT64:
                return Int64(int64_t(element));

            case simdjson::dom::element_type::UINT64:
                return Uint64(uint64_t(element));

            case simdjson::dom::element_type::DOUBLE:
                return Double(double(element));

            case simdjson::dom::element_type::STRING: {
                auto str = std::string_view(element);
                return String(str.data(), str.length(), false);
            }
            case simdjson::dom::element_type::BOOL:
                return Bool(bool(element));

            case simdjson::dom::element_type::NULL_VALUE:
                return Null();
                // break;
            default:
                return false;
        }
    }
    bool Null();
    bool Bool(bool b);
    bool Double(double d) ;


    bool Int(int d);
    bool Uint(unsigned d);

    bool Int64(int64_t d) ;
    bool Uint64(uint64_t d) ;
    bool RawNumber(const char* str, rapidjson::SizeType length, bool copy) ;
    bool String(const char* str, rapidjson::SizeType length, bool copy) ;
    bool StartObject() ;
    bool Key(const char* str, rapidjson::SizeType length, bool copy) ;
    bool EndObject(rapidjson::SizeType memberCount) ;
    bool StartArray();
    bool EndArray(rapidjson::SizeType elementCount);




};



#endif //GSM2_DATALOADERFROMSCHEMA_H
