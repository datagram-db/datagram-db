/*
 * PhiTable.h
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

#ifndef GSM2_PHITABLE_H
#define GSM2_PHITABLE_H

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <yaucl/hashing/pair_hash.h>
#include <fstream>
#include <yaucl/functional/assert.h>

namespace gsm2 {
    namespace tables {
        struct PhiTable {
            struct primary_index_2m {
                size_t l0;
                size_t begin;
                size_t end;
            };

            struct record {
                size_t l0_id;
                size_t graph_id;
                size_t object_id;
                double w_contained; // Weight
                size_t id_contained;
                size_t instance_id;
                size_t record_id;

                record(const record&) = default;
                record(record&&) = default;
                record& operator=(const record&) = default;
                record& operator=(record&&) = default;
                record(size_t l0Id = 0, const std::pair<size_t,size_t>& nodeId = {0,0}, double wContained = 1.0, size_t idContained = 1, size_t instance_id = 0);

                bool operator<(const record &rhs) const;
                bool operator>(const record &rhs) const;
                bool operator<=(const record &rhs) const;
                bool operator>=(const record &rhs) const;
            };

            struct secmem_record {
                size_t label_id;
                size_t l0_id;
                size_t graph_id;
                size_t object_id;
                double w_contained; // Weight
                size_t id_contained;
                size_t instance_id;
                size_t record_id;

                secmem_record(const secmem_record&) = default;
                secmem_record(secmem_record&&) = default;
                secmem_record& operator=(const secmem_record&) = default;
                secmem_record& operator=(secmem_record&&) = default;
                secmem_record(size_t label_id, size_t l0Id = 0, const std::pair<size_t,size_t>& nodeId = {0,0}, double wContained = 1.0, size_t idContained = 1, size_t instance_id = 0);

                secmem_record& operator=(const record& x) {
                    l0_id = x.l0_id;
                    graph_id = x.graph_id;
                    object_id = x.object_id;
                    w_contained = x.w_contained;
                    id_contained = x.id_contained;
                    instance_id = x.instance_id;
                    record_id = x.record_id;
                    return *this;
                }

                friend std::ostream& operator<<(std::ostream& os, const secmem_record& dt) {
                    return os << "graph #" << dt.graph_id << ": (" << dt.object_id << ")-->(" << dt.id_contained << ")";
                }
                bool operator<(const secmem_record &rhs) const;
                bool operator>(const secmem_record &rhs) const;
                bool operator<=(const secmem_record &rhs) const;
                bool operator>=(const secmem_record &rhs) const;
            };

//            struct primary_index {
//                size_t l0_id;
//                const struct record* begin;
//                const struct record* end;
//
//                primary_index(const primary_index&) = default;
//                primary_index(primary_index&&) = default;
//                primary_index& operator=(const primary_index&) = default;
//                primary_index& operator=(primary_index&&) = default;
//                primary_index(size_t l0Id = 0, const record *begin = nullptr, const record *anEnd = nullptr);
//            };

            std::vector<struct record> table;
            std::unordered_map<size_t, std::pair<const struct record*,const struct record*>> primary_index;
            std::unordered_map<size_t, std::unordered_map<size_t, std::vector<const struct record*>>> secondary_index;
            bool sorted = false;
            void add(size_t l0Id = 0, const std::pair<size_t,size_t>& nodeId = {0,0}, double wContained = 1.0, size_t idContained = 1, size_t instance_id = 0);

            size_t index(size_t first_record_id);

            inline void sort() {
                if (!sorted) {
                    std::sort(table.begin(), table.end()); // sorting the table in primary memory, for then serializing each record directly in secondary
                    sorted = true;
                }
            }

            size_t count_primary_entries() {
                DEBUG_ASSERT(sorted);
                size_t N = table.size();
                size_t lIdPrev = 0;
                size_t count = 0;
                for (size_t i = 0; i<N; i++ ) {
                    auto& ref = table[i];
                    if (i == 0) {
                        lIdPrev = ref.l0_id;
                    } else if (ref.l0_id != lIdPrev) {
                        count++;
                        lIdPrev = ref.l0_id;
                    }
                }
                return (count+1);
            }

            inline size_t secondary_memory_index(size_t edge_label_id,
                                          size_t first_record_id,
                                          const std::vector<size_t>& graph_to_n_objects,
                                          std::ofstream& table_storage,
                                          std::ofstream& primary_index_file,
                                          std::ofstream& secondary_index_file) {
                sort();
                size_t lIdPrev = 0;
                size_t begin_offset = 0;
                size_t N = table.size();
                std::pair<size_t, size_t> cp;
                size_t prev = 0;
                secmem_record cache(edge_label_id);
                primary_index_2m cache2;
                for (size_t i = 0; i<N; i++ ) {
                    auto& ref = table[i];
                    cp.first = ref.graph_id;
                    cp.second = ref.object_id;
                    secondary_index[cp.first][cp.second].emplace_back(&ref);
                    if (i == 0) {
                        lIdPrev = ref.l0_id;
                        begin_offset = i;
                    } else if (ref.l0_id != lIdPrev) {
                        cache2.l0 = lIdPrev;
                        cache2.begin = begin_offset;
                        cache2.end = prev;
                        primary_index_file.write((const char*)&cache2, sizeof(cache2));
//                        primary_index_size += ((sizeof(size_t))*3);
                        begin_offset = i;
                        lIdPrev = ref.l0_id;
                    }
                    ref.record_id = first_record_id+i;
                    cache = ref;
                    table_storage.write((const char*)&cache, sizeof(secmem_record));
#ifdef DEBUG
                    std::cout << cache << std::endl
#endif
                    prev = i;
                }
//                primary_index[lIdPrev] = {begin, (&table[N-1])};
                cache2.l0 = lIdPrev;
                cache2.begin = begin_offset;
                cache2.end = prev;
                primary_index_file.write((const char*)&cache2, sizeof(cache2));
//                primary_index_size += ((sizeof(size_t))*3);
                size_t offset_secondary_begin = 0;
                size_t offset_secondary_end = 0;
                for (size_t n_graphs = 0, N = graph_to_n_objects.size(); n_graphs<N; n_graphs++) {
                    auto it = secondary_index.find(n_graphs);
                    if (it == secondary_index.end()) {
                        for (size_t n_object = 0, M = graph_to_n_objects.at(n_graphs); n_object<M; n_object++ ) {
                            secondary_index_file.write((const char*)&offset_secondary_end, sizeof(size_t));
                            secondary_index_file.write((const char*)&offset_secondary_end, sizeof(size_t));
                        }
                    } else {
                        auto& mappa = it->second;
                        for (size_t n_object = 0, M = graph_to_n_objects.at(n_graphs); n_object<M; n_object++ ) {
                            auto it2 = mappa.find(n_object);
                            if (it2 == mappa.end()) {
                                secondary_index_file.write((const char*)&offset_secondary_end, sizeof(size_t));
                                secondary_index_file.write((const char*)&offset_secondary_end, sizeof(size_t));
                            } else {
                                offset_secondary_begin = offset_secondary_end;
                                secondary_index_file.write((const char*)&offset_secondary_begin, sizeof(size_t));
                                offset_secondary_end += ((sizeof(size_t ))* it2->second.size());
                                secondary_index_file.write((const char*)&offset_secondary_end, sizeof(size_t));
                            }
                        }
                    }
                }
                for (size_t n_graphs = 0, N = graph_to_n_objects.size(); n_graphs<N; n_graphs++) {
                    auto it = secondary_index.find(n_graphs);
                    if (it != secondary_index.end())  {
                        auto& mappa = it->second;
                        for (size_t n_object = 0, M = graph_to_n_objects.at(n_graphs); n_object<M; n_object++ ) {
                            auto it2 = mappa.find(n_object);
                            if (it2 != mappa.end()) {
                                for (const auto& item : it2->second) {
                                    secondary_index_file.write((const char*)&item->record_id, sizeof(item->record_id));
                                }
                            }
                        }
                    }
                }
                primary_index.clear();
                secondary_index.clear();
                size_t tmp =  first_record_id+table.size();
                table.clear();
                return tmp;
            }
        };
    } // gsm2
} // tables

#endif //GSM2_PHITABLE_H
