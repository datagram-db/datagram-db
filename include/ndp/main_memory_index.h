//
// Created by giacomo on 11/04/25.
//

#ifndef GSM2_MAIN_MEMORY_INDEX_H
#define GSM2_MAIN_MEMORY_INDEX_H

#include <vector>
#include <database/AttributeTable.h>

struct ActivityEntry {
    size_t offset_begin;
    size_t offset_end;
    size_t activity_id;
};

struct GraphToObjInformation {                               // Defines the pointer towards the information being stored for each object
    size_t activity_table_entry_id_offset;                   // Offset to the secondary memory activity table
    std::vector<double> scores;                              // Trustworthyness scores associated with each object
    std::vector<size_t> ells;                                // storing just the id for the labels
    std::vector<size_t> xis;                                 // storing just the id for the values
};

struct GraphInformation {                           // Defines the information stored and indexed for each graph
    size_t n_obj;                                   // Number of contained objects
    size_t n_obj_offset;                            // Pre-computed object offsets
    std::vector<GraphToObjInformation> objects;     // Pointer towards the collected object information
};

struct AttributeInformation {
    size_t attribute_id;
    size_t secondary_memory_table_records_offset_begin;
    size_t secondary_memory_table_records_offset_end;
    size_t table_length;
    gsm2::tables::AttributeTableType type;
};

struct ContainmentInformation {
    size_t containment_id;
    size_t secondary_memory_table_records_offset_begin;
    size_t secondary_memory_table_records_offset_end;
    size_t table_length;
    size_t secondary_index_offset_header;
    size_t secondary_index_offset_contents;
};

#endif //GSM2_MAIN_MEMORY_INDEX_H
