/*
 * DataQuery.cpp
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
// Created by giacomo on 20/01/2022.
//

#include "queries/DataQuery.h"
#include "yaucl/functional/assert.h"
//
//DataQuery DataQuery::ExistsQuery(const std::string &atom, size_t narg, LeafType lt) {
//    DataQuery returnable;
//    DEBUG_ASSERT(!atom.empty());
//    returnable.label = atom;
//    returnable.type = DataQueryType::ExistsQuery;
//    returnable.t = lt;
//    returnable.numeric_argument = narg;
//    return returnable;
//}
//
//DataQuery DataQuery::AbsenceQuery(const std::string &atom, size_t narg,  LeafType lt) {
//    DataQuery returnable;
//    DEBUG_ASSERT(!atom.empty());
//    returnable.label = atom;
//    returnable.type = DataQueryType::AbsenceQuery;
//    returnable.numeric_argument = narg;
//    returnable.t = lt;
//    return returnable;
//}
//
//DataQuery DataQuery::AtomQueries (DataQueryType t, const std::string &atom, LeafType lt) {
//    DataQuery returnable;
//    DEBUG_ASSERT(!atom.empty());
//    returnable.label = atom;
//    returnable.type = t;
//    returnable.t = lt;
//    return returnable;
//}
//
//DataQuery DataQuery::InitQuery(const std::string &atom) {
//    DataQuery returnable;
//    DEBUG_ASSERT(!atom.empty());
//    returnable.label = atom;
//    returnable.type = DataQueryType::InitQuery;
//    returnable.t = NoneLeaf;
//    return returnable;
//}
//
//DataQuery DataQuery::AtomQuery(const std::string &atom, LeafType lt) {
//    DataQuery returnable;
//    DEBUG_ASSERT(!atom.empty());
//    returnable.label = atom;
//    returnable.type = DataQueryType::AtomQuery;
//    returnable.t = lt;
//    return returnable;
//}
//
//DataQuery DataQuery::EndsQuery(const std::string &atom) {
//    DataQuery returnable;
//    DEBUG_ASSERT(!atom.empty());
//    returnable.label = atom;
//    returnable.type = DataQueryType::EndsQuery;
//    returnable.t = NoneLeaf;
//    return returnable;
//}
//
//DataQuery DataQuery::FirstQuery(LeafType lt) {
//    DataQuery returnable;
//    returnable.type = DataQueryType::FirstQuery;
//    returnable.t = lt;
//    returnable.numeric_argument = 0;
//    return returnable;
//}
//
//DataQuery DataQuery::LastQuery(LeafType lt) {
//    DataQuery returnable;
//    returnable.type = DataQueryType::LastQuery;
//    returnable.t = lt;
//    returnable.numeric_argument = 0;
//    return returnable;
//}

DataQuery
DataQuery::RangeQuery(const std::string &atom, const std::string &var, const std::variant<std::string, double> &lb,
                      const std::variant<std::string, double> &ub, double score) {
    DataQuery returnable;
    DEBUG_ASSERT(!atom.empty());
    returnable.label = atom;
    DEBUG_ASSERT(!var.empty());
    returnable.var = var;
    returnable.lower_bound = lb;
    returnable.upper_bound = ub;
    returnable.score = score;
//    returnable.type = DataQueryType::RangeQuery;
//    returnable.t = lt;
    return returnable;
}

bool DataQuery::operator==(const DataQuery &rhs) const {
    return /*type == rhs.type &&*/
           label == rhs.label &&
           var == rhs.var &&
           lower_bound == rhs.lower_bound &&
           upper_bound == rhs.upper_bound &&
           score == rhs.score;
}

bool DataQuery::operator!=(const DataQuery &rhs) const {
    return !(rhs == *this);
}

bool DataQuery::operator<(const DataQuery &rhs) const {
    /*if (type < rhs.type)
        return true;
    if (rhs.type < type)
        return false;*/
    if (label < rhs.label)
        return true;
    if (rhs.label < label)
        return false;
    if (var < rhs.var)
        return true;
    if (rhs.var < var)
        return false;
    if (lower_bound < rhs.lower_bound)
        return true;
    if (rhs.lower_bound < lower_bound)
        return false;
    if (upper_bound < rhs.upper_bound)
        return true;
    if (rhs.upper_bound < upper_bound)
        return false;
    if (rhs.score < score)
        return false;
    return (score < rhs.score);
}

bool DataQuery::operator>(const DataQuery &rhs) const {
    return rhs < *this;
}

bool DataQuery::operator<=(const DataQuery &rhs) const {
    return !(rhs < *this);
}

bool DataQuery::operator>=(const DataQuery &rhs) const {
    return !(*this < rhs);
}
