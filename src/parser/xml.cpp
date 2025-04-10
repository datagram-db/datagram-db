//
// Created by giacomo on 21/12/24.
//

#include <parser/xml.h>
#include <parser/schema/DataLoaderFromSchema.h>
#include <parser/writer.h>

static inline
void load_attribute_value(SchemaReader *loader, NestingState &toppe, std::string &val, const std::string& key = "") {
    std::string field_key = key.empty() ? toppe.key : key;
    const auto ref = toppe.entity_stack->find(field_key);
    DEBUG_ASSERT(ref.has_value());
    auto val2 = ref.value();
    if (loader->_isFirstPass) {
        if ((val2->is_id)) {
            loader->update(toppe.entity_stack->namespace_, toppe.entity_stack->name, val2->field_name,val,loader->globalObjectId);
        }
    } else {
        if (val2->asks_for_reference()) {
            auto result = loader->retrieve(val2->ext_namespace, val2->ext_entity, val2->ext_field, val);
            toppe.object.phi[field_key].emplace_back(result);
        } else {
            if (toppe._keyType != String) {
                toppe.object.content.emplace(field_key,std::stod(val));
            } else {
                toppe.object.content.emplace(field_key, val);
            }
        }
    }
    if (!toppe.key.empty())
        toppe.key.clear();
}

void start_element_callback(void *ctx, const xmlChar *name, const xmlChar **attrs) {
    SchemaReader* loader = (SchemaReader*)ctx;
    std::string tag{(char*)name};
    auto& toppe = *loader->state_stack.rbegin();
    const auto& e = toppe.entity_stack;
    toppe.key = tag;//std::string(str, length);
    toppe.skipCurrentKey = !loader->_isFirstPass;
    auto itv2 = e->find(toppe.key);
    DEBUG_ASSERT(itv2.has_value());
    if ( (itv2.has_value() && (itv2.value()->xml_tag))) {
        toppe._keyType = itv2.value()->native_field_type;
        auto root =  itv2.value()->type;

        switch (root) {
            case native_field:
                if (!(loader->first_pass)) {
                    loader->state_stack.rbegin()->current_tag_name = tag;
                }
                return;
            case external_reference:
            {
            }
                break;
            case local_nested_entity_list:
            case local_nested_entity:
            case multi_nested_entity: {
                loader->state_stack.emplace_back(&itv2.value()->nested_decl[0]).current_tag_name = tag;
                auto& local_toppe = *loader->state_stack.rbegin();
                while (NULL != attrs && NULL != attrs[0]) {
                    std::string key{(char*)attrs[0]};
                    auto dis = local_toppe.entity_stack->find(key);
                    DEBUG_ASSERT(dis.has_value());
                    if ((dis.has_value()) && (dis.value()->xml_property)) {
                        std::string val{(char*)attrs[1]};
                        load_attribute_value(loader, local_toppe, val, key);
                    }
                    attrs = &attrs[2];
                }
            }
                break;
        }
    }

}

void end_element(void *user_data, const xmlChar *name) {
    SchemaReader* loader = (SchemaReader*)user_data;
    std::string tag{(char*)name};
    auto it = loader->state_stack.rbegin();
    auto fieldInfo = it->entity_stack->find(tag);
    if ((fieldInfo.has_value()) && (fieldInfo.value()->type == native_field || fieldInfo.value()->type == external_reference)) {
        // Just closing a normal tag, doing nothign associated with it
    } else if ((!fieldInfo.has_value()) && (it->current_tag_name == tag)) {
        {
            auto prev = it;
            prev++;
            it->object.id = loader->globalObjectId;
            if (!loader->_isFirstPass) {
                it->object.ell.emplace_back(it->entity_stack->name);
                loader->writer->writeObject(it->object, {});
            }
            it->object.clear();
            prev->object.phi[prev->key].emplace_back( loader->globalObjectId);
            loader->globalObjectId++;
            loader->state_stack.pop_back();
        }
    }
}


#include <yaucl/strings/string_utils.h>



void character_callback(void *ctx,
                        const xmlChar *str,
                        int length) {
    SchemaReader* loader = (SchemaReader*)ctx;
    auto& toppe = *loader->state_stack.rbegin();
    std::string val((char*)str, length);
    yaucl::strings::trim(val);
    if ((!val.empty()) && (!toppe.key.empty())) {
        load_attribute_value(loader, toppe, val);
    }
}