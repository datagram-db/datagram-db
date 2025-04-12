/*
 * SimplifiedFuzzyStringMatching.cpp
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
// Created by giacomo on 28/05/23.
//

//
// Created by giacomo on 01/11/2021.
//

#include "database/SimplifiedFuzzyStringMatching.h"
#include "yaucl/strings/string_utils.h"

static inline
void compareStringHashMap_Internal(const std::string& str, std::unordered_map<std::string, size_t>& retMap, std::vector<size_t>& retList) {
    unsigned long numPairs = str.size() - 1;
    if (numPairs == 0) {
        retMap[str] = 0;
        retList.emplace_back(1);
    } else {
        if (numPairs < 0)
            numPairs = 0;
        int singleGrams = 0;
        for (unsigned long i= 0; i< numPairs; i++) {
            auto s = str.substr(i, 2);
            auto pos = retMap.find(s);
            if (pos == retMap.cend()) {
                retMap[s] = singleGrams++;
                retList.emplace_back(1);
            } else {
                retList[pos->second]++;
            }
        }
    }
}

static inline void compareStringHashmap1(const std::string& str,
                                         std::unordered_map<std::string, size_t> &map,

                                         std::vector<size_t> &vec) {
    size_t numPairs = str.length() - 1;
    if (numPairs == 0) {
        map[str] = 0;
        vec.emplace_back(1);
    } else {
        if (numPairs < 0) numPairs = 0;
        int singleGrams = 0;
        for (int i = 0; i < numPairs; i++) {
            std::string s{(str.substr(i, 2))};

            if (!s.empty()) {
                auto pos = map.find(s);
                if (pos == map.end()) {
                    map[s] = singleGrams++;
                    vec.emplace_back(1);
                } else {
                    /*size_t x = vec[pos->second];
                    vec[pos->second] = x + 1;*/
                    vec[pos->second]++;

                }
            }
        }
    }
}

#include <sstream>

static inline void compareStringHashmap2(const std::string &string,
                                         std::unordered_map<std::string, size_t> &map,

                                         std::vector<size_t> &vec) {
    {
        std::string str;
        std::stringstream ss(string);
        std::vector<std::string> pairs;

        // Getting the grams within each word, thus avoiding to create 2grams with a space
        while (ss >> str) {
            compareStringHashmap1(str, map, vec);
        }
    }
    // Updating the gram count with each elements' output
    for (auto & kp : map) {
        kp.second = vec[kp.second];
    }
}

////Credits to http://www.catalysoft.com/articles/StrikeAMatch.html
//static inline void compareString_letterPairs(const std::string& str, std::vector<std::string>& pairs) {
//
//    pairs.clear();
//    unsigned long numPairs = str.length()-1;
//    if (!numPairs) {
//        pairs.emplace_back(str);
//    }
//    if (numPairs < 0)
//        numPairs = 0;
//    for (unsigned long i = 0; i<numPairs; i++) {
//        pairs.emplace_back(str.substr(i, 2));
//    }
//}
//
////Credits to http://www.catalysoft.com/articles/StrikeAMatch.html
//std::vector<std::string> compareString_wordLetterPairs(const std::string& strMixed) {
//    std::istringstream iss(strMixed);
//    std::vector<std::string> allPairs{};
//    for(std::string s; iss >> s; )
//        compareString_letterPairs(s, allPairs);
//    return allPairs;
//}

static inline void compareStringHashMap(const std::string& str, std::unordered_map<std::string, size_t> & retMap) {
    std::vector<size_t> retList{};
    compareStringHashmap2(str, retMap, retList);
}

std::pair<size_t, bool> SimplifiedFuzzyStringMatching::put(const std::string &str) {
    auto it = unique_string.put(str);
    if (it.second) {
        std::unordered_map<std::string, size_t>& m = gram_multiplicity[str];
        std::vector<size_t> vec;
        compareStringHashmap2(str, m, vec);

        size_t sum = 0;
        for (size_t& j : vec) {
            sum += j;
        }
        objectGramSize[str] = sum;

        for (auto & twoGramFreq : m) {
            shift0_2grams_containment[twoGramFreq.first].emplace_back(it.first);
        }
    }
    return it;
}

#include <cassert>

std::string SimplifiedFuzzyStringMatching::get(size_t id) const {
    return unique_string.get(id);
}

void SimplifiedFuzzyStringMatching::getTwoGramAndString(const std::string &argument,
                                                        std::unordered_map<std::string, size_t> &map) const {
    auto ls = gram_multiplicity.find(argument);
    //void *ls = twogramAndStringToMultiplicity.searchForId(argument);
    if (ls != gram_multiplicity.end()) {
        // if the value is stored, then I use the hashmap
        map = ls->second;
    } else {
        // if not, recompute everything and put it in the map
        compareStringHashMap(argument, map);
    }
}

void SimplifiedFuzzyStringMatching::rankCollectionOf(std::unordered_set<size_t> &k,
                                                     std::unordered_map<std::string, size_t> &m1, unsigned long size,
                                                     double threshold,
                                                     yaucl::structures::PollMap<double, std::string> &pollMap) const {
    for (size_t itemId : k) {
        std::string associatedToElement = get(itemId);
        std::unordered_map<std::string , size_t> m2;

        getTwoGramAndString(associatedToElement, m2);

        // Set intersection of the keys. Intersection while iterating
        double score = 0;
        double e = 0;
        std::unordered_map<std::string , size_t>* lesser = m1.size() < m2.size() ? &m1 : &m2;
        std::unordered_map<std::string , size_t>* greater = m1.size() < m2.size() ? &m2 : &m1;
        for (auto it = lesser->begin(); it != lesser->cend(); it++) {
            auto it2 = greater->find(it->first);
            if (it2 != greater->cend()) {
                // key is in both elements
                e += std::min(it->second, it2->second)*1.0;
            }
        }
        // no memoization
        ////void *ls = objectGramSize.find(associatedToElement);
        double leftCount = 0;
        //if (ls == nullptr) {
        std::unordered_map<std::string , size_t> retMap;
        std::vector<size_t> retList;
        compareStringHashmap2(associatedToElement, retMap, retList);
        for (size_t& x : retList) leftCount+=x;
        score = (e * 2.0) / ((leftCount + size)*1.0);

        if (threshold >= 0 && score >= threshold) {
            pollMap.add(score, associatedToElement);
        }
    }
}

std::vector<std::string> compareString_wordLetterPairs(const std::string& strMixed);

void SimplifiedFuzzyStringMatching::fuzzyMatch(double threshold, size_t topk, const std::string &objectString,
                                               std::multimap<double, std::string> &result) const {
    yaucl::structures::PollMap<double, std::string> toReturnTop{topk};
    std::vector<std::string> objectGrams = compareString_wordLetterPairs(objectString);
    std::unordered_set<size_t> candidates{};
    std::unordered_map<std::string , size_t> m1;

    // obtain all the objects that are assoicated to the gram that are within the current objectString in objectStrings
    for (const std::string& gram : objectGrams) {
        auto gramPtr = shift0_2grams_containment.find(gram);
        if (gramPtr != shift0_2grams_containment.end()) {
            candidates.insert(gramPtr->second.begin(), gramPtr->second.end());
        }
    }

    // return the number of the occurences of the twoGrams within the current string
    compareStringHashMap(objectString, m1);
    size_t ogSize = 0;
    for (auto it = m1.begin(); it != m1.cend(); it++) ogSize += it->second;

    // Rank the similarity between all the candidates
    rankCollectionOf(candidates, m1, ogSize, threshold, toReturnTop);

    // providing the final result
    toReturnTop.getPoll(result);
}

void
FuzzyMatchSerializer::addGramsToMap(const std::string &string,  const std::pair<size_t,size_t>& id, const std::vector<std::string> &associatedOtherStrings) {
    if (string.empty()) return;
    termObject[string].emplace(id);
    objectMultipleStirngs[id].emplace_back(string);
    for (const std::string& x : associatedOtherStrings)
        objectMultipleStirngs[id].emplace_back(x);

    if (!consentToFullyFuzzyIndex)
        return;
    std::unordered_map<std::string,size_t> cp;
    std::vector<size_t> vec;

    compareStringHashmap2(string, cp, vec);
    size_t sum = 0;
    for (size_t& j : vec) {
        sum += j;
    }
    objectGramSize[string] = sum;

    for (auto & begin : cp) {
        std::string x = begin.first;
        twogramAndStringMultiplicity[(string)][x] = begin.second;
        gramToObject[x].emplace(id);
    }
}

void FuzzyMatchSerializer::fuzzyMatch(double threshold, size_t topk, const std::string &objectString,
                                      std::multimap<double, std::pair<size_t,size_t>>  &result) const {

    yaucl::structures::PollMap<double, std::pair<size_t,size_t>>  toReturnTop{topk};
    std::vector<std::string> objectGrams = compareString_wordLetterPairs(objectString);
    std::set<std::pair<size_t,size_t>> candidates{};
    std::unordered_map<std::string, size_t> m1;
    if (!consentToFullyFuzzyIndex)
        throw std::runtime_error("ERROR: cannot fuzzy match if you disabled the indexing (2)!");

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

    // providing the final result
    toReturnTop.getPoll(result);
}

void FuzzyMatchSerializer::rankCollectionOf(std::set<std::pair<size_t,size_t>> &k, std::unordered_map<std::string, size_t> &m1,
                                            unsigned long size, double threshold,
                                            yaucl::structures::PollMap<double, std::pair<size_t,size_t>> &pollMap)const {
    for (const auto& element : k) {
        auto it = objectMultipleStirngs.find(element);
        if (it != objectMultipleStirngs.end()) {
            for (const auto& associatedToElement : it->second) {
                std::unordered_map<std::string, size_t> m2;

                getTwoGramAndString(associatedToElement, m2);

                // Set intersection of the keys. Intersection while iterating
                double score = 0;
                double e = 0;
                std::unordered_map<std::string, size_t>* lesser = m1.size() < m2.size() ? &m1 : &m2;
                std::unordered_map<std::string, size_t>* greater = m1.size() < m2.size() ? &m2 : &m1;
                for (auto it = lesser->begin(); it != lesser->cend(); it++) {
                    auto it2 = greater->find(it->first);
                    if (it2 != greater->cend()) {
                        // key is in both elements
                        e += std::min(it->second, it2->second)*1.0;
                    }
                }

                // no memoization
//                auto ls = objectGramSize.find(associatedToElement);
                double leftCount = 0;
                //if (ls == nullptr) {
                std::unordered_map<std::string, size_t> retMap;
                std::vector<size_t> retList;
                compareStringHashMap(associatedToElement, retMap, retList);
                for (size_t& x : retList) leftCount+=x;
                score = (e * 2.0) / ((leftCount + size)*1.0);
                /*} else {
                    size_t* ptr = LONG_ARRAY_ELEMENT(ls);
                    score = (e * 2.0) / ((ptr[0] + size)*1.0);
                }*/

                if (threshold >= 0 && score >= threshold) {
                    pollMap.add(score, element);
                }
            }
        }

    }
}

void
FuzzyMatchSerializer::compareStringHashMap(const std::string &strMixed, std::unordered_map<std::string, size_t> &retMap,
                                           std::vector<size_t> &retList) const {
//        std::wstring wide = fmatch_converter.from_bytes(strMixed.c_str());
//        std::transform(wide.begin(), wide.end(), wide.begin(), std::towlower);
//        std::string str = fmatch_converter.to_bytes(wide);
    std::istringstream iss(strMixed);
    for(std::string s; iss >> s; )
        compareStringHashMap_Internal(s, retMap, retList);
    // Sanitizing
    for (auto it = retMap.begin(); it != retMap.cend(); it++) {
        it->second = retList[it->second];
    }
}

void FuzzyMatchSerializer::compareStringHashMap(const std::string &str, std::unordered_map<std::string, size_t> &retMap) const {
    std::vector<size_t> retList{};
    compareStringHashMap(str, retMap, retList);
}

void
FuzzyMatchSerializer::getTwoGramAndString(const std::string &argument, std::unordered_map<std::string, size_t> &map) const {
    auto ls = twogramAndStringMultiplicity.find(argument);
    if (ls != twogramAndStringMultiplicity.end()) {
        // if the value is stored, then I use the hashmap
        map.insert(ls->second.begin(), ls->second.end());
    } else {
        // if not, recompute everything and put it in the map
        compareStringHashMap(argument, map);
    }
}
