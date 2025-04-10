//
// Created by giacomo on 21/12/24.
//

#pragma once

#include <database/gsm_object.h>
#include <parser/schema/Entity.h>

struct NestingState {
    gsm_object object;
    const Entity* entity_stack{nullptr};
    bool skipCurrentKey;
    std::string key{};
    bool isThisScore{false};
    std::vector<size_t> objects;
    NativeTypes _keyType;
    size_t countFor = 0;
    size_t countAt = 0;
    std::string current_tag_name;

    NestingState(const Entity* e);
};