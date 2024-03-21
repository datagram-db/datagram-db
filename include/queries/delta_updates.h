/*
 * delta_updates.h
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
// Created by giacomo on 17/09/23.
//

#ifndef GSM2_DELTA_UPDATES_H
#define GSM2_DELTA_UPDATES_H

#include <database/utility.h>
#include <roaring64map.hh>
#include <database/gsm_inmemory_db.h>

/**
 * Represents the partial instantiation of the rewriting patterns, either removing some nodes, or adding/changing something
 */
struct delta_updates {
    // Storing the information pertaining to the newly-inserted or updated objects
    struct gsm_inmemory_db delta_plus_db;
    // Storing the replaced objects
    std::unordered_map<std::string, std::vector<size_t>> newly_inserted_vertices;
    roaring::Roaring64Map newIterationInsertedObjects;
    std::vector<size_t> no_inserted_node;
    // Storing all the nodes that are removed
    std::unordered_set<size_t> removed_objects;
    std::unordered_set<size_t> removed_edges;


    void clear_insertions();

    inline size_t getNovelInsertions() const {
        return std::max(delta_plus_db.max_id+1, no_inserted_node.size());
    }
    inline size_t getNodeRemovals() const {
        return removed_objects.size();
    }
    inline size_t getEdgeRemovals() const {
        return removed_edges.size();
    }

    inline const std::vector<size_t>& getNewlyInsertedVertices(const std::string&x) const {
        auto it = newly_inserted_vertices.find(x);
        if (it == newly_inserted_vertices.end())
            return no_inserted_node;
        else
            return it->second;
    }

    /**
     * class constructor
     * @param max_id        Id of the data existing
     */
    explicit delta_updates(size_t max_id);

    /**
     * Remove an object id
     * @param default_val
     */
    inline void set_removed(size_t default_val){
        size_t toRemove = getOrDefault(replacement_map, default_val, default_val);
//        if ((toRemove == 29) || (default_val == 29))
//            std::cerr << "EHRE" << std::endl;
        if (!newIterationInsertedObjects.contains(toRemove))
            removed_objects.insert(toRemove);
        else
            removed_objects.insert(default_val);
    }

    inline void edge_removed(size_t default_val) {
        removed_edges.insert(default_val);
    }

    /**
     * Determining that "dest" should replace "orig"
     * @param orig
     * @param dest
     */
    inline void replaceWith(size_t orig, size_t dest) {
        if (replacement_map.contains(orig))
            std::cerr << "WARNING!" << std::endl;
        replacement_map[orig] = dest;
    }
    /**
     * Associating a variable name to an object id
     * @param name
     * @param id
     */
    inline void associateNewToVar(const std::string& name, size_t id){
        newly_inserted_vertices[name].emplace_back(id);
        newIterationInsertedObjects.add(id);
    }
    /**
     * Generates a new object from the delta-update
     * @return
     */
    inline gsm_object& getNewObject(){
        delta_plus_db.max_id++;
        auto& obj = delta_plus_db.O[delta_plus_db.max_id];
        obj.id = delta_plus_db.max_id;
        return obj;
    }

    inline bool hasXBeenRemoved(size_t obj) const {
        return (!replacement_map.contains(obj)) && removed_objects.contains(obj);
    }

    void updateWith(const std::vector<delta_updates> &vector1);

    inline size_t replacedWith(size_t x) const {
        auto it = replacement_map.find(x);
        // empty set, add x to set
        std::unordered_set<size_t> replacement_set;
        replacement_set.insert(x);
        while (it != replacement_map.end()) {
            x = it->second;
            // add x to set, .insert returns bool if false then break
            if (!replacement_set.insert(x).second) {
                break;
            }
            it = replacement_map.find(x);
        }
        return x;
    }

private:
    std::unordered_map<size_t, size_t> replacement_map;
};


#endif //GSM2_DELTA_UPDATES_H
