/*
 * SimplifiedFuzzyStringMatching.h
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
// Created by giacomo on 01/11/2021.
//

#ifndef KNOBAB_SIMPLIFIEDFUZZYSTRINGMATCHING_H
#define KNOBAB_SIMPLIFIEDFUZZYSTRINGMATCHING_H

#include "simple_pair_hash.h"
#include <string>
#include "yaucl/structures/any_to_uint_bimap.h"
#include <vector>
#include "yaucl/structures/PollMap.h"
#include "PollMap.h"



std::vector<std::string> compareString_wordLetterPairs(const std::string& strMixed);

class SimplifiedFuzzyStringMatching {
    static_assert(sizeof(short) == sizeof(char)*2, "Chars should be double the size than chars");
    yaucl::structures::any_to_uint_bimap<std::string> unique_string;
    std::unordered_map<std::string, std::vector<size_t>> shift0_2grams_containment;
    std::unordered_map<std::string, std::unordered_map<std::string, size_t>> gram_multiplicity;
    std::unordered_map<std::string, size_t> objectGramSize;


public:
    std::pair<size_t, bool> put(const std::string& str);
    std::string get(size_t id) const;
    void clear() {
        shift0_2grams_containment.clear();
        gram_multiplicity.clear();
        objectGramSize.clear();
    }

    // TODO: import the code for the indexed approximate string matching
    void getTwoGramAndString(const std::string &argument, std::unordered_map<std::string , size_t> &map) const;

    void fuzzyMatch(double threshold,
                    size_t topk,
                    const std::string& objectString,
                    std::multimap<double, std::string> &result) const;

private:
    void rankCollectionOf(std::unordered_set<size_t> &k, std::unordered_map<std::string , size_t> &m1,
                          unsigned long size, double threshold,
                          yaucl::structures::PollMap<double, std::string> &pollMap) const;
};




#include <codecvt>
#include <sstream>

class FuzzyMatchSerializer {
    const std::vector<std::string> empty_vector{};
//    std::hash<std::string> hfunc;
    std::unordered_map<std::string, size_t> objectGramSize;
    std::unordered_map<std::string, std::set<std::pair<size_t,size_t>>> gramToObject, termObject;
    std::map<std::pair<size_t,size_t>, std::vector<std::string>> objectMultipleStirngs;
    std::unordered_map<std::string, std::unordered_map<std::string, size_t>> twogramAndStringMultiplicity;
//    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    bool consentToFullyFuzzyIndex = true;

    void compareStringHashMap(const std::string& strMixed, std::unordered_map<std::string, size_t>& retMap, std::vector<size_t>& retList) const;
    void compareStringHashMap(const std::string& str, std::unordered_map<std::string, size_t>& retMap) const;
    void getTwoGramAndString(const std::string &argument, std::unordered_map<std::string, size_t> &map) const;
    void rankCollectionOf(std::set<std::pair<size_t,size_t>>& , std::unordered_map<std::string, size_t> &m1,
                                              unsigned long size, double threshold,
                          yaucl::structures::PollMap<double, std::pair<size_t,size_t>>  &pollMap) const;
public:

    inline void deactivateFuzzyIndex() {
        consentToFullyFuzzyIndex = false;
    }

    void clear() {
        gramToObject.clear();
        objectGramSize.clear();
        termObject.clear();
        objectMultipleStirngs.clear();
        twogramAndStringMultiplicity.clear();
    }
    // TODO: approximate fuzzy matching for this!
    void addGramsToMap(const std::string &string,
                       const std::pair<size_t,size_t>& id,
                       const std::vector<std::string> &associatedOtherStrings);

    void fuzzyMatch(double threshold, size_t topk, const std::string& objectString,
                    std::multimap<double, std::pair<size_t,size_t>>  &result) const;

    template <typename Inserter>
    void fuzzyMatch(double threshold, size_t topk, const std::string& objectString,
                    Inserter  result) const {
        yaucl::structures::PollMap<double, std::pair<size_t,size_t>>  toReturnTop{topk};
        std::vector<std::string> objectGrams = compareString_wordLetterPairs(objectString);
        std::set<std::pair<size_t,size_t>> candidates{};
        std::unordered_map<std::string, size_t> m1;
        if (!consentToFullyFuzzyIndex)
            throw std::runtime_error("ERROR: cannot fuzzy match if you disabled the indexing!");

        // obtain all the objects that are assoicated to the gram that are within the current objectString in objectStrings
        for (const std::string& gram : objectGrams) {
            auto gramPtr = gramToObject.find((gram));
            if (gramPtr != gramToObject.end()) {
                candidates.insert(gramPtr->second.begin(), gramPtr->second.end());
            }
        }

        // return the number of the occurences of the twoGrams within the current string
        compareStringHashMap(objectString, m1);
        size_t ogSize = 0;
        for (auto it = m1.begin(); it != m1.cend(); it++) ogSize += it->second;

        // Rank the similarity between all the candidates
        rankCollectionOf(candidates, m1, ogSize, threshold, toReturnTop);

        result(toReturnTop.getValueScore());
    }

    const std::vector<std::string>& resolve_object_id(const std::pair<size_t,size_t>& object_id) const {
        auto it = objectMultipleStirngs.find(object_id);
        if (it == objectMultipleStirngs.end()) return empty_vector;
        else return it->second;
    }
};


#endif //KNOBAB_SIMPLIFIEDFUZZYSTRINGMATCHING_H
