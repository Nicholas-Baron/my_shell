#include "executor.h"
#include "parser.h"
#include "tokenizer.h"

#include <fstream>  // std::ifstream
#include <iostream> // std::cin
#include <sstream>  // std::stringstream
#include <string>

constexpr bool debug = false;

static std::string read_entire_file(std::ifstream && file) {

    std::stringstream stream;
    std::string buffer;
    while (std::getline(file, buffer)) { stream << buffer << '\n'; }

    return stream.str();
}

int main(int arg_count, const char ** args) {

    if (arg_count > 2) {
        std::cout << "Usage: " << *args << " [file to read]" << std::endl;
        return 1;
    }

    executor e;

    auto execute = [&e](std::string && input) {
        parser p{std::move(input)};

        auto command_list = p.parse();

        if constexpr (debug) {
            std::cout << "Parsed commands:\n";
            for (auto & command : command_list) { std::cout << *command << std::endl; }
        }

        for (auto & command : command_list) {
            command->execute(e);
            e.wait_on_running_command();
        }
    };

    if (arg_count == 2) {
        execute(read_entire_file(std::ifstream{args[1]}));
    } else {
        std::string input;
        while (std::getline(std::cin, input)) {
            execute(std::move(input));
            input.clear();
        }
    }
}
