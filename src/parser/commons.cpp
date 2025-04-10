//
// Created by giacomo on 22/12/24.
//

#include <parser/commons.h>
#include <iostream>
#include "parser/schema/DataLoaderFromSchema.h"
#include "parser/readers/GSMReader.h"
#include "parser/readers/BulkReader.h"
#include "parser/writers/PrimaryMemoryLoader.h"
#include "parser/writers/GSMWriter.h"
#include "parser/writers/BulkWriter.h"
#include "parser/writers/CallBackWriter.h"

static inline
void
loadInput(const DataFormat &input, std::pair<DataReader *, DataWriter *> &cp)  {
    switch (input) {
        case NoDataFormat:
            cp.first = nullptr;
            break;
        case PrimaryMemoryDB:
            std::cerr << "ERROR: cannot read from a path" << std::endl;
            cp.first = nullptr;
            break;
        case Schema:
            cp.first = new SchemaReader();
            break;
        case GSM:
            cp.first = new GSMReader();
            break;
        case BulkFolder:
            cp.first = new BulkReader();
            break;
    }
}

static inline
bool runReadWrite(const std::string &inPath, const std::string &outPath,
                                     std::pair<DataReader *, DataWriter *> &cp)  {
    bool status = false;
    if (cp.second) {
        cp.second->writeToPath(outPath);
        if (cp.first) {
            cp.first->setWriter(cp.second);
        }
    }
    if ((cp.first) && (cp.second)) {
        if (cp.first->readFromPath(inPath)) {
            status = true;
        }
    }
    return status;
}


bool DataFormatHandler::data_converter(const std::string& inPath,
                                       DataFormat input,
                                       const std::string& outPath,
                                       DataFormat output) {
    std::pair<DataReader*, DataWriter*> cp{nullptr, nullptr};
    loadInput(input, cp);

    switch (output) {
        case NoDataFormat:
        case PrimaryMemoryDB:
            cp.second = nullptr;
            break;
        case Schema:
            std::cerr << "ERROR: cannot write to a schema (yet)!" << std::endl;
            cp.second = nullptr;
            break;
        case GSM:
            cp.second = new GSMWriter();
            break;
        case BulkFolder:
            cp.second = new BulkWriter();
            break;
    }

    auto status = runReadWrite(inPath, outPath, cp);
    delete cp.first;
    delete cp.second;
    return status;
}



bool DataFormatHandler::callback_reader(const std::string& inPath,
                     DataFormat input,
                     std::function<void(const gsm_object&)> callback) {
    std::pair<DataReader*, DataWriter*> cp{nullptr, nullptr};
    loadInput(input, cp);
    if (!cp.first) {
        return false;
    }
    cp.second = new CallBackWriter(callback);
    cp.first->setWriter(cp.second);
    auto status = cp.first->readFromPath(inPath);
    delete cp.first;
    delete cp.second;
    return status;
}

bool DataFormatHandler::open_data_writer(const std::string& outPath,
                       DataFormat output) {
    if (writer)
        return false;
    switch (output) {
        case NoDataFormat:
            writer = nullptr;
            break;
        case PrimaryMemoryDB:
            writer = new PrimaryMemoryLoader();
            break;
        case Schema:
            std::cerr << "ERROR: cannot write to a schema (yet)!" << std::endl;
            writer = nullptr;
            break;
        case GSM:
            writer = new GSMWriter();
            break;
        case BulkFolder:
            writer = new BulkWriter();
            break;
    }
    if (writer) {
        writer->writeToPath(outPath);
    }

    return (writer);
}

#include <parser/ra_readers/RandomAccessBulkReader.h>
#include <parser/ra_readers/RandomAccessGSMReader.h>

RandomAccessBulkReader DataFormatHandler::read_from_bulk_data(const std::string& path) {
    return RandomAccessBulkReader{path};
}


void DataFormatHandler::load_to_primary_memory(const std::string& inPath,
                                                           DataFormat input) {
    std::pair<DataReader*, DataWriter*> cp{nullptr, nullptr};
    cp.second = this->lineargsm.getWriter();
    loadInput(input, cp);
    if (!cp.first) {
        init=false;
        return;
    }
    cp.first->setWriter(cp.second);
    if (!cp.first->readFromPath(inPath)) {
        delete cp.first;
        init=false;
        return;
    }
    init=true;
}


void DataFormatHandler::load_to_secondary_memory(const std::string& inPath,
                                               const std::string& outFolder,
                                               DataFormat input) {
    std::pair<DataReader*, DataWriter*> cp{nullptr, nullptr};
    cp.second = this->lineargsm.getWriter();
    ((PrimaryMemoryLoader*)cp.second)->deactivatePrimaryMemoryFutilities();

    loadInput(input, cp);
    if (!cp.first) {
        init=false;
        return;
    }
    cp.first->setWriter(cp.second);
    if (!cp.first->readFromPath(inPath)) {
        delete cp.first;
        init=false;
        return;
    }
    ((PrimaryMemoryLoader*)cp.second)->forloading.secondary_memory_index(outFolder);
    init=true;
}