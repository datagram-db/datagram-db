//
// Created by giacomo on 21/12/24.
//

#ifndef GSM2_SECONDARYMEMORYLOADER_H
#define GSM2_SECONDARYMEMORYLOADER_H

#include <parser/writer.h>
#include <fstream>
#include <database/LinearGSM.h>

struct PrimaryMemoryLoader : public DataWriter {

    void writeToPath(const std::string& path) override;
    void initDatabase() override;
    void writeObject(const gsm_object& object, const std::unordered_map<std::string, gsm2::tables::AttributeTableType>& propertyname_to_type) override;
    void close() override;
    DataReader* asReader(DataWriter*) override;

    bool start=true;
    gsm2::tables::LinearGSM forloading;
    size_t noLabel;
    std::pair<size_t,size_t> graphId_eventId{0,0};

    std::map<size_t, gsm_object> queued_objects;

    inline void deactivatePrimaryMemoryFutilities() {
        doPrimaryMemoryIndex = false;
        forloading.ells_and_xis_just_as_repositories();
    }

private:
    bool doPrimaryMemoryIndex = true;
    inline void actual_writeObject(const gsm_object& object, const std::unordered_map<std::string, gsm2::tables::AttributeTableType>& map_for_types) {
//        if (graphId_eventId.first > 0)
//            std::cerr << "WARNING: this is something should not happen when loading Schema" << std::endl;
        DEBUG_ASSERT(graphId_eventId.second == object.id);
        graphId_eventId.second = object.id;
        forloading.nodesInGraph[graphId_eventId.first] = std::max(forloading.nodesInGraph[graphId_eventId.first],object.id);
        size_t act_id = loadObjectEll2(forloading, object.ell, noLabel, graphId_eventId);
        registerObjectByFirstLabel(forloading, act_id, graphId_eventId);
        loadObjectXi2(forloading, object.xi, graphId_eventId);
        for (const auto& [k, v] : object.content) {
//            if (k == "length")
//                std::cerr << "HIS IS IT" << std::endl;
            if (std::holds_alternative<std::string>(v))
                loadObjectProperty(forloading, map_for_types, act_id, graphId_eventId, k, std::get<std::string>(v));
            else
                loadObjectProperty(forloading, map_for_types, act_id, graphId_eventId, k, std::get<double>(v));

        }
        for (const auto& [tmp, vaks] : object.phi) {
            for (const auto& val : vaks) {
                forloading.containment_tables[tmp].add(act_id, graphId_eventId, val.score, val.id);
                forloading.containment_relationships.emplace(tmp);
            }
        }
        while (forloading.objectScores.size() <= graphId_eventId.first)
            forloading.objectScores.emplace_back();
        auto& obj =  forloading.objectScores[graphId_eventId.first];
        while (obj.size() <= graphId_eventId.second)
            obj.emplace_back();
        obj[graphId_eventId.second].emplace_back(1.0);
        forloading.objectScoresLoading[graphId_eventId].emplace_back(1.0);
        graphId_eventId.second++;

    }

};

#endif //GSM2_PRIMARYMEMORYLOADER_H
