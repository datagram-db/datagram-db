/*
 * gsm_object.h
 * This file is part of gsm_gsql
 *
 * Copyright (C) 2018-2023 - Giacomo Bergami
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
// Created by giacomo on 05/02/23.
//

#ifndef GSM_GSQL_GSM_OBJECT_H
#define GSM_GSQL_GSM_OBJECT_H

#include <iomanip>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "yaucl/structures/setoids/basics.h"
#include "queries/DataPredicate.h"

#include <database/gsm_object_xi_content.h>

struct delta_updates;

struct gsm_object  {
    // Object's id
    size_t                                                id=-1;
    // Object's labels/types
    std::vector<std::string>                              ell;
    // Object's values
    std::vector<std::string>          xi;
    // Object's uncertainty scores
    std::vector<double>                                   scores;
    // TODO: Provenance
    // Object's containment
    std::unordered_map<std::string, std::vector<gsm_object_xi_content>> phi;
    std::unordered_map<std::string, union_minimal> content;

    inline void clear() {
        phi.clear();
        content.clear();
        ell.clear();
        xi.clear();
    }

    /**
     * Updating the current object with the values from the old one, if it does not exist (rough implementation,
     * under the assumption that new data is merely inserted and never removed: this should be changed if we also
     * postulate to remove information and we extend the language)
     * @param old    Old object
     */
    void updateWith(const gsm_object& old);

    void out_json(std::ostream &os, bool inserted  = false) const;

    gsm_object() {}
    gsm_object(uint_fast32_t id, const std::vector<std::string> &ell = {}, const std::vector<std::string> &xi = {},
               const std::vector<double> &scores = {}, const std::unordered_map<std::string, std::vector<gsm_object_xi_content>> &phi = {}, const std::unordered_map<std::string, union_minimal> &content = {});
    gsm_object(const gsm_object&) = default;
    gsm_object(gsm_object&&) = default;
    gsm_object& operator=(const gsm_object&) = default;
    gsm_object& operator=(gsm_object&&) = default;
    bool operator==(const gsm_object &rhs) const;
    bool operator!=(const gsm_object &rhs) const;
};

struct lightweight_gsm_object  {
    std::vector<std::string>                              ell;
    std::vector<std::string>                              xi;
    std::unordered_map<std::string, std::vector<size_t>>  phi;
    std::unordered_map<std::string, union_minimal>        pi;

    inline void clear() {
        phi.clear();
        pi.clear();
        ell.clear();
        xi.clear();
    }

    lightweight_gsm_object() {}
    lightweight_gsm_object(const lightweight_gsm_object&) = default;
    lightweight_gsm_object(lightweight_gsm_object&&) = default;
    lightweight_gsm_object& operator=(const lightweight_gsm_object&) = default;
    lightweight_gsm_object& operator=(lightweight_gsm_object&&) = default;
    bool operator==(const lightweight_gsm_object &rhs) const;
    bool operator!=(const lightweight_gsm_object &rhs) const;
};

#endif //GSM_GSQL_GSM_OBJECT_H
