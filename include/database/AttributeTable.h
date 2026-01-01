/*
 * AttributeTable.h
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

#ifndef GSM2_ATTRIBUTETABLE_H
#define GSM2_ATTRIBUTETABLE_H

#include <string>
#include <optional>
#include <variant>
#include <limits>
#include <unordered_map>
#include <vector>
#include <map>
#include <iostream>
#include "SimplifiedFuzzyStringMatching.h"
#include "queries/DataQuery.h"
#include "ActivityTable.h"
#include <yaucl/data/NDPFuzzyStringMatching.h>



namespace gsm2 {
    namespace tables {


        // Source: https://github.com/datagram-db/knobab/blob/v2.0/include/knobab/server/tables/AttributeTable.h

        enum AttributeTableType {
            DoubleAtt,
            SizeTAtt,
            LongAtt,
            StringAtt,
            BoolAtt
            // TODO: hierarchical types!, https://dl.acm.org/doi/10.1145/3410566.3410583
        };

        using union_type = std::variant<double, size_t, long long, std::string, bool>;
        using union_minimal = std::variant<std::string, double>;



        static inline union_type cast_unions(AttributeTableType type, const union_minimal &x) {
            switch (type) {
                case DoubleAtt:
                    return std::get<double>(x);
                case LongAtt:
                    return (long long)std::get<double>(x);
                case StringAtt:
                    return  std::get<std::string>(x);
                case BoolAtt:
                    return std::abs(std::get<double>(x)) > std::numeric_limits<double>::epsilon();
                    //case SizeTAtt:
                default:
                    // TODO: hierarchical types!, https://dl.acm.org/doi/10.1145/3410566.3410583
                    return (size_t)std::get<double>(x);
            }
        }

        static inline double similarityFunction(const union_type& lhs, const union_type& rhs, double c = 2.0) {
            if (lhs.index() != rhs.index())
                return std::numeric_limits<double>::max(); // If the elements are not of the same type, then they are associated to the maximum distance.
            if (std::holds_alternative<double>(lhs)) {
                return 1.0 / ( (std::abs(std::get<double>(lhs)- std::get<double>(rhs)))/c + 1.0);
            } else if (std::holds_alternative<size_t>(lhs)) {
                auto l = std::get<size_t>(lhs);
                auto r = std::get<size_t>(rhs);
                return 1.0 / ( ((double)(((l > r) ? (l-r) : (r-l))))/c + 1.0);
            } else if (std::holds_alternative<long long>(lhs)) {
                return 1.0 / ( (std::abs((double)std::get<long long>(lhs)- (double)std::get<long long>(rhs)))/c + 1.0);
            }
            return std::numeric_limits<double>::max();
        }

        template <typename T>
         size_t my_storeLoad(gsm2::tables::AttributeTableType type,
                                          const std::variant<double, size_t, long long int, std::string, bool> &x,
                                          T& ptr) {
            switch (type) {
                case gsm2::tables::DoubleAtt: {
                    DEBUG_ASSERT(std::holds_alternative<double>(x));
                    double tmp = std::get<double>(x);
                    return *(size_t*)(&tmp);
                }

                case gsm2::tables::LongAtt: {
                    DEBUG_ASSERT(std::holds_alternative<long long>(x));
                    long long tmp = std::get<long long>(x);
                    return *(size_t*)(&tmp);
                }

                case gsm2::tables::StringAtt: {
                    DEBUG_ASSERT(std::holds_alternative<std::string>(x));
                    std::string tmp = std::get<std::string>(x);
                    return ptr.put(tmp).first;
                }
                case gsm2::tables::BoolAtt: {
                    DEBUG_ASSERT(std::holds_alternative<bool>(x));
                    return std::get<bool>(x) ? 1 : 0;
                }
                    //case SizeTAtt:
                default:
                    DEBUG_ASSERT(std::holds_alternative<size_t>(x));
                    // TODO: hierarchical types!, https://dl.acm.org/doi/10.1145/3410566.3410583
                    return std::get<size_t>(x);
            }
        }

        struct AttributeTable {
            std::string attr_name;
            SimplifiedFuzzyStringMatching ptr;
            AttributeTableType type;
            std::unordered_map<std::string, std::vector<size_t>> string_offset_mapping;


            /**
             * If we want to perform approximate string matching, then this query is rewritten as a query among
             * all the approximate matches that were provided in the single instance of the strings from the
             * appropriate index
             *
             * @param ell0      Label associated to the node containing the object
             * @param threshold Minimum match threshold
             * @param topk      Maximum amount of scores to be returned
             * @param x         String to be approximated
             * @return Resulting interval queries expressing the approximate match (please observe that this cannot be
             *         necessarily rewritten into a exact match)
             */
            std::vector<DataQuery> generateDataQueriesForApproximateString(const std::string& ell0, double threshold, size_t topk, const std::string& x) {
                std::vector<DataQuery> v;
                if (type != StringAtt) return v;
                std::multimap<double, std::string> result;
                ptr.fuzzyMatch(threshold, topk, x, result);
                for (const auto& [score, match] : result) {
                    v.emplace_back(DataQuery::RangeQuery(ell0, attr_name, match, match, score));
                }
                return v;
            }

            struct record {
                size_t act;
                size_t value;
                size_t act_table_offset;

                record(size_t act, size_t value, size_t actTableOffset);
                record() = default;
                record(const record&) = default;
                record(record&&) = default;
                record& operator=(const record&) = default;
                record& operator=(record&&) = default;
            };

            std::vector<record> table;
            size_t overall_size = 0;
            /// TODO: struct disjunctive_range_query_result {
            /// TODO: struct range_query_result


            /**
             * Mapping the act id to the offset of where it appears within the record
             */
            std::vector<std::pair<size_t, size_t>> primary_index;

            /**
             * This data structure maps an offset in the ActTable, identifying a specific event with a specific act/event-label
             * A within a given trace/run, to an offset within the current table, thus stating that there exists an attribute
             * A.attr_name in the payload
             */
            std::unordered_map<size_t, size_t> secondary_index;
            std::unordered_map<std::pair<size_t, size_t>, size_t> secondary_index2;

            AttributeTable() : attr_name(""), type{BoolAtt} {}

            AttributeTable(const std::string &attr, AttributeTableType type)
                    : attr_name(attr), type{type} {}

            AttributeTable(const AttributeTable &) = default;
            AttributeTable(AttributeTable &&) = default;
            AttributeTable &operator=(const AttributeTable &) = default;
            AttributeTable &operator=(AttributeTable &&) = default;

            const record *resolve_record_if_exists(size_t actTableOffset) const;
            std::optional<union_minimal> resolve_record_if_exists2(size_t actTableOffset) const;
            std::ostream &resolve_and_print(std::ostream &os, const AttributeTable::record &x) const;
            void record_load(size_t act_id, const union_type &val, size_t tid, size_t eid);
            void index(const ActivityTable&, const std::vector<std::vector<size_t>> &trace_id_to_event_id_to_offset);

            template <typename T>
            void secondary_memory_index(const std::vector<std::vector<size_t>> &idx2,
                                        const std::vector<size_t>& graph_to_n_objects,
                                        std::ofstream& data_table,
                                        std::ofstream& flat_secondary_index_f,
                                        T& all_string_attributes_repo) {
                record cache;
                size_t count = 0;
                for (size_t val : graph_to_n_objects)
                    count+=val;
                std::vector<size_t> flat_secondary_index(count, -1);
                size_t current_table_size = 0;
                for (size_t act_id = 0, N = elements.size(); act_id < N; act_id++) {
                    cache.act = act_id;
                    auto& ref = elements[act_id];
//                    size_t begin = 0;
                    if (!ref.empty()) {
                        std::map<union_type, std::vector<size_t>> valueToOffsetInTable;
                        for (const auto& val_offset : ref) {
                            for (const auto& traceid_eventid : val_offset.second) {
                                size_t offset = idx2.at(traceid_eventid.first).at(traceid_eventid.second);
                                valueToOffsetInTable[val_offset.first].emplace_back(offset);
                            }
                        }
                        for (auto it = valueToOffsetInTable.begin(); it != valueToOffsetInTable.end(); it++) {
                            std::sort(it->second.begin(), it->second.end());
                            size_t val = my_storeLoad(type, it->first, all_string_attributes_repo); // TODO
                            cache.value = val;
                            std::string current_string;
                            if (type == StringAtt) {
                                current_string = std::get<std::string>(it->first);
                            }
                            for (const auto& refx : it->second) {
                                if (type == StringAtt)
                                    string_offset_mapping[current_string].emplace_back(current_table_size);
                                DEBUG_ASSERT(refx < flat_secondary_index.size());
                                flat_secondary_index[refx] = current_table_size;
//                                secondary_index2[{at.table.at(refx).graph_id, at.table.at(refx).event_id}] =
                                cache.act_table_offset = refx;
                                data_table.write((const char*)&cache, sizeof(cache));
                                current_table_size += 1;
                            }
                        }
                        valueToOffsetInTable.clear();
                        ref.clear();
                    }
//                    size_t end = current_table_size;
//                    primary_index_f.write((const char*)&begin, sizeof(begin));
//                    primary_index_f.write((const char*)&end, sizeof(end));
                }
                for (const size_t& offset : flat_secondary_index) {
                    flat_secondary_index_f.write((const char*)&offset, sizeof(offset));
                }
                elements.clear();
            }

            union_type resolve(const record &x) const;

            std::vector<std::vector<std::pair<const AttributeTable::record *, const AttributeTable::record *>>>
            exact_range_query(const std::vector<std::pair<size_t, std::vector<DataQuery*>>>& propList) const;


        private:
            size_t storeLoad(const union_type &x);
            bool assertVariant(const union_type &val);

            std::vector<std::map<union_type, std::vector<std::pair<size_t, size_t>>>> elements;
        };

        static inline
        union_minimal resolveUnionMinimal(const AttributeTable &table, const AttributeTable::record &x) {
            switch (table.type) {
                case DoubleAtt:
                    return *(double*)(&x.value);
                case LongAtt:
                    return (double)(*(long long*)(&x.value));
                case StringAtt:
                    return table.ptr.get(x.value);
                case BoolAtt:
                    return (x.value != 0 ? 0.0 : 1.0);
                    //case SizeTAtt:
                default:
                    // TODO: hierarchical types!, https://dl.acm.org/doi/10.1145/3410566.3410583
                    return (double)x.value;
            }
        }

    } // gsm2
} // tables


#include <magic_enum/magic_enum.hpp>

static inline void parse_schema_map(std::istream& stream,
                                    std::unordered_map<std::string, gsm2::tables::AttributeTableType>& result) {
    std::string line;
    size_t count = 0;
    std::string field_name;
    gsm2::tables::AttributeTableType field_type = gsm2::tables::AttributeTableType::StringAtt;
    while (std::getline(stream, line)) {
        count++;
        if (count == 1)
            field_name = line;
        else {
            auto val = magic_enum::enum_cast<gsm2::tables::AttributeTableType>(line);
            if (val)
                field_type = val.value();
            else
                field_type = gsm2::tables::AttributeTableType::StringAtt;
            result[field_name] = field_type;
        }
    }
}

#endif //GSM2_ATTRIBUTETABLE_H
