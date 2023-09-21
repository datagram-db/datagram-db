/*
 * gsm_object.cpp
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

#include <database/gsm_object.h>

gsm_object::gsm_object(uint_fast32_t id, const std::vector<std::string> &ell, const std::vector<std::string> &xi,
                       const std::vector<double> &scores,
                       const std::unordered_map<std::string, std::vector<gsm_object_xi_content>> &phi)
        : id(id), ell(ell), xi(xi), scores(scores), phi(phi) {}

void gsm_object::updateWith(const gsm_object& old) {
    if ((old.id != id) && (id == -1)) {
        id = old.id;
    }
    if (ell.empty())
        ell = old.ell;
    if (xi.empty())
        xi = old.xi;
    if (scores.empty())
        scores = old.scores;
    for (const auto& [oldKey, oldValue] : old.content) {
        content.emplace(oldKey, oldValue); // Inserting the old content only if something new is not available
    }
    for (const auto& [oldKey, oldValue] : old.phi) {
        remove_duplicates(phi.emplace(oldKey, oldValue).first->second); // Inserting the old content only if something new is not available
    }
}

bool gsm_object::operator==(const gsm_object &rhs) const {
    return id == rhs.id &&
           ell == rhs.ell &&
           xi == rhs.xi &&
           scores == rhs.scores &&
           phi == rhs.phi;
}

bool gsm_object::operator!=(const gsm_object &rhs) const {
    return !(rhs == *this);
}