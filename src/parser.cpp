#include "parser.h"

#include <cstdlib>  // exit
#include <iostream> // std::cerr

std::vector<command_ptr> parser::parse() {
    // The optional shebang is handled by the tokenizer skipping comments

    std::vector<command_ptr> result;
    while (not token_generator.empty()) result.push_back(parse_command());
    return result;
}

command_ptr parser::parse_command() {
    auto tok = next();
    switch (tok.type()) {
    case token_type::str:
        return parse_simple_command(tok.raw());
    default:
        std::cerr << "Unknown token in " << __PRETTY_FUNCTION__ << " : " << tok << std::endl;
        exit(1);
    }
    return {};
}

token parser::next() {
    if (peeked.has_value()) {
        auto tok = peeked.value();
        peeked.reset();
        return tok;
    }

    return token_generator.next();
}

token & parser::peek() {
    if (not peeked.has_value()) peeked = token_generator.next();
    return peeked.value();
}

command_ptr parser::parse_simple_command(std::string && name) {
    simple_command cmd{std::move(name)};

    while (peek().type() == token_type::str) cmd.append_arg(std::move(next().raw()));

    return std::make_unique<simple_command>(std::move(cmd));
}
