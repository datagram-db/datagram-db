/*
 * closure.h
 * This file is part of gsm_gsql
 *
 * Copyright (C) 2023 - Giacomo Bergami
 *
 * fuzzyStringMatching is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * fuzzyStringMatching is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fuzzyStringMatching. If not, see <http://www.gnu.org/licenses/>.
 */
//
// Created by giacomo on 27/10/23.
//

#ifndef GSM2_CLOSURE_H
#define GSM2_CLOSURE_H

#include <iostream>
#include <vector>
#include <fstream>
#include "database/LinearGSM.h"
#include "queries/DataPredicate.h"
#include "ANTLRInputStream.h"
#include "graph_grammar/simple_graph_grammarLexer.h"
#include "graph_grammar/simple_graph_grammarParser.h"
#include "database/GSMPatternVisitor.h"

#define PRINT_GROUP_MATCHING

// https://stackoverflow.com/a/66969964/1376095
template <typename T>
std::optional<T> get_v_opt(const std::any & tmp)
{
    try {
        return std::optional<T>(std::any_cast<T>(tmp));
    } catch (...) {
        return std::nullopt;
    }
}


#include "preserve_results.h"

template <typename T>
void fill_vector_with_case(std::vector<T>& to_fill, const abstract_value& opts) {
    if (std::holds_alternative<T>(opts))
        to_fill.emplace_back(std::get<T>(opts));
}


#include "delta_updates.h"
#include "scriptv2/ScriptVisitor.h"
#include "NestedResultTable.h"
#include <easylogging++.h>



/**
 * This class is referred to Closure as it wraps around both the loaded data and the intermediate values generating
 * the resulting graph as per matching.
 */
struct closure {
    GSMPatternVisitor pv;
    std::vector<node_match> vl;

    gsm2::tables::LinearGSM* forloading = nullptr;
    preserve_results pr;

    std::vector<delta_updates> delta_updates_per_graph;
    std::vector<std::string> empty_string_vector;
    std::string empty_string;
    std::vector<gsm_object_xi_content> empty_content;
    bool isMaterialised = false;

    void new_data_slate();

    void asObjects(std::vector<std::vector<gsm_object>>& graphs) const {
        forloading->asObjects(graphs);
    }

    void fillInForSerialization(std::vector<std::vector<gsm_object>>& dbs) {
        dbs.clear();
        dbs.resize(delta_updates_per_graph.size());
        generate_materialised_view();
        std::vector<FlexibleGraph<std::string, std::string>> simpleGraphs;
        simpleGraphs.resize(delta_updates_per_graph.size());
        std::pair<size_t, size_t> cp;
        std::vector<yaucl::structures::any_to_uint_bimap<size_t>> nodesBeingInsertedAlready(delta_updates_per_graph.size());
        size_t offsetMainRegistryTable = 0;
        std::vector<std::vector<size_t>> initialNodes(delta_updates_per_graph.size());
        std::vector<std::unordered_map<size_t, size_t>> time(delta_updates_per_graph.size());
        auto real_time_start = std::chrono::high_resolution_clock::now();
        std::vector<std::unordered_map<size_t, gsm_object>> tmp;
        tmp.resize(delta_updates_per_graph.size());
        for (size_t i = 0, N = delta_updates_per_graph.size(); i<N; i++) {
            initialNodes[i].reserve(N);
            auto& nodeMap = nodesBeingInsertedAlready[i];

            for (const auto& [idX,object] : delta_updates_per_graph.at(i).delta_plus_db.O) {
                if (delta_updates_per_graph.at(i).removed_objects.contains(idX)) {
                    continue;
                }
                cp.first = i;
                cp.second = idX;

                auto& g = simpleGraphs[i];
                size_t id = nodeMap.put(idX).first;
                tmp[i][idX] = object;
                size_t gid;
                gid = g.addNewNodeWithLabel(std::to_string(idX));
                DEBUG_ASSERT(id == gid);
                offsetMainRegistryTable++;
            }

            for (const auto& [idX,object] : delta_updates_per_graph.at(i).delta_plus_db.O) {
                if (delta_updates_per_graph.at(i).removed_objects.contains(idX))
                    continue;
//                if (idX == 3)
//                    std::cout << "HERE" << std::endl;
                for (const auto& [edgelabel,records] : object.phi) {
                    size_t recordsId = 0;
                    for (const auto& record: records) {
                        if (delta_updates_per_graph.at(i).hasXBeenRemoved(record.id)) {
//                            std::cout << idX << "R.." << record.id <<std::endl;
                            recordsId++;
                            continue;
                        }
                        const auto& map = nodesBeingInsertedAlready.at(i);
                        auto src = map.getKey(idX);
                        auto dst = map.signed_get(record.id);
                        if (dst >= 0) {
                            auto& g = simpleGraphs[i];
                            g.addNewEdgeFromId(src, dst, std::to_string(recordsId++)+"_"+edgelabel);
                        }
                    }
                }
            }

            const auto& cn = forloading->all_indices.at(i).container_order;
            if (cn.empty()) {
                for (size_t j = 0, M = forloading->objectScores.at(i).size(); j<M; j++) {
                    size_t candidate = delta_updates_per_graph.at(i).replacedWith(j);//getOrDefault(delta_updates_per_graph.at(i).replacement_map, j, j);
                    if (delta_updates_per_graph.at(i).removed_objects.contains(candidate)) {
                        continue;
                    }
                    initialNodes[i].emplace_back(nodeMap.put(candidate).first);
                }
                // This implies that nodes are not contained between each other. So, any of these nodes will be the same
            } else {
                const auto& graph_index = cn.at(0);
                for (size_t j = 0, M = graph_index.size(); j<M; j++) {
                    size_t candidate = delta_updates_per_graph.at(i).replacedWith(graph_index.at(j));//getOrDefault(delta_updates_per_graph.at(i).replacement_map, graph_index.at(j), graph_index.at(j));
                    if (delta_updates_per_graph.at(i).removed_objects.contains(candidate)) {
                        continue;
                    }
                    initialNodes[i].emplace_back(nodeMap.put(candidate).first);
                }
            }
        }


        for (size_t k = 0, N = delta_updates_per_graph.size(); k<N; k++) {
            roaring::Roaring64Map visited;
            std::vector<size_t> Stack;
            auto& g = simpleGraphs[k];
            for (size_t start_from : initialNodes[k]) {
                g.g.topologicalSortUtil(start_from, visited, Stack);
            }
            for (size_t i = 0; i < g.g.V_size; i++)
                if (!visited.contains(i))
                    g.g.topologicalSortUtil(i, visited, Stack);
            bool firstVisit = true;
            std::reverse(Stack.begin(), Stack.end());
            for (size_t v : Stack) {
                if (firstVisit) {
                    time[k][v] = 0;
                    firstVisit = false;
                } else {
                    time[k][v] = 0;
                    auto it = g.g.ingoing_edges.find(v);
                    if (it != g.g.ingoing_edges.end()) {
                        for (size_t edgeId : it->second)
                            time[k][v] = std::max(time[k][g.g.edge_ids.at(edgeId).first], time[k][v]);
                        time[k][v]++;
                    }
                }
            }
            for (size_t v = 0, vN = g.vertexSize(); v<vN; v++) {
                size_t vt = time[k][v];
                std::vector<size_t> edgesToRemove;
                for (size_t edgeId : g.g.nodes.at(v)) {
                    if (time[k][g.g.edge_ids.at(edgeId).second] <= vt) {
                        edgesToRemove.emplace_back(edgeId);
                    }
                }
                if (!edgesToRemove.empty()) {
                    for (size_t edgeId : edgesToRemove) {
                        remove_edge(&g.g, edgeId);
                    }
                }
            }
            auto& mp = tmp[k];
            const auto& map = nodesBeingInsertedAlready.at(k);
            for (auto& [idX, obj] : mp) {
//                if (idX == 6)
//                    std::cout << "HERE" << std::endl;
                auto src = map.getKey(idX);
                std::unordered_map<std::string, roaring::Roaring64Map> toRemove;
                for (const auto& [keys,v] : obj.phi) {
                    toRemove[keys].addRange(0, v.size());
                }
                for (const size_t edge_id : g.g.getOutgoingEdgesId(src)) {
                    const std::string& label = g.getEdgeLabel(edge_id);
                    size_t offset = label.find('_');
                    size_t no = std::stoull(label.substr(0, offset));
                    std::string labella = label.substr(offset+1);
                    toRemove[labella].remove(no);
                }
                std::unordered_set<std::string> removeKeys;
                for (auto& [keys,v] : obj.phi) {
                    std::vector<size_t> cleared(toRemove[keys].begin(), toRemove[keys].end());
                    std::sort(cleared.begin(), cleared.end());
                    remove_index(v, cleared);
                    if (v.empty())
                        removeKeys.insert(keys);
                }
                for (const auto& k : removeKeys)
                    obj.phi.erase(k);
                dbs[k].emplace_back(obj);
            }
        }
        auto real_time_end = std::chrono::high_resolution_clock::now();
        materialise_time_final = std::chrono::duration<double, std::milli>(real_time_end-real_time_start).count();
    }

    ~closure() {
        if (forloading) {
            delete forloading;
            forloading = nullptr;
        }
    }

    std::unordered_set<std::string> nodeVars, edgeVars;

    inline void sortVL() {
        std::vector<std::unordered_set<std::string>> outgoing(vl.size());
        std::unordered_map<std::string, std::unordered_set<size_t>> ingoing;
        NodeLabelBijectionGraph<std::string, std::string> dependencies;
        auto  N = vl.size();
        for (size_t i = 0; i<N; i++) {
            const auto& pattern = vl.at(i);
            dependencies.addUniqueStateOrGetExisting(pattern.pattern_name);
            bool hasActions = !pattern.rwr_to.empty();
            std::unordered_set<std::string> generated_variables, removed_variables;
            for (const auto& actions: pattern.rwr_to) {
                switch (actions.t) {
                    case rewrite_to::DEL_RW:
                        removed_variables.insert(actions.others);
                        break;
                    case rewrite_to::NEU_RW:
                        generated_variables.insert(actions.others);
                        break;
                    default:
                        continue;
                }
            }

            if (pattern.var.size() != 1) {
                std::cerr << "ERROR: the entrypoint variable should be associated to just one variable (pattern #" << i << ")" << std::endl;
                exit(1);
            }
            if (pattern.vec) {
                if (pattern.in.empty()) {
                    std::cerr << "ERROR: a vec entry-point requires some ingoing edges for the aggregation!"<< std::endl;
                    exit(2);
                } else {
                    for (const auto& [n,e] : vl.at(i).in) {
                        for (const auto& varName : n.var) {
                            if (varName != "ANY") {
                                if (e.forall) {
                                    outgoing[i].insert(varName);
                                } else
                                    ingoing[varName].insert(i);
                            }
                        }
                    }
                }
            } else {
                auto varName = pattern.var.at(0);
                if (pattern.rewrite_match_dst && !generated_variables.contains(pattern.rewrite_match_dst->var.at(0))) {
                    varName = pattern.rewrite_match_dst->var.at(0);
                }
                if (pattern.var.at(0) != "ANY")
                    outgoing[i].insert(varName);
                for (const auto& [n,e] : vl.at(i).in) {
                    for (const auto& varName : n.var) {
                        if (varName != "ANY")
                            ingoing[varName].insert(i);
                    }
                }
            }
            for (const auto& [e,n] : vl.at(i).out) {
                for (const auto& varName : n.var) {
                    if (varName != "ANY")
                        ingoing[varName].insert(i);
                }
            }

            for (const auto& nOther : vl.at(i).join_edges) {
                for (const auto& varName : nOther.var) {
                    if (varName != "ANY")
                        ingoing[varName].insert(i);
                }
                for (const auto& [e,n] : nOther.out) {
                    for (const auto& varName : n.var) {
                        if (varName != "ANY")
                            ingoing[varName].insert(i);
                    }
                }
                for (const auto& [n,e] : nOther.in) {
                    for (const auto& varName : n.var) {
                        if (varName != "ANY")
                            ingoing[varName].insert(i);
                    }
                }
            }
        }
        for (size_t i = 0, N = vl.size(); i<N; i++) {
            for (const auto& reacher : outgoing.at(i)) {
                if (!reacher.empty()) {
                    auto it = ingoing.find(reacher);
                    if (it != ingoing.end()) {
                        for (const auto& tgt : it->second) {
                            if (i != tgt)
                                dependencies.addNewEdgeFromId(i, tgt, reacher);
                        }
                    }
                }
            }
        }
//        dependencies.dot(std::cout);
        std::unordered_map<std::string, size_t> memento;
        auto tso = dependencies.g.topological_sort2(-1);
        for (size_t i = 0; i<N; i++) {
            if ((!dependencies.outgoingEdges(i).empty()) || (!dependencies.ingoingEdges(i).empty())) {
                memento[vl.at(i).pattern_name] = tso.at(i)+1;
            } else {
                DEBUG_ASSERT(tso.at(i) == 0);
                memento[vl.at(i).pattern_name] = 0;
            }
        }
        std::sort(vl.begin(), vl.end(), [&memento](const auto& objL, const auto& objR) {
            return memento.at(objL.pattern_name) < memento.at(objR.pattern_name);
        });
        for (size_t i = 0; i<N; i++) {
            nodeVars.insert(vl.at(i).var.at(0));
            for (auto& [n,e] : vl.at(i).in) {
                std::set<std::string> vars{n.var.begin(), n.var.end()};
                n.var = {std::accumulate(
                        vars.begin(),
                        vars.end(),
                        std::string(""),
                        [](const std::string& b, const std::string& a) {
                            if (b.empty())
                                return a;
                            else
                                return  b + (a.empty() ? "" : "_"+a);
                        }
                )};
                nodeVars.insert(n.var.at(0));
                if (e.var.has_value()) {
                    edgeVars.insert(e.var.value());
                }
            }
            for (auto& [e,n] : vl.at(i).out) {
                std::set<std::string> vars{n.var.begin(), n.var.end()};
                n.var = {std::accumulate(
                        vars.begin(),
                        vars.end(),
                        std::string(""),
                        [](const std::string& b, const std::string& a) {
                            if (b.empty())
                                return a;
                            else
                                return  b + (a.empty() ? "" : "_"+a);
                        }
                )};
                nodeVars.insert(n.var.at(0));
                if (e.var.has_value()) {
                    edgeVars.insert(e.var.value());
                }
            }
            for (auto& nOther : vl.at(i).join_edges) {
                std::set<std::string> vars{nOther.var.begin(), nOther.var.end()};
                nOther.var = {std::accumulate(
                        vars.begin(),
                        vars.end(),
                        std::string(""),
                        [](const std::string& b, const std::string& a) {
                            if (b.empty())
                                return a;
                            else
                                return  b + (a.empty() ? "" : "_"+a);
                        }
                )};
                nodeVars.insert(nOther.var.at(0));
                for (auto& [e,n] : nOther.out) {
                    for (const auto& varName : n.var) {
                        nodeVars.insert(varName);
                        if (varName != "ANY")
                            ingoing[varName].insert(i);
                    }
                    if (e.var.has_value()) {
                        edgeVars.insert(e.var.value());
                    }
                }
                for (auto& [n,e] : nOther.in) {
                    std::set<std::string> vars{n.var.begin(), n.var.end()};
                    n.var = {std::accumulate(
                            vars.begin(),
                            vars.end(),
                            std::string(""),
                            [](const std::string& b, const std::string& a) {
                                if (b.empty())
                                    return a;
                                else
                                    return  b + (a.empty() ? "" : "_"+a);
                            }
                    )};
                    nodeVars.insert(n.var.at(0));
                    if (e.var.has_value()) {
                        edgeVars.insert(e.var.value());
                    }
                }
            }
        }
    }

#pragma region BENCHMARKING
    double loading_time = -1.0, indexing_time = -1.0, materialise_time_collection = -1.0, materialise_time_final = -1.0;
    double query_collect_node_match = -1.0, query_collect_edge_match = -1.0, generate_nested_morphisms = -1.0, run_transform = -1.0;
    size_t n_patterns = 0, n_graphs = 0, n_total_objects = 0;
    std::string query_name, data_name;
    void log_data(std::ostream& f) const {
        f << query_name << "," << data_name << "," << n_patterns << "," << n_graphs << "," << n_total_objects << ","
          << loading_time << "," << indexing_time << ","<< materialise_time_collection << ","<< materialise_time_final<< ","
          << query_collect_node_match << "," << query_collect_edge_match << "," << generate_nested_morphisms << "," << run_transform << std::endl;
    }
    void log_header(std::ostream& f) const {
        f << "query_name" << "," << "data_name" << "," << "n_patterns" << "," << "n_graphs" << "," << "n_total_objects" << ","
          << "loading_time" << "," << "indexing_time" << ","<< "materialise_time_collection" << ","<< "materialise_time_final" << ","
          << "query_collect_node_match" << "," << "query_collect_edge_match" << "," << "generate_nested_morphisms" << "," << "run_transform" << std::endl;
    }
#pragma endregion BENCHMARKING


    /**
     * Loading the query to be run on top of the data
     *
     * @param stream:       Filestream from which read the file
     */
    void load_query_from_file(const std::string& stream);

    void load_query_from_string(std::istream& stream);

    /**
     * Loading the data to be queried
     *
     * @param filename:     Data expressed in the GSM syntax for fast parsing
     */
    void load_data_from_file(const std::string& filename);

    /**
     * For plotting purposes, representing the loaded data as graphs
     */

    void asGraphs(std::vector<FlexibleGraph<std::string,std::string>>& graphs) const;

    /**
     * For plotting purposes, generating the graphs representing the final result
     */
    void generateGraphsFromMaterialisedViews(std::vector<FlexibleGraph<std::string,std::string>>& simpleGraphs);
    void generateOODbFromMaterialisedViews(gsm2::tables::LinearGSM& simpleGraphs);

    void perform_query(bool verbose = false, const std::string& output_folder = std::filesystem::path("viz") / "data") {
        LOG(INFO) << "Performing the query";
        bool materialise = false;
        isMaterialised = false;
        pr.init(); // For future reference, if this will become a query engine, we need to clear all intermediate results first!

        // TODO: rewrite the matching variables so to allow morphisms to be effectively queried
        bool hasDelRewrite = false;
        {
            LOG(TRACE) << "Collecting the node matches";
            auto node_q_start = std::chrono::high_resolution_clock::now();
            for (auto& nm : vl) {
                auto outcome = nm.compileNodeVariableOptionality();
                hasDelRewrite = hasDelRewrite || outcome;
                pr.collect_node_match(nm, *forloading);
            }
            // Inialising the morphisms, substantiating the match for the nodes (just resizing and clearing)
            pr.finalise_after_all_collections(forloading->all_indices.size());
            auto node_q_end = std::chrono::high_resolution_clock::now();
            query_collect_node_match = std::chrono::duration<double, std::milli>(node_q_end-node_q_start).count();
        }

        // First, matching all the single patterns within the queries, as described in the cached results
        {
            LOG(TRACE) << "Running the simple edge queries";
            auto edge_q_start = std::chrono::high_resolution_clock::now();
            pr.run_simple_edge_queries(*forloading);
            auto edge_q_end = std::chrono::high_resolution_clock::now();
            query_collect_edge_match = std::chrono::duration<double, std::milli>(edge_q_end-edge_q_start).count();
        }

        // Then, running each morphism separately (now, we are assuming any order will do)
        {
            LOG(TRACE) << "Instantiating the nested morphisms";
            auto nmorph_start = std::chrono::high_resolution_clock::now();
            pr.instantiate_morphisms(vl, verbose, nodeVars, edgeVars, output_folder);
            auto nmorph_end = std::chrono::high_resolution_clock::now();
            generate_nested_morphisms = std::chrono::duration<double, std::milli>(nmorph_end-nmorph_start).count();
        }

        // Only afterwards, we are applying all of the transformations for each of the matches collected in the tables as morphisms
        {
            LOG(TRACE) << "Running the transformations over the matches";
            auto t_start = std::chrono::high_resolution_clock::now();
            run_transformations(hasDelRewrite);
            auto t_end = std::chrono::high_resolution_clock::now();
            run_transform = std::chrono::duration<double, std::milli>(t_end-t_start).count();
        }
        // The previous run just generate delta updates. If we want to better see the results, then we need to materialise such intermediate results
        // while providing a cohesive view of the graph.
//        if (materialise) {
//            generate_materialised_view();
//        }
    }

    /**
     * For plotting purposes, extending the delta updates with the old objects within the originally loaded data
     */
    void generate_materialised_view();

//    inline bool has_content(size_t graphid,
//                                                              size_t id,
//                                                              const std::string& key_content) const {
//        if (!delta_updates_per_graph.empty()) {
//            auto& updates = delta_updates_per_graph.at(graphid);
//            {
//                auto it = updates.replacement_map.find(id);
//                if (it != updates.replacement_map.end() && (!updates.newIterationInsertedObjects.contains(it->second)))
//                    id = it->second;
//            }
//            if (updates.hasXBeenRemoved(id))
//                return false;
//            {
//                auto it = updates.delta_plus_db.O.find(id);
//                if (it != updates.delta_plus_db.O.end()) {
//                    auto it2 =  it->second.phi.find(key_content);
//                    if (it2 != it->second.phi.end()) {
//                        return true;
//                    }
//                }
//            }
//        }
//        return forloading->hasContent(graphid, id, key_content);
//    }

    inline std::vector<std::string> phi_keys(size_t graphid,
                                             size_t id) {
        std::unordered_set<std::string> result;
        if (!delta_updates_per_graph.empty()) {
            auto& updates = delta_updates_per_graph.at(graphid);
            {
                size_t tmp = updates.replacedWith(id);
//                auto it = updates.replacement_map.find(id);
                if (/*it != updates.replacement_map.end() &&*/ (!updates.newIterationInsertedObjects.contains(tmp)))
                    id = tmp;
            }
            if (updates.hasXBeenRemoved(id))
                return {};
            {
                auto it = updates.delta_plus_db.O.find(id);
                if (it != updates.delta_plus_db.O.end()) {
                    for (const auto& [key_content,v] : it->second.phi) {
                        std::vector<size_t> W;
                        for (const auto& id : v) {
                            size_t tmp = updates.replacedWith(id.id);
                            if (updates.newIterationInsertedObjects.contains(tmp))
                                tmp = id.id;
                            if (!updates.hasXBeenRemoved(tmp)) {
                                W.emplace_back(tmp);
                            }
                        }
                        if (!W.empty())
                        result.insert(key_content);
                    }
                }
            }
        }
//        auto v = forloading->resolveContainmentLabels(graphid, id);
//        std::vector<std::string> result;
//                std::pair<size_t,size_t>cp{graphid, id};
        auto& updates = delta_updates_per_graph.at(graphid);
        for (const auto& [key, table] : forloading->containment_tables) {
            auto it = table.secondary_index.find(graphid);
            if (table.secondary_index.find(graphid)!= table.secondary_index.end()) {
                auto it2 = it->second.find(id);
                if (it2 != it->second.end()) {
                    bool hasInstance = false;
                    for (const auto& recotd_ptr : it2->second) {
                        auto currId = recotd_ptr->id_contained;
                        size_t tmp =updates.replacedWith(currId);
                        if (/*it != updates.replacement_map.end() &&*/ (!updates.newIterationInsertedObjects.contains(tmp)))
                            currId = tmp;
                        if (!updates.hasXBeenRemoved(currId))
                            hasInstance = true;
                    }
                    if (hasInstance) {
                        result.insert(key);
                    }
                }
            }
        }
//        return result;
//        result.insert(v.begin(), v.end());
//        v.clear();
//        v.insert(v.begin(), result.begin(), result.end());
        return {result.begin(), result.end()};
    }



    /**
     * Returning the content associated to either an existing node from the loaded data, or something being recently
     * created
     *
     * @param graphid       Graph Id of interest
     * @param id            ObjectId of reference
     * @param key_content   Key string associated to the φ containment function
     * @return  The contained object associated to the object. If the object is missing, we are returning an empty vector
     */
    inline std::vector<gsm_object_xi_content> resolve_content(size_t graphid,
                                                              size_t id,
                                                              const std::string& key_content) const {
        if (!delta_updates_per_graph.empty()) {
            auto& updates = delta_updates_per_graph.at(graphid);
            {
                size_t tmp = updates.replacedWith(id);
//                auto it = updates.replacement_map.find(id);
                if (/*it != updates.replacement_map.end() &&*/ (!updates.newIterationInsertedObjects.contains(tmp)))
                    id = tmp;
//                auto it = updates.replacement_map.find(id);
//                if (it != updates.replacement_map.end() && (!updates.newIterationInsertedObjects.contains(it->second)))
//                    id = it->second;
            }
            if (updates.hasXBeenRemoved(id))
                return empty_content;
            {
                auto it = updates.delta_plus_db.O.find(id);
                if (it != updates.delta_plus_db.O.end()) {
                    auto it2 =  it->second.phi.find(key_content);
                    if (it2 != it->second.phi.end()) {
                        return it2->second;
                    }
                }
            }
        }
        auto legacy = forloading->resolveContent(graphid, id, key_content);
        bool subst=false;
        if (!delta_updates_per_graph.empty()) {
            auto& updates = delta_updates_per_graph.at(graphid);
            std::vector<size_t> idx_removal;
            size_t idx = 0;
            for (auto& ref : legacy) {
                size_t tmp = delta_updates_per_graph.at(graphid).replacedWith(ref.id);
//                auto& map = delta_updates_per_graph.at(graphid).replacement_map;
//                auto it =  (map.find(ref.id));
                if (tmp != ref.id) {
                    ref.id = tmp;
                    subst = true;
                }
                if (updates.hasXBeenRemoved(ref.id)) {
                    idx_removal.emplace_back(idx);
                }
                idx++;
            }
            remove_index(legacy, idx_removal);
        }
        if (subst)
            remove_duplicates(legacy);
        return legacy;
    }

    /**
     * Resolving a property π associated to the node. This behaves similarly to resolve_content
     *
     * @param graphid
     * @param id
     * @param key_prop
     * @return
     */
    inline std::string resolve_prop(size_t graphid, size_t id, const std::string& key_prop) const {
        if (!delta_updates_per_graph.empty()) {
            auto& updates = delta_updates_per_graph.at(graphid);
            {

//                auto it = updates.replacement_map.find(id);
//                if (it != updates.replacement_map.end() && (!updates.newIterationInsertedObjects.contains(it->second)))
//                    id = it->second;
            }
            if (updates.hasXBeenRemoved(id))
                return empty_string;
            {
                auto it = updates.delta_plus_db.O.find(id);
                if (it != updates.delta_plus_db.O.end()) {
                    auto it2 = it->second.content.find(key_prop);
                    if (it2 != it->second.content.end()) {
                        if (std::holds_alternative<std::string>(it2->second))
                            return std::get<std::string>(it2->second);
                        else
                            return std::to_string(std::get<double>(it2->second));
                    }
                }
            }
        }
        std::optional<union_minimal> result = forloading->resolveProperties(graphid, id, key_prop);
        if (result.has_value()) {
            if (std::holds_alternative<std::string>(result.value()))
                return std::get<std::string>(result.value());
            else
                return std::to_string(std::get<double>(result.value()));
        } else
            return empty_string;
    }

    inline std::vector<std::string> objProperties(size_t graphid, size_t id) const {
        std::unordered_set<std::string> result;
        if (!delta_updates_per_graph.empty()) {
            auto& updates = delta_updates_per_graph.at(graphid);
            {
                size_t tmp = updates.replacedWith(id);
//                auto it = updates.replacement_map.find(id);
                if (/*it != updates.replacement_map.end() &&*/ (!updates.newIterationInsertedObjects.contains(tmp)))
                    id = tmp;
//                auto it = updates.replacement_map.find(id);
//                if (it != updates.replacement_map.end() && (!updates.newIterationInsertedObjects.contains(it->second)))
//                    id = it->second;
            }
            if (updates.hasXBeenRemoved(id))
                return {};
            {
                auto it = updates.delta_plus_db.O.find(id);
                if (it != updates.delta_plus_db.O.end()) {
                    for (const auto& [k,v] : it->second.content) {
                        result.insert(k);
                    }
                }
            }
        }
        auto v = forloading->resolvePropertyLabels(graphid, id);
        result.insert(v.begin(), v.end());
        v.clear();
        v.insert(v.begin(), result.begin(), result.end());
        return v;
    }

    inline std::optional<union_minimal> resolve_GoodProp(size_t graphid, size_t id, const std::string& key_prop) const {
        if (!delta_updates_per_graph.empty()) {
            auto& updates = delta_updates_per_graph.at(graphid);
            {
                size_t tmp = updates.replacedWith(id);
//                auto it = updates.replacement_map.find(id);
                if (/*it != updates.replacement_map.end() &&*/ (!updates.newIterationInsertedObjects.contains(tmp)))
                    id = tmp;
//                auto it = updates.replacement_map.find(id);
//                if (it != updates.replacement_map.end() && (!updates.newIterationInsertedObjects.contains(it->second)))
//                    id = it->second;
            }
            if (updates.hasXBeenRemoved(id))
                return empty_string;
            {
                auto it = updates.delta_plus_db.O.find(id);
                if (it != updates.delta_plus_db.O.end()) {
                    auto it2 = it->second.content.find(key_prop);
                    if (it2 != it->second.content.end()) {
                        return it2->second;
                    }
                }
            }
        }
        return forloading->resolveProperties(graphid, id, key_prop);
    }

    inline const std::vector<std::string>& resolve_ell(size_t graphid, size_t id) const {
        return resolve_ellxi(graphid, id, false);
    }

    inline const std::vector<std::string>& resolve_xi(size_t graphid, size_t id) const {
        return resolve_ellxi(graphid, id, true);
    }

private:

    inline const std::vector<std::string>& resolve_ellxi(size_t graphid, size_t id, bool isXiEllOtherwise) const {
        if (!delta_updates_per_graph.empty()) {
        auto& updates = delta_updates_per_graph.at(graphid);
        {
            size_t tmp = updates.replacedWith(id);
//                auto it = updates.replacement_map.find(id);
                if (/*it != updates.replacement_map.end() &&*/ (!updates.newIterationInsertedObjects.contains(tmp)))
                    id = tmp;
//            auto it = updates.replacement_map.find(id);
//            if (it != updates.replacement_map.end() && (!updates.newIterationInsertedObjects.contains(it->second)))
//                id = it->second;
        }
        if (updates.hasXBeenRemoved(id))
            return empty_string_vector;
        {
            auto it = updates.delta_plus_db.O.find(id);
            if (it != updates.delta_plus_db.O.end()) {
                if (isXiEllOtherwise) {
                    if (!it->second.xi.empty()) {
                        return it->second.xi;
                    }
                } else {
                    if (!it->second.ell.empty()) {
                        return it->second.ell;
                    }
                }

            }
        }
        }
        return isXiEllOtherwise ? forloading->xi(graphid, id) : forloading->ell(graphid, id);
    }


public:
    class Interpret {
        size_t graph_id;
        size_t pattern_id;
        const std::vector<std::string>& schema;
        const nested_table& table;
        size_t record_id;
        closure& clos;
        const std::vector<std::unordered_map<std::string, std::pair<std::vector<std::string>, std::unordered_map<size_t, nested_table>>>>& ptr;
        const gsm2::tables::LinearGSM* ptr2;
    public:
        Interpret(size_t graphId,
                  size_t patternId,
                  const std::vector<std::string> &schema,
                  const nested_table &table,
                  size_t recordId,
                  closure &clos,
                  const std::vector<std::unordered_map<std::string, std::pair<std::vector<std::string>, std::unordered_map<size_t, nested_table>>>>& ptr,
                  const gsm2::tables::LinearGSM* ptr2) : graph_id(graphId), pattern_id(patternId), schema(schema),
                                                          table(table), record_id(recordId), clos(clos), ptr{ptr}, ptr2{ptr2} {}

        NestedResultTable interpret_closure_evaluate(rewrite_expr* ptr, bool force, bool node_or_edge_otherwise) /*const*/;
//        std::vector<size_t> interpret_closure_evaluate2(rewrite_expr* ptr) /*const*/;

        OrderedSet interpret(test_pred& ptr, size_t maxN) /*const*/;

        NestedResultTable resolve(const NestedResultTable& x, OrderedSet& containment,
                                                NestedResultTable::variant_type_cpp script_cast = NestedResultTable::RT_NONE) const;


        std::tuple<std::vector<size_t>,bool,bool> resolveIDs(const NestedResultTable &x,
//                                    OrderedSet& containment,
                                    NestedResultTable::variant_type_cpp script_cast) const;

        OrderedSet getFullOrEmptySet(bool result);
    };

    /*template <typename T>
    inline void updateContents(rewrite_expr *ptr, rewrite_expr *target_ptr,
                               size_t graph_id, size_t pattern_id, size_t record_id,
                               const nested_table &table, Interpret& I,
                               const std::function<T(const std::any)>& f) {
        std::any match_rhs = I.interpret_closure_evaluate(target_ptr);
        if (match_rhs.has_value()) {
            auto hasProp = I.interpret_closure_evaluate(ptr->pi_key_arg_or_then.get());
            if (!hasProp.has_value()) return;
            auto NAME = std::any_cast<std::tuple<std::pair<ssize_t,ssize_t>,std::vector<std::string>>>(hasProp); // prop_name
            auto hasVar = I.interpret_closure_evaluate(ptr->ptr_or_else.get());
            if (!hasVar.has_value()) return;
            auto variable_name = std::any_cast<std::string>(hasVar);
            const auto& record = table.datum.at(record_id);
            auto VAR = resolveIdsOverVariableName2(graph_id, pattern_id, variable_name, record, true);
            auto VAL = f(match_rhs);

            if ((std::get<0>(VAR).first == std::get<0>(VAL).first) && (std::get<0>(VAL).first == std::get<0>(NAME).first)) {
                const auto& var = std::get<1>(VAR);
                const auto& val = std::get<1>(VAL);
                const auto& name = std::get<1>(NAME);
                for (size_t i = 0, N = std::min({var.size(), val.size(), name.size()}); i<N; i++) {
                    delta_updates_per_graph[graph_id].delta_plus_db.generateId(var.at(i)).content[name.at(i)] = val.at(i);
                }
            } else if (std::get<0>(VAR).first == std::get<0>(VAL).first) {
                const auto& var = std::get<1>(VAR);
                const auto& val = std::get<1>(VAL);
                for (const std::string& x : std::get<1>(NAME)) {
                    for (size_t i = 0, N = std::min({var.size(), val.size()}); i<N; i++) {
                        delta_updates_per_graph[graph_id].delta_plus_db.generateId(var.at(i)).content[x] = val.at(i);
                    }
                }
            } else if ((std::get<0>(NAME).first == std::get<0>(VAR).first) && (std::get<0>(NAME).first == -1)) {
                const auto& var = std::get<1>(VAR);
                const auto& name = std::get<1>(NAME);
                const char* const delim = " ";
                std::ostringstream imploded;
                std::copy(std::get<1>(VAL).begin(), std::get<1>(VAL).end(),
                          std::ostream_iterator<std::string>(imploded, delim));
                for (size_t i = 0, N = std::min({var.size(), name.size()}); i<N; i++) {
                    delta_updates_per_graph[graph_id].delta_plus_db.generateId(var.at(i)).content[name.at(i)] = imploded.str();
                }
            } else if ((std::get<0>(NAME).first == std::get<0>(VAR).first) && (std::get<0>(VAL).first == -1)) {
                const auto& var = std::get<1>(VAR);
                const auto& name = std::get<1>(NAME);
                const auto& val = std::get<1>(VAL);
                DEBUG_ASSERT(val.size() == 1);
                for (size_t i = 0, N = std::min({var.size(), name.size()}); i<N; i++) {
                    delta_updates_per_graph[graph_id].delta_plus_db.generateId(var.at(i)).content[name.at(i)] = val.at(0);
                }
            } else if ((std::get<0>(NAME).first == std::get<0>(VAL).first) && (std::get<0>(VAR).first == -1)) {
                const auto& var = std::get<1>(VAR);
                const auto& name = std::get<1>(NAME);
                const auto& val = std::get<1>(VAL);
                DEBUG_ASSERT(var.size() == 1);
                std::unordered_map<std::string, std::unordered_set<std::string>> vals;
                for (size_t i = 0, N = std::min({name.size(), val.size()}); i<N; i++) {
                    vals[name.at(i)].insert(val.at(i));
                }
                for (const auto& [k,v] : vals) {
                    const char* const delim = " ";
                    std::ostringstream imploded;
                    std::copy(v.begin(), v.end(),
                              std::ostream_iterator<std::string>(imploded, delim));
                    delta_updates_per_graph[graph_id].delta_plus_db.generateId(var.at(0)).content[k] = imploded.str();
                }
            } else if ((std::get<0>(NAME).first == std::get<0>(VAL).first) && (std::get<0>(VAL).first == -1)) {
                const auto& var = std::get<1>(VAR);
                const auto& name = std::get<1>(NAME);
                const auto& val = std::get<1>(VAL);
                DEBUG_ASSERT((val.size() == 1) && (name.size() == 1));
                for (unsigned long i : var) {
                    delta_updates_per_graph[graph_id].delta_plus_db.generateId(i).content[name.at(0)] = val.at(0);
                }
            } else {
                DEBUG_ASSERT((std::get<0>(NAME).first != std::get<0>(VAL).first) &&
                             (std::get<0>(VAR).first != std::get<0>(VAL).first) &&
                             (std::get<0>(NAME).first != std::get<0>(VAR).first));
                if (std::get<0>(VAR).first == -1) {
                    DEBUG_ASSERT(std::get<1>(VAR).size() == 1);
                    size_t id = std::get<1>(VAR).at(0);
                    std::unordered_map<std::string, std::unordered_set<std::string>> vals;
                    std::unordered_map<std::string, std::string> flattened;
                    for (const auto& name : std::get<1>(NAME)) {
                        for (const auto& val : std::get<1>(VAL)) {
                            vals[name].insert(val);
                        }
                    }
                    for (const auto& [k,v] : vals) {
                        const char* const delim = " ";
                        std::ostringstream imploded;
                        std::copy(v.begin(), v.end(),
                                  std::ostream_iterator<std::string>(imploded, delim));
                        flattened[k] = imploded.str();
                    }
                    vals.clear();
                    for (const auto& [name,val] : flattened) {
                        delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).content[name] = val;
                    }
                } else if (std::get<0>(VAL).first == -1) {
                    DEBUG_ASSERT(std::get<1>(VAL).size() == 1);
                    const auto& val = std::get<1>(VAL).at(0);
                    for (const auto& name : std::get<1>(NAME)) {
                        for (const size_t id: std::get<1>(VAR)) {
                            delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).content[name] = val;
                        }
                    }
                } else {
                    DEBUG_ASSERT(std::get<0>(NAME).first == -1);
                    DEBUG_ASSERT(std::get<1>(NAME).size() == 1);
                    const auto& name = std::get<1>(NAME).at(0);
                    for (const auto& val : std::get<1>(VAL)) {
                        for (const size_t id: std::get<1>(VAR)) {
                            delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).content[name] = val;
                        }
                    }
                }
            }
        }
    }*/

    inline
    size_t resolve(size_t graph_id, size_t ref) const {
        size_t tmp = delta_updates_per_graph.at(graph_id).replacedWith(ref);
        if ((!delta_updates_per_graph.at(graph_id).newIterationInsertedObjects.contains(tmp))) {
            return tmp;
        } else
            return ref;
    }

    /**
     *(graph_id, pattern_id, schema, table, record_id, *this, pr.morphisms, forloading);
     */
    void interpret_closure_set(rewrite_expr* ptr,
                               size_t graph_id,
                               size_t pattern_id,
                               const std::vector<std::string>& schema,
                               const nested_table& table,
                               size_t record_id,
                               rewrite_expr* ptrTarget,
                               Interpret&);


    inline
    NestedResultTable
            resolveEdgesOverVariableName2(size_t pattern_id, const std::string &variable_name, const std::vector<value> &record) const {
        std::vector<size_t> object_id;
        if (!edgeVars.contains(variable_name)) {
            return {};
        }
        auto offset = pr.resolve_entry_match(pattern_id, variable_name);
        if (offset.second<0)
            return {}; //{std::tuple<std::pair<ssize_t,ssize_t>,std::vector<size_t>>{offset,object_id}};
        if (offset.first<0) {
            const auto& cell = record.at(offset.second);
            DEBUG_ASSERT(!cell.isNested);
            {
                if (std::holds_alternative<size_t>(cell.val))
                    return {std::get<size_t>(cell.val), false, offset.first, offset.second};
                else
                    return {};
//                fill_vector_with_case(object_id, cell.val);
            }
        } else {
            for (const auto& record_internal : record.at(pr.nested_index.at(pattern_id).at(offset.first)).table.datum) {
                const auto& cell = record_internal.at(offset.second);
                DEBUG_ASSERT(!cell.isNested);
                if (std::holds_alternative<size_t>(cell.val))
                    object_id.emplace_back(std::get<size_t>(cell.val));
                else
                    object_id.emplace_back(-1);
//                fill_vector_with_case(object_id, cell.val);
            }
        }
        return {std::move(object_id), false, offset.first, offset.second};
    }

//    inline
//    std::vector<size_t>
//    resolveEdgesOverVariableName(size_t pattern_id, const std::string &variable_name, const std::vector<value> &record) const {
//        std::vector<size_t> object_id;
//        if (!edgeVars.contains(variable_name)) {
//            return object_id;
//        }
//        auto offset = pr.resolve_entry_match(pattern_id, variable_name);
//        if (offset.second<0)
//            return object_id;
//        if (offset.first<0) {
//            const auto& cell = record.at(offset.second);
//            DEBUG_ASSERT(!cell.isNested);
//            {
//                fill_vector_with_case(object_id, cell.val);
//            }
//        } else {
//            for (const auto& record_internal : record.at(pr.nested_index.at(pattern_id).at(offset.first)).table.datum) {
//                const auto& cell = record_internal.at(offset.second);
//                DEBUG_ASSERT(!cell.isNested);
//                fill_vector_with_case(object_id, cell.val);
//            }
//        }
//        remove_duplicates(object_id);
//        return object_id;
//    }

    /**
     * Incremental update of the graph query, one at a time
     */
    size_t performQueryAndUpdateDB() {
        perform_query(false);
        size_t totalInsertions = 0, total_removals = 0;
        for (const auto& ref : delta_updates_per_graph) {
            totalInsertions += ref.getNovelInsertions();
            total_removals += ref.getNovelInsertions() + ref.getEdgeRemovals();
        }
        if ((totalInsertions + total_removals) > 0) {
            auto ptr = new gsm2::tables::LinearGSM();
            generateOODbFromMaterialisedViews(*ptr);
            delete forloading;
            forloading = nullptr;
            forloading = ptr;
        }
        return totalInsertions;
    }

    inline size_t newObjectToVariable(size_t graph_id, const std::string& var) {
        auto& updates = delta_updates_per_graph[graph_id];
        auto& obj = updates.getNewObject();
        updates.associateNewToVar(var, obj.id);
        return obj.id;
    }


    inline
    NestedResultTable
    resolveIdsOverVariableName2(size_t graph_id,
                               size_t pattern_id,
                               const std::string &variable_name,
                               const std::vector<value> &record,
                               bool buildNewIds = false) /*const*/ {
        std::vector<size_t> object_id;
        const std::vector<size_t>& v = delta_updates_per_graph.at(graph_id).getNewlyInsertedVertices(variable_name);
        if (!v.empty())
            return {v, true};
        auto offset = pr.resolve_entry_match(pattern_id, variable_name);
        if (offset.second<0) {
            if ((offset.first < 0) && buildNewIds)
                return {resolve(graph_id, newObjectToVariable(graph_id, variable_name)), true, offset.first, offset.second};
            else
                return {v, true, offset.first, offset.second};
        }
        if (offset.first<0) {
            const auto& cell = record.at(offset.second);
            DEBUG_ASSERT(!cell.isNested);
                if (std::holds_alternative<bool>(cell.val)) {
                    if (buildNewIds) {
                        return {resolve(graph_id, newObjectToVariable(graph_id, variable_name)), true, offset.first, offset.second};
                    }
                } else if (std::holds_alternative<size_t>(cell.val)) {
                    return {resolve(graph_id, std::get<size_t>(cell.val)), true, offset.first, offset.second};
                }
                return {};
        } else {
            for (const auto& record_internal : record.at(pr.nested_index.at(pattern_id).at(offset.first)).table.datum) {
                const auto& cell = record_internal.at(offset.second);
                DEBUG_ASSERT(!cell.isNested);
                if (std::holds_alternative<size_t>(cell.val))
                    object_id.emplace_back(resolve(graph_id, std::get<size_t>(cell.val)));
                else
                    object_id.emplace_back(-1);
            }
            return {std::move(object_id), true, offset.first, offset.second};
        }
    }

    inline
    std::vector<size_t>
    resolveIdsOverVariableName(size_t graph_id,
                               size_t pattern_id,
                               const std::string &variable_name,
                               const std::vector<value> &record,
                               bool buildNewIds = false) /*const*/ {
        std::vector<size_t> object_id;
        auto v = delta_updates_per_graph.at(graph_id).getNewlyInsertedVertices(variable_name);
        if (!v.empty())
            return v;
        auto offset = pr.resolve_entry_match(pattern_id, variable_name);
        if (offset.second<0)
            return object_id;
        if (offset.first<0) {
            const auto& cell = record.at(offset.second);
            DEBUG_ASSERT(!cell.isNested);
            {
                if (std::holds_alternative<bool>(cell.val)) {
                    if (buildNewIds)
                        object_id.emplace_back(newObjectToVariable(graph_id, variable_name));
                } else {
                    fill_vector_with_case(object_id, cell.val);
                }
            }
        } else {
            for (const auto& record_internal : record.at(pr.nested_index.at(pattern_id).at(offset.first)).table.datum) {
                const auto& cell = record_internal.at(offset.second);
                DEBUG_ASSERT(!cell.isNested);
                fill_vector_with_case(object_id, cell.val);
            }
        }
        for (auto& ref : object_id) {
           size_t tmp =  delta_updates_per_graph.at(graph_id).replacedWith(ref);
            if ((!delta_updates_per_graph.at(graph_id).newIterationInsertedObjects.contains(tmp))) {
                ref = tmp;
            }
        }
        remove_duplicates(object_id);
        return object_id;
    }

    inline void run_transformations(bool hasDelRewrite) {
        delta_updates_per_graph.clear();
        for (size_t i = 0, N = forloading->all_indices.size(); i<N; i++) {
            delta_updates_per_graph.emplace_back(forloading->main_registry.secondary_index.at(i).second->event_id);
        }
        for (size_t graph_id = 0, N = forloading->all_indices.size(); graph_id < N; graph_id++) { // on parallel...do
            // For each graph in the collection
            reinitAndClearNestedIndices();
            const auto& g = forloading->all_indices.at(graph_id);
            /*const*/ auto& morphs = pr.morphisms.at(graph_id);
            auto& updates = delta_updates_per_graph[graph_id];

            // For each graph in the actual graphs, visitn the nodes in lexicographic order
            for (size_t time = 0, T = g.container_order.size(); time<T; time++)
                // Visiting all the vertices associated to the same time
                for (const auto& vertex : g.container_order.at(T-time-1)) {

                    rewriteOverAllPatterns(hasDelRewrite, graph_id, morphs, updates, vertex);

                }

//            rewriteOverAllPatterns(hasDelRewrite, graph_id, morphs, updates, std::numeric_limits<size_t>::max());
            rewriteOverAllPatterns(hasDelRewrite, graph_id, morphs, updates, -1);
        }
    }

    void reinitAndClearNestedIndices();

    inline
    void rewriteOverAllPatterns(bool hasDelRewrite, size_t graph_id,
                                std::unordered_map<std::string, std::pair<std::vector<std::string>, std::unordered_map<size_t, nested_table>>> &morphs,
                                delta_updates &updates,
                                size_t vertex) {/// TODO: sort the patterns in dependency order, i.e., depending which should be run first
///       This needs to be inferred previously
        for (size_t pattern_id = 0, M = vl.size(); pattern_id < M; pattern_id++) {
            auto& pattern = vl[pattern_id];

            if (morphs.find(pattern.pattern_name) == morphs.end())
                continue; // Skipping if there are no results
            /*const*/ auto& pattern_result = morphs.at(pattern.pattern_name);
            DEBUG_ASSERT(pattern.var.size() == 1);

            // Ignoring the match if this was removed!
            if (updates.hasXBeenRemoved(vertex)) continue;

//                        auto updatesCopy = updates;

            // Whether there was a single node being matched
            auto it = pattern_result.second.find(vertex);
            if (it != pattern_result.second.end()) {
//                            DEBUG_ASSERT(updates.replacement_map.find(vertex) == updates.replacement_map.end());
                size_t table_offset = 0;
//                            std::vector<delta_updates> elementi;
//                            elementi.reserve(it->second.datum.size());
                for (auto& entries : it->second.datum) {
                    updates.clear_insertions();
//                                updates.newIterationInsertedObjects = updatesCopy.newIterationInsertedObjects; // Need to clear the associations, otherwise, we have incremental definitions of the veriables
//                                updates.newly_inserted_vertices = updatesCopy.newly_inserted_vertices;

                    // this operation needs to be performed only if some rewriting have a deletion operation
                    // Otherwise, we can skip this check.
                    if (hasDelRewrite) {
                        // Now, I am checking whether the current entry has some elements that are
                        DEBUG_ASSERT(!pattern_result.first.empty());
                        DEBUG_ASSERT(pattern.compiled_node_variables_optionality);
                        bool skip = false;
                        for (size_t i = 0, O = pattern_result.first.size(); i<O; i++) {
                            const auto& colName = pattern_result.first.at(i);
                            if ((colName == "*") || (entries.at(i).isNested)) {
                                std::vector<size_t> internal_cell_indices_to_remove;
                                for (size_t k = 0, Q =  entries.at(i).table.datum.size(); k<Q; k++) {
                                    const auto& entries2 = entries.at(i).table.datum.at(k);
                                    bool removeRow = false;
                                    for (size_t j = 0, P = entries.at(i).table.Schema.size(); j<P; j++) {
                                        const auto& colName2 = entries.at(i).table.Schema.at(j);
                                        if (pattern.hasRequiredMatch.contains(colName2)) {
                                            if (updates.hasXBeenRemoved(std::get<size_t>(entries2.at(j).val))) {
                                                removeRow = true;
                                                break;
                                            }
                                        }
                                    }
                                    if (removeRow)
                                        internal_cell_indices_to_remove.emplace_back(k);
                                }
                                if (!internal_cell_indices_to_remove.empty()) {
                                    if (internal_cell_indices_to_remove.size() == entries[i].table.datum.size()) {
                                        skip = true;
                                        break;
                                    } else {
                                        remove_index(entries[i].table.datum, internal_cell_indices_to_remove);
                                    }
                                }
                            } else if (pattern.hasRequiredMatch.contains(colName)) {
                                if (updates.hasXBeenRemoved(std::get<size_t>(entries.at(i).val))) {
                                    skip = true;
                                    break;
                                }
                            }
                        }
                        if (skip) {
                            table_offset++;
                            continue; //next entry
                        }
                    }

//                    if ((vertex == 0) && (graph_id == 0))
//                        std::cout << "HERE" << std::endl;

                    Interpret I(graph_id, pattern_id, pattern_result.first, it->second, table_offset, *this, pr.morphisms,
                                forloading);
                    if (pattern.has_where) {
                        if ((graph_id == 1) && (vertex == 18446744073709551615ULL))
                            std::cout << "HERE" << std::endl;
#ifdef PRINT_GROUP_MATCHING
                        std::cout << "TGraph #" << graph_id << ": applying pattern " << pattern.pattern_name << " for node " << vertex << std::endl;
#endif
                        if ((I.interpret(pattern.where, 1).empty())) {
                            table_offset++;
                            continue; //next entry
                        }
                    }
#ifdef PRINT_GROUP_MATCHING
                    std::cout << "FGraph #" << graph_id << ": applying pattern " << pattern.pattern_name << " for node " << vertex << std::endl;
#endif

                    for (const auto& operation : pattern.rwr_to) {
                        switch (operation.t) {

                            case rewrite_to::DEL_RW:
                            {
                                // Removing objects from the final result
                                auto idx = pr.resolve_entry_match(pattern_id, operation.others);
                                if (idx.second >= 0) { // If this is in the table
                                    if (idx.first >=
                                        0) { // If a nested variable, removing all the associated entries in the nested
                                        for (const auto &sub_entries: entries.at(
                                                pr.nested_index.at(pattern_id).at(idx.first)).table.datum) {
                                            if (!std::holds_alternative<bool>(sub_entries.at(
                                                    idx.second).val)) { // If this was not an optional match
                                                size_t default_val = std::get<size_t>(
                                                        sub_entries.at(idx.second).val);
//                                                            DEBUG_ASSERT(nodeVars.contains(operation.others));
                                                if (nodeVars.contains(operation.others)) {
                                                    updates.set_removed(default_val);
                                                } else if (edgeVars.contains(operation.others)) {
                                                    updates.edge_removed(default_val);
                                                }
                                            }

                                        }
                                    } else { // Otherwise, just removing this instance
                                        if (!std::holds_alternative<bool>(entries.at(idx.second).val)) {
                                            size_t default_val = std::get<size_t>(
                                                    entries.at(idx.second).val);
                                            if (nodeVars.contains(operation.others)) {
                                                updates.set_removed(default_val);
                                            } else if (edgeVars.contains(operation.others)) {
                                                updates.edge_removed(default_val);
                                            }
                                        }
                                    }
                                }
                            } break;

                            case rewrite_to::NEU_RW: {
                                newObjectToVariable(graph_id, operation.others);
//                                            auto& obj = updates.getNewObject();
//                                            updates.associateNewToVar(operation.others, obj.id);
                            } break;

                            case rewrite_to::SET_RW: {
                                DEBUG_ASSERT(operation.from);
                                DEBUG_ASSERT(operation.to);
//                                if ((graph_id == 1) && (pattern.pattern_name == "p4"))
//                                    std::cout << "HERE" << std::endl;

                                //std::any rhs = I.interpret_closure_evaluate(operation.from.get());
                                interpret_closure_set(operation.to.get(), graph_id, pattern_id, pattern_result.first, it->second, table_offset, operation.from.get(), I);
                            } break;

                            case rewrite_to::INHERIT_PROP: {
                                // Removing objects from the final result
                                auto target_id = resolveIdsOverVariableName2(graph_id, pattern_id, operation.others2,  it->second.datum.at(table_offset), false);
                                auto source_id = resolveIdsOverVariableName2(graph_id, pattern_id, operation.others,  it->second.datum.at(table_offset), true);

                                for (size_t j = 0, MM = source_id.size(); j<MM; j++) {
                                    auto idx_src = source_id.getInt(j);
                                    auto srcProps = objProperties(graph_id, idx_src);
                                    std::unordered_set<std::string> S{srcProps.begin(), srcProps.end()};
                                    for (size_t i = 0, N = target_id.size(); i<N; i++) {
                                        auto idx_dst = target_id.getInt(i);
                                        for (const auto& dstProp :objProperties(graph_id, idx_dst) ) {
                                            if (!S.contains(dstProp)) {
                                                auto val = resolve_prop(graph_id, idx_dst, dstProp);
                                                delta_updates_per_graph[graph_id].delta_plus_db.generateId(idx_src).content[dstProp] = val;
                                            }
                                        }
                                    }
                                }


                            }

                            case rewrite_to::NONE_OF_REWRITE:
                                break;
                        }
                    }
//                                if (pattern_id == 2)
//                                    std::cout << "debug" << std::endl;
                    if (pattern.rewrite_match_dst && (pattern.rewrite_match_dst->var.at(0) != pattern.var.at(0))) {
                        // Perform the rewrite only if the variables appear to be different
                        if (pattern.vec) {
                            DEBUG_ASSERT(pattern.in.size() == 1);
                            DEBUG_ASSERT(pattern.in.at(0).first.var.size() == 1);
                            auto toReplace = resolveIdsOverVariableName(graph_id, pattern_id, pattern.var.at(0), entries);
                            auto replaceWith = resolveIdsOverVariableName(graph_id, pattern_id, pattern.rewrite_match_dst->var.at(0), entries);
                            auto originals = resolveIdsOverVariableName(graph_id, pattern_id, pattern.in.at(0).first.var.at(0), entries);
                            for (size_t check : toReplace) {
                                for (size_t id : replaceWith) {
                                    delta_updates_per_graph[graph_id].replaceWith(check, id);
                                    for (size_t orig : originals) {
                                        for (const auto& contK : forloading->containment_relationships) {
                                            auto content = resolve_content(graph_id, orig, contK);
                                            for (const auto& containmentRel : content) {
                                                if (containmentRel.id == check) {
                                                    delta_updates_per_graph[graph_id].delta_plus_db.O[orig].phi[contK].emplace_back(id);
                                                }
                                            }
                                        }
                                    }

                                }
                            }
                        } else {
                            DEBUG_ASSERT(pattern.var.size() == 1);
                            auto originals = resolveIdsOverVariableName(graph_id, pattern_id, pattern.var.at(0), entries);
                            auto replacements = resolveIdsOverVariableName(graph_id, pattern_id, pattern.rewrite_match_dst->var.at(0), entries);
                            for (size_t orig : originals) {
                                for (size_t repl : replacements) {
                                    updates.replaceWith(orig, repl);
                                }
                            }
                        }
                    }
                    table_offset++;
//                                elementi.emplace_back(updates);
                }
//                            updatesCopy.updateWith(elementi);
            }
        }
    }

//    std::string
//    getOstringstream(const Interpret &I,
//                     const char *delim,
//                     const NestedResultTable &containingStrings) const;



};


std::string getFromScript(const NestedResultTable & containingStrings);

static inline
std::string getOstringstream(
        const closure::Interpret &I,
        const char *delim,const NestedResultTable &containingStrings)  {
//    std::ostringstream imploded;
    switch (containingStrings.expectedType) {
        case NestedResultTable::RT_STRING:
            return std::get<std::string>(containingStrings.content);

        case NestedResultTable::RT_VSTRING: {
            const auto& it = std::get<std::vector<std::string>>(containingStrings.content);
            size_t len = 0;
            for (const auto& x : it) len += x.size();
            std::string s;
            s.reserve(len);
            for (size_t i = 0, N = it.size(); i<N; i++) {
                s += it.at(i);
                if (i != (N-2))
                    s += (*delim);
            }
//            std::copy(it.begin(), it.end(),
//                      std::ostream_iterator<std::string>(imploded, delim));
            return s;
        }

        case NestedResultTable::RT_SIZET: {
            OrderedSet toIgnore(0);
            auto result = I.resolve(containingStrings, toIgnore, NestedResultTable::RT_STRING);
            if (result.size() == 0)
                return "";
            else
                return std::get<std::string>(result.content);
        }

        case NestedResultTable::RT_VSIZET: {
            const auto& V = std::get<std::vector<size_t>>(containingStrings.content);
            size_t index = 0;
            std::map<size_t, std::unordered_set<std::string>> M;

            OrderedSet toIgnore{(size_t)0};
            auto tmp = I.resolve(containingStrings,toIgnore, NestedResultTable::RT_VSTRING );
            for (size_t i = 0, N = tmp.size(); i<N; i++) {
                if (toIgnore.contains(i))
                    M[V.at(index)].insert(tmp.getString(i));
                index++;
            }
            std::string s; size_t len = 0;
            for (auto it = M.begin(), en = M.end(); it != en; ) {
                for (const auto& x : it->second) len += x.size();
                it++;
            }
            for (auto it = M.begin(), en = M.end(); it != en; ) {
                s.reserve(len);
                for (auto ptr = it->second.begin(), en = it->second.end(); ptr != en; ) {
                    s += *ptr;
                    ptr++;
                    if (ptr != en)
                        s += (*delim);
                }
//                std::ostringstream local;
//                std::copy(it->second.begin(), it->second.end(),
//                          std::ostream_iterator<std::string>(local, delim));
//                imploded << local.str();
                it++;
                if (it != en)
                    s += (*delim);
            }
            return s;
        }


        case NestedResultTable::RT_SCRIPT:
            return getFromScript(containingStrings);

        case NestedResultTable::RT_NONE:
            return "";

        case NestedResultTable::RT_CONTENT:
        case NestedResultTable::RT_VCONTENT:
            throw std::runtime_error("Unexpected type");
    }
}

static inline
std::vector<gsm_object_xi_content> getFlattenedContent(
        const closure::Interpret &I,
        const char *delim,const NestedResultTable &containingStrings)  {
    std::vector<gsm_object_xi_content> imploded;
    switch (containingStrings.expectedType) {
        case NestedResultTable::RT_CONTENT:
            return std::get<std::vector<gsm_object_xi_content>>(containingStrings.content);

        case NestedResultTable::RT_VCONTENT: {
            for (const auto& ref : std::get<std::vector<std::vector<gsm_object_xi_content>>>(containingStrings.content))
                imploded.insert(imploded.end(), ref.begin(), ref.end());
            remove_duplicates(imploded);
            return imploded;
        }

        case NestedResultTable::RT_SIZET: {
            OrderedSet toIgnore{0};
            return I.resolve(containingStrings, toIgnore, NestedResultTable::RT_CONTENT).getContent(0);
        }

        case NestedResultTable::RT_VSIZET: {
            OrderedSet toIgnore{(size_t)0};
            const auto W = I.resolve(containingStrings, toIgnore, NestedResultTable::RT_VCONTENT );
            for (size_t i = 0, N = W.size(); i<N; i++) {
                if (toIgnore.contains(i)) {
                    auto x = W.getContent(i);
                    imploded.insert(imploded.end(), x.begin(), x.end());
                }
            }
            remove_duplicates(imploded);
            return imploded;
        }

        case NestedResultTable::RT_NONE:
            return imploded;

        default:
            throw std::runtime_error("Unexpected type");
    }
}


template <typename T>
static inline
   void complexInstantiate(size_t graph_id,
            closure& self,
            const closure::Interpret &I,
            const char *delim,
            const NestedResultTable::variant_type_cpp vectorType,
    const NestedResultTable::variant_type_cpp simpleType,
            const std::function<void(size_t, size_t, const std::string &, const T &)> &resolve,
            const std::function<T(const std::set<T>&)> &flatten2,
            const std::function<T(const closure::Interpret,const char*,const NestedResultTable &)> flattenT,
            const std::function<T(const NestedResultTable &, size_t)> projector,
            const NestedResultTable &VAR,
                   NestedResultTable &VAL,
                   NestedResultTable &NAME) {
       auto nameType = (NAME.size()) > 1 ? NestedResultTable::RT_VSTRING : NestedResultTable::RT_STRING;
       auto valType = (VAL.size()) > 1 ? vectorType : simpleType;
//       if (VAR.containsInt(30))
//           std::cerr << "DEBUG" << std::endl;
       if ((VAR.cell_nested_morphism == VAL.cell_nested_morphism) &&
           (VAL.cell_nested_morphism == NAME.cell_nested_morphism)) {
           OrderedSet rLS{(size_t)0}, rNS{(size_t)0};
           VAL = I.resolve(VAL, rLS, valType);
           NAME = I.resolve(NAME,rNS, nameType);
           rLS &= rNS;
           for (size_t i = 0, N = std::min({VAR.size(), VAL.size(), NAME.size()}); i < N; i++) {
               if (rLS.contains(i))
               resolve(graph_id, VAR.getInt(i), NAME.getString(i), projector(VAL,i));
//               else
//                   DEBUG_ASSERT((NAME.getInt(i) != (size_t)-1)||(VAL.getInt(i) != (size_t)-1));
           }
       } else if (VAR.cell_nested_morphism == VAL.cell_nested_morphism) {
           OrderedSet rLS{(size_t)0}, rNS{(size_t)0};
           VAL = I.resolve(VAL, rLS, valType);
           NAME = I.resolve(NAME, rNS, nameType);
           for (size_t i = 0, N = NAME.size(); i<N; i++) {
               if (rNS.contains(i)) {
                   for (size_t j = 0, M = VAR.size(); j < M; j++) {
                       if (rLS.contains(j))
                       resolve(graph_id, VAR.getInt(j), NAME.getString(i), projector(VAL,j));
//                       else
//                           DEBUG_ASSERT(VAL.getInt(i) == (size_t)-1);
                   }
               } //else
//                   DEBUG_ASSERT(NAME.getInt(i) == (size_t)-1);
           }
       } else if ((NAME.cell_nested_morphism == VAR.cell_nested_morphism) && (NAME.cell_nested_morphism == -1)) {
           OrderedSet rLS{(size_t)0};
           NAME = I.resolve(NAME, rLS, nameType);
           auto result = flattenT(I, delim, VAL);
           for (size_t i = 0, N = std::min({VAR.size(), NAME.size()}); i < N; i++) {
               if (rLS.contains(i))
               resolve(graph_id, VAR.getInt(i), NAME.getString(i), result);
//               else
//                   DEBUG_ASSERT(NAME.getInt(i) == (size_t)-1);
           }
       } else if ((NAME.cell_nested_morphism == VAR.cell_nested_morphism) && (VAL.cell_nested_morphism == -1)) {
           OrderedSet rLS{(size_t)0};
           NAME = I.resolve(NAME, rLS, nameType);
           auto val = flattenT(I, delim, VAL);
           for (size_t i = 0, N = std::min({VAR.size(), NAME.size()}); i < N; i++) {
               if (rLS.contains(i))
               resolve(graph_id, VAR.getInt(i), NAME.getString(i), val);
//               else
//                   DEBUG_ASSERT(NAME.getInt(i) == (size_t)-1);
           }
       } else if ((NAME.cell_nested_morphism == VAL.cell_nested_morphism) && (VAR.cell_nested_morphism == -1)) {
           OrderedSet rLS{(size_t)0}, rNS{(size_t)0};
           VAL = I.resolve(VAL,rLS, valType);
           NAME = I.resolve(NAME,rNS, nameType);
           DEBUG_ASSERT(VAR.size() == 1);
           std::unordered_map<std::string, std::set<T>> vals;
           rLS &= rNS;
           for (size_t i = 0, N = rLS.cardinality(); i < N; i++) {
               if (rLS.contains(i))
               vals[NAME.getString(i)].insert(projector(VAL,i));
//               else
//                   DEBUG_ASSERT((NAME.getInt(i) == (size_t)-1)||(VAL.getInt(i) == (size_t)-1));
           }
           for (const auto &[k, v]: vals) {
               resolve(graph_id, VAR.getInt(0), k, flatten2(v));
           }
       } else if ((NAME.cell_nested_morphism == VAL.cell_nested_morphism) && (VAL.cell_nested_morphism == -1)) {
           OrderedSet rLS{(size_t)0};
           NAME = I.resolve(NAME, rLS, nameType);
           DEBUG_ASSERT((VAL.size() <= 1) && (NAME.size() == 1));
           auto val = flattenT(I, delim, VAL);
           auto name = getOstringstream(I, delim, NAME);
           for (size_t idx = 0, N = VAR.size(); idx < N; idx++) {
               resolve(graph_id, VAR.getInt(0), name, val);
           }
       } else {
           OrderedSet rLS{(size_t)0};
           NAME = I.resolve(NAME, rLS, nameType);
           DEBUG_ASSERT((NAME.cell_nested_morphism != VAL.cell_nested_morphism) &&
                        (VAR.cell_nested_morphism != VAL.cell_nested_morphism) &&
                        (NAME.cell_nested_morphism != VAR.cell_nested_morphism));
           if (VAR.cell_nested_morphism == -1) {
               DEBUG_ASSERT(VAR.size() == 1);
               size_t id = VAR.getInt(0);
               const auto &namesV = std::get<std::vector<std::string>>(NAME.content);
               std::unordered_set<std::string> names;
               for (size_t idx = 0, N = rLS.cardinality(); idx<N; idx++) {
                   if (rLS.contains(idx))
                       names.insert(namesV.at(idx));
//                   else
//                       DEBUG_ASSERT((NAME.getInt(idx) == (size_t)-1));
               }
//               std::unordered_set<std::string> names(namesV.begin(), namesV.end());
               auto str = flattenT(I, delim, VAL);
               for (const auto &name: names) {
                   resolve(graph_id, VAR.getInt(0), name, str);
//                            delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).content[name] = str;
               }
           } else if (VAL.cell_nested_morphism == -1) {
               DEBUG_ASSERT(VAL.size() == 1);
               const auto &namesV = std::get<std::vector<std::string>>(NAME.content);
               std::unordered_set<std::string> names;
               for (size_t idx = 0, N = rLS.cardinality(); idx<N; idx++) {
                   if (rLS.contains(idx))
                       names.insert(namesV.at(idx));
//                   else
//                       DEBUG_ASSERT((NAME.getInt(idx) == (size_t)-1));
               }
               const auto val = flattenT(I, delim, VAL);
               for (size_t i = 0, N = VAR.size(); i < N; i++) {
                   auto id = VAR.getInt(i);
                   for (const auto &name: names) {
                       resolve(graph_id, id, name, val);
//                                delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).content[name] = val;
                   }
               }
           } else {
               DEBUG_ASSERT(NAME.cell_nested_morphism == -1);
               DEBUG_ASSERT(NAME.size() == 1);
               const auto &name = NAME.getString(0);
               const auto val = flattenT(I, delim, VAL);
//               std::cout << rLS << std::endl;
               if (rLS.contains(0))
               for (size_t i = 0, N = VAR.size(); i < N; i++) {
                   resolve(graph_id, VAR.getInt(i), name, val);
//                            delta_updates_per_graph[graph_id].delta_plus_db.generateId(VAR.getInt(i)).content[name] = val;
               }
//               else
//                   DEBUG_ASSERT(NAME.getInt(0) == (size_t)-1);
           }
       }
   }





//    template <> inline std::vector<gsm_object_xi_content> flattenT<gsm_object_xi_content>(
//            const NestedResultTable &containingStrings) const {
//        return {};
//    }

#endif //GSM2_CLOSURE_H
