#pragma once

#include <stdexcept>

namespace regex {

    class ErroRegex : public std::runtime_error {
        public:
            using std::runtime_error::runtime_error;
    };
}