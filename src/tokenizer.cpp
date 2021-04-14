#include "tokenizer.h"

#include <cassert>
#include <cctype>  // isspace
#include <iomanip> // std::hex
#include <iostream>

std::ostream & operator<<(std::ostream & lhs, const token & rhs) { return lhs << rhs.raw_data; }

token tokenizer::next() {
    // Skip whitespace
    while (isspace(current_char())) ++pos;

    const auto c = current_char();
    assert(not isspace(c));
    ++pos;
    switch (c) {
    case EOF:
        return token_type::eof;
    case '\'':
    case '\"': {
        // Consume an entire in-quotes string
        std::string raw;
        for (auto escaped = false; escaped or current_char() != c; ++pos) {
            assert(not empty());
            const auto current = current_char();
            if (current == '\\') {
                // Add the slash if the last character was a slash.
                if (escaped) { raw += current; }
                escaped = not escaped;
                continue;
            }

            if (current == c) {
                // We must be escaped
                assert(escaped);
                raw += current;
                escaped = false;
                continue;
            }

            // TODO: Support escapes like \n, \r, \t
            assert(not escaped);
            raw += current;
        }
        ++pos;
        return {token_type::str, std::move(raw)};
    } break;
    case '|':
        if (peek() == c) {
            ++pos;
            return token_type::t_or;
        }
        return token_type::pipe;
    case '<':
        return token_type::left_arrow;
    case '>':
        return token_type::right_arrow;
    case '&':
        assert(peek() == c);
        ++pos;
        return token_type::t_and;
    default:
        if (isalnum(c) or c == '/') {
            // Consume the raw string
            std::string raw;
            raw += c;
            while (not isspace(current_char()) and not empty()) {
                raw += current_char();
                ++pos;
            }
            return {token_type::str, std::move(raw)};
        }
        std::cerr << "Found unknown character: '" << c << "' (" << std::hex << (unsigned)c << ')'
                  << std::endl;
        exit(2);
    }
}
