//
// Created by Nguyen Thai Binh on 17/1/22.
//

#ifndef FIRESTORM_UTILITY_HPP
#define FIRESTORM_UTILITY_HPP

#include <exception>

struct FirestormError : public std::runtime_error {
    explicit FirestormError(const char *msg) : std::runtime_error(msg) {}
};

#endif //FIRESTORM_UTILITY_HPP
