/*
 * LinearGSM.h
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

#ifndef GSM2_LINEARGSM_H
#define GSM2_LINEARGSM_H

#include <database/SimplifiedFuzzyStringMatching.h>
#include <database/PhiTable.h>
#include <database/AttributeTable.h>
#include <database/ActivityTable.h>
#include <filesystem>
#include "gsm_object.h"

#include <ostream>
#include <result.h>
#include <database/gsm_indices.h>
#include <database/gsm_object_xi_content.h>
#include <yaucl/graphs/FlexibleGraph.h>

namespace std {
    template <> struct hash<result> {
        size_t operator()(const result& x ) const {
            return x.graphid ^ x.eventid;
        }
    };
}

#include "parsing.h"

namespace gsm2 {
    namespace tables {

        struct LinearGSM {
            size_t nGraphs = 0;
            std::map<size_t, std::string> minRecordToContaimentLabel;
            const std::string noLabelStr;
            yaucl::structures::any_to_uint_bimap<std::string> label_map;
            std::unordered_map<std::string, PhiTable> containment_tables;
            std::unordered_set<std::string> containment_relationships;
            std::unordered_map<std::string, AttributeTable> KeyValueProperties;
            FuzzyMatchSerializer ell_values, xi_values;
            ActivityTable                main_registry;
            std::vector<gsm_db_indices> all_indices; // index associated to each graph
            std::vector<gsm_object_xi_content> empty_containment;
            std::vector<size_t> nodesInGraph;
            std::vector<std::vector<std::vector<double>>> objectScores;
            std::unordered_map<std::pair<size_t,size_t>, std::vector<double>> objectScoresLoading;
            bool doInitLoading{true};
            size_t noLabel{0};

            void ells_and_xis_just_as_repositories(){
                ell_values.deactivateFuzzyIndex();
                xi_values.deactivateFuzzyIndex();
            }

            const std::map<size_t, std::string>::const_iterator findLabelFromID(size_t k) const {
                DEBUG_ASSERT(minRecordToContaimentLabel.begin()->first == 0);
                auto it = minRecordToContaimentLabel.lower_bound(k);
                if ((it == minRecordToContaimentLabel.end())) {
                    if (!minRecordToContaimentLabel.empty()) {
                        auto it2 = minRecordToContaimentLabel.end();
                        return std::prev(it2);
                    }else {
                        return it;
                    }
                }
                if ((it->first>k))
                    it--;
                return it;
            }
            const std::map<size_t, std::string>::const_iterator notALabel() const {
                return minRecordToContaimentLabel.end();
            }
            const gsm2::tables::PhiTable::record* resolveRecord(size_t k) const {
                DEBUG_ASSERT(minRecordToContaimentLabel.begin()->first == 0);
                auto it = minRecordToContaimentLabel.lower_bound(k);
                if ((it == minRecordToContaimentLabel.end())) {
                    if (!minRecordToContaimentLabel.empty()) {
                        auto it2 = minRecordToContaimentLabel.end();
                        it2 = std::prev(it2);
                        return &containment_tables.at(it->second).table.at(k-it2->first);
                    }else {
                        return nullptr;
                    }
                }
                if (it->first>k) it--;
                return &containment_tables.at(it->second).table.at(k-it->first);
            }

            std::optional<gsm_object> resolve_object(size_t graph_id, size_t object_id) const {
                if (nodesInGraph.size() <= graph_id)
                    return {};
                if (nodesInGraph[graph_id] <= object_id)
                    return {};
                gsm_object result;
                result.id = object_id;
                std::pair<size_t, size_t> cp{graph_id, object_id};
                result.xi = xi_values.resolve_object_id(cp);
                result.ell = ell_values.resolve_object_id(cp);
                for (const auto& [keyAttribute, Table] : KeyValueProperties) {
                    auto it3= Table.secondary_index2.find(cp);
                    if (it3 !=Table.secondary_index2.end() ) {
                        result.content[keyAttribute] = resolveUnionMinimal(Table, Table.table.at(it3->second));
                    }
                }
                for (const auto& [edgeLabel, outEdges] : containment_tables) {
                    auto it3 = outEdges.secondary_index.find(graph_id);
                    if (it3 == outEdges.secondary_index.end())
                        continue;
                    auto it2 = it3->second.find(object_id);
                    if (it2 == it3->second.end())
                        continue;
                    else for (const auto& record : it2->second) {
                            result.phi[edgeLabel].emplace_back(record->id_contained, record->w_contained);
                        }
                }
                return {result};
            }

            void asObjects(std::vector<std::vector<gsm_object>> &simpleGraphs) const {
                simpleGraphs.clear();
                simpleGraphs.resize(all_indices.size());
                for(size_t i = 0, N = all_indices.size(); i<N; i++) {
                    simpleGraphs[i].resize(nodesInGraph.at(i));
                }
                std::pair<size_t, size_t> cp;
                std::vector<yaucl::structures::any_to_uint_bimap<size_t>> nodesBeingInsertedAlready(all_indices.size());
                size_t offsetMainRegistryTable = 0;
                for (const auto& record : main_registry.table) {
                    cp.first = record.graph_id;
                    cp.second = record.event_id;
                    auto& nodeMap = nodesBeingInsertedAlready[record.graph_id];
                    auto& g = simpleGraphs[record.graph_id];
                    size_t id = nodeMap.put(record.event_id).first;
                    gsm_object& result = simpleGraphs[record.graph_id][record.event_id];
                    result.id = record.event_id;
                    result.xi = xi_values.resolve_object_id(cp);
                    result.ell = ell_values.resolve_object_id(cp);
                    for (const auto& [keyAttribute, Table] : KeyValueProperties) {
                        auto tmp2 = Table.resolve_record_if_exists2(offsetMainRegistryTable);
                        if (tmp2) {
                            result.content[keyAttribute] = tmp2.value();
//                            tmp = tmp+"|"+keyAttribute+"="+(std::holds_alternative <std::string>(tmp2.value()) ? std::get<std::string>(tmp2.value()) : std::to_string(std::get<double>(tmp2.value())));
                        }
                    }
                    offsetMainRegistryTable++;
                }
                for (const auto& [edgeLabel, outEdges] : containment_tables) {
                    for (const auto& record : outEdges.table) {
                        const auto& map = nodesBeingInsertedAlready.at(record.graph_id);
                        gsm_object& result = simpleGraphs[record.graph_id][record.object_id];
                        result.phi[edgeLabel].emplace_back(record.id_contained);
                    }
                }
            }


            void loadGSMObject(size_t graphId,
                               std::unordered_map<std::string, gsm2::tables::AttributeTableType>& propertyname_to_type,
                               const struct gsm_object& object);

            /**
             * Returning the conent of a specific graph and object id associated to the key
             *
             * @param graph
             * @param object
             * @param key_content
             * @return
             */
            std::vector<gsm_object_xi_content> resolveContent(size_t graphid,
                                                                size_t object,
                                                                const std::string& key_content) const;


            /**
             * Returns the value associated to the key <key_prop> associated to the object in a specific graph
             *
             * @param graph
             * @param object
             * @param key_prop
             * @return
             */
            std::optional<union_minimal> resolveProperties(size_t graph, size_t object, const std::string& key_prop) const;

            void clear();
            void index();

            /**
             *
             * @param path The folder where to serialize the content.
             */
            void secondary_memory_index(const std::filesystem::path& path);


            inline ssize_t getMappedValueFromAction(const std::string &act) const {
                try{
                    return label_map.get(act);
                }
                catch(const std::exception& e){
                    return -1;
                }
            }

            inline const std::vector<std::string>& objectRelationships() const {
                return label_map.int_to_T;
            }

            inline const std::unordered_set<std::string>& containmentRelationships() const {
                return containment_relationships;
            }

            /**
             * Iterating over all the objects in the database
             *
             * @param f Iterator lambda function, accepting the graph id and the information associated to a single
             *          object represented as a legacy gsm object
             */
            void iterateOverObjects(const std::function<void(size_t, const gsm_object&)>& f);

            /**
             * Collecting the data as graph data structures, so to better visualise the data that was loaded
             *
             * @param simpleGraphs  Resulting collection of graphs
             */
            void asGraphs(std::vector<FlexibleGraph<std::string,std::string>>& simpleGraphs) const;

            /**
             * Returns all the records associated to a given first label. Records not being associated to an activity
             * label are associated to an empty one
             *
             * @param act       Activity Label
             * @param removed   Whether the matched elements should be removed in the final result or not
             * @return      Events with the given activity label and score
             */
            std::vector<result> timed_dataless_exists(const std::string &act) const;

            /**
             * Returns the approximate match over the strings for the given value
             * @param min_threshold     Minimum match threshold
             * @param nmax              Maximum number of scores
             * @param xi                String to match
             * @param removed   Whether the matched elements should be removed in the final result or not
             * @return  Objects in graph matching this
             */
            std::vector<result> approx_value_query(double min_threshold, size_t nmax, const std::string& xi) const;

            /**
             * Returns the approximate match over the strings for the given label value
             * @param min_threshold     Minimum match threshold
             * @param nmax              Maximum number of scores
             * @param xi                String to match
             * @param removed   Whether the matched elements should be removed in the final result or not
             * @return  Objects in graph matching this
             */
            std::vector<result> approx_label_query(double min_threshold, size_t nmax, const std::string& xi) const;


            /**
             * Returns all the objects being contained (container_containementOth=false) or containing (container_containementOth=true)
             * objects, where the former is associated to a container_object_label and contains the latter
             *
             * @param container_object_label        Activity labels associated to the container object
             * @param phi_label                     Containment label associated to the containment function
             * @param container_containementOth     Whether to return the object containing or the objects being contained
             * @param removed   Whether the matched elements should be removed in the final result or not
             * @return
             */
            std::vector<result> query_container_or_containment(const std::string& container_object_label,
                                                               const std::string& phi_label,
                                                               std::function<double(const std::vector<double>&)> aggregate_scores,
                                                               bool container_containementOth = false) const;

            /**
             * Returning the labels associated to the object
             * @param object    Graph and object association
             * @return  Labels associated to the object
             */
            inline const std::vector<std::string>& ell(const result& object) const {
                return ell_values.resolve_object_id({object.graphid, object.eventid});
            }

            inline const std::vector<std::string>& ell_(size_t graphid, size_t eventid) const {
                return ell_values.resolve_object_id({graphid, eventid});
            }

            inline const std::vector<std::string>& ell(size_t graphid, size_t eventid) const {
                return ell_values.resolve_object_id({graphid, eventid});
            }

            inline std::vector<std::string> resolveContainmentLabels(size_t graphid, size_t id) const {
                std::vector<std::string> result;
                for (const auto& [key, table] : containment_tables) {
                    auto it = table.secondary_index.find(graphid);
                    if (table.secondary_index.find(graphid)!= table.secondary_index.end()) {
                        if (it->second.find(id) != it->second.end()) {
                            result.emplace_back(key);
                        }
                    }
                }
                return result;
            }

            inline std::vector<std::string> resolvePropertyLabels(size_t graphid, size_t id ) const {
                std::pair<size_t,size_t>cp{graphid, id};
                std::vector<std::string> result;
                for (const auto& [key, table] : KeyValueProperties) {
                    if (table.secondary_index2.find(cp)!= table.secondary_index2.end())
                        result.emplace_back(key);
                }
                return result;
            }

           inline bool hasContent(size_t graphid, size_t id, const std::string &key_content) const {
                auto it = (containment_tables.find(key_content));
                if (it == containment_tables.end())
                    return false;
                std::vector<gsm_object_xi_content> result;
                auto it3 = it->second.secondary_index.find(graphid);
                if (it3 == it->second.secondary_index.end())
                    return false;
                else return it3->second.find(id) != it3->second.end();
            }

            /**
             * Returning the values associated to the object
             * @param object    Graph and object association
             * @return  Labels associated to the object
             */
            inline const std::vector<std::string>& xi(const result& object) const {
                return xi_values.resolve_object_id({object.graphid, object.eventid});
            }

            inline const std::vector<std::string>& xi_(size_t graphid, size_t eventid) const {
                return xi_values.resolve_object_id({graphid, eventid});
            }

            inline const std::vector<std::string>& xi(size_t graphid, size_t eventid) const {
                return xi_values.resolve_object_id({graphid, eventid});
            }

            std::pair<double,double> load_databases(std::vector<std::vector<gsm_object>> &db,
                                                                       const std::unordered_map<std::string, gsm2::tables::AttributeTableType> &schema
            );


            /**
             * This returns a map associating the containers to the contents across multiple graphs
             * @param container_object_label
             * @param phi_label
             * @param aggregate_scores
             * @param removed   Whether the matched elements should be removed in the final result or not
             * @return
             */
            void query_container_or_containment(std::unordered_map<std::pair<size_t, size_t>, std::vector<result>>& result,
                                                                                                            const std::string &container_object_label,
                                                                                                            const std::string &phi_label,
                                                                                                            bool container_containementOth = false) const;


            /**
             * Performing range/propertyValue queries over the nodes
             * @param field_name
             * @param ActNameToPredicates
             * @param Qs
             */
            void exact_range_query(const std::string &field_name,
                                                  const std::unordered_map<std::string, std::vector<size_t>> &ActNameToPredicates,
                                                  std::vector<std::pair<DataQuery, std::vector<result>>> &Qs) const;
        };

        /**
         * Loading the file representing multiple GSM databases into a single database db
         *
         * @param path  File containing the serialised representation of the databases
         * @param db    Object where to load the databases
         * @return      The loading and indexing time
         */
        std::pair<double,double> primary_memory_load_gsm2(const std::filesystem::path& path, gsm2::tables::LinearGSM& db);

        /**
         * Direct loading of the GSM model from a collection of databases, where each database is a collection of objects
         *
         * @param db        Collection of databases
         * @param schema    Schema associated to each field type
         * @param target    Element where
         * @return          The loading and indexing time
         */
        std::pair<double,double> primary_memory_load_direct(std::vector<std::vector<gsm_object>>& db,
                const std::unordered_map<std::string, gsm2::tables::AttributeTableType>& schema,
                gsm2::tables::LinearGSM& target);
    } // gsm2
} // structures




#include "yaucl/data/json.h"

static inline size_t loadObjectEll(gsm2::tables::LinearGSM &db,
                                   std::vector<std::string> &ell,
                                   size_t noLabel,
                                   const std::pair<size_t, size_t> &graphId_eventId) {
    size_t act_id;
    if (!ell.empty()) {
        // Sending to fuzzy ell match
        std::string pop = ell.front();
        ell.erase(ell.begin());
        db.ell_values.addGramsToMap(pop, graphId_eventId, ell);
        act_id = db.label_map.put(pop).first;
    } else {
        act_id = noLabel;
    }
    return act_id;
}

static inline size_t loadObjectEll2(gsm2::tables::LinearGSM &db,
                                    const std::vector<std::string> &ell,
                                    size_t noLabel,
                                    const std::pair<size_t, size_t> &graphId_eventId) {
    size_t act_id;
    if (!ell.empty()) {
        // Sending to fuzzy ell match
        std::string pop = ell.front();
        if (ell.size() > 1) {
            std::vector<std::string> rest{ell.begin()+1, ell.end()};
            db.ell_values.addGramsToMap(pop, graphId_eventId, rest);
        } else {
            std::vector<std::string> rest;
            db.ell_values.addGramsToMap(pop, graphId_eventId, rest);
        }
        act_id = db.label_map.put(pop).first;
    } else {
        act_id = noLabel;
    }
    return act_id;
}

static inline void registerObjectByFirstLabel(gsm2::tables::LinearGSM &db,
                                              size_t act_id,
                                              const std::pair<size_t, size_t> &graphId_eventId) {
    // Setting up the new object
    db.main_registry.load_record(graphId_eventId.first, act_id, graphId_eventId.second);
}

static inline void loadObjectXi(gsm2::tables::LinearGSM &db, std::vector<std::string> &xi,
                                const std::pair<size_t, size_t> &graphId_eventId) {
    if (!xi.empty()) {
        // Sending to fuzzy xi match
        std::string front = xi[0];
        xi.erase(xi.begin());
        db.xi_values.addGramsToMap(front, graphId_eventId, xi);
    }
}

static inline void loadObjectXi2(gsm2::tables::LinearGSM &db, const std::vector<std::string> &xi,
                                 const std::pair<size_t, size_t> &graphId_eventId) {
    if (!xi.empty()) {
        // Sending to fuzzy xi match
        std::string front = xi[0];
        if (xi.size() > 1) {
            std::vector<std::string> rest{xi.begin()+1, xi.end()};
            db.xi_values.addGramsToMap(front, graphId_eventId, rest);
        } else {
            std::vector<std::string> rest;
            db.xi_values.addGramsToMap(front, graphId_eventId, rest);
        }
    }
}

static inline void loadObjectProperty(gsm2::tables::LinearGSM &db,
                                      const std::unordered_map<std::string, gsm2::tables::AttributeTableType> &property_to_type,
                                      size_t act_id,
                                      const std::pair<size_t, size_t> &graphId_eventId,
                                      const std::string &property_key,
                                      const std::string &property_value) {
    bool holdsType = true;
    if (!db.KeyValueProperties.contains(property_key)) {
        holdsType = false;
    }
    auto& table = db.KeyValueProperties[property_key];
    table.attr_name = property_key;
    if (!holdsType) {
        auto it = property_to_type.find(property_key);
        if (it != property_to_type.end())
            table.type = it->second;
        else
            table.type = gsm2::tables::StringAtt;
    }
    switch (table.type) {
        case gsm2::tables::DoubleAtt:
            table.record_load(act_id, std::stod(property_value), graphId_eventId.first, graphId_eventId.second);
            break;
        case gsm2::tables::SizeTAtt:
            table.record_load(act_id, std::stoull(property_value), graphId_eventId.first, graphId_eventId.second);
            break;
        case gsm2::tables::LongAtt:
            table.record_load(act_id, std::stoll(property_value), graphId_eventId.first, graphId_eventId.second);
            break;
        case gsm2::tables::StringAtt:
            table.record_load(act_id, property_value, graphId_eventId.first, graphId_eventId.second);
            break;
        case gsm2::tables::BoolAtt:
            table.record_load(act_id, (property_value == "True" || property_value == "true" || property_value == "tt" || property_value == "t"), graphId_eventId.first, graphId_eventId.second);
            break;
    }
}

static inline void loadObjectProperty(gsm2::tables::LinearGSM &db,
                                      const std::unordered_map<std::string, gsm2::tables::AttributeTableType> &property_to_type,
                                      size_t act_id,
                                      const std::pair<size_t, size_t> &graphId_eventId,
                                      const std::string &property_key,
                                      const union_type &property_value) {
    bool holdsType = true;
    if (!db.KeyValueProperties.contains(property_key)) {
        holdsType = false;
    }
    auto& table = db.KeyValueProperties[property_key];
    if (!holdsType) {
        auto it = property_to_type.find(property_key);
        if (it != property_to_type.end())
            table.type = it->second;
        else if (!holdsType) {
            table.attr_name = property_key;
            if (std::holds_alternative<std::string>(property_value)) {
                table.type = gsm2::tables::StringAtt;
            } else if (std::holds_alternative<double>(property_value)) {
                table.type = gsm2::tables::DoubleAtt;
            }else if (std::holds_alternative<long long>(property_value)) {
                table.type = gsm2::tables::LongAtt;;
            }else if (std::holds_alternative<size_t>(property_value)) {
                table.type = gsm2::tables::SizeTAtt;;
            } else if (std::holds_alternative<bool>(property_value)) {
                table.type = gsm2::tables::BoolAtt;;
            }
        }
    }

//    else
    switch (table.type) {
        case gsm2::tables::DoubleAtt:
            if (std::holds_alternative<std::string>(property_value))
                table.record_load(act_id, std::stod(std::get<std::string>(property_value)), graphId_eventId.first, graphId_eventId.second);
            else if (std::holds_alternative<double>(property_value))
                table.record_load(act_id, (std::get<double>(property_value)), graphId_eventId.first, graphId_eventId.second);
            else
                throw std::runtime_error("Type mismatch: expecting " +property_key+" to have type: double");
            break;
        case gsm2::tables::SizeTAtt:
            if (std::holds_alternative<std::string>(property_value))
                table.record_load(act_id, std::stoull(std::get<std::string>(property_value)), graphId_eventId.first, graphId_eventId.second);
            else if (std::holds_alternative<size_t>(property_value))
                table.record_load(act_id, (std::get<size_t>(property_value)), graphId_eventId.first, graphId_eventId.second);
            else
                throw std::runtime_error("Type mismatch: expecting " +property_key+" to have type: stoull");
            break;
        case gsm2::tables::LongAtt:
            if (std::holds_alternative<std::string>(property_value))
                table.record_load(act_id, std::stoll(std::get<std::string>(property_value)), graphId_eventId.first, graphId_eventId.second);
            else if (std::holds_alternative<long long>(property_value))
                table.record_load(act_id, (std::get<long long>(property_value)), graphId_eventId.first, graphId_eventId.second);
            else
                throw std::runtime_error("Type mismatch: expecting " +property_key+" to have type: stoll");
            break;
        case gsm2::tables::StringAtt:
            table.record_load(act_id, property_value, graphId_eventId.first, graphId_eventId.second);
            break;
        case gsm2::tables::BoolAtt:
            if (std::holds_alternative<std::string>(property_value)) {
                const auto& actual = std::get<std::string>(property_value);
                table.record_load(act_id, (actual == "True" || actual == "true" || actual == "tt" || actual == "t"), graphId_eventId.first, graphId_eventId.second);
            } else if (std::holds_alternative<bool>(property_value)) {
                table.record_load(act_id, std::get<bool>(property_value), graphId_eventId.first, graphId_eventId.second);
            } else {
                throw std::runtime_error("Type mismatch: expecting " +property_key+" to have type: bool");
            }
            break;
    }
}


static inline size_t initLoading(gsm2::tables::LinearGSM &db) {
    if (db.doInitLoading) {
        size_t noLabel = db.noLabel = db.label_map.put("").first;
        size_t act_id = noLabel;
        db.doInitLoading = false;
        return noLabel;
    } else {
        return db.noLabel;
    }
}



#include <fstream>
#include "magic_enum.hpp"

static inline
void convertMap(const NodeLabelBijectionGraph<size_t, std::string>& map,
                std::unordered_map<size_t, std::vector<size_t>>& original) {
    std::unordered_map<size_t, std::vector<size_t>> final;
    for (const auto& [k, ls] : original) {
        auto newKey = k;
        auto& v = final[newKey];
        for (size_t x : ls) {
            v.emplace_back(map.getUniqueLabel(x));
        }
    }
    std::swap(final, original);
}

static inline bool parse(char* string,
                         size_t len,
                         gsm2::tables::LinearGSM& forloading,
                         const std::unordered_map<std::string, gsm2::tables::AttributeTableType>& map_for_types,
                         size_t& maxGraphId) {
    string = skipSpaces(string, &len);
//    size_t graphId = 0;
    std::vector<std::string> ell, xi;
    double weight; size_t content;
    size_t act_id;
    size_t noLabel = initLoading(forloading);
    int scanSkip = 0;
    constexpr char const * ID = "id:";
    constexpr char const  DOT = '.';
    constexpr char const * ELL = "ell:";
    constexpr char const * XI = "xi:";
    constexpr char const * PROP = "properties:";
    constexpr char const * PHI = "phi:";
//    size_t id;
    std::pair<size_t,size_t> graphId_eventId{0,0};
    // Defining a simple format to scan, so to avoid using a parser for this.
    while (string && *string) {
        // Reading a new object from the database for value querying
        graphId_eventId.second = 0;
        if (forloading.nodesInGraph.size() == graphId_eventId.first) {
            forloading.nodesInGraph.emplace_back(graphId_eventId.second);
        }
        ell.clear();
        if (!(string = haspos(string, (char*)ID, 0, &len)))
            return false;
        if (sscanf(string, "%lu%n", &graphId_eventId.second, &scanSkip)==(-1))
            return false;
        forloading.nodesInGraph[graphId_eventId.first] = std::max(forloading.nodesInGraph[graphId_eventId.first],graphId_eventId.second);
        len-=scanSkip;
        string+=scanSkip;
        if (!(string = skipSpaces(string, &len)))
            return false;
        if (!(string = haspos(string,  (char*)ELL, 0, &len)))
            return false;
        act_id = noLabel;

        if (!(string = atreturn(string, &len)))
            return false;
        if (!(string = skipSpaces(string, &len)))
            return false;
        while (*string != DOT) {
            char* beforeReturn = atreturn(string, &len);
            ell.emplace_back(string, beforeReturn-1);
            string = beforeReturn;
            if (!(string = skipSpaces(string, &len)))
                return false;
        }

        act_id = loadObjectEll(forloading, ell, noLabel, graphId_eventId);//// HOC!

        string++; len--;
        if (!(string = skipSpaces(string, &len)))
            return false;
        if (!(string = haspos(string,  (char*)XI, 0, &len)))
            return false;
        registerObjectByFirstLabel(forloading, act_id, graphId_eventId); //// HOC!

        xi.clear();
        if (!(string = atreturn(string, &len)))
            return false;
        if (!(string = skipSpaces(string, &len)))
            return false;
        while (*string != DOT) {
            char* beforeReturn = atreturn(string, &len);
            xi.emplace_back(string, beforeReturn-1);
            string = beforeReturn;
            if (!(string = skipSpaces(string, &len)))
                return false;
        }
        loadObjectXi(forloading, xi, graphId_eventId);//// HOC!

        string++; len--;
        if (!(string = skipSpaces(string, &len)))
            return false;
        if (!(string = haspos(string,  (char*)PROP, 0, &len)))
            return false;

        if (!(string = atreturn(string, &len)))
            return false;
        if (!(string = skipSpaces(string, &len)))
            return false;
        while (*string != DOT) {
            char* beforeReturn = atreturn(string, &len);
            std::string tmp{string, beforeReturn-1};
            size_t idx = tmp.find("\t");
            if (idx == std::string::npos)
                return false;
            std::string key = UNESCAPE(tmp.substr(0, idx));
            std::string val = UNESCAPE(tmp.substr(idx+1));

            // Setting up the properties associated to the node
            loadObjectProperty(forloading, map_for_types, act_id, graphId_eventId, key, val);

            string = beforeReturn;
            if (!(string = skipSpaces(string, &len)))
                return false;
        }
        string++; len--;
        if (!(string = skipSpaces(string, &len)))
            return false;
        if (!(string = haspos(string,  (char*)PHI, 0, &len)))
            return false;

        if (!(string = atreturn(string, &len)))
            return false;
        if (!(string = skipSpaces(string, &len)))
            return false;

        forloading.objectScoresLoading[graphId_eventId].emplace_back(1.0);
        while (*string != DOT) {
            char* beforeReturn = atreturn(string, &len);
            std::string tmp{string, beforeReturn-1};
            tmp = UNESCAPE(tmp);
            string =beforeReturn;
            if (!(string = skipSpaces(string, &len)))
                return false;
            while (*string != ';') {
                if (sscanf(string, "%lf\t%lu%n", &weight, &content,&scanSkip)==(-1))
                    return false;
                len-=scanSkip;
                string+=scanSkip;
                // Setting up the containment associated to the node
                forloading.containment_tables[tmp].add(act_id, graphId_eventId, weight, content);
                forloading.containment_relationships.emplace(tmp);
                if (!(string = skipSpaces(string, &len)))
                    return false;
            }
            string++;
            len--;
            if (!(string = skipSpaces(string, &len)))
                return false;
        }
        string++;
        len--;
        if (!(string = skipSpaces(string, &len)))
            return false;

        if (strncmp(string,"~~",2)==0) {
            // Finding a new graph
            forloading.nodesInGraph.emplace_back(0);
//            std::cout << graphId_eventId.first << std::endl;
            graphId_eventId.first++; string+=2; len-=2;
            if (!(string = skipSpaces(string, &len)))
                return false;
        }
    }
//    std::cout << graphId_eventId.first << "... done!" << std::endl;
    maxGraphId = graphId_eventId.first;
    return true;
}

static inline bool parse(char* string,
                         size_t len,
                         std::vector<std::vector<gsm_object>>& db) {
    string = skipSpaces(string, &len);
//    size_t graphId = 0;
//    std::vector<std::string> ell, xi;
    double weight; size_t content;
    size_t act_id;
//    size_t noLabel = initLoading(forloading);
    int scanSkip = 0;
    constexpr char const * ID = "id:";
    constexpr char const  DOT = '.';
    constexpr char const * ELL = "ell:";
    constexpr char const * XI = "xi:";
    constexpr char const * PROP = "properties:";
    constexpr char const * PHI = "phi:";
//    size_t id;
//    std::pair<size_t,size_t> graphId_eventId{0,0};

    // Defining a simple format to scan, so to avoid using a parser for this.
    while (string && *string) {
        // Reading a new object from the database for value querying
        if (db.empty())
            db.emplace_back();
        auto& current = db.back().emplace_back();
        current.id = 0;
//        if (forloading.nodesInGraph.size() == graphId_eventId.first) {
//            forloading.nodesInGraph.emplace_back(graphId_eventId.second);
//        }
//        ell.clear();
        if (!(string = haspos(string, (char*)ID, 0, &len)))  return false;
        current.scores.clear();
        current.ell.clear();
        current.xi.clear();
        current.content.clear();
        current.phi.clear();
        if (sscanf(string, "%lu%n", &current.id, &scanSkip)==-1)  return false;
//        forloading.nodesInGraph[graphId_eventId.first] = std::max(forloading.nodesInGraph[graphId_eventId.first],graphId_eventId.second);
        len-=scanSkip;
        string+=scanSkip;
        if (!(string = skipSpaces(string, &len)))  return false;
        if (!(string = haspos(string,  (char*)ELL, 0, &len)))  return false;
//        act_id = noLabel;

        if (!(string = atreturn(string, &len)))  return false;
        if (!(string = skipSpaces(string, &len)))  return false;
        while (*string != DOT) {
            char* beforeReturn = atreturn(string, &len);
            current.ell.emplace_back(string, beforeReturn-1);
            string = beforeReturn;
            if (!(string = skipSpaces(string, &len)))  return false;
        }


//        act_id = loadObjectEll(forloading, ell, noLabel, graphId_eventId);//// HOC!

        string++; len--;
        if (!(string = skipSpaces(string, &len)))  return false;
        if (!(string = haspos(string,  (char*)XI, 0, &len)))  return false;
//        registerObjectByFirstLabel(forloading, act_id, graphId_eventId); //// HOC!

//        xi.clear();
        if (!(string = atreturn(string, &len)))  return false;
        if (!(string = skipSpaces(string, &len)))  return false;
        while (*string != DOT) {
            char* beforeReturn = atreturn(string, &len);
            current.xi.emplace_back(string, beforeReturn-1);
            string = beforeReturn;
            if (!(string = skipSpaces(string, &len)))  return false;
        }
//        loadObjectXi(forloading, xi, graphId_eventId);//// HOC!

        string++; len--;
        if (!(string = skipSpaces(string, &len)))  return false;
        if (!(string = haspos(string,  (char*)PROP, 0, &len)))  return false;

        if (!(string = atreturn(string, &len)))  return false;
        if (!(string = skipSpaces(string, &len)))  return false;
        while (*string != DOT) {
            char* beforeReturn = atreturn(string, &len);
            std::string tmp{string, beforeReturn-1};
            size_t idx = tmp.find("\t");
            if (idx == std::string::npos)  return false;
            std::string key = UNESCAPE(tmp.substr(0, idx));
            std::string val = UNESCAPE(tmp.substr(idx+1));

            current.content[key] = val;
            // Setting up the properties associated to the node
//            loadObjectProperty(forloading, map_for_types, act_id, graphId_eventId, key, val);

            string = beforeReturn;
            if (!(string = skipSpaces(string, &len)))  return false;
        }
        string++; len--;
        if (!(string = skipSpaces(string, &len)))  return false;
        if (!(string = haspos(string,  (char*)PHI, 0, &len)))  return false;

        if (!(string = atreturn(string, &len)))  return false;
        if (!(string = skipSpaces(string, &len)))  return false;

//        forloading.objectScoresLoading[graphId_eventId].emplace_back(1.0);
        while (*string != DOT) {
            char* beforeReturn = atreturn(string, &len);
            std::string tmp{string, beforeReturn-1};
            tmp = UNESCAPE(tmp);
            string =beforeReturn;
            if (!(string = skipSpaces(string, &len)))  return false;
            while (*string != ';') {
                if (sscanf(string, "%lf\t%lu%n", &weight, &content,&scanSkip)==-1)  return false;
                len-=scanSkip;
                string+=scanSkip;
                // Setting up the containment associated to the node
                current.phi[tmp].emplace_back(content, weight);
//                forloading.containment_tables[tmp].add(act_id, graphId_eventId, weight, content);
//                forloading.containment_relationships.emplace(tmp);
                if (!(string = skipSpaces(string, &len)))  return false;
            }
            string++;
            len--;
            if (!(string = skipSpaces(string, &len)))  return false;
        }
        string++;
        len--;
        if (!(string = skipSpaces(string, &len)))  return false;


        if (strncmp(string,"~~",2)==0) {
            // Finding a new graph
            db.emplace_back();
            string+=2; len-=2;
            if (!(string = skipSpaces(string, &len)))  return false;
        }
    }
    return true;
//    maxGraphId = graphId_eventId.first;
}

static inline void parse(const std::filesystem::path& file,
                         std::vector<std::vector<gsm_object>>& db) {
    std::string buffer;
    std::ifstream f(file);
    f.seekg(0, std::ios::end);
    size_t n = f.tellg();
    buffer.resize(f.tellg());
    f.seekg(0);
    f.read(buffer.data(), buffer.size());
    parse((char*)buffer.c_str(), n, db);
}

#endif //GSM2_LINEARGSM_H
