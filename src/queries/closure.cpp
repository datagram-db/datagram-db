/*
 * closure.cpp
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

#include "queries/closure.h"
#include "scriptv2/ScriptAST.h"
#include "database/SchemaIndexer.h"

void closure::load_query_from_file(const std::string& filename) {
//    if (forloading) {
//        delete forloading;
//        forloading = nullptr;
//    }
//    forloading = new gsm2::tables::LinearGSM();

    std::ifstream stream(filename);
    antlr4::ANTLRInputStream input(stream);
    simple_graph_grammarLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    simple_graph_grammarParser parser(&tokens);
    GSMPatternVisitor pv;
    // Loading the query from the visitor
    vl = std::any_cast<std::vector<node_match>>(pv.visit(parser.all_matches()));
    // Sorting the patterns according to the order of execution. TODO: improve, depending on the pattern analysis
    sortVL();
    n_patterns = vl.size();
    query_name = filename;
    if (script::structures::ScriptAST::idxers) {
        delete script::structures::ScriptAST::idxers;
        script::structures::ScriptAST::idxers = nullptr;
    }
    if (!script::structures::ScriptAST::idxers)
        script::structures::ScriptAST::idxers = new TemplateIndexer();
}

void closure::load_data_from_file(const std::string &filename) {
    std::tie(loading_time,indexing_time) = gsm2::tables::primary_memory_load_gsm2( filename, *forloading);
    isMaterialised = false;
    n_graphs = forloading->nGraphs;
    n_total_objects = forloading->main_registry.table.size();
    data_name = filename;
}

void closure::asGraphs(std::vector<FlexibleGraph<std::string, std::string>> &graphs) const {
    forloading->asGraphs(graphs);
}


void closure::generateOODbFromMaterialisedViews(gsm2::tables::LinearGSM& newDB) {
    std::unordered_map<std::string, gsm2::tables::AttributeTableType> propertyname_to_type;
    std::vector<adjacency_graph> simpleGraphs(delta_updates_per_graph.size());
    generate_materialised_view();
    std::pair<size_t, size_t> cp;
    std::vector<yaucl::structures::any_to_uint_bimap<size_t>> nodesBeingInsertedAlready(delta_updates_per_graph.size());
    size_t offsetMainRegistryTable = 0;
    std::vector<std::vector<size_t>> initialNodes(delta_updates_per_graph.size());
    std::vector<std::unordered_map<size_t, size_t>> time(delta_updates_per_graph.size());
    for (size_t i = 0, N = delta_updates_per_graph.size(); i<N; i++) {
        const auto& graph_index = forloading->all_indices.at(i).container_order.at(0);
        initialNodes[i].reserve(N);
        for (size_t j = 0, M = graph_index.size(); j<M; j++) {
            initialNodes[i][j] =delta_updates_per_graph.at(i).replacedWith(graph_index.at(j));// getOrDefault(delta_updates_per_graph.at(i).replacement_map, graph_index.at(j), graph_index.at(j));
        }
        for (const auto& [idX,object] : delta_updates_per_graph.at(i).delta_plus_db.O) {
            if (delta_updates_per_graph.at(i).hasXBeenRemoved(idX)) {
                continue;
            }
            cp.first = i;
            cp.second = idX;
            auto& nodeMap = nodesBeingInsertedAlready[i];
            auto& g = simpleGraphs[i];
            size_t id = nodeMap.put(idX).first;
            size_t gid;
            gid = g.add_node();
            std::string tmp;
            DEBUG_ASSERT(id == gid);
            offsetMainRegistryTable++;
        }
        for (const auto& [idX,object] : delta_updates_per_graph.at(i).delta_plus_db.O) {
            if (delta_updates_per_graph.at(i).hasXBeenRemoved(idX))
                continue;
            for (const auto& [edgelabel,records] : object.phi) {
                for (const auto& record: records) {
                    if (delta_updates_per_graph.at(i).hasXBeenRemoved(record.id))
                        continue;
                    const auto& map = nodesBeingInsertedAlready.at(i);
                    auto src = map.getKey(idX);
                    auto dst = map.signed_get(record.id);
                    if (dst >= 0) {
                        auto& g = simpleGraphs[i];
                        g.add_edge(src, dst);
                    }
                }
            }
        }
    }
    for (size_t k = 0, N = delta_updates_per_graph.size(); k<N; k++) {
        roaring::Roaring64Map visited;
        std::vector<size_t> Stack;
        auto& g = simpleGraphs[k];
        for (size_t start_from : initialNodes[k]) {
            g.topologicalSortUtil(start_from, visited, Stack);
        }
        for (size_t i = 0; i < g.V_size; i++)
            if (!visited.contains(i))
                g.topologicalSortUtil(i, visited, Stack);
        bool firstVisit = true;
        std::reverse(Stack.begin(), Stack.end());
        for (size_t v : Stack) {
            if (firstVisit) {
                time[k][v] = 0;
                firstVisit = false;
            } else {
                time[k][v] = 0;
                auto it = g.ingoing_edges.find(v);
                if (it != g.ingoing_edges.end()) {
                    for (size_t edgeId : it->second)
                        time[k][v] = std::max(time[k][g.edge_ids.at(edgeId).first], time[k][v]);
                    time[k][v]++;
                }
            }
        }
    }
    gsm_object orig;
    size_t vt;
    std::vector<size_t> idx_to_remove;
    for (size_t i = 0, N = delta_updates_per_graph.size(); i<N; i++) {
        const auto& map = nodesBeingInsertedAlready.at(i);
        for (const auto& [idX,object] : delta_updates_per_graph.at(i).delta_plus_db.O) {
            if (delta_updates_per_graph.at(i).hasXBeenRemoved(idX)) {
                continue;
            }
            cp.first = i;
            cp.second = idX;
            orig = object;
            orig.id = map.signed_get(orig.id);
            vt = time.at(i).at(orig.id);
            for (auto it = orig.phi.begin(); it != orig.phi.end(); it++) {
                idx_to_remove.clear();
                for (size_t j = 0; j<it->second.size(); j++) {
                    auto& content = it->second[j];
                    content.id = map.signed_get(content.id);
                    if (time[i][content.id] <= vt) {
                        idx_to_remove.emplace_back(j);
                    }
                }
                if (!idx_to_remove.empty())
                    remove_index(it->second, idx_to_remove);
                newDB.loadGSMObject(i, propertyname_to_type, orig);
            }
        }
    }
    newDB.index();
}

void closure::generateGraphsFromMaterialisedViews(std::vector<FlexibleGraph<std::string, std::string>> &simpleGraphs) {
    generate_materialised_view();
    simpleGraphs.clear();
    simpleGraphs.resize(delta_updates_per_graph.size());
    std::pair<size_t, size_t> cp;
    std::vector<yaucl::structures::any_to_uint_bimap<size_t>> nodesBeingInsertedAlready(delta_updates_per_graph.size());
    size_t offsetMainRegistryTable = 0;
    std::vector<std::vector<size_t>> initialNodes(delta_updates_per_graph.size());
    std::vector<std::unordered_map<size_t, size_t>> time(delta_updates_per_graph.size());
    auto real_time_start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0, N = delta_updates_per_graph.size(); i<N; i++) {
        initialNodes[i].reserve(N);
        const auto& cn = forloading->all_indices.at(i).container_order;
        if (cn.empty()) {
            for (size_t j = 0, M = forloading->objectScores.at(i).size(); j<M; j++) {
                initialNodes[i][j] = delta_updates_per_graph.at(i).replacedWith(j); // getOrDefault(delta_updates_per_graph.at(i).replacement_map, j, j);
            }
            // This implies that nodes are not contained between each other. So, any of these nodes will be the same
        } else {
            const auto& graph_index = cn.at(0);
            for (size_t j = 0, M = graph_index.size(); j<M; j++) {
                initialNodes[i][j] = delta_updates_per_graph.at(i).replacedWith(graph_index.at(j)); //  getOrDefault(delta_updates_per_graph.at(i).replacement_map, graph_index.at(j), graph_index.at(j));
            }
        }


        for (const auto& [idX,object] : delta_updates_per_graph.at(i).delta_plus_db.O) {
            if (delta_updates_per_graph.at(i).hasXBeenRemoved(idX)) {
                continue;
            }
            cp.first = i;
            cp.second = idX;
            auto& nodeMap = nodesBeingInsertedAlready[i];
            auto& g = simpleGraphs[i];
            size_t id = nodeMap.put(idX).first;
            size_t gid;
            std::string tmp;
            if (object.xi.empty())
                if (object.ell.empty())
                    tmp = std::to_string(idX);
                else
                    // If no content exists, using the first label, which should exist
                    tmp = "\""+object.ell[0]+"\"="+std::to_string(idX)+"";
            else
                // Otherwise, using the first value
                tmp = object.xi[0]+"="+std::to_string(idX);
            for (const auto& [keyAttribute, value] : object.content) {
                if (std::holds_alternative<std::string>(value)) {
                    tmp = tmp+"|"+keyAttribute+"="+(std::get<std::string>(value));
                } else {
                    tmp = tmp+"|"+keyAttribute+"="+std::to_string(std::get<double>(value));
                }
            }
            gid = g.addNewNodeWithLabel(tmp);
            DEBUG_ASSERT(id == gid);
            offsetMainRegistryTable++;
        }
        for (const auto& [idX,object] : delta_updates_per_graph.at(i).delta_plus_db.O) {
            if (delta_updates_per_graph.at(i).hasXBeenRemoved(idX))
                continue;
            for (const auto& [edgelabel,records] : object.phi) {
                for (const auto& record: records) {
                    if (delta_updates_per_graph.at(i).hasXBeenRemoved(record.id))
                        continue;
                    const auto& map = nodesBeingInsertedAlready.at(i);
                    auto src = map.getKey(idX);
                    auto dst = map.signed_get(record.id);
                    if (dst >= 0) {
                        auto& g = simpleGraphs[i];
                        g.addNewEdgeFromId(src, dst, edgelabel);
                    }
                }
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
    }
    auto real_time_end = std::chrono::high_resolution_clock::now();
    materialise_time_final = std::chrono::duration<double, std::milli>(real_time_end-real_time_start).count();
}

#include <chrono>

void closure::generate_materialised_view() {
    if (!isMaterialised) {
        auto real_time_start = std::chrono::high_resolution_clock::now();
        isMaterialised = true;
        forloading->iterateOverObjects([this](size_t graphid, const gsm_object& legacy_object_old_data) {
            auto& updates = delta_updates_per_graph[graphid];
            if ((!updates.hasXBeenRemoved(legacy_object_old_data.id))) {
                size_t i =0;
                std::vector<size_t> removed_indices;
                size_t new_id = legacy_object_old_data.id;
                auto& obj = updates.delta_plus_db.O[new_id];
                obj.id = new_id;
//                if ((graphid== 2) && (legacy_object_old_data.id == 3))
//                    std::cout << "DREBUG" << std::endl;
                obj.updateWith(legacy_object_old_data);
                for (auto& [k, v] :obj.phi) {
                    i = 0;
                    removed_indices.clear();
                    for (auto& w : v) {
                        w.id = updates.replacedWith(w.id);
                        if ((w.orig_edge_id != -1) &&  delta_updates_per_graph.at(graphid).removed_edges.contains(w.orig_edge_id)) {
                            removed_indices.emplace_back(i);
                        }
                        i++;
                    }
                    remove_index(v, removed_indices);
                }
            }
        });
        auto real_time_end = std::chrono::high_resolution_clock::now();
        materialise_time_collection = std::chrono::duration<double, std::milli>(real_time_end-real_time_start).count();
    }

}

void closure::reinitAndClearNestedIndices() {
    script::structures::ScriptAST::idxers->map.clear();
}

#include <ranges>


std::string getFromScript(const NestedResultTable & containingStrings) {
    return std::get<std::shared_ptr<script::structures::ScriptAST>>(containingStrings.content).get()->run()->toString();
}

void closure::interpret_closure_set(rewrite_expr *ptr,
                                    size_t graph_id,
                                    size_t pattern_id,
                                    const std::vector<std::string> &schema,
                                    const nested_table &table,
                                    size_t record_id,
                                    rewrite_expr *target_ptr,
                                    Interpret& I) /** non-const!*/ {

    const char* const delim = " ";
    if (!ptr)
        return;

    switch (ptr->t) {
        case rewrite_expr::NODE_OR_EDGE:
        case rewrite_expr::SCRIPT_CASE:
        case rewrite_expr::NONE_CASES_REWRITE:
            // Ignoring setting the expression
            break;

        case rewrite_expr::NODE_ELL:
        case rewrite_expr::NODE_XI: {
            if (ptr->ptr_or_else->t != rewrite_expr::VARIABLE) {
                throw std::runtime_error("ERROR: we expect that the second argument fod XI is always a variable");
            }
            const auto& record = table.datum.at(record_id);
            auto lhs = resolveIdsOverVariableName2(graph_id, pattern_id, ptr->ptr_or_else->prop, record, true);
            if (((lhs.cell_nested_morphism == -1) || (lhs.column == -1)) && (ptr->ptr_or_else->prop == "Z")) {
                resolveIdsOverVariableName2(graph_id, pattern_id, ptr->ptr_or_else->prop, record, true);
            }
            bool isXI = ptr->t == rewrite_expr::NODE_XI;
            NestedResultTable rhs = I.interpret_closure_evaluate(target_ptr, true, true);
            size_t xi_offset = ptr->id;
            DEBUG_ASSERT((lhs.expectedType == NestedResultTable::RT_SIZET) || (lhs.expectedType == NestedResultTable::RT_VSIZET));
            if (lhs.cell_nested_morphism == rhs.cell_nested_morphism) {
                OrderedSet including{(size_t)0};
                rhs = I.resolve(rhs, including, std::max(rhs.size(), lhs.size()) > 1 ? NestedResultTable::RT_VSTRING : NestedResultTable::RT_STRING);
                for (size_t i = 0, N = including.cardinality(); i<N; i++) {
                    if (including.contains(i)) {
                        const auto &id = lhs.getInt(i);
                        auto &xi = isXI ? delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).xi
                                        : delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).ell;
                        if (xi.empty())
                            xi.resize(xi_offset+1, "");
                        if (xi.size() <= xi_offset)
                            xi.insert(xi.end(), xi_offset - xi.size() + 1, "");
                        xi[xi_offset] = rhs.getString(i);
                    }
                }
            } else if (lhs.cell_nested_morphism == -1) {
                const auto &id = lhs.getInt(0);
                auto &xi = isXI ? delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).xi
                                : delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).ell;
                if (xi.empty())
                    xi.resize(xi_offset+1, "");
                if (xi.size() <= xi_offset)
                    xi.insert(xi.end(), xi_offset - xi.size() + 1, "");
                auto tmp = getOstringstream(I, delim, rhs);
                xi[xi_offset] = tmp;
            } else if (rhs.cell_nested_morphism == -1) {
                OrderedSet including{0};
                if (rhs.expectedType != NestedResultTable::RT_STRING) {
                    rhs = I.resolve(rhs, including, NestedResultTable::RT_STRING);
                }
                const auto& R0 = rhs.getString(0);
                for (size_t i = 0, N = lhs.size(); i<N; i++) {
                    const auto &id = lhs.getInt(i);
                    auto &xi = isXI ? delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).xi
                                    : delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).ell;
                    if (xi.empty())
                        xi.resize(xi_offset+1, "");
                    if (xi.size() <= xi_offset)
                        xi.insert(xi.end(), xi_offset - xi.size() + 1, "");
                    xi[xi_offset] = R0;
                }
            } else {
                std::string imploded = getOstringstream(I, delim, rhs);
                for (size_t i = 0, N = lhs.size(); i<N; i++) {
                    const size_t& id = lhs.getInt(i);
                    auto &xi = isXI ? delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).xi
                                    : delta_updates_per_graph[graph_id].delta_plus_db.generateId(id).ell;
                    if (xi.empty())
                        xi.resize(xi_offset+1, "");
                    if (xi.size() <= xi_offset)
                        xi.insert(xi.end(), xi_offset - xi.size() + 1, "");
                    xi[xi_offset] = imploded;
                }
            }
        } break;


        case rewrite_expr::NODE_PROP: {
            auto vectorType = NestedResultTable::RT_VSTRING;
            auto simpleType = NestedResultTable::RT_STRING;
            if (ptr->ptr_or_else->t != rewrite_expr::VARIABLE) {
                throw std::runtime_error("ERROR: we expect that the second argument fod XI is always a variable");
            }
            const auto& record = table.datum.at(record_id);
            NestedResultTable VAR = resolveIdsOverVariableName2(graph_id, pattern_id, ptr->ptr_or_else->prop, record, true);
            NestedResultTable VAL = I.interpret_closure_evaluate(target_ptr, true, true);
            NestedResultTable NAME = I.interpret_closure_evaluate(ptr->pi_key_arg_or_then.get(), false, true, NestedResultTable::variant_type_cpp::RT_STRING);
            std::function<void(size_t, size_t, const std::string&, const std::string&)> resolve = [this](size_t graph_id, size_t var, const std::string& x, const std::string& val) {
                delta_updates_per_graph[graph_id].delta_plus_db.generateId(var).content[x] = val;
            };
            std::function<std::string(const std::set<std::string>&)> resolve2 = [](const std::set<std::string>& v) {
                std::string s;
                size_t len = 0;
                for (const auto& x : v) len += x.size();
                s.reserve(len);
                for (auto ptr = v.begin(), en = v.end(); ptr != en; ) {
                    s += *ptr;
                    ptr++;
                    if (ptr != en)
                        s += " ";
                }
                return s;
            };
            std::function<std::string(const NestedResultTable &, size_t)> projS = [this](const NestedResultTable & VAL, size_t i) {
                return VAL.getString(i);
            };
            complexInstantiate<std::string>(graph_id, *this,I, delim, vectorType, simpleType,  resolve, resolve2,getOstringstream,projS,  VAR, VAL, NAME);
        } break;

        case rewrite_expr::NODE_CONT: {
            auto vectorType = NestedResultTable::RT_VCONTENT;
            auto simpleType = NestedResultTable::RT_CONTENT;
            if (ptr->ptr_or_else->t != rewrite_expr::VARIABLE) {
                throw std::runtime_error("ERROR: we expect that the second argument fod XI is always a variable");
            }
            const auto& record = table.datum.at(record_id);
            NestedResultTable VAR = resolveIdsOverVariableName2(graph_id, pattern_id, ptr->ptr_or_else->prop, record, true);
            NestedResultTable VAL = I.interpret_closure_evaluate(target_ptr, true, true);
            NestedResultTable NAME = I.interpret_closure_evaluate(ptr->pi_key_arg_or_then.get(), false, true, NestedResultTable::variant_type_cpp::RT_STRING);

            std::function<void(size_t, size_t, const std::string&, const std::vector<gsm_object_xi_content>&)> resolve = [this,&I,&ptr](size_t graph_id, size_t var, const std::string& x, const std::vector<gsm_object_xi_content>& val) {
                if (x.contains("is is"))
                    std::cout << "ERROR" << std::endl;
                delta_updates_per_graph[graph_id].delta_plus_db.generateId(var).phi[x] = val;
            };
            std::function<std::vector<gsm_object_xi_content>(const std::set<std::vector<gsm_object_xi_content>>&)> resolve2 = [](const std::set<std::vector<gsm_object_xi_content>>& v) {
                std::vector<gsm_object_xi_content> imploded;
                for (const auto& x : v)
                    imploded.insert(imploded.end(), x.begin(), x.end());
                remove_duplicates(imploded);
                return imploded;
            };
            std::function<std::vector<gsm_object_xi_content>(const NestedResultTable &, size_t)> projS = [](const NestedResultTable & VAL, size_t i) {
                return VAL.getContent(i);
            };
            complexInstantiate<std::vector<gsm_object_xi_content>>(graph_id, *this,I, delim, vectorType, simpleType,  resolve, resolve2, getFlattenedContent,projS, VAR, VAL, NAME);
        } break;

        case rewrite_expr::EDGE_LABEL: {
            throw std::runtime_error("ERROR: cannot change the edge label! Go for the creation of a new content.");
        } break;

        case rewrite_expr::EDGE_SRC:
        case rewrite_expr::EDGE_DST:
            throw std::runtime_error("ERROR: cannot change the edge's SRC/DST at runtime! Please use the containment function jointly with edge removal");
            break;

        case rewrite_expr::VARIABLE: {
            throw std::runtime_error("ERROR: you cannot set a variable directly! You need to specify which property of such variable you want to set");
        } break;

        case rewrite_expr::IFTE_RW: {
            throw std::runtime_error("ERROR: at this stage of implementation, cannot have a statement which is directly an if-then-else: this should be necessarily evaluated (but, this can be extended)");
//            if (I.interpret(ptr->ifcond)) {
//                interpret_closure_set(ptr->pi_key_arg_or_then.get(), graph_id, pattern_id, schema, table, record_id, target_ptr, I);
//            } else {
//                interpret_closure_set(ptr->ptr_or_else.get(), graph_id, pattern_id, schema, table, record_id, target_ptr, I);
//            }
        } break;

        default:
            throw std::runtime_error("ERROR: the rest, has to be implemented");

    }
}



void closure::load_query_from_string(std::istream &stream) {
    antlr4::ANTLRInputStream input(stream);
    simple_graph_grammarLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    simple_graph_grammarParser parser(&tokens);
    GSMPatternVisitor pv;
    vl = std::any_cast<std::vector<node_match>>(pv.visit(parser.all_matches()));
    // Sorting the patterns according to the order of execution. TODO: improve, depending on the pattern analysis
    sortVL();
    n_patterns = vl.size();
    query_name = "stream";
}

void closure::new_data_slate() {
    if (forloading) {
        delete forloading;
        forloading = nullptr;
    }
    forloading = new gsm2::tables::LinearGSM();
}

#include <scriptv2/ScriptVisitor.h>
#include <scriptv2/ScriptAST.h>

NestedResultTable closure::Interpret::interpret_closure_evaluate(rewrite_expr *ptr, bool force, bool node_or_edge_otherwise, NestedResultTable::variant_type_cpp expected) /*const*/ {
    if (!ptr)
        return {};
    switch (ptr->t) {
        case rewrite_expr::SCRIPT_CASE: {
            return {std::move(script::compiler::ScriptVisitor::eval(ptr->prop.c_str(), ptr->prop.length(),pattern_id, ptr->ptrResult, schema, table.datum.at(record_id)))};
        }

            // Just returning the variable name associated to the object
        case rewrite_expr::VARIABLE:{
            if (force) {
                if (node_or_edge_otherwise) {
                    return clos.resolveIdsOverVariableName2(graph_id, pattern_id, ptr->prop, table.datum.at(record_id));
                } else {
                    return clos.resolveEdgesOverVariableName2(pattern_id, ptr->prop, table.datum.at(record_id));
                }
            } else {
                return {ptr->prop};
            }
        } break;

            // Returning the edge label (if nested, imploding the collection into a string!) TODO: what if we need singled out?
        case rewrite_expr::EDGE_LABEL: {
            auto edges = interpret_closure_evaluate(ptr->ptr_or_else.get(), true, false);
//            if (edges.t == NestedResultTable::R_NONE) {
//                interpret_closure_evaluate(ptr->ptr_or_else.get(), true, false);
//            }
//            DEBUG_ASSERT((edges.t == NestedResultTable::R_EDGE) || (edges.t == NestedResultTable::R_NESTED_EDGE));
            if (edges.t == NestedResultTable::R_EDGE) {
                edges.t = NestedResultTable::R_DO_EDGE_LABEL;
            } else if (edges.t == NestedResultTable::R_NESTED_EDGE) {
                edges.t = NestedResultTable::R_DO_NESTED_EDGE_LABEL;
            } /*else if (edges.t != NestedResultTable::R_NONE)
                DEBUG_ASSERT(false);*/
            return edges;
        } break;

            // Returning the specific XI for the nodes
        case rewrite_expr::NODE_XI: {
            NestedResultTable edges = interpret_closure_evaluate(ptr->ptr_or_else.get(), true, true);
//            DEBUG_ASSERT((edges.t == NestedResultTable::R_NODE) || (edges.t == NestedResultTable::R_NESTED_NODE));
            if (edges.t == NestedResultTable::R_NODE) {
                edges.t = NestedResultTable::R_DO_XI;
            } else if (edges.t == NestedResultTable::R_NESTED_NODE) {
                edges.t = NestedResultTable::R_DO_NESTED_XI;
            } /*else
                DEBUG_ASSERT(false);*/
            edges.opt_offset = ptr->id;
            return edges;

        } break;

            // Returning the specific ELLS for the nodes
        case rewrite_expr::NODE_ELL: {
            NestedResultTable edges = interpret_closure_evaluate(ptr->ptr_or_else.get(), true, true);
//            DEBUG_ASSERT((edges.t == NestedResultTable::R_NODE) || (edges.t == NestedResultTable::R_NESTED_NODE));
            if (edges.t == NestedResultTable::R_NODE) {
                edges.t = NestedResultTable::R_DO_ELL;
            } else if (edges.t == NestedResultTable::R_NESTED_NODE) {
                edges.t = NestedResultTable::R_DO_NESTED_ELL;
            } /*else if (edges.t != NestedResultTable::R_NONE)
                DEBUG_ASSERT(false);*/
            edges.opt_offset = ptr->id;
            return edges;
        }

            // Returning the property associated to a node
        case rewrite_expr::NODE_PROP: {
            NestedResultTable prop_name = interpret_closure_evaluate(ptr->pi_key_arg_or_then.get(), false, false);
            NestedResultTable evaluation = interpret_closure_evaluate(ptr->ptr_or_else.get(), true, true);
            evaluation.fields = std::make_shared<NestedResultTable>(std::move(prop_name));
            if (evaluation.t == NestedResultTable::R_NODE) {
                evaluation.t = NestedResultTable::R_DO_PROP;
            } else if (evaluation.t == NestedResultTable::R_NESTED_NODE) {
                evaluation.t = NestedResultTable::R_DO_NESTED_PROP;
            } /*else
                DEBUG_ASSERT(false);*/
            return evaluation;
        } break;

        case rewrite_expr::NODE_CONT: {
            NestedResultTable prop_name = interpret_closure_evaluate(ptr->pi_key_arg_or_then.get(), false, false);
            NestedResultTable evaluation = interpret_closure_evaluate(ptr->ptr_or_else.get(), true, true);
            evaluation.fields = std::make_shared<NestedResultTable>(std::move(prop_name));
            if (evaluation.t == NestedResultTable::R_NODE) {
                evaluation.t = NestedResultTable::R_DO_CONTENT;
            } else if (evaluation.t == NestedResultTable::R_NESTED_NODE) {
                evaluation.t = NestedResultTable::R_DO_NESTED_CONTENT;
            } /*else
                DEBUG_ASSERT(false);*/
            return evaluation;
        } break;

        case rewrite_expr::IFTE_RW: {
            auto result = interpret(ptr->ifcond, 1);

            if (result.full()) {
                return interpret_closure_evaluate(ptr->pi_key_arg_or_then.get(), force, node_or_edge_otherwise);
            } else if (result.empty()) {
                return interpret_closure_evaluate(ptr->ptr_or_else.get(), force, node_or_edge_otherwise);
            } else {
                NestedResultTable variableResolution;
                if (node_or_edge_otherwise) {
                    variableResolution = clos.resolveIdsOverVariableName2(graph_id, pattern_id, ptr->prop, table.datum.at(record_id));
                } else {
                    variableResolution = clos.resolveEdgesOverVariableName2(pattern_id, ptr->prop, table.datum.at(record_id));
                }
                if (variableResolution.expectedType != NestedResultTable::variant_type_cpp::RT_VSIZET) {
                    throw std::runtime_error("ERROR: variable "+ptr->prop+" should be an int!");
                }
                DEBUG_ASSERT(result.cardinality() > 1);
                OrderedSet toConsider{(size_t)0}, none{(size_t)0};
                auto l = interpret_closure_evaluate(ptr->pi_key_arg_or_then.get(), force, node_or_edge_otherwise);

                // TODO: estimate the type of R just from the expression, so to save computational time.
                //       by doing this, we can then compute r after updating toConsider with result, and only compute the results for the specific positions
                auto r = interpret_closure_evaluate(ptr->ptr_or_else.get(), force, node_or_edge_otherwise);
                NestedResultTable::variant_type_cpp casting = NestedResultTable::variant_type_cpp::RT_NONE;
                if (l.t != NestedResultTable::variant_type::R_SCRIPT) {
                    casting = getExpectedType(l.t);
                } else if (r.t != NestedResultTable::variant_type::R_SCRIPT) {
                    casting = getExpectedType(r.t);
                } else {
                    casting = expected;
                }
//                // PATCH
//                if ((casting == NestedResultTable::variant_type_cpp::RT_SCRIPT) || (casting == NestedResultTable::variant_type_cpp::RT_NONE))
//                    std::cout << "ERROR HERE!";
//                if (std::holds_alternative<std::string>(l.content)) {
//                    casting = NestedResultTable::variant_type_cpp::RT_STRING;
//                }

                l = resolve(l,toConsider,casting);
//                std::cout << "R" << result << std::endl;
//                std::cout << toConsider << std::endl;
                r = resolve(r,none,casting);
//                std::cout << none << std::endl;
//                toConsider &= result;
//                std::cout << toConsider << std::endl;
                switch (casting) {
                    case NestedResultTable::RT_VSTRING: {
                        std::vector<std::string> v;
                        for (size_t j = 0; j< variableResolution.size(); j++) {
                            size_t i = variableResolution.getInt(j);
                            if ((result.contains(i)) && (l.hasTInVector<std::string>(j)))
                                v.emplace_back(l.getV<std::string>(j));
                            else if (r.hasTInVector<std::string>(j))
                                v.emplace_back(r.getV<std::string>(j));
                        }
                        return {std::move(v), -1, -1};
                    }
                        break;

                    case NestedResultTable::RT_STRING: {
                        std::vector<std::string> v;
                        for (size_t j = 0; j< variableResolution.size(); j++) {
                            size_t i = variableResolution.getInt(j);
                            if ((result.contains(i)))
                                v.emplace_back(l.getString(j));
                            else if (r.hasTInVector<std::string>(j))
                                v.emplace_back(r.getString(j));
                        }
                        return {std::move(v), -1, -1};
                    }

                    case NestedResultTable::RT_VSIZET: {
                        std::vector<size_t> v;
                        for (size_t j = 0; j< variableResolution.size(); j++) {
                            size_t i = variableResolution.getInt(j);
                            if ((result.contains(i)) && (l.hasTInVector<size_t>(j)))
                                v.emplace_back(l.getV<size_t>(j));
                            else if (r.hasTInVector<size_t>(j))
                                v.emplace_back(r.getV<size_t>(j));
                        }
                        return {std::move(v), true, -1, -1};
                    }
                        break;

                    case NestedResultTable::RT_SIZET: {
                        std::vector<size_t> v;
                        for (size_t j = 0; j< variableResolution.size(); j++) {
                            size_t i = variableResolution.getInt(j);
                            if ((result.contains(i)) && (l.containsInt(j)))
                                v.emplace_back(l.getInt(j));
                            else if (r.hasTInVector<size_t>(j))
                                v.emplace_back(r.getInt(j));
                        }
                        return {std::move(v), true, -1, -1};
                    }
                        break;

                    case NestedResultTable::RT_VCONTENT:{
                        std::vector<std::vector<gsm_object_xi_content>> v;
                        for (size_t j = 0; j< variableResolution.size(); j++) {
                            size_t i = variableResolution.getInt(j);
                            if (result.contains(i)  && (l.hasTInVector<gsm_object_xi_content>(j)))
                                v.emplace_back(l.getV<std::vector<gsm_object_xi_content>>(j));
                            else if (r.hasTInVector<gsm_object_xi_content>(j))
                                v.emplace_back(r.getV<std::vector<gsm_object_xi_content>>(j));
                        }
                        return {std::move(v), -1, -1};
                    }
                        break;

                    case NestedResultTable::RT_CONTENT:
                    case NestedResultTable::RT_SCRIPT:
                    case NestedResultTable::RT_NONE:
                        throw std::runtime_error("ERROR: Having a single value, while I'm expecting ");
                        break;
                }
            }
            // TODO
//            return interpret(ptr->ifcond) ?
//                    interpret_closure_evaluate(ptr->pi_key_arg_or_then.get(), force, node_or_edge_otherwise) :
//                    interpret_closure_evaluate(ptr->ptr_or_else.get(), force, node_or_edge_otherwise);
        } break;

        case rewrite_expr::EDGE_SRC:  {
            auto edges = interpret_closure_evaluate(ptr->ptr_or_else.get(), true, false);
            DEBUG_ASSERT((edges.t == NestedResultTable::R_EDGE) || (edges.t == NestedResultTable::R_NESTED_EDGE));
            if (edges.t == NestedResultTable::R_EDGE) {
                edges.t = NestedResultTable::R_DO_EDGE_SRC;
            } else if (edges.t == NestedResultTable::R_NESTED_EDGE) {
                edges.t = NestedResultTable::R_DO_NESTED_EDGE_SRC;
            } /*else
                DEBUG_ASSERT(false);*/
            return edges;
        } break;

        case rewrite_expr::EDGE_DST: {
            auto edges = interpret_closure_evaluate(ptr->ptr_or_else.get(), true, false);
            DEBUG_ASSERT((edges.t == NestedResultTable::R_EDGE) || (edges.t == NestedResultTable::R_NESTED_EDGE));
            if (edges.t == NestedResultTable::R_EDGE) {
                edges.t = NestedResultTable::R_DO_EDGE_DST;
            } else if (edges.t == NestedResultTable::R_NESTED_EDGE) {
                edges.t = NestedResultTable::R_DO_NESTED_EDGE_DST;
            } /*else
                DEBUG_ASSERT(false);*/
            return edges;
        }

        default:
            throw std::runtime_error("UNSUPPORTED OPERATION (FUTURE)");
    }
}

static inline bool var_extractor(const test_pred& ptr,
                                 size_t& offsetForStar,
                                 size_t& offsetForValue,
                                 size_t& offsetNested,
                                 const std::string& _for_match,
                                 const std::vector<std::string>& schema,
                                 const nested_table& table) {
    for (size_t i = 0, N = schema.size(); i<N; i++) {
        if (schema.at(i) == "*")
            offsetForStar = i;
        else if (schema.at(i) == _for_match)
            offsetForValue = i;
        if (offsetForValue != (size_t)-1)
            break;
    }
    if ((offsetForValue == (size_t)-1)) {
        if (offsetForStar == -1) {
            for (size_t i = 0, N = table.datum.at(0).size(); i<N; i++) {
                if (table.datum.at(0).at(i).isNested) {
                    offsetForStar = i;
                    break;
                }
            }
        }
        if ((offsetForStar==-1))
            return false;
        DEBUG_ASSERT(!table.datum.empty());
        DEBUG_ASSERT(table.datum.at(0).at(offsetForStar).isNested);
        const auto& access = table.datum.at(0).at(offsetForStar).table.Schema;
        for (size_t i = 0, N = access.size(); i<N; i++) {
            if (access.at(i) == ptr.variable_matched) {
                offsetNested = i;
                break;
            }
        }
        if ((offsetNested) == (size_t)-1)
            return false;
    }
    return true;
}

static inline std::string interpret_implosive_string(const std::any& ptr) {
    auto tmp = get_v_opt<std::string>(ptr);
    if (tmp.has_value())
        return tmp.value();
    auto x = std::get<1>(std::any_cast<std::tuple<std::pair<ssize_t,ssize_t>,std::vector<std::string>>>(ptr));
    const char* const delim = " ";
    std::ostringstream imploded;
    std::copy(x.begin(), x.end(),
              std::ostream_iterator<std::string>(imploded, delim));
    return imploded.str();
}

OrderedSet closure::Interpret::interpret(test_pred &ptr, size_t maxSize) /*const*/ {
    switch (ptr.t) {
        case test_pred::TEST_PRED_CASE_LT:
        case test_pred::TEST_PRED_CASE_LEQ:
        case test_pred::TEST_PRED_CASE_NEQ:
        case test_pred::TEST_PRED_CASE_EQ: {
            ComparisonForNestedResultTable cmp;
            if (ptr.t == test_pred::TEST_PRED_CASE_LT)
                cmp = CFNRT_LT;
            else if (ptr.t == test_pred::TEST_PRED_CASE_LEQ)
                cmp = CFNRT_LEQ;
            else if (ptr.t == test_pred::TEST_PRED_CASE_EQ)
                cmp = CFNRT_EQ;
            else if (ptr.t == test_pred::TEST_PRED_CASE_NEQ)
                cmp = CFNRT_NEQ;
            NestedResultTable L, R;
            bool leftResolved = false, rightResolved = false;
            if (std::holds_alternative<std::string>(ptr.args.at(0))) {
                L = NestedResultTable{std::get<std::string>(ptr.args.at(0))};
            } else {
                auto j = std::get<std::shared_ptr<rewrite_expr>>(ptr.args.at(0)).get();
                L = interpret_closure_evaluate(j, false, false);
                leftResolved = true;
            }
            if (std::holds_alternative<std::string>(ptr.args.at(1))) {
                R = NestedResultTable{std::get<std::string>(ptr.args.at(1))};
            } else {
                auto j = std::get<std::shared_ptr<rewrite_expr>>(ptr.args.at(1)).get();
                R = interpret_closure_evaluate(j, false, false);
                rightResolved = true;
            }
            if ((L.size() == 1) || (R.size() == 1) || (L.size() == R.size())) {
                OrderedSet toIgnore{0};
                auto tmp = resolve(L, toIgnore, getExpectedType(R.t));
                auto nestedResult = tmp.compare(resolve(R, toIgnore, getExpectedType(L.t)), cmp);
                auto idResolutionL = resolveIDs(L, getExpectedType(R.t));
                if (std::get<1>(idResolutionL)) {
                    return getFullOrEmptySet(std::get<2>(idResolutionL));
                } else {
                    toIgnore.clearWithMaxCardinality(L.size());
                    for (const size_t val : nestedResult.set)
                        toIgnore.add(std::get<0>(idResolutionL)[val]);
                    return toIgnore;
                }
            } else {
                throw std::runtime_error("ERROR: either both morphisms should be of the same size, of at least one of them should be of size 1");
            }

        } break;

        case test_pred::FILL: {
            auto l = interpret(ptr.child_logic.at(0), maxSize);
            return getFullOrEmptySet(!l.empty());
        }

        case test_pred::TEST_PRED_CASE_AND:
        {
            auto l = interpret(ptr.child_logic.at(0), maxSize);
            if (l.empty()) return l;
            auto l2 = interpret(ptr.child_logic.at(1), maxSize);
//            std::cout << "AT THIS POINT:" << std::endl;
//            std::cout << "==============" << std::endl;
//            std::cout<<l<<std::endl;
//            std::cout<<l2<<std::endl;
            l &= l2;
            return l;
        } break;

        case test_pred::TEST_PRED_CASE_OR:
        {
            auto l = interpret(ptr.child_logic.at(0), maxSize);
            if (l.full()) return l;
            l |= interpret(ptr.child_logic.at(1), maxSize);
            return l;
        } break;

        case test_pred::TRUE:
        {
            return {maxSize};
        } break;

        case test_pred::TEST_PRED_CASE_SCRIPT: {
            bool result = script::compiler::ScriptVisitor::evalBool(ptr.nsoe.data(),ptr.nsoe.length(), pattern_id, ((ptr.ptrResult)), schema, table.datum.at(record_id));
            return getFullOrEmptySet(result);
        }
            break;
        case test_pred::MATCHED: {
            OrderedSet m{(size_t)0};
            auto it = this->ptr.at(graph_id).find(ptr.pattern_matched);
            if (it == this->ptr.at(graph_id).end())
                return m;

            std::unordered_set<size_t> currentMatches;
            {
                size_t offsetForStar1 = -1;
                size_t offsetForValue1 = -1;
                size_t offsetNested1 = -1;
                if (!var_extractor(ptr, offsetForStar1, offsetForValue1, offsetNested1, ptr.nsoe, schema, table))
                    return m;
                if (offsetNested1 == (size_t)(-1)) {
                    if (!std::holds_alternative<size_t>(
                            table.datum.at(record_id).at(offsetForValue1).val))
                        return m;
                    currentMatches.insert(
                            std::get<size_t>(
                                    table.datum.at(record_id).at(offsetForValue1).val));
                } else {
                    if (!table.datum.at(record_id).at(offsetForValue1).isNested)
                        return m;
                    for (const auto& row :
                            table.datum.at(record_id).at(offsetForValue1).table.datum) {
                        if (!std::holds_alternative<size_t>(row.at(offsetNested1).val))
                            return m;
                        else
                            currentMatches.insert(
                                    std::get<size_t>(row.at(offsetNested1).val));
                    }
                }
            }
            if (currentMatches.empty())
                return m;

            size_t offsetForStar = -1;
            size_t offsetForValue = -1;
            size_t offsetNested = -1;
            const std::string& _for_match = ptr.variable_matched;
            const nested_table& table2 = it->second.second.begin()->second;
            if (!var_extractor(ptr, offsetForStar, offsetForValue, offsetNested, ptr.variable_matched, it->second.first, table2))
                return m;

            for (const auto& [k,tablese] : it->second.second) {
                for (size_t i = 0, N = tablese.datum.size(); i<N; i++) {
                    if (offsetForStar == (size_t) - 1) {
                        if (std::holds_alternative<size_t>(tablese.datum.at(i).at(offsetForValue).val)) {
                            if (currentMatches.contains(std::get<size_t>(tablese.datum.at(i).at(offsetForValue).val))) {
//                                m.fillWithPreviouslyLess(maxSize);
                                m.add(std::get<size_t>(tablese.datum.at(i).at(offsetForValue).val));
//                                return m;
                            }
                        }
                    } else {
                        if (!tablese.datum.at(i).at(offsetForStar).isNested)
                            return m;
                        for (const auto& row : tablese.datum.at(i).at(offsetForStar).table.datum) {
                            if (std::holds_alternative<size_t>(row.at(offsetForStar).val)) {
                                if (currentMatches.contains(std::get<size_t>(row.at(offsetNested).val))) {
//                                    m.fillWithPreviouslyLess(maxSize);
                                    m.add(std::get<size_t>(row.at(offsetNested).val));
//                                    return m;
                                }
                            }
                        }
                    }
                }
            }
            return m;
        }
            break;
        case test_pred::UNMATCHED: {
            OrderedSet m{(size_t)0};
            auto it = this->ptr.at(graph_id).find(ptr.pattern_matched);

            std::set<size_t> currentMatches, otherMatches;
            {
                size_t offsetForStar1 = -1;
                size_t offsetForValue1 = -1;
                size_t offsetNested1 = -1;
                if (!var_extractor(ptr, offsetForStar1, offsetForValue1, offsetNested1, ptr.nsoe, schema, table)) {
//                    m.fillWithPreviouslyLess(maxSize);
                    return getFullOrEmptySet(true);
                }
                if (offsetNested1 == (size_t)(-1)) {
                    const auto&x =
                            table.datum.at(record_id).at(offsetForValue1).val;
                    if (std::holds_alternative<size_t>(x))
                        currentMatches.insert(
                                std::get<size_t>(
                                        table.datum.at(record_id).at(offsetForValue1).val));
                } else {
                    if (!table.datum.at(record_id).at(offsetForStar1).isNested)
                        return m;
                    for (const auto& row :
                            table.datum.at(record_id).at(offsetForStar1).table.datum) {
                        if (std::holds_alternative<size_t>(row.at(offsetNested1).val))
                            currentMatches.insert(
                                    std::get<size_t>(row.at(offsetNested1).val));
                    }
                }
            }

            if (it == this->ptr.at(graph_id).end()) {
                for (unsigned long currentMatch : currentMatches)
                    m.add(currentMatch);
                return m;
            }

            if (currentMatches.empty()) {
//                m.fillWithPreviouslyLess(maxSize);
                return getFullOrEmptySet(true);
            }

            size_t offsetForStar = -1;
            size_t offsetForValue = -1;
            size_t offsetNested = -1;
            const std::string& _for_match = ptr.variable_matched;
            if (!var_extractor(ptr, offsetForStar, offsetForValue, offsetNested, ptr.variable_matched, it->second.first, it->second.second.begin()->second))
            {
//                m.fillWithPreviouslyLess(maxSize);
                return getFullOrEmptySet(true);
            }

            for (const auto& [k,tablese] : it->second.second) {
                for (size_t i = 0, N = tablese.datum.size(); i<N; i++) {
                    if (offsetForStar == (size_t) - 1) {
                        if (std::holds_alternative<size_t>(tablese.datum.at(i).at(offsetForValue).val)) {
                            otherMatches.insert(std::get<size_t>(tablese.datum.at(i).at(offsetForValue).val));
                        }
                    } else {
                        if (!tablese.datum.at(i).at(offsetForStar).isNested)
                            return m;
                        for (const auto& row : tablese.datum.at(i).at(offsetForStar).table.datum) {
                            if (std::holds_alternative<size_t>(row.at(offsetNested).val)) {
                                otherMatches.insert(std::get<size_t>(row.at(offsetNested).val));
                            }
                        }
                    }
                }
            }
            std::vector<size_t> mat;
            std::set_difference(currentMatches.begin(), currentMatches.end(), otherMatches.begin(), otherMatches.end(), std::back_inserter(mat));
            for (unsigned long ma : mat)
                m.add(ma);
            return m;
        }
            break;
    }
}

OrderedSet closure::Interpret::getFullOrEmptySet(bool result) {
    if (!result) {
        return {0};
    } else {
        OrderedSet n{(size_t)0};
        for (auto rec : table.datum.at(record_id)) {
            if (std::holds_alternative<size_t>(rec.val))
                n.add(std::get<size_t>(rec.val));
            else if (rec.isNested)
                for (auto nestedRec : rec.table.datum) {
                    for (auto nestedCell : nestedRec) {
                        if (std::holds_alternative<size_t>(nestedCell.val))
                            n.add(std::get<size_t>(nestedCell.val));
                    }
                }
        }
        return n;
    }
}

std::tuple<std::vector<size_t>,bool,bool> closure::Interpret::resolveIDs(const NestedResultTable &x,
//                             OrderedSet& containment,
                             NestedResultTable::variant_type_cpp script_cast) const {
    std::tuple<std::vector<size_t>,bool,bool> result{{}, false, false}; // ii)if the result would have contained strings instead
                                                                        // iii)actually empty
    switch (x.t) {
        case NestedResultTable::R_NONE: {
            return result;
        }

        case NestedResultTable::R_DO_EDGE_DST:
        case NestedResultTable::R_DO_EDGE_SRC:
        case NestedResultTable::R_DO_CONTENT:
        case NestedResultTable::R_DO_PROP:
        case NestedResultTable::R_DO_ELL:
        case NestedResultTable::R_DO_XI:
        case NestedResultTable::R_DO_EDGE_LABEL:
        case NestedResultTable::R_NODE:
        case NestedResultTable::R_EDGE:
        case NestedResultTable::R_EDGE_SRC:
        case NestedResultTable::R_EDGE_DST: {
            size_t resolution = std::get<size_t>(x.content);
            if (resolution!=(size_t)-1) {
                std::get<0>(result).emplace_back(resolution);
                std::get<2>(result) = true;
            }
            return result;
        }

        case NestedResultTable::R_CONTENT:
        case NestedResultTable::R_XI:
        case NestedResultTable::R_ELL:
        case NestedResultTable::R_PROP:
        case NestedResultTable::R_LABEL:
        case NestedResultTable::R_EDGE_LABEL: {
            std::get<2>(result) = std::get<1>(result) = true;
            return result;
        }

        case NestedResultTable::R_NESTED_LABEL:
        case NestedResultTable::R_NESTED_EDGE_LABEL: {
            std::get<1>(result) = true;
            if (!std::get<std::vector<std::string>>(x.content).empty()) {
                std::get<2>(result) = true;
            }
            return result;
        }

        case NestedResultTable::R_NESTED_CONTENT: {
            auto& ref = std::get<0>(result);
            for (const auto& elementV : std::get<std::vector<std::vector<gsm_object_xi_content>>>(x.content)) {
                for (const auto& element : elementV) {
                    ref.emplace_back(element.id);
                }
            }
            ref.erase(std::remove(ref.begin(), ref.end(), -1), ref.end());
            std::get<2>(result) = !ref.empty();
            return result;
        }

        case NestedResultTable::R_DO_NESTED_EDGE_DST:
        case NestedResultTable::R_DO_NESTED_EDGE_SRC:
        case NestedResultTable::R_DO_NESTED_CONTENT:
        case NestedResultTable::R_DO_NESTED_PROP:
        case NestedResultTable::R_DO_NESTED_ELL:
        case NestedResultTable::R_DO_NESTED_XI:
        case NestedResultTable::R_DO_NESTED_EDGE_LABEL:
        case NestedResultTable::R_NESTED_EDGE_SRC:
        case NestedResultTable::R_NESTED_EDGE_DST:
        case NestedResultTable::R_NESTED_NODE:
        case NestedResultTable::R_NESTED_EDGE:
        case NestedResultTable::R_NESTED_PROP:
        case NestedResultTable::R_NESTED_XI:
        case NestedResultTable::R_NESTED_ELL:  {
            auto& ref = std::get<0>(result);
            ref = std::get<std::vector<size_t>>(x.content);
            ref.erase(std::remove(ref.begin(), ref.end(), -1), ref.end());
            std::get<2>(result) = !ref.empty();
            return result;
        }

        case NestedResultTable::R_SCRIPT: {
            switch (script_cast) {
                case NestedResultTable::RT_STRING: {
                    std::get<1>(result) = std::get<2>(result) = true;
                    return result;
                }

                case NestedResultTable::RT_VSTRING: {
                    std::vector<std::string> W;
                    auto tmp = std::get<std::shared_ptr<script::structures::ScriptAST>>(x.content)->run();
                    if (tmp->type == script::structures::String) {
                        std::get<1>(result) = std::get<2>(result) = true;
                    } else
                    if ((tmp->type == script::structures::Tuple) || (tmp->type == script::structures::Array)) {
                        std::get<1>(result) = true;
                        std::get<2>(result) = !tmp->toList().empty();
                    } else
                        throw std::runtime_error("ERROR: expected a vector!");
                    return result;
                }

                case NestedResultTable::RT_SIZET: {
                    auto resolution = (size_t)std::get<std::shared_ptr<script::structures::ScriptAST>>(x.content)->toInteger(true);
                    if (resolution!=(size_t)-1) {
                        std::get<0>(result).emplace_back(resolution);
                        std::get<2>(result) = true;
                    }
                    return result;
                }

                case NestedResultTable::RT_VSIZET:{
                    auto tmp = std::get<std::shared_ptr<script::structures::ScriptAST>>(x.content)->run();
                    if ((tmp->type == script::structures::Tuple) || (tmp->type == script::structures::Array)) {
                        std::vector<size_t> W;
                        for (const auto& y : tmp->toList()) {
                            W.emplace_back(y->toInteger());
                        }
                    } else
                        throw std::runtime_error("ERROR: expected a vector!");
                }

                case NestedResultTable::RT_CONTENT: {
                    auto tmp = std::get<std::shared_ptr<script::structures::ScriptAST>>(x.content)->run();
                    if (tmp->type == script::structures::Array) {
                        if (tmp->arrayList.empty())
                            return result;
                        else {
                            auto& ref = std::get<0>(result);
                            for (const auto& y : tmp->arrayList) {
                                auto r = y->run();
                                ref.emplace_back(r->tuple["id"]->toInteger(true));
                            }
                            ref.erase(std::remove(ref.begin(), ref.end(), -1), ref.end());
                            std::get<2>(result) = !ref.empty();
                            return result;
                        }
                    }else
                        throw std::runtime_error("ERROR: expected a vector!");
                }

                case NestedResultTable::RT_VCONTENT:{
                    auto tmp = std::get<std::shared_ptr<script::structures::ScriptAST>>(x.content)->run();
                    if (tmp->type == script::structures::Array) {
                        if (tmp->arrayList.empty())
                            return result;
                        else {
                            auto& ref = std::get<0>(result);
                            for (const auto& y : tmp->arrayList) {
                                auto r = y->run();
                                if (r ->type != script::structures::Array)
                                    throw std::runtime_error("ERROR: expected a vector of vectors!");
                                for (const auto& z : y->arrayList) {
                                    auto s = z->run();
                                    ref.emplace_back(r->tuple["id"]->toInteger(true));
                                }
                            }
                            ref.erase(std::remove(ref.begin(), ref.end(), -1), ref.end());
                            std::get<2>(result) = !ref.empty();
                            return result;
                        }
                    }else
                        throw std::runtime_error("ERROR: expected a vector!");
                }

                case NestedResultTable::RT_NONE:
                case NestedResultTable::RT_SCRIPT:
                    throw std::runtime_error("ERROR: you need to provide a non-script other argument, so that the script can be correctly casted!");
            }
        }
            break;
            break;
    }
}

NestedResultTable
closure::Interpret::resolve(const NestedResultTable &x,
                            OrderedSet& containment,
                            NestedResultTable::variant_type_cpp script_cast) const {
    switch (x.t) {
        case NestedResultTable::R_NONE: {
            containment.clearWithMaxCardinality(0);
            return x;
        }

        // Cardinality 1
        case NestedResultTable::R_NODE:
        case NestedResultTable::R_EDGE:
        case NestedResultTable::R_EDGE_SRC:
        case NestedResultTable::R_EDGE_DST: {
            containment.clearWithMaxCardinality(1);
            if (std::get<size_t>(x.content)!=(size_t)-1)
                containment.add(0);
            return x;
        }

        case NestedResultTable::R_CONTENT:
        case NestedResultTable::R_XI:
        case NestedResultTable::R_ELL:
        case NestedResultTable::R_PROP:
        case NestedResultTable::R_LABEL:
        case NestedResultTable::R_EDGE_LABEL: {
            containment.clearWithMaxCardinality(1);
            containment.add(0);
            return x;
        }

        case NestedResultTable::R_NESTED_LABEL:
        case NestedResultTable::R_NESTED_EDGE_LABEL: {
            containment.fillWithPreviouslyLess(std::get<std::vector<std::string>>(x.content).size());
            return x;
        }
        case NestedResultTable::R_NESTED_CONTENT: {
            containment.fillWithPreviouslyLess(std::get<std::vector<std::vector<gsm_object_xi_content>>>(x.content).size());
            return x;
        }

        case NestedResultTable::R_NESTED_EDGE_SRC:
        case NestedResultTable::R_NESTED_EDGE_DST:
        case NestedResultTable::R_NESTED_NODE:
        case NestedResultTable::R_NESTED_EDGE:
        case NestedResultTable::R_NESTED_PROP:
        case NestedResultTable::R_NESTED_XI:
        case NestedResultTable::R_NESTED_ELL:  {
            const auto& W = std::get<std::vector<size_t>>(x.content);
            containment.clearWithMaxCardinality(W.size());
            for(size_t i = 0, N = W.size(); i<N; i++) {
                if (W.at(i) != (size_t)-1)
                    containment.add(i);
            }
            return x;
        }

        case NestedResultTable::R_DO_EDGE_LABEL:{
            containment.clearWithMaxCardinality(1);
            auto ptr = ptr2->findLabelFromID(std::get<size_t>(x.content));
            if (ptr != ptr2->notALabel()) {
                containment.add(0);
                return {ptr->second, x.cell_nested_morphism, x.column};
            } else
                return {};
        }

        case NestedResultTable::R_DO_NESTED_EDGE_LABEL: {
            std::vector<std::string> v;
            containment.clearWithMaxCardinality(x.size());
            v.reserve(x.size());
            const auto& W = std::get<std::vector<size_t>>(x.content);
            for (size_t idx = 0, N = W.size(); idx<N; idx++) {
                auto ptr = ptr2->findLabelFromID(W.at(idx));
                if (ptr != ptr2->notALabel()) {
                    containment.add(idx);
                    v.emplace_back(ptr->second);
                } else
                    v.emplace_back("");
            }
            return {std::move(v), x.cell_nested_morphism, x.column};
        }

        case NestedResultTable::R_DO_EDGE_SRC: {
            containment.clearWithMaxCardinality(1);
            const gsm2::tables::PhiTable::record* ptr = ptr2->resolveRecord(std::get<size_t>(x.content));
            if (ptr) {
                containment.add(0);
                return {ptr->object_id,  NestedResultTable::R_EDGE_SRC, x.cell_nested_morphism, x.column};
            } else
                return {};
        }

        case NestedResultTable::R_DO_NESTED_EDGE_SRC: {
            std::vector<size_t> v;
            v.reserve(x.size());
            containment.clearWithMaxCardinality(x.size());
            const auto& W =std::get<std::vector<size_t>>(x.content);
            for (size_t idx = 0, N = W.size(); idx<N; idx++) {
                const gsm2::tables::PhiTable::record* ptr = ptr2->resolveRecord(W.at(idx));
                if (ptr) {
                    containment.add(idx);
                    v.emplace_back(ptr->object_id);
                } else
                    v.emplace_back(-1);
            }
            return {std::move(v),  NestedResultTable::R_EDGE_SRC, x.cell_nested_morphism, x.column};
        }

        case NestedResultTable::R_DO_EDGE_DST: {
            containment.clearWithMaxCardinality(1);
            const gsm2::tables::PhiTable::record* ptr = ptr2->resolveRecord(std::get<size_t>(x.content));
            if (ptr) {
                containment.add(0);
                return {ptr->id_contained,  NestedResultTable::R_EDGE_SRC, x.cell_nested_morphism, x.column};
            } else
                return {};
        }

        case NestedResultTable::R_DO_NESTED_EDGE_DST: {
            std::vector<size_t> v;
            v.reserve(x.size());
            containment.clearWithMaxCardinality(x.size());
            const auto& W =std::get<std::vector<size_t>>(x.content);
            for (size_t idx = 0, N = W.size(); idx<N; idx++) {
                const gsm2::tables::PhiTable::record* ptr = ptr2->resolveRecord(W.at(idx));
                if (ptr) {
                    containment.add(idx);
                    v.emplace_back(ptr->id_contained);
                } else
                    v.emplace_back(-1);
            }
            return {std::move(v), NestedResultTable::R_EDGE_SRC, x.cell_nested_morphism, x.column};
        }

        case NestedResultTable::R_DO_XI: {
            containment.clearWithMaxCardinality(1);
            const auto& resolution = clos.resolve_xi(graph_id, std::get<size_t>(x.content));
            if (resolution.size() > x.opt_offset) {
                containment.add(0);
                return {resolution.at(x.opt_offset), x.cell_nested_morphism, x.column};
            } else {
                return {};
            }
        }

        case NestedResultTable::R_DO_NESTED_XI: {
            std::vector<std::string> v;
            v.reserve(x.size());
            containment.clearWithMaxCardinality(x.size());
            const auto& W = std::get<std::vector<size_t>>(x.content);
            for (size_t idx = 0, N = W.size(); idx<N; idx++) {
                const auto& resolution = clos.resolve_xi(graph_id, W.at(idx));
                if (resolution.size() > x.opt_offset) {
                    containment.add(idx);
                    v.emplace_back(resolution.at(x.opt_offset));
                } else {
                    v.emplace_back("");
                }
            }
            return {std::move(v), x.cell_nested_morphism, x.column};
        }

        case NestedResultTable::R_DO_ELL: {
            containment.clearWithMaxCardinality(1);
            const auto& resolution = clos.resolve_ell(graph_id, std::get<size_t>(x.content));
            if (resolution.size() > x.opt_offset) {
                containment.add(0);
                return {resolution.at(x.opt_offset), x.cell_nested_morphism, x.column};
            } else {
                return {};
            }
        }

        case NestedResultTable::R_DO_NESTED_ELL: {
            std::vector<std::string> v;
            v.reserve(x.size());
            containment.clearWithMaxCardinality(x.size());
            const auto& W = std::get<std::vector<size_t>>(x.content);
            for (size_t idx = 0, N = W.size(); idx<N; idx++) {
                const auto& resolution = clos.resolve_ell(graph_id, W.at(idx));
                if (resolution.size() > x.opt_offset) {
                    containment.add(idx);
                    v.emplace_back(resolution.at(x.opt_offset));
                } else {
                    v.emplace_back("");
                }
            }
            return {std::move(v), x.cell_nested_morphism, x.column};
        }

        case NestedResultTable::R_DO_PROP: {
            if ((x.fields->size() == 1) || (x.size() == 1)) {
                containment.fillWithPreviouslyLess(1);
                return {clos.resolve_prop(graph_id, std::get<size_t>(x.content), std::get<std::string>(x.fields->content)), x.cell_nested_morphism, x.column};
            } else
                throw std::runtime_error(std::string("ERROR: the fields are not 1, "+std::to_string(x.fields->size())+", while the objects are also not 1, "+std::to_string(x.size())));
        }
            break;

        case NestedResultTable::R_DO_NESTED_PROP: {
            size_t N = x.fields->size();
            std::vector<std::string> v;
            if (N == 1) {
                const std::string& prop = std::get<std::string>(x.fields->content);
                v.reserve(x.size());
                for (size_t y : std::get<std::vector<size_t>>(x.content)) {
                    v.emplace_back(clos.resolve_prop(graph_id, y, prop));
                }
            } else if (x.size() == N) {
                const std::vector<std::string>& prop = std::get<std::vector<std::string>>(x.fields->content);
                const auto& V = std::get<std::vector<size_t>>(x.content);
                v.reserve(N);
                for (size_t i = 0; i<N; i++) {
                    v.emplace_back(clos.resolve_prop(graph_id, V.at(i), prop.at(i)));
                }
            }
            if ((x.fields->size() == 1) || (x.size() == x.fields->size())) {
                v.emplace_back(clos.resolve_prop(graph_id, std::get<size_t>(x.content), std::get<std::string>(x.fields->content)));
            } else
                throw std::runtime_error(std::string("ERROR: the fields are "+std::to_string(x.fields->size())+" while the objects are "+std::to_string(x.size())));
            containment.fillWithPreviouslyLess(v.size());
            return {std::move(v), x.cell_nested_morphism, x.column};
        } break;

        case NestedResultTable::R_DO_CONTENT: {
            if ((x.fields->size() == 1) || (x.size() == 1)) {
                containment.fillWithPreviouslyLess(1);
                return {clos.resolve_content(graph_id, std::get<size_t>(x.content), std::get<std::string>(x.fields->content)), x.cell_nested_morphism, x.column};
            } else
                throw std::runtime_error(std::string("ERROR: the fields are not 1, "+std::to_string(x.fields->size())+", while the objects are also not 1, "+std::to_string(x.size())));
        } break;

        case NestedResultTable::R_DO_NESTED_CONTENT: {
            size_t N = x.fields->size();
            std::vector<std::vector<gsm_object_xi_content>> v;
            if (N == 1) {
                const std::string& prop = std::get<std::string>(x.fields->content);
                v.reserve(x.size());
                for (size_t y : std::get<std::vector<size_t>>(x.content)) {
                    v.emplace_back(clos.resolve_content(graph_id, y, prop));
                }
            } else if (x.size() == N) {
                const std::vector<std::string>& prop = std::get<std::vector<std::string>>(x.fields->content);
                const auto& V = std::get<std::vector<size_t>>(x.content);
                v.reserve(N);
                for (size_t i = 0; i<N; i++) {
                    v.emplace_back(clos.resolve_content(graph_id, V.at(i), prop.at(i)));
                }
            }
            if ((x.fields->size() == 1) || (x.size() == x.fields->size())) {
                v.emplace_back(clos.resolve_content(graph_id, std::get<size_t>(x.content), std::get<std::string>(x.fields->content)));
            } else
                throw std::runtime_error(std::string("ERROR: the fields are "+std::to_string(x.fields->size())+" while the objects are "+std::to_string(x.size())));
            containment.fillWithPreviouslyLess(v.size());
            return {std::move(v), x.cell_nested_morphism, x.column};
        } break;

        case NestedResultTable::R_SCRIPT: {
            switch (script_cast) {
                case NestedResultTable::RT_STRING: {
                    containment.fillWithPreviouslyLess(1);
                    return {std::get<std::shared_ptr<script::structures::ScriptAST>>(x.content)->run()->toString(false), x.cell_nested_morphism, x.column};
                }

                case NestedResultTable::RT_VSTRING: {
                    std::vector<std::string> W;
                    auto tmp = std::get<std::shared_ptr<script::structures::ScriptAST>>(x.content)->run();
                    if (tmp->type == script::structures::String) {
                        W.emplace_back(tmp->string);
                    } else
                    if ((tmp->type == script::structures::Tuple) || (tmp->type == script::structures::Array)) {
                        for (const auto& y : tmp->toList()) {
                            W.emplace_back(y->toString(false));
                        }
                    } else
                        throw std::runtime_error("ERROR: expected a vector!");
                    containment.fillWithPreviouslyLess(W.size());
                    return {std::move(W), x.cell_nested_morphism, x.column};
                }

                case NestedResultTable::RT_SIZET:
                    containment.fillWithPreviouslyLess(1);
                    return {(size_t)std::get<std::shared_ptr<script::structures::ScriptAST>>(x.content)->toInteger(true), false, x.cell_nested_morphism, x.column};

                case NestedResultTable::RT_VSIZET:{
                    auto tmp = std::get<std::shared_ptr<script::structures::ScriptAST>>(x.content)->run();
                    if ((tmp->type == script::structures::Tuple) || (tmp->type == script::structures::Array)) {
                        std::vector<size_t> W;
                        for (const auto& y : tmp->toList()) {
                            W.emplace_back(y->toInteger());
                        }
                        containment.fillWithPreviouslyLess(W.size());
                        return {std::move(W), true, x.cell_nested_morphism, x.column};
                    } else
                        throw std::runtime_error("ERROR: expected a vector!");
                }

                case NestedResultTable::RT_CONTENT: {
                    auto tmp = std::get<std::shared_ptr<script::structures::ScriptAST>>(x.content)->run();
                    if (tmp->type == script::structures::Array) {
                        if (tmp->arrayList.empty())
                            return {std::vector<gsm_object_xi_content>{}, x.cell_nested_morphism, x.column};
                        else {
                            std::vector<gsm_object_xi_content> W;
                            for (const auto& y : tmp->arrayList) {
                                auto r = y->run();
                                W.emplace_back(r->tuple["id"]->toInteger(true), r->tuple["score"]->toDouble(true));
                            }
                            containment.fillWithPreviouslyLess(W.size());
                            return {std::move(W), x.cell_nested_morphism, x.column};
                        }
                    }else
                        throw std::runtime_error("ERROR: expected a vector!");
                }

                case NestedResultTable::RT_VCONTENT:{
                    auto tmp = std::get<std::shared_ptr<script::structures::ScriptAST>>(x.content)->run();
                    if (tmp->type == script::structures::Array) {
                        if (tmp->arrayList.empty())
                            return {std::vector<std::vector<gsm_object_xi_content>>{}, x.cell_nested_morphism, x.column};
                        else {
                            std::vector<std::vector<gsm_object_xi_content>> W;
                            for (const auto& y : tmp->arrayList) {
                                auto r = y->run();
                                if (r ->type != script::structures::Array)
                                    throw std::runtime_error("ERROR: expected a vector of vectors!");
                                auto& U = W.emplace_back();
                                for (const auto& z : y->arrayList) {
                                    auto s = z->run();
                                    U.emplace_back(r->tuple["id"]->toInteger(true), r->tuple["score"]->toDouble(true));
                                }
                                W.emplace_back(U);
                            }
                            containment.fillWithPreviouslyLess(W.size());
                            return {std::move(W), x.cell_nested_morphism, x.column};
                        }
                    }else
                        throw std::runtime_error("ERROR: expected a vector!");
                }

                default:
                    throw std::runtime_error("ERROR: you need to provide a non-script other argument, so that the script can be correctly casted!");
            }
        }
            break;
            break;
    }
}
