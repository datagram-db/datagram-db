//
// Created by giacomo on 23/12/24.
//

#ifndef GSM2_RANDOMACCESSGSMREADER_H
#define GSM2_RANDOMACCESSGSMREADER_H

#include <string>
#include <parser/writers/PrimaryMemoryLoader.h>
#include <parser/RandomAccessReader.h>
#include <filesystem>

struct RandomAccessGSMReader  {
    ~RandomAccessGSMReader() {}
    size_t count_databases() ;
    ssize_t database_size(size_t graph_id) ;
    std::optional <gsm_object> retrieve(size_t graph_id, size_t id) ;
    DataWriter* getWriter() {
        return &db;
    }
    void serialize_and_index_into_secondary(const std::filesystem::path& path) {
        db.forloading.secondary_memory_index(path);
    }
private:
    PrimaryMemoryLoader db;
};

#endif //GSM2_RANDOMACCESSGSMREADER_H
