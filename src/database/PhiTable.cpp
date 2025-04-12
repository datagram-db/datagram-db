/*
 * PhiTable.cpp
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

#include "database/PhiTable.h"

namespace gsm2 {
    namespace tables {
        PhiTable::secmem_record::secmem_record(size_t label_id, size_t l0Id, const std::pair<size_t,size_t>& nodeId, double wContained, size_t idContained, size_t instanceId) : label_id(label_id), l0_id(l0Id), graph_id(nodeId.first),
                                                                                                                                                  object_id(nodeId.second),
                                                                                                                                                  w_contained(wContained),
                                                                                                                                                  id_contained(idContained),
                                                                                                                                                  instance_id{instanceId} {}

        PhiTable::record::record(size_t l0Id, const std::pair<size_t,size_t>& nodeId, double wContained, size_t idContained, size_t instanceId) : l0_id(l0Id), graph_id(nodeId.first),
        object_id(nodeId.second),
                                                                                                      w_contained(wContained),
                                                                                                      id_contained(idContained),
                                                                                                      instance_id{instanceId} {}
        bool PhiTable::secmem_record::operator<(const PhiTable::secmem_record &rhs) const {
            if (label_id < rhs.label_id)
                return true;
            if (l0_id < rhs.l0_id)
                return true;
            if (rhs.l0_id < l0_id)
                return false;
            if (graph_id < rhs.graph_id)
                return true;
            if (rhs.graph_id < graph_id)
                return false;
            if (object_id < rhs.object_id)
                return true;
            if (rhs.object_id < object_id)
                return false;
            if (w_contained < rhs.w_contained)
                return true;
            if (rhs.w_contained < w_contained)
                return false;
            if (id_contained < rhs.id_contained)
                return true;
            if (id_contained > rhs.id_contained)
                return false;
            return instance_id < rhs.instance_id;
        }

        bool PhiTable::record::operator<(const PhiTable::record &rhs) const {
            if (l0_id < rhs.l0_id)
                return true;
            if (rhs.l0_id < l0_id)
                return false;
            if (graph_id < rhs.graph_id)
                return true;
            if (rhs.graph_id < graph_id)
                return false;
            if (object_id < rhs.object_id)
                return true;
            if (rhs.object_id < object_id)
                return false;
            if (w_contained < rhs.w_contained)
                return true;
            if (rhs.w_contained < w_contained)
                return false;
            if (id_contained < rhs.id_contained)
                return true;
            if (id_contained > rhs.id_contained)
                return false;
            return instance_id < rhs.instance_id;
        }

        bool PhiTable::record::operator>(const PhiTable::record &rhs) const {
            return rhs < *this;
        }

        bool PhiTable::record::operator<=(const PhiTable::record &rhs) const {
            return !(rhs < *this);
        }

        bool PhiTable::record::operator>=(const PhiTable::record &rhs) const {
            return !(*this < rhs);
        }

        bool PhiTable::secmem_record::operator>(const PhiTable::secmem_record &rhs) const {
            return rhs < *this;
        }

        bool PhiTable::secmem_record::operator<=(const PhiTable::secmem_record &rhs) const {
            return !(rhs < *this);
        }

        bool PhiTable::secmem_record::operator>=(const PhiTable::secmem_record &rhs) const {
            return !(*this < rhs);
        }

//        PhiTable::primary_index::primary_index(size_t l0Id, const PhiTable::record *begin,
//                                               const PhiTable::record *anEnd) : l0_id(l0Id), begin(begin), end(anEnd) {}

        void PhiTable::add(size_t l0Id, const std::pair<size_t,size_t>& nodeId, double wContained, size_t idContained, size_t instanceId) {
            table.emplace_back(l0Id, nodeId, wContained, idContained, instanceId);
        }

        size_t PhiTable::index(size_t first_record_id) {
            sort();
            size_t lIdPrev = 0;
            const struct record *begin;
            size_t N = table.size();
            std::pair<size_t, size_t> cp;
            for (size_t i = 0; i<N; i++ ) {
                const auto& ref = table[i];
                cp.first = ref.graph_id;
                cp.second = ref.object_id;
                secondary_index[cp.first][cp.second].emplace_back(&ref);
                if (i == 0) {
                    lIdPrev = ref.l0_id;
                    begin = &ref;
                } else if (ref.l0_id != lIdPrev) {
                    primary_index[lIdPrev] = {begin, (&ref)-1};
                    begin = &ref;
                    lIdPrev = ref.l0_id;
                }
                table[i].record_id = first_record_id+i;
            }
            primary_index[lIdPrev] = {begin, (&table[N-1])};
            return first_record_id+table.size();
        }
    } // gsm2
} // tables
