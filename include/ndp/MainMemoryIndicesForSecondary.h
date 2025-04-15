//
// Created by giacomo on 12/04/25.
//

#ifndef GSM2_MAINMEMORYINDICESFORSECONDARY_H
#define GSM2_MAINMEMORYINDICESFORSECONDARY_H

#include <yaucl/data/SimpleStringSerializer.h>
#include <ndp/main_memory_index.h>
#include <database/gsm_object.h>
#include <database/PhiTable.h>


#include <lemon/static_graph.h>
#include <lemon/dijkstra.h>

struct QuickGraph {
    std::vector<std::pair<int,int> > arcs;
    std::unordered_map<size_t, int> obj_to_idx;
    std::vector<size_t> idx_to_obj;
    lemon::StaticDigraph gr;
    size_t n = 0;
    using LengthMap =  lemon::concepts::ReadMap<lemon::StaticDigraph::Arc,int> ;

    std::vector<size_t> dijkstra(size_t src, size_t dst) {
        std::vector<size_t> result;
        auto ptr_src = obj_to_idx.find(src);
        if (ptr_src == obj_to_idx.end())
            return result;
        auto ptr_dst = obj_to_idx.find(dst);
        if (ptr_dst == obj_to_idx.end())
            return result;
        std::vector<int> min_distance;
        std::vector<int> previous;
        min_distance.resize(n, n*n);
        min_distance[ptr_src->second] = 0;
        previous.resize(n, -1);
        std::set<std::pair<int, int> > vertex_queue;
        vertex_queue.insert(std::make_pair(min_distance[ptr_src->second], ptr_src->second));

        while (!vertex_queue.empty())
        {
            int dist = vertex_queue.begin()->first;
            int u = vertex_queue.begin()->second;
            vertex_queue.erase(vertex_queue.begin());
            auto node = gr.nodeFromId(u);

            // Visit each edge exiting u

            for (lemon::StaticDigraph::OutArcIt a(gr, node); a != lemon::INVALID; ++a) {
                auto v = lemon::StaticDigraph::id(gr.target(a));
                constexpr int weight = 1;
                auto distance_through_u = dist + weight;
                if (distance_through_u < min_distance[v]) {
                    vertex_queue.erase(std::make_pair(min_distance[v], v));

                    min_distance[v] = distance_through_u;
                    previous[v] = u;
                    vertex_queue.insert(std::make_pair(min_distance[v], v));

                }
            }
        }
        std::vector<size_t> path;
        auto dstF = ptr_dst->second;
        for ( ; dstF != -1; dstF = previous[dstF])
            path.emplace_back(idx_to_obj[dstF]);
        std::reverse(path.begin(), path.end());
        return path;
    }

    std::vector<size_t> kHop(size_t src, int k) {
        std::vector<size_t> result;
        auto ptr = obj_to_idx.find(src);
        if((k < 0) || (!isBuilt) || obj_to_idx.end() == ptr) {
            return result;
        }
        std::unordered_set<int> visited;
        std::deque<int> nodeQueue;
        nodeQueue.emplace_back(ptr->second);

        int currentDepth = 0,
                elementsToDepthIncrease = 1,
                nextElementsToDepthIncrease = 0;

        while (!nodeQueue.empty()) {
            int current = nodeQueue.front();
            if (!visited.emplace(current).second) {
                nodeQueue.pop_front();
                continue;
            }

            nodeQueue.pop_front();
            auto node = gr.node(current);
            nextElementsToDepthIncrease += lemon::countOutArcs(gr, node);
            if (--elementsToDepthIncrease == 0) {
                if (++currentDepth > k)
                    break;
                elementsToDepthIncrease = nextElementsToDepthIncrease;
                nextElementsToDepthIncrease = 0;
            }
            for (lemon::StaticDigraph::OutArcIt a(gr, node); a != lemon::INVALID; ++a)
                nodeQueue.emplace_back(lemon::StaticDigraph::id(gr.target(a)));
        }
        result.reserve(visited.size());
        for (int val : visited)
            result.emplace_back(idx_to_obj[val]);
        return result;
    }

    inline int add_node(size_t node) {
        if (!isBuilt) {
            auto result = obj_to_idx.emplace(node, n);
            if (result.second) {
                idx_to_obj.emplace_back(node);
                n++;
            }
            return result.first->second;
        }
        return -1;
    }

    inline void add_edge(size_t src, size_t dst) {
        if (!isBuilt) {
            arcs.emplace_back(add_node(src), add_node(dst));
        }
    }

    inline void add_certain_edge(int src, int dst) {
        if (!isBuilt) {
            arcs.emplace_back(src, dst);
        }
    }

    void build();
private:
    bool isBuilt = false;
};

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
    inline ssize_t activityId(const std::string& name) const {
        auto it = activity_name_to_offset.find(name);
        if (it == activity_name_to_offset.end())
            return -1;
        return it->second.activity_id;
    }
    inline const std::pair<const gsm2::tables::ActivityTable::secmem_record*,const gsm2::tables::ActivityTable::secmem_record*> getTable(const std::string& activity) const {
        auto it = activity_name_to_offset.find(activity);
        if (it == activity_name_to_offset.end())
            return {nullptr, nullptr};
        return {activity_table.begin() + it->second.offset_begin, activity_table.begin() + it->second.offset_end};
    }

    // Containment tables
    std::vector<std::string> phi_table_names;
    std::unordered_map<std::string, ContainmentInformation> containment_information;

    inline const std::vector<std::string>& getContainments() const { return phi_table_names; }
    inline bool hasContainment(const std::string&name) const {
        return containment_information.find(name) != containment_information.end();
    }
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
    inline std::pair<gsm2::tables::PhiTable::secmem_record*,gsm2::tables::PhiTable::secmem_record*> getContainments(size_t graph_id,
                                                                                                                    const std::string& name) const {
        if (graph_information.size() <= graph_id)
            return {nullptr, nullptr};
        const auto& ref = graph_information.at(graph_id);
        auto it = containment_information.find(name);
        if (it == containment_information.end())
            return {nullptr, nullptr};
        gsm2::tables::PhiTable::secmem_record* ptr = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
        return {ptr, ptr+it->second.table_length};
    }

    inline void fillContainmentsAsEdges(size_t graph_id,
                                QuickGraph& g,
                                const std::string& name) const {
        if (graph_information.size() <= graph_id)
            return;
        const auto& ref = graph_information.at(graph_id);
        auto it = containment_information.find(name);
        if (it == containment_information.end())
            return;
        gsm2::tables::PhiTable::secmem_record* ptr = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
        for (size_t i = 0; i<it->second.table_length; i++) {
            g.add_edge(ptr[i].object_id, ptr[i].id_contained);
        }
    }

    inline void fillObjectsByEll0AsEdges(size_t graph_id,
                                        QuickGraph& g,
                                        const std::string& name,
                                        const std::string& src,
                                        const std::string& dst) const {
        if (graph_information.size() <= graph_id)
            return;
        auto src_it = containment_information.find(src);
        if (src_it == containment_information.end())
            return;

        auto dst_it = containment_information.find(dst);
        if (dst_it == containment_information.end())
            return;
        auto cp = getTable(name);
        for (auto it = cp.first; it != cp.second; it++) {
            const auto& ref = graph_information.at(graph_id);
            size_t computd_offset_src = src_it->second.secondary_index_offset_header + (ref.n_obj_offset + it->event_id)*2;
            size_t begin_offset_src = containment_offsets[computd_offset_src];
            size_t end_offset_src = containment_offsets[computd_offset_src+1];
            size_t len = end_offset_src-begin_offset_src;
            if (len == 0)
                continue;
            size_t computd_offset_dst = dst_it->second.secondary_index_offset_header + (ref.n_obj_offset + it->event_id)*2;
            size_t begin_offset_dst = containment_offsets[computd_offset_dst];
            size_t end_offset_dst = containment_offsets[computd_offset_dst+1];
            size_t len2 = end_offset_dst-begin_offset_dst;
            if (len2 == 0)
                continue;
            gsm2::tables::PhiTable::secmem_record* ptr_src = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+src_it->second.secondary_memory_table_records_offset_begin);
            gsm2::tables::PhiTable::secmem_record* ptr_dst = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+dst_it->second.secondary_memory_table_records_offset_begin);
            std::vector<int> cache_j(len2), cache_i(len);
            for (int i = 0; i<len; i++) {
                auto idx_i = containment_offsets[src_it->second.secondary_index_offset_contents + begin_offset_src + i];
                const auto &current_src = ptr_src[idx_i];
                cache_i[i] = g.add_node(current_src.id_contained);
            }
            for (int j = 0; j<len2; j++) {
                auto idx_j = containment_offsets[dst_it->second.secondary_index_offset_contents + begin_offset_dst + j];
                const auto& current_dst = ptr_dst[idx_j];
                cache_j[j] = g.add_node(current_dst.id_contained);
            }
            for (int i = 0; i<len; i++) {
                for (int j = 0; j<len2; j++) {
                    g.add_certain_edge(cache_i[i], cache_j[j]);
                }
            }
        }
    }


    inline std::vector<const gsm2::tables::PhiTable::secmem_record*> getContainments(size_t graph_id, size_t object_id, const std::string& name) const {
        std::vector<const gsm2::tables::PhiTable::secmem_record*> result;
        if (graph_information.size() <= graph_id)
            return result;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return result;
        auto it = containment_information.find(name);
        if (it == containment_information.end())
            return result;
        size_t computd_offset = it->second.secondary_index_offset_header + (ref.n_obj_offset + object_id)*2;
//        size_t computd_offset = (it->second.containment_id * total_objects + ref.n_obj_offset + object_id)*2;
        DEBUG_ASSERT(computd_offset < containment_offsets.size());
        size_t begin_offset = containment_offsets[computd_offset];
        size_t end_offset = containment_offsets[computd_offset+1];
        size_t len = end_offset - begin_offset;
        if (len == 0)
            return result;
        gsm2::tables::PhiTable::secmem_record* ptr = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
        for (int i = 0; i<len; i++) {
            auto idx = containment_offsets[it->second.secondary_index_offset_contents + begin_offset + i];
            const auto& current = ptr[idx];
            result.emplace_back(&current);
        }
        return result;
    }
    inline std::vector<const gsm2::tables::PhiTable::secmem_record*> getInvContainments(size_t graph_id, size_t object_id, const std::string& name) const {
        std::vector<const gsm2::tables::PhiTable::secmem_record*> result;
        if (graph_information.size() <= graph_id)
            return result;
        const auto& ref = graph_information.at(graph_id);
        if (ref.objects.size() <= object_id)
            return result;
        auto it = containment_information.find(name);
        if (it == containment_information.end())
            return result;
        size_t computd_offset = it->second.secondary_index_offset_header + (ref.n_obj_offset + object_id)*2;
//        size_t computd_offset = (it->second.containment_id * total_objects + ref.n_obj_offset + object_id)*2;
        DEBUG_ASSERT(computd_offset < inv_containment_offsets.size());
        size_t begin_offset = inv_containment_offsets[computd_offset];
        size_t end_offset = inv_containment_offsets[computd_offset+1];
        size_t len = end_offset - begin_offset;
        if (len == 0)
            return result;
        gsm2::tables::PhiTable::secmem_record* ptr = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
        for (int i = 0; i<len; i++) {
            auto idx = inv_containment_offsets[it->second.secondary_index_offset_contents + begin_offset + i];
            const auto& current = ptr[idx];
            result.emplace_back(&current);
        }
        return result;
    }
//    inline gsm2::tables::PhiTable::secmem_record* getIthContainments(size_t graph_id, size_t object_id, const std::string& name, size_t i) const {
//        if (graph_information.size() <= graph_id)
//            return nullptr;
//        const auto& ref = graph_information.at(graph_id);
//        if (ref.objects.size() <= object_id)
//            return nullptr;
//        auto it = containment_information.find(name);
//        if (it == containment_information.end())
//            return nullptr;
//        size_t computd_offset = (it->second.containment_id * total_objects + ref.n_obj_offset + object_id)*2;
//        DEBUG_ASSERT(computd_offset < containment_offsets.size());
//        size_t begin_offset = containment_offsets[computd_offset];
//        size_t end_offset = containment_offsets[computd_offset+1];
//        size_t len = end_offset - begin_offset;
//        if (len <= i)
//            return nullptr;
//        gsm2::tables::PhiTable::secmem_record* ptr = (gsm2::tables::PhiTable::secmem_record*)(database_containment_table.raw_data()+it->second.secondary_memory_table_records_offset_begin);
//        return ptr + begin_offset + i;
//    }
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
//            containment.orig_edge_id = current.instance_id;
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
    size_t* getUintAttribute(size_t graph_id, size_t object_id, const std::string& name) const {
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
        if (it->second.type == gsm2::tables::AttributeTableType::SizeTAtt) {
            return &x.value;
        } else {
            return nullptr;
        }
    }
    double* getDoubleAttribute(size_t graph_id, size_t object_id, const std::string& name) const {
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
        if (it->second.type == gsm2::tables::AttributeTableType::DoubleAtt) {
            return (double*)(&x.value);
        } else {
            return nullptr;
        }
    }
    const char* getStringAttribute(size_t graph_id, size_t object_id, const std::string& name) const {
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
        if (it->second.type == gsm2::tables::AttributeTableType::StringAtt) {
            return string_attributes.str(x.value);
        } else {
            return nullptr;
        }
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
    inline void setAttributeToObject(size_t graph_id, size_t object_id, const std::string& name, lightweight_gsm_object&object) const {
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
                object.pi[name] = *(double*)(&x.value);
                break;
            case gsm2::tables::AttributeTableType::LongAtt:
                object.pi[name] = (double)*(long long*)(&x.value);
                break;
            case gsm2::tables::AttributeTableType::StringAtt:
                object.pi[name] = std::string{string_attributes.str(x.value),string_attributes.strlen(x.value)};
                break;
            case gsm2::tables::AttributeTableType::BoolAtt:
                object.pi[name] = (double)(((x.value != 0)) && (x.value != 0.0));
                break;
                //case SizeTAtt:
            default:
                // TODO: hierarchical types!, https://dl.acm.org/doi/10.1145/3410566.3410583
                object.pi[name] = (double)x.value;
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

    inline void import_properties(size_t from, lightweight_gsm_object& target) {
        for (const auto& attribute : attribute_names) {
            setAttributeToObject(0, from, attribute, target);
        }
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

//    void print() {
//        for (size_t graph_id = 0, N = getNGraphs(); graph_id < N; graph_id ++) {
//            for (size_t object_id = 0, M = getNObjects(graph_id); object_id < M; object_id++) {
//                gsm_object object;
//                reconstruct_object(graph_id, object_id, object);
//                object.out_json(std::cout);
//                std::cout << std::endl;
//            }
//        }
//    }


    yaucl::data::FixedSizeArrayElements<size_t> containment_offsets, inv_containment_offsets, attributes_offsets, database_attributes_table, database_containment_table;
    yaucl::data::FixedSizeArrayElements<gsm2::tables::ActivityTable::secmem_record> activity_table;
    NDPFuzzyStringMatching ells, xis;
    SimpleStringReader string_attributes;
    size_t n_graphs;

private:


};


#endif //GSM2_MAINMEMORYINDICESFORSECONDARY_H
