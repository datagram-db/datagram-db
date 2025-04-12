//
// Created by giacomo on 12/04/25.
//

#ifndef GSM2_MAINMEMORYINDICESFORSECONDARY_H
#define GSM2_MAINMEMORYINDICESFORSECONDARY_H

#include <yaucl/data/SimpleStringSerializer.h>
#include <ndp/main_memory_index.h>
#include <database/gsm_object.h>
#include <database/PhiTable.h>

struct MainMemoryIndicesForSecondary {
    std::vector <GraphInformation> graph_information;
    size_t total_objects;


    MainMemoryIndicesForSecondary(const std::filesystem::path &path);

    inline size_t getNGraphs() const { return graph_information.size(); }
    inline size_t getNObjects() const { return total_objects; }
    inline size_t getNObjects(size_t graphId) const {
        if (graph_information.size() <= graphId)
            return 0;
        DEBUG_ASSERT(graph_information.at(graphId).n_obj == graph_information.at(graphId).objects.size());
        return graph_information.at(graphId).n_obj;
    }

    // Activity Table                                                           ~ activities are the first label occurring for each object
    std::vector<std::string> activity_list;
    std::unordered_map<std::string, ActivityEntry> activity_name_to_offset;
    inline const std::pair<const gsm2::tables::ActivityTable::secmem_record*,const gsm2::tables::ActivityTable::secmem_record*> getTable(const std::string& activity) {
        auto it = activity_name_to_offset.find(activity);
        if (it == activity_name_to_offset.end())
            return {nullptr, nullptr};
        return {activity_table.begin() + it->second.offset_begin, activity_table.begin() + it->second.offset_end};
    }

    // Containment tables
    std::vector<std::string> phi_table_names;
    std::unordered_map<std::string, ContainmentInformation> containment_information;

    inline const std::vector<std::string>& getContainments() const { return phi_table_names; }
    inline const size_t numberOfContainments(size_t idx) const {
        if (idx >= attribute_names.size()) return 0;
        return numberOfContainments(attribute_names.at(idx));
    }
    inline const size_t numberOfContainments(const std::string&name) const {
        auto it = containment_information.find(name);
        if (it == containment_information.end()) return 0;
        return it->second.table_length;
    }
    inline size_t countContainments(size_t graph_id, size_t object_id, const std::string& name) const {
        if (graph_information.size() <= graph_id)
            return 0;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return 0;
        auto it = containment_information.find(name);
        if (it == containment_information.end())
            return 0;
        size_t computd_offset = (it->second.containment_id * total_objects + ref.n_obj_offset + object_id)*2;
        DEBUG_ASSERT(computd_offset < containment_offsets.size());
        size_t begin_offset = containment_offsets[computd_offset];
        size_t end_offset = containment_offsets[computd_offset+1];
        return end_offset - begin_offset;
    }
    inline gsm2::tables::PhiTable::secmem_record* getIthContainments(size_t graph_id, size_t object_id, const std::string& name, size_t i) const {
        if (graph_information.size() <= graph_id)
            return nullptr;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return nullptr;
        auto it = containment_information.find(name);
        if (it == containment_information.end())
            return nullptr;
        size_t computd_offset = (it->second.containment_id * total_objects + ref.n_obj_offset + object_id)*2;
        DEBUG_ASSERT(computd_offset < containment_offsets.size());
        size_t begin_offset = containment_offsets[computd_offset];
        size_t end_offset = containment_offsets[computd_offset+1];
        size_t len = end_offset - begin_offset;
        if (len <= i)
            return nullptr;
        gsm2::tables::PhiTable::secmem_record* ptr = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
        return ptr + begin_offset + i;
    }
    inline void setContainmentToObject(size_t graph_id, size_t object_id, const std::string& name, gsm_object& object) const {
        if (graph_information.size() <= graph_id)
            return ;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return ;
        auto it = containment_information.find(name);
        if (it == containment_information.end())
            return ;
        size_t computd_offset = it->second.secondary_index_offset_header + (ref.n_obj_offset + object_id)*2;
        DEBUG_ASSERT(computd_offset < containment_offsets.size());
        size_t begin_offset = containment_offsets[computd_offset];
        size_t end_offset = containment_offsets[computd_offset+1];
        size_t len = end_offset - begin_offset;
        if (len == 0)
            return;
        gsm_object_xi_content containment;
        gsm2::tables::PhiTable::secmem_record* ptr = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
        for (int i = 0; i<len; i++) {
            auto idx = containment_offsets[it->second.secondary_index_offset_contents + begin_offset + i];
            const auto& current = ptr[idx];
            containment.id = current.id_contained;
            containment.orig_edge_id = current.instance_id;
            containment.score = current.w_contained;
            object.phi[name].emplace_back(containment);
        }
    }

    // Attribute table
    std::vector<std::string> attribute_names;
    std::unordered_map<std::string, AttributeInformation> attribute_information;

    inline const std::vector<std::string>& getAttributes() const { return attribute_names; }
    inline const size_t attributeTableLength(size_t idx) const {
        if (idx >= attribute_names.size()) return 0;
        return attributeTableLength(attribute_names.at(idx));
    }
    inline const size_t attributeTableLength(const std::string& name) const {
        auto it = attribute_information.find(name);
        if (it == attribute_information.end()) return 0;
        return it->second.table_length;
    }
    std::optional<union_type> getAttribute(size_t graph_id, size_t object_id, const std::string& name) const {
        auto cp = hasAttribute(graph_id, object_id, name);
        if (!cp.second)
            return {};
        auto it = attribute_information.find(name);
        gsm2::tables::AttributeTable::record* ptr = (gsm2::tables::AttributeTable::record*)(database_attributes_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
#ifdef DEBUG
        size_t offset_to_displacement = cp.first * sizeof(gsm2::tables::AttributeTable::record*);
        DEBUG_ASSERT(it->second.secondary_memory_table_records_offset_begin + offset_to_displacement < it->second.secondary_memory_table_records_offset_end);
#endif
        auto& x = ptr[cp.first];
        switch (it->second.type) {
            case gsm2::tables::AttributeTableType::DoubleAtt:
                return {*(double*)(&x.value)};
            case gsm2::tables::AttributeTableType::LongAtt:
                return {*(long long*)(&x.value)};
            case gsm2::tables::AttributeTableType::StringAtt:
                return {std::string{string_attributes.str(x.value),string_attributes.strlen(x.value)} };
            case gsm2::tables::AttributeTableType::BoolAtt:
                return {((x.value != 0)) && (x.value != 0.0)};
                //case SizeTAtt:
            default:
                // TODO: hierarchical types!, https://dl.acm.org/doi/10.1145/3410566.3410583
                return {x.value};
        }
    }

    inline std::pair<size_t, bool> hasAttribute(size_t graph_id, size_t object_id, const std::string& name) const {
        std::pair<size_t, bool> result{-1, false};
        if (graph_information.size() <= graph_id)
            return result;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return result;
        auto it = attribute_information.find(name);
        if (it == attribute_information.end())
            return result;
        size_t computd_offset = it->second.attribute_id * total_objects + ref.objects.at(object_id).activity_table_entry_id_offset;
        DEBUG_ASSERT(computd_offset < attributes_offsets.size());
        result.first = attributes_offsets[computd_offset];
        if (result.first == (size_t)-1) {
            return result;
        }
        result.second = true;
        return result;
    }
    inline void setAttributeToObject(size_t graph_id, size_t object_id, const std::string& name, gsm_object&object) const {
        std::pair<size_t, bool> result{-1, false};
        if (graph_information.size() <= graph_id)
            return;// no update
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return;// no update
        auto it = attribute_information.find(name);
        if (it == attribute_information.end())
            return;// no update
        size_t computd_offset = it->second.attribute_id * total_objects + ref.objects.at(object_id).activity_table_entry_id_offset;
        DEBUG_ASSERT(computd_offset < attributes_offsets.size());
        result.first = attributes_offsets[computd_offset];
        if (result.first == (size_t)-1) {
            return; // no update
        }
        result.second = true;
        gsm2::tables::AttributeTable::record* ptr = (gsm2::tables::AttributeTable::record*)(database_attributes_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
#ifdef DEBUG
        size_t offset_to_displacement = result.first * sizeof(gsm2::tables::AttributeTable::record*);
        DEBUG_ASSERT(it->second.secondary_memory_table_records_offset_begin + offset_to_displacement < it->second.secondary_memory_table_records_offset_end);
#endif
        auto& x = ptr[result.first];
        switch (it->second.type) {
            case gsm2::tables::AttributeTableType::DoubleAtt:
                object.content[name] = *(double*)(&x.value);
                break;
            case gsm2::tables::AttributeTableType::LongAtt:
                object.content[name] = (double)*(long long*)(&x.value);
                break;
            case gsm2::tables::AttributeTableType::StringAtt:
                object.content[name] = std::string{string_attributes.str(x.value),string_attributes.strlen(x.value)};
                break;
            case gsm2::tables::AttributeTableType::BoolAtt:
                object.content[name] = (double)(((x.value != 0)) && (x.value != 0.0));
                break;
                //case SizeTAtt:
            default:
                // TODO: hierarchical types!, https://dl.acm.org/doi/10.1145/3410566.3410583
                object.content[name] = (double)x.value;
                break;
        }
    }



    inline size_t getNumberOfLabels(size_t graph_id, size_t objet_id) const {
        if (graph_information.size() <= graph_id)
            return 0;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= objet_id)
            return 0;
        return ref.objects[objet_id].ells.size();
    }

    inline std::string getIthLabel(size_t graph_id, size_t objet_id, size_t i) const {
        if (graph_information.size() <= graph_id)
            return "";
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= objet_id)
            return "";
        if (ref.objects[objet_id].ells.size() <= i)
            return "";
        return ells.get(ref.objects[objet_id].ells.at(i));
    }

    inline size_t getNumberOfValues(size_t graph_id, size_t objet_id) const {
        if (graph_information.size() <= graph_id)
            return 0;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= objet_id)
            return 0;
        return ref.objects[objet_id].xis.size();
    }

    inline std::string getIthValue(size_t graph_id, size_t objet_id, size_t i) const {
        if (graph_information.size() <= graph_id)
            return "";
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= objet_id)
            return "";
        if (ref.objects[objet_id].xis.size() <= i)
            return "";
        return xis.get(ref.objects[objet_id].xis.at(i));
    }

    void  reconstruct_object(size_t graph_id, size_t object_id, gsm_object& object) const {
        object.clear();
        object.id = object_id;
        if (getNGraphs() <= graph_id)
            return;
        if (getNObjects(graph_id) <= object_id)
            return;
        for (size_t n_label = 0, N = getNumberOfLabels(graph_id, object_id); n_label < N; n_label++) {
            object.ell.emplace_back(getIthLabel(graph_id, object_id, n_label));
        }
        for (size_t n_label = 0, N = getNumberOfValues(graph_id, object_id); n_label < N; n_label++) {
            object.xi.emplace_back(getIthValue(graph_id, object_id, n_label));
        }
        for (const auto& attribute : attribute_names) {
            setAttributeToObject(graph_id, object_id, attribute, object);
        }
        for (const auto& containment : phi_table_names) {
            setContainmentToObject(graph_id, object_id, containment, object);
        }
    }

    void print() {
        for (size_t graph_id = 0, N = getNGraphs(); graph_id < N; graph_id ++) {
            for (size_t object_id = 0, M = getNObjects(graph_id); object_id < M; object_id++) {
                gsm_object object;
                reconstruct_object(graph_id, object_id, object);
                object.out_json(std::cout);
                std::cout << std::endl;
            }
        }
    }


    yaucl::data::FixedSizeArrayElements<size_t> containment_offsets, attributes_offsets, database_attributes_table, database_containment_table;
    yaucl::data::FixedSizeArrayElements<gsm2::tables::ActivityTable::secmem_record> activity_table;
    NDPFuzzyStringMatching ells, xis;
    SimpleStringReader string_attributes;
    size_t n_graphs;

private:


};


#endif //GSM2_MAINMEMORYINDICESFORSECONDARY_H
