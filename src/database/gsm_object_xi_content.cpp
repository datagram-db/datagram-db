/*
 * gsm_object_xi_content.cpp
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

#include "database/gsm_object_xi_content.h"

gsm_object_xi_content::gsm_object_xi_content(size_t id, double score, size_t orig_edge_id, const std::map<std::string, union_minimal>& property_values) : id(id), score(score), orig_edge_id(orig_edge_id), property_values(property_values) {}

bool gsm_object_xi_content::operator==(const gsm_object_xi_content &rhs) const {
    return id == rhs.id && score == rhs.score && property_values == rhs.property_values;
}

bool gsm_object_xi_content::operator!=(const gsm_object_xi_content &rhs) const {
    return !(rhs == *this);
}
