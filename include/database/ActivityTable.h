/*
 * ActivityTable.h
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

#ifndef GSM2_ACTIVITYTABLE_H
#define GSM2_ACTIVITYTABLE_H

#include <tuple>
#include <vector>
#include <ostream>
#include <string>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <yaucl/functional/assert.h>

namespace gsm2 {
    namespace tables {




uint16_t cast_to_float(size_t x, size_t l);
uint16_t cast_to_float2(size_t x, size_t l);

/**
 * TODO: use a graph to store the prev/next information
 *        look up at the original setting, too
 *  then, populate using emplace_back
 */

struct ActivityTable {

    struct record {
        size_t          l0_id;
        size_t          graph_id;
        size_t          event_id;
        struct record*  prev;
        struct record*  next;

        record();
        record(size_t activity_label, size_t event_id, size_t time, struct record* prev, struct record* next);
        record(const record& ) = default;
        record& operator=(const record&) = default;


        bool operator==(const record &rhs) const;
        bool operator!=(const record &rhs) const;
        bool operator<(const record &rhs) const;
        bool operator>(const record &rhs) const;
        bool operator<=(const record &rhs) const;
        bool operator>=(const record &rhs) const;
    };

    struct secmem_record {
        size_t l0_id;
        size_t graph_id;
        size_t event_id;

        secmem_record();
        secmem_record(const record& x) {
            l0_id = x.l0_id;
            graph_id = x.graph_id;
            event_id = x.event_id;
        }
    };


    /**
     * Actual table containing the data and the navigation indices. Those are mainly required for reconstructing the log
     * information for isomorphism purposes
     */
    std::vector<struct record> table;


    std::vector<size_t> primary_index;

    /**
     * Associating an act id to all of the events from all the traces having the same act id
     */
    [[nodiscard]] std::pair<const size_t, const size_t> resolve_index(size_t id) const;

    /**
     * Mapping the trace id to the first and last event (see the log printer from the KnowledgeBase for a usage example)
     */
    std::vector<std::pair<record*, record*>> secondary_index;

    void load_record(size_t seq_id, size_t activity_label, size_t event_id); // rename: loading_step (emplace_back)
    const std::vector<std::vector<size_t>> & indexing1();
    const std::vector<std::vector<size_t>>& secondary_memory_indexing1(const std::filesystem::path& folder) {
        size_t offset = 0;
        secmem_record cache;
        // Phase 1
        std::ofstream  data(folder / "activity_table_data.bin",  std::ios::out | std::ios::binary);
        std::ofstream  data_primary_index(folder / "activity_table_data_pi.txt",  std::ios::out);
        size_t offsets = 0;
        size_t previous_offset = 0;
        for (size_t k = 0, N = builder.act_id_to_trace_id_and_time.size(); k < N; k++) {
            cache.l0_id = k;
            primary_index.emplace_back(offset);
            auto& ref = builder.act_id_to_trace_id_and_time[k];
            offsets += ref.size();
            for (const std::pair<size_t, size_t>& cp : ref) {
                cache.graph_id = cp.first;
                cache.event_id = cp.second;
                data.write((const char*)&cache, sizeof(record));
                builder.trace_id_to_event_id_to_offset[cp.first][cp.second] = offset++;
            }
            data_primary_index << previous_offset << "," << offsets << std::endl;
            previous_offset = offsets;
            ref.clear(); // freeing some memory
        }
        builder.act_id_to_trace_id_and_time.clear(); // freeing some memory
        return builder.trace_id_to_event_id_to_offset;
    }

    void indexing2();
    void secondary_memory_index2(const std::filesystem::path& folder) {
        std::ofstream  data(folder / "activity_table_secondary_index.bin",  std::ios::out | std::ios::binary);
        // Phase 2, creating the secondary index, for accessing the beginning and the end of the trace from the table
        for (size_t sigma_id = 0, M = builder.trace_id_to_event_id_to_offset.size(); sigma_id < M ; sigma_id++) {
            auto& ref = builder.trace_id_to_event_id_to_offset[sigma_id];
            for (size_t time = 0, T = ref.size(); time < T; time++) {
                size_t offset = ref[time];
                data.write((const char*)&offset, sizeof(offset));
            }
        }
    }
    void sanityCheck();
    void clear();
    void clearIDX() {
        builder.trace_id_to_event_id_to_offset.clear();
    }

    std::vector<size_t> trace_length; // L1
    size_t getTraceLength(size_t id) const { return trace_length.at(id); }

    friend std::ostream &operator<<(std::ostream &os, const ActivityTable &table);

private:
    std::vector<std::tuple<size_t, size_t, size_t>> expectedOrdering; // TODO: remove?

    struct table_builder {
        std::vector<std::vector<std::pair<size_t, size_t>>> act_id_to_trace_id_and_time; // M1
        std::vector<std::vector<size_t>> trace_id_to_event_id_to_offset; // M2
    };

    table_builder builder;

public:
    const std::vector<size_t>& getTraceLengths() const {
        return trace_length;
    }

    const table_builder& getBuilder() const{
        return builder;
    }
};





    } // gsm2
} // tables

#endif //GSM2_ACTIVITYTABLE_H
