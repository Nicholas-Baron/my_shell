#ifndef PARSER_H
#define PARSER_H

#include "command.h"
#include "tokenizer.h"

#include <optional>
#include <string>
#include <utility> // std::move
#include <vector>

class parser final {
  public:
    explicit parser(tokenizer && tok)
        : token_generator{std::move(tok)} {}

    explicit parser(std::string && input)
        : token_generator{std::move(input)} {}

    [[nodiscard]] std::vector<command_ptr> parse();

  private:
    [[nodiscard]] command_ptr parse_command();
    [[nodiscard]] command_ptr parse_simple_command(std::string && name);
    [[nodiscard]] command_ptr parse_command_block();
    [[nodiscard]] command_ptr parse_pipeline(simple_command &&);

    token next();
    token & peek();

    tokenizer token_generator;
    std::optional<token> peeked;
    bool in_pipeline{false};
};

#endif
