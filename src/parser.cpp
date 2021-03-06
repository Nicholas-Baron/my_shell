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
    case token_type::left_brace:
        return parse_command_block();
    default:
        // TODO: Turn this into a common macro
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

    while (true) {
        // A raw string is just an argument
        if (peek().type() == token_type::str) {
            cmd.append_arg(std::move(next().raw()));
            continue;
        }

        // Redirects can occur anywhere in the argument list
        if (peek().file_redirect()) {
            auto redirection = next();
            auto file = next();
            assert(file.type() == token_type::str);

            switch (redirection.type()) {
            case token_type::left_arrow:
                cmd.append_redirect(redirect::stdin, file.raw());
                break;
            case token_type::right_arrow:
                cmd.append_redirect(redirect::stdout, file.raw());
                break;
            default:
                std::cerr << "Unknown redirection type to file " << file << " : " << redirection
                          << std::endl;
                exit(2);
            }
            continue;
        }

        if (peek().type() == token_type::pipe and not in_pipeline) {
            return parse_pipeline(std::move(cmd));
        }

        break;
    }

    return std::make_unique<simple_command>(std::move(cmd));
}

command_ptr parser::parse_pipeline(simple_command && start) {

    in_pipeline = true;

    pipeline result;
    result.append_command(std::make_unique<simple_command>(std::move(start)));

    while (peek().type() == token_type::pipe) {
        next();
        result.append_command(parse_command());
    }

    in_pipeline = false;
    return std::make_unique<pipeline>(std::move(result));
}

command_ptr parser::parse_command_block() {
    command_block block;

    while (peek().type() != token_type::right_brace) {
        if (peek().type() == token_type::newline or peek().type() == token_type::semicolon) {
            next();
        } else {
            block.append_command(parse_simple_command(next().raw()));
        }
    }

    assert(next().type() == token_type::right_brace);
    return std::make_unique<command_block>(std::move(block));
}
