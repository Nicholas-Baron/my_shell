#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <cassert>
#include <iosfwd>
#include <string>
#include <utility> // std::move

enum class token_type {
    eof,
    str,
    newline,
    semicolon,
    // TODO: Support redirecting the error stream (2>, 2|)
    left_arrow,
    right_arrow,
    pipe,
    left_brace,
    right_brace,
    t_or,
    t_and,
};

class token final {
  public:
    token(token_type t, std::string && data)
        : typ{t}
        , raw_data{std::move(data)} {}

    token(token_type t)
        : typ{t} {
        assert(t != token_type::str);
    }

    [[nodiscard]] token_type type() const noexcept { return typ; }
    [[nodiscard]] bool file_redirect() const noexcept {
        return typ == token_type::left_arrow or typ == token_type::right_arrow;
    }

    [[nodiscard]] const std::string & raw() const { return raw_data; }
    [[nodiscard]] std::string raw() { return raw_data; }

  private:
    friend std::ostream & operator<<(std::ostream &, const token &);

    token_type typ;
    std::string raw_data;
};

class tokenizer final {
  public:
    explicit tokenizer(std::string && input)
        : input_buffer{std::move(input)} {}

    [[nodiscard]] bool empty() const noexcept {
        return input_buffer.empty() or pos >= input_buffer.size();
    }

    [[nodiscard]] token next();

  private:
    [[nodiscard]] char current_char() const {
        if (pos >= input_buffer.size()) return EOF;
        return input_buffer.at(pos);
    }

    [[nodiscard]] char peek() const {
        auto peek_loc = pos + 1;
        if (peek_loc >= input_buffer.size()) return EOF;
        return input_buffer.at(peek_loc);
    }

    std::string input_buffer;
    size_t pos{0};
    // TODO: Support reading from files
};

#endif
