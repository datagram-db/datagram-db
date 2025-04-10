/*
 * LinearGSM.cpp
 * This file is part of gsm_gsql
 *
 * Copyright (C) 2023 - Giacomo Bergami
 *
 * gsm_gsql is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * gsm_gsql is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gsm_gsql. If not, see <http://www.gnu.org/licenses/>.
 */

 
//
// Created by giacomo on 28/05/23.
//

#include "database/LinearGSM.h"


namespace gsm2 {
    namespace tables {
        std::pair<double,double> primary_memory_load_gsm2(const std::filesystem::path& path, gsm2::tables::LinearGSM& db) {
            auto schema = path.parent_path() / "schema.txt";
            std::unordered_map<std::string, gsm2::tables::AttributeTableType> propertyname_to_type;
            if (exists(schema)) {
                std::ifstream fschema{schema};
                std::string line;
                size_t count = 0;
                std::string field_name;
                gsm2::tables::AttributeTableType field_type = gsm2::tables::AttributeTableType::StringAtt;
                while (std::getline(fschema, line)) {
                    count++;
                    if (count == 1)
                        field_name = line;
                    else {
                        auto val = magic_enum::enum_cast<gsm2::tables::AttributeTableType>(line);
                        if (val)
                            field_type = val.value();
                        else
                            field_type = gsm2::tables::AttributeTableType::StringAtt;
                        propertyname_to_type[field_name] = field_type;
                    }
                }
            }

            double loading = 0.0, indexing = 0.0;
            {
                auto loading_start = std::chrono::high_resolution_clock::now();
                std::string buffer;
                std::ifstream f(path);
                f.seekg(0, std::ios::end);
                size_t n = f.tellg();
                buffer.resize(f.tellg());
                f.seekg(0);
                f.read(buffer.data(), buffer.size());
                parse(buffer.data(), n, db, propertyname_to_type, db.nGraphs);
                db.nGraphs++;
                auto loading_end = std::chrono::high_resolution_clock::now();
                loading = std::chrono::duration<double, std::milli>(loading_end-loading_start).count();
            }
            {
                auto indexing_start = std::chrono::high_resolution_clock::now();
                db.index();
                auto indexing_end = std::chrono::high_resolution_clock::now();
                indexing = std::chrono::duration<double, std::milli>(indexing_end-indexing_start).count();
            }
            return {loading, indexing};
        }

        std::pair<double, double> primary_memory_load_direct(std::vector<std::vector<gsm_object>> &db,
                                                             const std::unordered_map<std::string, gsm2::tables::AttributeTableType> &schema,
                                                             LinearGSM &target) {
            size_t noLabel = initLoading(target);
            double loading = 0.0, indexing = 0.0;
            {
                auto loading_start = std::chrono::high_resolution_clock::now();
                {
                    for (auto& actual_db : db) {
                        yaucl::structures::any_to_uint_bimap<size_t> conversion;
                        for (auto& obj : actual_db) {
                            obj.id = conversion.put(obj.id).first;
                            for (auto& [k,v] : obj.phi) {
                                for (auto& e : v) {
                                    e.id = conversion.put(e.id).first;
                                }
                            }
                        }
                    }
                    std::pair<size_t, size_t> cp;
                    target.nodesInGraph.clear();
                    for (cp.first = 0; cp.first<db.size(); cp.first++) {
                        const auto& actual_db = db.at(cp.first);
                        target.nodesInGraph.emplace_back(actual_db.size());
                        for (const auto& obj : actual_db) {
                            cp.second = obj.id;
                            target.objectScoresLoading[cp] = obj.scores;
                            auto act = loadObjectEll2(target, obj.ell, noLabel, cp);
                            registerObjectByFirstLabel(target, act, cp);
                            loadObjectXi2(target, obj.xi, cp);
                            for (const auto& [key,val] : obj.content) {
                                if (std::holds_alternative<std::string>(val))
                                    loadObjectProperty(target, schema, act, cp, key, std::get<std::string>(val));
                                else
                                    loadObjectProperty(target, schema, act, cp, key, std::to_string(std::get<double>(val)));
                            }

                            for (const auto& [k,u] : obj.phi) {
                                for (const auto& v : u) {
                                    target.containment_tables[k].add(act, cp, v.score, v.id);
                                    target.containment_relationships.emplace(k);
                                }
                            }
                        }
                    }
                }
                target.nGraphs = db.size();
                auto loading_end = std::chrono::high_resolution_clock::now();
                loading = std::chrono::duration<double, std::milli>(loading_end-loading_start).count();
            }
            {
                auto indexing_start = std::chrono::high_resolution_clock::now();
                target.index();
                auto indexing_end = std::chrono::high_resolution_clock::now();
                indexing = std::chrono::duration<double, std::milli>(indexing_end-indexing_start).count();
            }
            target.nGraphs = db.size();
            return {loading, indexing};
        }

        std::pair<double,double> LinearGSM::load_databases(std::vector<std::vector<gsm_object>> &db,
                                                const std::unordered_map<std::string, gsm2::tables::AttributeTableType> &schema
        ) {
            return primary_memory_load_direct(db, schema, *this);
        }

        void LinearGSM::clear() {
            label_map.clear();
            containment_tables.clear();
            containment_relationships.clear();
            KeyValueProperties.clear();
            ell_values.clear();
            xi_values.clear();
            main_registry.clear();
        }

        void LinearGSM::secondary_memory_index(const std::filesystem::path& path) {
            if (! std::filesystem::exists(path)) {
                std::filesystem::create_directories(path);
            }

            // 1. First phase indexing of Activity Tables
            auto idx = main_registry.secondary_memory_indexing1(path);
            std::vector<size_t> graphs_to_n_objects;
            {
                auto activities_names = std::fstream(path / "activities_list.txt", std::ios::out);
                for (const auto& label : label_map.int_to_T) {
                    activities_names << label << std::endl;
                }
            }

            // 2. Serializing the secondary memory object scores
            {
                size_t total_offset = 0;
                // These files will be eventually merged into one, so to avoid multiple iterations in primary memory
//                auto graph_index_offset = std::fstream(path / "database_index.binary", std::ios::out | std::ios::binary);
                auto object_index_offset = std::fstream(path / "object_index_with_scores_offset.binary", std::ios::out | std::ios::binary);
                auto scores_data = std::fstream(path / "actual_scores.binary", std::ios::out | std::ios::binary);


                // Writing how many graphs are there
//                graph_index_offset.write((const char*)&nGraphs, sizeof(nGraphs));
                object_index_offset.write((const char*)&nGraphs, sizeof(nGraphs));


                graphs_to_n_objects.reserve(idx.size());
                for (size_t database_id = 0; database_id < idx.size(); database_id++) {
                    auto& currentScoreHolder = objectScores[database_id];
                    size_t n_objects = currentScoreHolder.size();
                    graphs_to_n_objects.emplace_back(n_objects);
                    // For each object, it is sufficient to store how many of them are stored, as we
                    // are assuming an incremental representation for those.
                    object_index_offset.write((const char*)&n_objects, sizeof(size_t));
                }

                for (size_t database_id = 0; database_id < idx.size(); database_id++) {
                    auto& currentScoreHolder = objectScores[database_id];
                    size_t n_objects = currentScoreHolder.size();
                    for (size_t j = 0; j<objectScores.at(database_id).size(); j++) {
                        // Writing the offset for the current object's scores
                        const auto& scores = objectScores.at(database_id).at(j);
//                        object_index_offset.write((const char*)&total_offset, sizeof(size_t));
                        size_t n_scores_total = scores.size();
                        for (const auto& ref : scores) {
                            scores_data.write((const char*)&ref, sizeof(double));
                        }
                        object_index_offset.write((const char*)&n_scores_total, sizeof(size_t));
                    }
                }
            }

            // 3. Containment Table Serialization
            {
                //to keep the number of file descriptors compact, we restain to store all of th elements within the containment tables
                //into one single table, where a major external index is going to store the offset for each of the tables of interest
                std::vector<std::string> ensure_label_order;
                {
                    std:: ofstream containment_table_headers(path / "containment_headers.txt", std::ios::out);
                    for (const auto& [key, table] : containment_tables) {
                        containment_table_headers << key << std::endl;
                        ensure_label_order.emplace_back(key);
                    }
                }

                size_t tables_start = 0;
                std::ofstream containment_tables_f(path / "database_containment_table.binary", std::ios::out | std::ios::binary);
                std::ofstream containment_primary_indices(path / "database_containment_table_pi.binary", std::ios::out | std::ios::binary);
                std::ofstream containment_secondary_indices(path / "database_containment_table_si.binary", std::ios::out | std::ios::binary);

                size_t primary_offsets = 0, first_record_id = 0, table_entries = 0;
                for (auto& [key, table] : containment_tables) {
                    table.sort();
                    primary_offsets = table.count_primary_entries();
                    table_entries = table.table.size();
                    containment_primary_indices.write((const char*)&primary_offsets, sizeof(size_t));
                    containment_tables_f.write((const char*)&table_entries, sizeof(size_t));
                }
                for (size_t i = 0, N = ensure_label_order.size(); i<N; i++) {
                    const auto& key = ensure_label_order.at(i);
#ifdef DEBUG
                    std::cout << "containment: " << key << std::endl;
#endif
                    auto it = containment_tables.find(key);
                    DEBUG_ASSERT(it != containment_tables.end());
                    first_record_id = it->second.secondary_memory_index(i, first_record_id, graphs_to_n_objects, containment_tables_f,containment_primary_indices, containment_secondary_indices);
                }
            }


            // 3. Object Attributes Serialization
            {
                //to keep the number of file descriptors compact, we restain to store all of th elements within the containment tables
                //into one single table, where a major external index is going to store the offset for each of the tables of interest
                std::vector<std::string> ensure_label_order;
                {
                    std:: ofstream containment_table_headers(path / "attributes_headers.txt", std::ios::out);
                    for (const auto& [key, table] : KeyValueProperties) {
                        containment_table_headers << key << std::endl;
                        containment_table_headers << magic_enum::enum_name(table.type) << std::endl;
                        ensure_label_order.emplace_back(key);
                    }
                }

                size_t tables_start = 0;
                std::ofstream containment_tables_f(path / "database_attributes_table.binary", std::ios::out | std::ios::binary);
                std::ofstream containment_primary_indices(path / "database_attributes_table_pi.binary", std::ios::out | std::ios::binary);
                std::ofstream containment_secondary_indices(path / "database_attributes_table_si.binary", std::ios::out | std::ios::binary);

                size_t primary_offsets = 0, first_record_id = 0;
                for (size_t i = 0, N = ensure_label_order.size(); i<N; i++) {
                    const auto& key = ensure_label_order.at(i);
                    auto it = KeyValueProperties.find(key);
                    DEBUG_ASSERT(it != KeyValueProperties.end());
                    it->second.secondary_memory_index(idx, graphs_to_n_objects, containment_tables_f,containment_primary_indices, containment_secondary_indices);
                }
            }

            // 4. Finalising indeixng
            main_registry.secondary_memory_index2(path);
        }

        void LinearGSM::index() {
            objectScores.resize(nGraphs);
            const auto& idx = main_registry.indexing1();
            for (size_t i = 0; i<idx.size(); i++) {
                auto& currentScoreHolder = objectScores[i];
                currentScoreHolder.resize(objectScores.at(i).size());
                for (size_t j = 0; j<objectScores.at(i).size(); j++) {
                    currentScoreHolder[j] = objectScores.at(i).at(j);
                }
            }
            objectScores.clear();
            size_t tables_start = 0;
            for (auto& [key, table]: containment_tables) {
                minRecordToContaimentLabel.emplace(tables_start, key);
                tables_start = table.index(tables_start);
            }
            for (auto& [key, table] : KeyValueProperties) {
                DEBUG_ASSERT(key == table.attr_name);
//                if (key == "price")
//                    std::cerr << key << std::endl;
                table.index(main_registry, idx);
            }
            main_registry.indexing2();
            main_registry.sanityCheck();
            result resolver{0,0,0};
            all_indices.insert(all_indices.begin(), nGraphs, {});
//            for (const auto& entry : main_registry.table) {
//                auto id = entry.event_id;
//            }
            // Creating a gsm_index
            for (const auto& [k1,v] : containment_tables) {
                for (const auto& record_obj : v.table) {
                    auto& gRef = all_indices[record_obj.graph_id];
//                    auto cDst = gRef.containedBy.addUniqueStateOrGetExisting(record_obj.object_id);
                    auto oDst = gRef.containerOf.addUniqueStateOrGetExisting(record_obj.object_id);
                    const auto& labels = ell(resolver);
//                    ssize_t act_label = labels.empty() ? getMappedValueFromAction("") : getMappedValueFromAction(labels.at(0));
//                    auto src = gRef.siblinghood.addUniqueStateOrGetExisting(record_obj.id_contained);
//                    auto cSrc = gRef.containedBy.addUniqueStateOrGetExisting(record_obj.id_contained);
                    auto oSrc = gRef.containerOf.addUniqueStateOrGetExisting(record_obj.id_contained);
//                    gRef.containedBy.addNewEdgeFromId(cSrc, cDst, k1);
                    gRef.containerOf.addNewEdgeFromId(oDst, oSrc, k1);
                    resolver.graphid = record_obj.graph_id;
                    resolver.eventid = record_obj.object_id;

//                    const auto& labels2 = ell(resolver);
//                    ssize_t act_label2 = labels.empty() ? getMappedValueFromAction("") : getMappedValueFromAction(labels2.at(0));
//                    for (const auto& [k2,v2] : containment_tables) {
//                        if (k2 == k1) continue;
//                        auto it2 = v.primary_index.find(act_label2);
//                        if (it2 == v.primary_index.end())
//                            continue;
//                        auto iterator = it2->second;
//                        iterator.second++;
//                        for (; iterator.first != iterator.second; iterator.first++) {
//                            auto dst = gRef.siblinghood.addUniqueStateOrGetExisting(iterator.first->id_contained);
//                            gRef.siblinghood.addNewEdgeFromId(src, dst, k2);
//                        }
//                    }
                }
            }
            for (auto& ref : all_indices){
//                // Converting from the index id to the id of the actual data structure
//                ref.containement_order = ref.containedBy.g.topological_sort(-1);
//                convertMap(ref.containedBy, ref.containement_order);
                ref.container_order = ref.containerOf.g.topological_sort(0);
                for (auto& [time,order] : ref.container_order) {
                    for (auto& val : order) {
                        val = ref.containerOf.getUniqueLabel(val);
                    }
                }
                ref.containerOf.clear();
//                convertMap(ref.containerOf, ref.container_order);
//                ref.traversal_order = ref.siblinghood.g.topological_sort(-1);
//                convertMap(ref.siblinghood, ref.traversal_order);
            }
            objectScores.resize(nGraphs);
            for (size_t i = 0; i<nGraphs; i++) {
                auto& ref = objectScores[i];
                ref.resize(++nodesInGraph[i]);
            }
            for (auto& [cp, vals] : objectScoresLoading) {
                objectScores[cp.first][cp.second] = std::move(vals);
            }
            objectScoresLoading.clear();
        }



        std::vector<result> LinearGSM::timed_dataless_exists(const std::string &act) const {
            std::vector<result> foundData;
            ssize_t mappedVal = getMappedValueFromAction(act);
            if(mappedVal < 0){
                return foundData;
            }
            auto indexes = main_registry.resolve_index(mappedVal);
            if(indexes.first < 0){
                return foundData;
            }
            for (auto it = main_registry.table.begin() + indexes.first; it != main_registry.table.begin() + indexes.second + 1; ++it) {
                foundData.emplace_back(it->graph_id, it->event_id, -1);
            }
            return foundData;
        }

        std::vector<result>
        LinearGSM::approx_value_query(double min_threshold, size_t nmax, const std::string &xi) const {
            std::vector<result> result;
            std::function<void(const std::unordered_map<std::pair<size_t,size_t>, double>&)> f = [&result](const std::unordered_map<std::pair<size_t,size_t>, double>& m) {
                for (const auto& [k,v]: m) {
                    result.emplace_back(k.first, k.second, v);
                }
                std::sort(result.begin(), result.end());
            };
            xi_values.fuzzyMatch(min_threshold, nmax, xi, f);
            return result;
        }

        std::vector<result>
        LinearGSM::approx_label_query(double min_threshold, size_t nmax, const std::string &xi) const {
            std::vector<result> result;
            std::function<void(const std::unordered_map<std::pair<size_t,size_t>, double>&)> f = [&result](const std::unordered_map<std::pair<size_t,size_t>, double>& m) {
                for (const auto& [k,v]: m) {
                    result.emplace_back(k.first, k.second, v);
                }
                std::sort(result.begin(), result.end());
            };
            ell_values.fuzzyMatch(min_threshold, nmax, xi, f);
            return result;
        }

        std::vector<result> LinearGSM::query_container_or_containment(const std::string &container_object_label,
                                                                      const std::string &phi_label,
                                                                      std::function<double(const std::vector<double> &)> aggregate_scores,
                                                                      bool container_containementOth) const {
            std::vector<result> result;
            ssize_t label_id = getMappedValueFromAction(container_object_label);
            if (label_id<0)
                return result;
            std::unordered_map<std::pair<size_t,size_t>, std::vector<double>> int_result;
            if (container_object_label.empty()) {
                auto it = containment_tables.find(phi_label);
                if (it == containment_tables.end())
                    return result;
                for (const auto& x : it->second.table) {
                    int_result[{x.graph_id,
                                container_containementOth ? x.object_id : x.id_contained}]
                            .emplace_back(x.w_contained);
                }
                for (const auto& [k2,v2] : int_result) {
                    result.emplace_back(k2.first, k2.second, aggregate_scores(v2));
                }
            } else {
                auto it = containment_tables.find(phi_label);
                if (it == containment_tables.end())
                    return result;
                auto it2 = it->second.primary_index.find(label_id);
                if (it2 == it->second.primary_index.end())
                    return result;
                auto iterator = it2->second;
                iterator.second++;
                for (; iterator.first != iterator.second; iterator.first++) {
                    int_result[{iterator.first->graph_id,
                                container_containementOth ? iterator.first->object_id : iterator.first->id_contained}]
                            .emplace_back(iterator.first->w_contained);
                }
                for (const auto& [k,v] : int_result) {
                    result.emplace_back(k.first, k.second, aggregate_scores(v));
                }
            }


            std::sort(result.begin(), result.end());
            return result;
        }


        void LinearGSM::query_container_or_containment(std::unordered_map<std::pair<size_t, size_t>, std::vector<result>>& result,
                                                  const std::string &container_object_label,
                                                  const std::string &phi_label,
                                                  bool container_containementOth) const {
//            std::unordered_map<std::pair<size_t,size_t>,std::vector<result>> result;
            ssize_t label_id = getMappedValueFromAction(container_object_label);
            if (label_id<0)
                return; // result;

            if (container_object_label.empty()) {
                auto it = containment_tables.find(phi_label);
                if (it == containment_tables.end())
                    return; // result;
                for (const auto& x : it->second.table) {
                    result[{x.graph_id,
                            container_containementOth ? x.object_id : x.id_contained}]
                            .emplace_back(x.graph_id,
                                          container_containementOth ?  x.id_contained : x.object_id,
                                          x.record_id);
                }
            } else {
                auto it = containment_tables.find(phi_label);
                if (it == containment_tables.end())
                    return; // result;
                auto it2 = it->second.primary_index.find(label_id);
                if (it2 == it->second.primary_index.end())
                    return; // result;
                auto iterator = it2->second;
                iterator.second++;
                for (; iterator.first != iterator.second; iterator.first++) {
                    result[{iterator.first->graph_id,
                            container_containementOth ? iterator.first->object_id : iterator.first->id_contained}]
                            .emplace_back(iterator.first->graph_id,
                                          container_containementOth ? iterator.first->id_contained : iterator.first->object_id,
                                          iterator.first->record_id);
                }
            }


            for (auto& [k,c] : result) {
                std::sort(c.begin(), c.end());
            }
            return; // result;
        }



        std::vector<gsm_object_xi_content>
        LinearGSM::resolveContent(size_t graphid, size_t id, const std::string &key_content) const {
            auto it = (containment_tables.find(key_content));
            if (it == containment_tables.end())
                return empty_containment;
            std::vector<gsm_object_xi_content> result;
            auto it3 = it->second.secondary_index.find(graphid);
            if (it3 == it->second.secondary_index.end())
                return empty_containment;
            auto it2 = it3->second.find(id);
            if (it2 == it3->second.end())
                return empty_containment;
            else for (const auto& record : it2->second) {
                    result.emplace_back(record->id_contained, record->w_contained);
                }
            return result;
        }

        std::optional<union_minimal>
        LinearGSM::resolveProperties(size_t graph_id, size_t node_id, const std::string &key_prop) const {
            auto it = KeyValueProperties.find(key_prop);
            if (it == KeyValueProperties.end())
                return {};
            size_t i, N;
            auto it3= it->second.secondary_index2.find({graph_id,node_id});
            if (it3 !=it->second.secondary_index2.end() ) {
                return resolveUnionMinimal(it->second, it->second.table.at(it3->second));
            }
            return {};
        }

        void LinearGSM::iterateOverObjects(const std::function<void(size_t, const gsm_object &)> &f) {

//            std::pair<size_t, size_t> cp;
            size_t offsetMainRegistryTable = 0;
            gsm_object legacy_obj;
            for (const auto& record : main_registry.table) {
//                cp.first = record.graph_id;
//                cp.second = legacy_obj.id = record.event_id;
                legacy_obj.id = record.event_id;
                legacy_obj.content.clear();
                legacy_obj.phi.clear();
                legacy_obj.ell = ell(record.graph_id, legacy_obj.id);
                legacy_obj.xi = xi(record.graph_id, legacy_obj.id);
                for (const auto& [keyAttribute, Table] : KeyValueProperties) {
                    auto tmp2 = Table.resolve_record_if_exists2(offsetMainRegistryTable);
                    if (tmp2) {
                        legacy_obj.content[keyAttribute] = (std::holds_alternative <std::string>(tmp2.value()) ? std::get<std::string>(tmp2.value()) : std::to_string(std::get<double>(tmp2.value())));
                    }
                }
                for (const auto& [keyAttribute, Table] : containment_tables) {
//                    if (cp.second == 6) {
//                        std::cout << keyAttribute << ": SIZE=" << Table.table.size() << " T=" << Table.secondary_index.size() << std::endl;
//                    }
                    auto it = Table.secondary_index.find( record.graph_id);
                    if (it != Table.secondary_index.end()) {
                        auto it2 = it->second.find(legacy_obj.id);
                        if (it2 != it->second.end()) {
//                            if (cp.second == 6) {
//                                std::cout << "|"<< keyAttribute << std::endl;
//                            }
                            for (const auto& record2 : it2->second)
                                legacy_obj.phi[keyAttribute].emplace_back(record2->id_contained, record2->w_contained, record2->record_id);
                        }

                    }
                }
                f(record.graph_id, legacy_obj);
                offsetMainRegistryTable++;
            }
        }

        void LinearGSM::asGraphs(std::vector<FlexibleGraph<std::string, std::string>> &simpleGraphs) const {
            simpleGraphs.clear();
            simpleGraphs.resize(all_indices.size());
            std::pair<size_t, size_t> cp;
            std::vector<yaucl::structures::any_to_uint_bimap<size_t>> nodesBeingInsertedAlready(all_indices.size());
            size_t offsetMainRegistryTable = 0;
            for (const auto& record : main_registry.table) {
                cp.first = record.graph_id;
                cp.second = record.event_id;
                auto& nodeMap = nodesBeingInsertedAlready[record.graph_id];
                auto& g = simpleGraphs[record.graph_id];
                size_t id = nodeMap.put(record.event_id).first;
                const auto& xis = xi_values.resolve_object_id(cp);
                size_t gid;
                std::string tmp;
                if (xis.empty())
                    // If no content exists, using the first label, which should exist
                    tmp = ""+label_map.getValue(record.l0_id)+"="+std::to_string(record.event_id)+"";
                else
                    // Otherwise, using the first value
                    tmp = xis.at(0)+"="+std::to_string(record.event_id);
                for (const auto& [keyAttribute, Table] : KeyValueProperties) {
                    auto tmp2 = Table.resolve_record_if_exists2(offsetMainRegistryTable);
                    if (tmp2) {
                        tmp = tmp+"|"+keyAttribute+"="+(std::holds_alternative <std::string>(tmp2.value()) ? std::get<std::string>(tmp2.value()) : std::to_string(std::get<double>(tmp2.value())));
                    }
                }
                gid = g.addNewNodeWithLabel(tmp);
                DEBUG_ASSERT(id == gid);
                offsetMainRegistryTable++;
            }
            for (const auto& [edgeLabel, outEdges] : containment_tables) {
                for (const auto& record : outEdges.table) {
                    const auto& map = nodesBeingInsertedAlready.at(record.graph_id);
                    auto src = map.getKey(record.object_id);
                    auto dst = map.getKey(record.id_contained);
                    auto& g = simpleGraphs[record.graph_id];
                    g.addNewEdgeFromId(src, dst, edgeLabel);
                }
            }
        }

        void LinearGSM::exact_range_query(const std::string &field_name,
                                          const std::unordered_map<std::string, std::vector<size_t>> &ActNameToPredicates,
                                          std::vector<std::pair<DataQuery, std::vector<result>>> &Qs) const {

//                bool doTemporalMatchQuery = temporalTimeMatch.has_value();
//                uint16_t isTemproalVal = temporalTimeMatch.value_or(0);
            auto it = KeyValueProperties.find(field_name);
            if (it == KeyValueProperties.end()) {
                // if no attribute is there, for the exact match I assume that no value was matched
                return;
            } else {
                // The attribute exists within the dataset
                std::vector<std::pair<size_t, std::vector<DataQuery*>>> V;
                std::unordered_map<DataQuery*, size_t> qToItsId;
                for (const auto& mapRef : ActNameToPredicates) {
                    std::pair<size_t, std::vector<DataQuery*>>& DQ = V.emplace_back(label_map.get(mapRef.first), std::vector<DataQuery*>{});
                    for (const auto& qId : mapRef.second) {
                        auto& prop = Qs.at(qId).first;
                        auto it2 = qToItsId.emplace(&prop, qId);
                        DEBUG_ASSERT(it2.second);
                        DQ.second.emplace_back(&prop);
                    }
                    // I do not need to compare the pointers, rather than compare the values associated to those
                    std::sort(DQ.second.begin(), DQ.second.end(), [](auto lhs, auto rhs) {return *lhs < *rhs;});
                }

                // TODO: this is just the concrete doing.
                auto tmp = it->second.exact_range_query(V);

                for (size_t i = 0, N = V.size(); i<N; i++) {
                    const auto& actIdToPropList = V.at(i);
                    auto v_actId = actIdToPropList.first;
                    auto& v_propList = actIdToPropList.second;
                    auto& tmpResult = tmp.at(i);
                    DEBUG_ASSERT(v_propList.size() == tmpResult.size()); // We return the result for each query
                    for (size_t j = 0, M = v_propList.size(); j<M; j++) {
                        auto& tmpRef = tmpResult.at(j);
                        if (!((tmpRef.first == tmpRef.second) && tmpRef.first == nullptr )) {
                            auto qPTr = v_propList.at(j);
                            auto qId = qToItsId.at(qPTr);
                            auto& refQ = Qs.at(qId);
                            DEBUG_ASSERT(refQ.first == *qPTr); // They are the same query
//                    LeafType qT = refQ.first.t;

                            auto& S = refQ.second;
                            size_t Nx = std::distance(tmpRef.first, tmpRef.second);
                            for (size_t k = 0; i<=Nx; k++) {
                                const auto& exactIt = tmpRef.first[k];
                                const auto& resolve = main_registry.table.at(exactIt.act_table_offset);
// TODO: for the approximate match, in the later future
//                        bool doInsert = true;
//                        float satisfiability = 1.0;
//                        if (doTemporalMatchQuery) {
//                            auto L = resolve.event_id;
//                            satisfiability = getSatisifiabilityBetweenValues(((L <= 1) ? 0 : isTemproalVal),
//                                                                             cast_to_float2(resolve.event_id,L), approxConstant);
//                            doInsert = satisfiability >= 1.0;
//                        }
//                        if (doInsert)
                                S.emplace_back(resolve.graph_id, resolve.event_id,1.0);
                            }
                            std::sort(S.begin(), S.end());
                            S.erase(std::unique(S.begin(), S.end()), S.end());
                        }
                    }
                }
            }
        }

        void LinearGSM::loadGSMObject(size_t graphId,
                                      std::unordered_map<std::string, gsm2::tables::AttributeTableType>& property_to_type,
                                      const gsm_object &object) {
            std::pair<size_t,size_t> cp{graphId, object.id};
            nGraphs = std::max(graphId, nGraphs);
            size_t noLabel = initLoading(*this);
            std::vector<std::string> ell{object.ell.begin(), object.ell.end()};
            size_t act_id = loadObjectEll(*this, ell, noLabel, cp);
            std::vector<std::string> xi{object.xi.begin(), object.xi.end()};
            loadObjectXi(*this, xi, cp);
            registerObjectByFirstLabel(*this, act_id, cp);
            objectScoresLoading[cp] = object.scores;
            while (nodesInGraph.size() <= graphId) {
                nodesInGraph.emplace_back(0);
            }
            nodesInGraph[graphId] = std::max(nodesInGraph[graphId], object.id);
            for (const auto& [property_key,property_value] : object.content) {
                if (std::holds_alternative<std::string>(property_value)) {
                    auto& table = KeyValueProperties[property_key];
                    auto it = property_to_type.find(property_key);
                    if (it != property_to_type.end())
                        table.type = it->second;
                    else
                        table.type = gsm2::tables::StringAtt;
                    switch (table.type) {
                        case gsm2::tables::DoubleAtt:
                            table.record_load(act_id, std::stod(std::get<std::string>(property_value)), cp.first, cp.second);
                            break;
                        case gsm2::tables::SizeTAtt:
                            table.record_load(act_id, std::stoull(std::get<std::string>(property_value)), cp.first, cp.second);
                            break;
                        case gsm2::tables::LongAtt:
                            table.record_load(act_id, std::stoll(std::get<std::string>(property_value)), cp.first, cp.second);
                            break;
                        case gsm2::tables::StringAtt:
                            table.record_load(act_id, std::get<std::string>(property_value), cp.first, cp.second);
                            break;
                        case gsm2::tables::BoolAtt:
                            table.record_load(act_id, (std::get<std::string>(property_value) == "True" || std::get<std::string>(property_value) == "true" || std::get<std::string>(property_value) == "tt" || std::get<std::string>(property_value) == "t"), cp.first, cp.second);
                            break;
                    }
                } else {
                    auto& table = KeyValueProperties[property_key];
                    auto it = property_to_type.find(property_key);
                    if (it != property_to_type.end())
                        table.type = gsm2::tables::DoubleAtt;
                    if (table.type == gsm2::tables::DoubleAtt) {
                        table.record_load(act_id, (std::get<double>(property_value)), cp.first, cp.second);
                    }
                }
            }


        }
    } // gsm2
} // structures
