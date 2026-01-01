/*
 * DataPredicate.cpp
 * This file is part of bpm21
 *
 * Copyright (C) 2021 - Giacomo Bergami
 *
 * bpm21 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * bpm21 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *0
 * You should have received a copy of the GNU General Public License
 * along with bpm21. If not, see <http://www.gnu.org/licenses/>.
 */
//
// Created by giacomo on 10/03/21.
//

#include <iomanip>
#include "queries/DataPredicate.h"

HCQSingleQuery HCQSingleQuery::TRUTH{"Truth"};
HCQSingleQuery HCQSingleQuery::FALSEHOOD{"Falsehood"};

numeric_atom_cases invert_predicate_direction(numeric_atom_cases val) {
    switch (val) {
        case LT:
            return GT;
        case GT:
            return LT;
        case LEQ:
            return GEQ;
        case GEQ:
            return LEQ;
        case EQ:
        case NEQ:
        case TTRUE:
            return val;
        default:
            throw std::runtime_error("ERROR: you cannot invert a relationship which is just an interval! An interval is not a valid predicate, rather than a combination of intervals. There is some flaw in the logic!");
    }
}

double      DataPredicate::MIN_DOUBLE = -std::numeric_limits<double>::max();
double      DataPredicate::MAX_DOUBLE =  std::numeric_limits<double>::max();
std::string DataPredicate::MIN_STRING = "";
std::string DataPredicate::MAX_STRING =  std::string(MAXIMUM_STRING_LENGTH, std::numeric_limits<char>::max());
size_t      DataPredicate::msl = MAXIMUM_STRING_LENGTH;

DataPredicate::DataPredicate(numeric_atom_cases x) : casusu{x}, wasReversed{false} {}


DataPredicate::DataPredicate(const std::string &var, numeric_atom_cases casusu, const union_minimal &value, const std::string &label) : var(
        var), casusu(casusu), value(value), wasReversed{false}, label(label)  {}

DataPredicate::DataPredicate(const std::string &var, numeric_atom_cases casusu, const std::string &value, const std::string &label) : var(var), label(label) , wasReversed{false} , casusu(casusu), value(value) {}

DataPredicate::DataPredicate(const std::string &var, numeric_atom_cases casusu, const double &value, const std::string &label) : var(var), label(label), wasReversed{false} , casusu(casusu), value(value) {}


#include <cassert>
#include <yaucl/functional/assert.h>
#include <iostream>

std::ostream &operator<<(std::ostream &os, const DataPredicate &predicate) {
    if ((predicate.casusu == TTRUE) ) {
        if (predicate.BiVariableConditions.empty())
            os << "true";
    } else if ((predicate.casusu == FFALSE) ) {
        if (predicate.BiVariableConditions.empty())
            os << "false";
    }else if (predicate.casusu == INTERVAL) {
        double isString = std::holds_alternative<std::string>(predicate.value);
        DEBUG_ASSERT(predicate.varRHS.empty() && predicate.labelRHS.empty());

        if (isString)
            os << std::quoted(std::get<std::string>(predicate.value).c_str());
        else
            os << std::get<double>(predicate.value);
        os << " ≤ ";
        if (!predicate.label.empty())
            os <<  predicate.label << '.';
        os << predicate.var << " ≤ ";
        if (isString)
            os << std::quoted(std::get<std::string>(predicate.value_upper_bound).c_str());
        else
            os << std::get<double>(predicate.value_upper_bound);
        if (!predicate.exceptions.empty()) {
            os << "\\ {";
            size_t N = predicate.exceptions.size();
            size_t i = 0;
            for (const auto& x : predicate.exceptions) {
                if (isString)
                    os << std::quoted(std::get<std::string>(x).c_str());
                else
                    os << std::get<double>(x);
                if (i != (N-1)) {
                    os << ", ";
                    i++;
                }
            }
            os << "}";
        }
        return os;
    } else {
        if (!predicate.label.empty())
            os  << predicate.label << '.';
        os << predicate.var;
        switch (predicate.casusu) {
            case LT:
                os << " < "; break;
            case GT:
                os << " > "; break;
            case LEQ:
                os << " <= "; break;
            case GEQ:
                os << " >= "; break;
            case EQ:
                os << " = "; break;
            case NEQ:
                os << " != "; break;
            default:
                std::cerr << "ERROR: unexpected case" << std::endl;
                exit(1);
        }
        if (!predicate.varRHS.empty())
            os << predicate.labelRHS << '.' << predicate.varRHS;
        else if (std::holds_alternative<std::string>(predicate.value))
            os << std::quoted(std::get<std::string>(predicate.value).c_str());
        else
            os << std::get<double>(predicate.value);
    }
    if (!predicate.BiVariableConditions.empty()) {
        static std::string AND{" ∧ "};
        if (predicate.casusu != TTRUE) os << AND << '(';
        size_t i = 0, N = predicate.BiVariableConditions.size();
        for (const DataPredicate& elem : predicate.BiVariableConditions) {
            os << elem;
            if (i != (N-1)) {
                os << AND;
            }
            i++;
        }
        if (predicate.casusu != TTRUE) os << ')';
    }
    return os;
}

std::string prev_char(const std::string &val, size_t max_size) {

    static const char MIN_CHAR = static_cast<char>(std::numeric_limits<unsigned char>::min()+1);
    static const char MAX_CHAR = static_cast<char>(std::numeric_limits<unsigned char>::max());
    if (val.empty()) return val;

    std::string result = val;
    size_t idx = val.size()-1;
    char last_char = val.at(idx);
    if (last_char == MIN_CHAR) {
        result.pop_back();
    } else {
        result[val.size()-1] = last_char-1;
        result += std::string(max_size - result.size(), MAX_CHAR);
    }
    return result;
}

std::string prev_printable_char(const std::string &val, size_t max_size) {
    static const char MIN_CHAR = static_cast<char>(33);
    static const char MAX_CHAR = static_cast<char>(126);
    if (val.empty()) return val;

    std::string result = val;
    size_t idx = val.size()-1;
    char last_char = val.at(idx);
    if (last_char == MIN_CHAR) {
        result.pop_back();
    } else {
        result[val.size()-1] = last_char-1;
        result += std::string(max_size - result.size(), MAX_CHAR);
    }
    return result;
}

std::string next_printable_char(const std::string &val, size_t max_size) {
    static const char MIN_CHAR = static_cast<char>(33);
    static const char MAX_CHAR = static_cast<char>(126);
    if (val == DataPredicate::MAX_STRING) return val;

    std::string next = val;
    size_t idx = next.size();
    if (idx < max_size) {
        next += MIN_CHAR;
    } else {
        idx--;
        if (next.at(idx) != MAX_CHAR) {
            next[idx]++;
            return next;
        } else {
            while (!next.empty()) {
                if (next.at(idx) != MAX_CHAR) {
                    next[idx]++;
                    return next;
                } else {
                    next.pop_back();
                    idx--;
                }
            }
            if (next.empty())
                return DataPredicate::MAX_STRING;
        }

    }
    return next;
}

std::string next_char(const std::string &val, size_t max_size) {
    static const char MIN_CHAR = static_cast<char>(std::numeric_limits<unsigned char>::min()+1);
    static const char MAX_CHAR = static_cast<char>(std::numeric_limits<unsigned char>::max());
    if (val == DataPredicate::MAX_STRING) return val;

    std::string next = val;
    size_t idx = next.size();
    if (idx < max_size) {
        next += MIN_CHAR;
    } else {
        idx--;
        if (next.at(idx) != MAX_CHAR) {
            next[idx]++;
            return next;
        } else {
            while (!next.empty()) {
                if (next.at(idx) != MAX_CHAR) {
                    next[idx]++;
                    return next;
                } else {
                    next.pop_back();
                    idx--;
                }
            }
            if (next.empty())
                return DataPredicate::MAX_STRING;
        }

    }
    return next;
}

std::vector<std::vector<DataPredicate>> flip(const std::vector<std::vector<DataPredicate>> &vv) {
    std::vector<std::vector<DataPredicate>> result(vv.size(), std::vector<DataPredicate>{});
    for (size_t i = 0, N = vv.size(); i<N; i++) {
        auto& ref = result[i];
        const auto& Mv = vv.at(i);
        size_t M = Mv.size();
        ref.reserve(M);
        for (size_t j = 0; j<M; j++) {
            ref.emplace_back(Mv.at(j).reverseBiVariablePredicate());
        }
    }
    return result;
}


#include <cassert>
bool DataPredicate::intersect_with(const DataPredicate& predicate) {
    if (predicate.casusu == TTRUE)
        return true;
    if (predicate.casusu == FFALSE)
        return false;
    if ((casusu == TTRUE) && (!predicate.isBiVariableCondition())) {
        *this = predicate;
        return true;
    }
    DEBUG_ASSERT(var == predicate.var && label == predicate.label);
    if (predicate.isBiVariableCondition()) {
        BiVariableConditions.emplace_back(predicate);
        return true;
    } else {
        if (casusu == predicate.casusu) {
            switch (casusu) {
                case LT:
                case LEQ:
                    value = std::min(value, predicate.value);
                    break;

                case GT:
                case GEQ:
                    value = std::max(value, predicate.value);
                    break;

                case EQ:
                    // The intersection of equality should consider that the equivalence is among identical values
                    DEBUG_ASSERT(value == predicate.value);
                    break;

                case NEQ:
                    casusu = INTERVAL;
                    exceptions.insert(value);
                    exceptions.insert(predicate.value);
                    break;

                case INTERVAL:// (1)
                {
                    value = std::max(value, predicate.value);
                    value_upper_bound = std::min(value_upper_bound, predicate.value_upper_bound);
                    if (value > value_upper_bound) {
                        return false;
                    }

                    std::set<union_minimal> S;
                    for (const auto& x : exceptions) {
                        if ((value <= x) && (x <= value_upper_bound))
                            S.insert(x);
                    }
                    for (const auto& x : predicate.exceptions) {
                        if ((value <= x) && (x <= value_upper_bound))
                            S.insert(x);
                    }
                    exceptions = S;
                }
                    break;

                default:
                    std::cerr << "ERROR: unexpected case" << std::endl;
                    exit(1);
            }
            return true;
        } else {
            asInterval();
            DataPredicate rightCopy = predicate;
            rightCopy.asInterval();
            return intersect_with(rightCopy); // (1)
        }
    }
}

#include <iostream>

void DataPredicate::asInterval() {
    if ((casusu == INTERVAL) && (value > value_upper_bound)) {
        std::cerr << "Critical issue" << std::endl;
    }
    if ((casusu == INTERVAL) || (casusu == TTRUE)  || (casusu == FFALSE) || (isBiVariableCondition())) return;

    bool isString = std::holds_alternative<std::string>(value);
    union_minimal prev, next;
    union_minimal min, max;
    if (isString)  {
        std::string s = std::get<std::string>(value);
        min = MIN_STRING;
        max = MAX_STRING;
        prev = PREV_STRING(s);
        next = NEXT_STRING(s);
    } else {
        double x_val = std::get<double>(value);
        min = MIN_DOUBLE;
        max = MAX_DOUBLE;
        prev = PREV_DOUBLE(x_val);
        next = NEXT_DOUBLE(x_val);
    }

    switch (casusu) {
        case LT:
            value = min;
            value_upper_bound = prev;
            break;
        case GT:
            value = next;
            value_upper_bound = max;
            break;
        case LEQ:
            value_upper_bound = value;
            value = min;
            break;
        case GEQ:
            value_upper_bound = max;
            break;
        case EQ:
            value_upper_bound = value;
            break;
        case NEQ:
            exceptions.insert(value);
            value = min;
            value_upper_bound = max;
            break;

        default:
            std::cerr << "ERROR: unexpected case" << std::endl;
            exit(1);
    }
    if (value > value_upper_bound) {
        std::cerr << "Critical issue" << std::endl;
    }

    casusu = INTERVAL;
}

bool DataPredicate::testOverSingleVariable(const std::string &val) const {
    DEBUG_ASSERT(!isBiVariableCondition());
    bool isString = std::holds_alternative<std::string>(value);
    if (!isString) return false;
    std::string current = std::get<std::string>(value);
    switch (casusu) {
        case TTRUE:
            return true;
        case FFALSE:
            return false;
        case LT:
            return val < current;
        case GT:
            return val > current;
        case LEQ:
            return val <= current;
        case GEQ:
            return val >= current;
        case EQ:
            return val == current;
        case NEQ:
            return val != current;
        case INTERVAL: {
            std::variant<std::string,double> curr{current};
            if (exceptions.contains(curr)) return false;
            return ((current <= val)
                    && (val <= std::get<std::string>(value_upper_bound)));
        }
        default:
            throw std::runtime_error("UNEXPECTED CASE!");
    }
}

bool DataPredicate::testOverSingleVariable(double val) const {
    DEBUG_ASSERT(!isBiVariableCondition());
    bool isDouble = std::holds_alternative<double>(value);
    if (!isDouble) return false;
    double current = std::get<double>(value);
    switch (casusu) {
        case TTRUE:
            return true;
        case FFALSE:
            return false;
        case LT:
            return val < current;
        case GT:
            return val > current;
        case LEQ:
            return val <= current;
        case GEQ:
            return val >= current;
        case EQ:
            return val == current;
        case NEQ:
            return val != current;
        case INTERVAL: {
            std::variant<std::string,double> curr{current};
            if (exceptions.contains(curr)) return false;
            return ((current <= val)
                    && (val <= std::get<double>(value_upper_bound)));
        }
        default:
            throw std::runtime_error("UNEXPECTED CASE!");
    }
}

bool DataPredicate::operator==(const DataPredicate &rhs) const {
    bool tmp= label == rhs.label &&
              var == rhs.var &&
              casusu == rhs.casusu &&
              value == rhs.value &&
              value_upper_bound == rhs.value_upper_bound &&
              labelRHS == rhs.labelRHS &&
              varRHS == rhs.varRHS &&
              exceptions == rhs.exceptions &&
              BiVariableConditions == rhs.BiVariableConditions;
    return tmp;
}

bool DataPredicate::operator!=(const DataPredicate &rhs) const {
    return !(rhs == *this);
}

union_minimal DataPredicate::prev_of(const union_minimal &x) {
    if (std::holds_alternative<std::string>(x))
        return {PREV_STRING(std::get<std::string>(x))};
    else
        return {PREV_DOUBLE(std::get<double>(x))};
}

union_minimal DataPredicate::next_of(const union_minimal &x) {
    if (std::holds_alternative<std::string>(x))
        return {NEXT_STRING(std::get<std::string>(x))};
    else
        return {NEXT_DOUBLE(std::get<double>(x))};
}

#include <cassert>

bool DataPredicate::isStringPredicate() const {
    if (casusu == TTRUE)
        return false;
    if (casusu ==FFALSE)
        return false;
    bool isString = std::holds_alternative<std::string>(value);
    if (casusu == INTERVAL)
        DEBUG_ASSERT(isString == std::holds_alternative<std::string>(value_upper_bound));
    return isString;
}

bool DataPredicate::isDoublePredicate() const {
    if (casusu == TTRUE)
        return false;
    if (casusu ==FFALSE)
        return false;
    bool isString = std::holds_alternative<double>(value);
    if (casusu == INTERVAL)
        DEBUG_ASSERT(isString == std::holds_alternative<double>(value_upper_bound));
    return isString;
}


std::variant<std::vector<std::pair<std::string, std::string>>,
        std::vector<std::pair<double, double>>> DataPredicate::decompose_single_variable_into_intervals() const {
    DEBUG_ASSERT(!isBiVariableCondition());
    bool isString = isStringPredicate();
    std::variant<std::vector<std::pair<std::string, std::string>>,
            std::vector<std::pair<double, double>>> result;
    if (isString) {
        result = std::vector<std::pair<std::string, std::string>>{};
    } else {
        result = std::vector<std::pair<double, double>>{};
    }

    union_minimal prev, next;
    union_minimal min, max;
    if (isString)  {
        std::string s = std::get<std::string>(value);
        min = MIN_STRING;
        max = MAX_STRING;
        prev = PREV_STRING(s);
        next = NEXT_STRING(s);
    } else {
        double x_val = std::get<double>(value);
        min = MIN_DOUBLE;
        max = MAX_DOUBLE;
        prev = PREV_DOUBLE(x_val);
        next = NEXT_DOUBLE(x_val);
    }

    switch (casusu) {
        case LT:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(min), std::get<0>(prev));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(min), std::get<1>(prev));
            }
            return result;

        case GT:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(next), std::get<0>(max));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(next), std::get<1>(max));
            }
            return result;

        case LEQ:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(min), std::get<0>(value));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(min), std::get<1>(value));
            }
            return result;

        case GEQ:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(value), std::get<0>(max));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(value), std::get<1>(max));
            }
            return result;

        case EQ:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(value), std::get<0>(value));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(value), std::get<1>(value));
            }
            return result;

        case NEQ:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(min), std::get<0>(prev));
                std::get<0>(result).emplace_back(std::get<0>(next), std::get<0>(max));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(min), std::get<1>(prev));
                std::get<1>(result).emplace_back(std::get<1>(next), std::get<1>(max));
            }
            return result;

        case INTERVAL:
            if (exceptions.empty()) {
                if (isString) {
                    std::get<0>(result).emplace_back(std::get<0>(value), std::get<0>(value_upper_bound));
                } else {
                    std::get<1>(result).emplace_back(std::get<1>(value), std::get<1>(value_upper_bound));
                }
            } else {
                size_t i = 0, N = exceptions.size();
                union_minimal prev = value;
                for (const auto& val : exceptions) {
                    if (i == 0) {
                        if (isString)
                            std::get<0>(result).emplace_back(std::get<0>(value), PREV_STRING(std::get<0>(val)));
                        else
                            std::get<1>(result).emplace_back(std::get<1>(value), PREV_DOUBLE(std::get<1>(val)));
                    } /*else if (i == (N-1)) {

                    }*/ else {
                        if (isString)
                            std::get<0>(result).emplace_back(PREV_STRING(std::get<0>(prev)), PREV_STRING(std::get<0>(val)));
                        else
                            std::get<1>(result).emplace_back(PREV_DOUBLE(std::get<1>(prev)), PREV_DOUBLE(std::get<1>(val)));
                    }
                    prev = val;
                    i++;
                }
                if (isString)
                    std::get<0>(result).emplace_back(PREV_STRING(std::get<0>(prev)), (std::get<0>(value_upper_bound)));
                else
                    std::get<1>(result).emplace_back(PREV_DOUBLE(std::get<1>(prev)), (std::get<1>(value_upper_bound)));
            }
            return result;
        default:
            throw std::runtime_error("UNEXPECTED CASE!");
    }
}

DataPredicate::DataPredicate(const std::string &label, const std::string &var, double lb, double ub) : label{label}, wasReversed{false} , var{var}, casusu{INTERVAL} {
    value = lb;
    value_upper_bound = ub;
}

DataPredicate::DataPredicate(const std::string &label, const std::string &var, const std::string &lb,
                             const std::string &ub) : label{label}, wasReversed{false} , var{var}, casusu{INTERVAL} {
    value = lb;
    value_upper_bound = ub;
}

std::variant<std::vector<std::pair<std::string, std::string>>,
        std::vector<std::pair<double, double>>> DataPredicate::decompose_single_variable_into_intervals_with_missing() const {
    DEBUG_ASSERT(!isBiVariableCondition());
    bool isString = isStringPredicate();
    std::variant<std::vector<std::pair<std::string, std::string>>,
            std::vector<std::pair<double, double>>> result;
    if (isString) {
        result = std::vector<std::pair<std::string, std::string>>{};
    } else {
        result = std::vector<std::pair<double, double>>{};
    }

    union_minimal prev, next;
    union_minimal min, max;
    if (isString)  {
        std::string s = std::get<std::string>(value);
        min = MIN_STRING;
        max = MAX_STRING;
        prev = PREV_STRING(s);
        next = NEXT_STRING(s);
    } else {
        double x_val = std::get<double>(value);
        min = MIN_DOUBLE;
        max = MAX_DOUBLE;
        prev = PREV_DOUBLE(x_val);
        next = NEXT_DOUBLE(x_val);
    }

    switch (casusu) {
        case LT:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(min), std::get<0>(prev));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(min), std::get<1>(prev));
            }
            return result;

        case GT:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(next), std::get<0>(max));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(next), std::get<1>(max));
            }
            return result;

        case LEQ:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(min), std::get<0>(value));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(min), std::get<1>(value));
            }
            return result;

        case GEQ:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(value), std::get<0>(max));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(value), std::get<1>(max));
            }
            return result;

        case EQ:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(min), std::get<0>(prev));
                std::get<0>(result).emplace_back(std::get<0>(next), std::get<0>(max));
                std::get<0>(result).emplace_back(std::get<0>(value), std::get<0>(value));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(min), std::get<1>(prev));
                std::get<1>(result).emplace_back(std::get<1>(next), std::get<1>(max));
                std::get<1>(result).emplace_back(std::get<1>(value), std::get<1>(value));
            }
            return result;

        case NEQ:
            if (isString) {
                std::get<0>(result).emplace_back(std::get<0>(min), std::get<0>(prev));
                std::get<0>(result).emplace_back(std::get<0>(next), std::get<0>(max));
                std::get<0>(result).emplace_back(std::get<0>(value), std::get<0>(value));
            } else {
                std::get<1>(result).emplace_back(std::get<1>(min), std::get<1>(prev));
                std::get<1>(result).emplace_back(std::get<1>(next), std::get<1>(max));
                std::get<1>(result).emplace_back(std::get<1>(value), std::get<1>(value));
            }
            return result;

        case INTERVAL:
            if (exceptions.empty()) {
                if (isString) {
                    std::get<0>(result).emplace_back(std::get<0>(value), std::get<0>(value_upper_bound));
                } else {
                    std::get<1>(result).emplace_back(std::get<1>(value), std::get<1>(value_upper_bound));
                }
            } else {
                size_t i = 0, N = exceptions.size();
                union_minimal prev = value;
                for (const auto& val : exceptions) {
                    if (i == 0) {
                        if (isString)
                            std::get<0>(result).emplace_back(std::get<0>(value), PREV_STRING(std::get<0>(val)));
                        else
                            std::get<1>(result).emplace_back(std::get<1>(value), PREV_DOUBLE(std::get<1>(val)));
                    } else if (i == (N-1)) {
                        if (isString)
                            std::get<0>(result).emplace_back(PREV_STRING(std::get<0>(prev)), (std::get<0>(value_upper_bound)));
                        else
                            std::get<1>(result).emplace_back(PREV_DOUBLE(std::get<1>(prev)), (std::get<1>(value_upper_bound)));
                    } else {
                        if (isString)
                            std::get<0>(result).emplace_back(PREV_STRING(std::get<0>(prev)), PREV_STRING(std::get<0>(val)));
                        else
                            std::get<1>(result).emplace_back(PREV_DOUBLE(std::get<1>(prev)), PREV_DOUBLE(std::get<1>(val)));
                    }
                    prev = val;
                    i++;
                }
            }
            return result;
        default:
            throw std::runtime_error("UNEXPECTED CASE!");
    }
}

bool DataPredicate::isBiVariableCondition() const {
    return !varRHS.empty();
}

DataPredicate::DataPredicate(const std::string &label, const std::string &var, union_minimal lb, union_minimal ub) : label{label}, wasReversed{false} , var{var}, casusu{INTERVAL} {
    value = lb;
    value_upper_bound = ub;
}

DataPredicate DataPredicate::instantiateRHSWith(const union_minimal& val) const {
    // This RHS instantiation is not applicable to intervals, which have no RHS variable
    DEBUG_ASSERT(casusu != INTERVAL);
    // In order to run this method, I shall have the rhs variable
    DEBUG_ASSERT(!varRHS.empty());
    DataPredicate cpy = *this;
    cpy.varRHS.clear();
    cpy.labelRHS.clear();
    cpy.value = val;
    return cpy;
}

DataPredicate DataPredicate::reverseBiVariablePredicate() const {
    DEBUG_ASSERT(casusu != INTERVAL);
    DEBUG_ASSERT(!varRHS.empty());
    DataPredicate cpy = *this;
    std::swap(cpy.var, cpy.varRHS);
    std::swap(cpy.label, cpy.labelRHS);
    cpy.casusu = invert_predicate_direction(cpy.casusu);
    cpy.wasReversed = !wasReversed;
    return cpy;
}

bool DataPredicate::operator<(const DataPredicate &rhs) const {
    if (label < rhs.label)
        return true;
    if (rhs.label < label)
        return false;
    if (var < rhs.var)
        return true;
    if (rhs.var < var)
        return false;
    if (casusu < rhs.casusu)
        return true;
    if (rhs.casusu < casusu)
        return false;
    if (value < rhs.value)
        return true;
    if (rhs.value < value)
        return false;
    if (labelRHS < rhs.labelRHS)
        return true;
    if (rhs.labelRHS < labelRHS)
        return false;
    if (varRHS < rhs.varRHS)
        return true;
    if (rhs.varRHS < varRHS)
        return false;
    return value_upper_bound < rhs.value_upper_bound;
}

bool DataPredicate::operator>(const DataPredicate &rhs) const {
    return rhs < *this;
}

bool DataPredicate::operator<=(const DataPredicate &rhs) const {
    return !(rhs < *this);
}

bool DataPredicate::operator>=(const DataPredicate &rhs) const {
    return !(*this < rhs);
}

DataPredicate DataPredicate::flip() const {
    if (!varRHS.empty()) {
        DataPredicate result;
        result.var = varRHS;
        result.varRHS = var;
        result.casusu = casusu;
        result.label = labelRHS;
        result.labelRHS = label;
        return result;
    } else {
        DataPredicate result = *this;
        result.is_left_for_activation = !is_left_for_activation;
        return result;
    }
}


#include <magic_enum/magic_enum.hpp>


nlohmann::json HCQSingleQuery::asJson() const {
    nlohmann::json data;
    data["name"] = template_or_act_name;
    std::stringstream ss;
    ss << dnfPredicates;
    data["pred"] = ss.str();
    return data;
}

nlohmann::json DataPredicate::asJson() const {
    nlohmann::json data;
    if (!label.empty()) data["label"] = label;
    if (!var.empty()) data["var"] = var;
    data["casusu"] = magic_enum::enum_name(casusu);
    if (!varRHS.empty()) {
        data["labelRHS"] = labelRHS;
        data["varRHS"] = varRHS;
    } else {
        if (std::holds_alternative<std::string>(value)) {
            data["value"] = std::get<std::string>(value);
        } else {
            data["value"] = std::get<double>(value);
        }
        if (casusu == INTERVAL) {
            if (std::holds_alternative<std::string>(value_upper_bound)) {
                data["value"] = std::get<std::string>(value_upper_bound);
            } else {
                data["value"] = std::get<double>(value_upper_bound);
            }
        }
    }
    if (!exceptions.empty()) {
        auto& item = data["exceptions"];
        for (const auto& value : exceptions) {
            if (std::holds_alternative<std::string>(value)) {
                item.emplace_back( std::get<std::string>(value));
            } else {
                item.emplace_back( std::get<double>(value));
            }
        }
    }
    return data;
}

bool DataPredicate::testOverSingleVariable(const std::variant<std::string, double> &val) const {
    return std::holds_alternative<std::string>(val) ?
           testOverSingleVariable(std::get<std::string>(val)) :
           testOverSingleVariable(std::get<double>(val));
}

HCQSingleQuery::HCQSingleQuery(const std::string &templateName,
                               const std::vector<std::vector<DataPredicate>> &dnfPredicates)
        : template_or_act_name(templateName), dnfPredicates(dnfPredicates) {}

bool HCQSingleQuery::operator==(const HCQSingleQuery &rhs) const {
    return template_or_act_name == rhs.template_or_act_name &&
           dnfPredicates == rhs.dnfPredicates;
}

bool HCQSingleQuery::operator!=(const HCQSingleQuery &rhs) const {
    return !(rhs == *this);
}

std::ostream &operator<<(std::ostream &os, const HCQSingleQuery &query) {
    return os << query.template_or_act_name << ":" << query.dnfPredicates;
}

DNFPredicates::DNFPredicates(const std::vector<std::vector<DataPredicate>> &x) : dnfPredicates(x) {}

DNFPredicates::DNFPredicates(std::vector<std::vector<DataPredicate>> &&x) : dnfPredicates(x) {}

DNFPredicates &DNFPredicates::operator=(const std::vector<std::vector<DataPredicate>> &x) {
    dnfPredicates = x;
    return *this;
}

DNFPredicates &DNFPredicates::operator=(std::vector<std::vector<DataPredicate>> &&x) {
    dnfPredicates = std::move(x);
    return *this;
}

bool DNFPredicates::operator<(const DNFPredicates &rhs) const {
    return dnfPredicates < rhs.dnfPredicates;
}

bool DNFPredicates::operator>(const DNFPredicates &rhs) const {
    return rhs < *this;
}

bool DNFPredicates::operator<=(const DNFPredicates &rhs) const {
    return !(rhs < *this);
}

bool DNFPredicates::operator>=(const DNFPredicates &rhs) const {
    return !(*this < rhs);
}

bool DNFPredicates::operator==(const DNFPredicates &rhs) const {
    return dnfPredicates == rhs.dnfPredicates;
}

bool DNFPredicates::operator!=(const DNFPredicates &rhs) const {
    return !(rhs == *this);
}

std::ostream &operator<<(std::ostream &os, const DNFPredicates &predicates) {
    size_t N = predicates.dnfPredicates.size();
    if (N > 0) {
        for (size_t i = 0; i<N; i++) {
            size_t M = predicates.dnfPredicates.at(i).size();
            if (M == 0) {
                os << "true";
            } else if (M == 1) {
                os << predicates.dnfPredicates.at(i).at(0);
            } else {
                os << "( ";
                for (size_t j = 0; j<M; j++) {
                    os << predicates.dnfPredicates.at(i).at(j);
                    if (j < (M-1)) os << " && ";
                }
                os << " )";
            }
            if (i < (N-1)) os << " || ";
        }
        return os;
    } else {
        return os << "true";
    }
}
