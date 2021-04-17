#include "executor.h"
#include "parser.h"
#include "tokenizer.h"

#include <iostream> // std::cin
#include <string>

int main() {

    executor e;

    // TODO: Take command line args
    std::string input;
    while (std::getline(std::cin, input)) {
        parser p{std::move(input)};
        input.clear();

        auto command_list = p.parse();
        std::cout << "Parsed commands:\n";
        for (auto & command : command_list) { std::cout << *command << std::endl; }

        for (auto & command : command_list) command->execute(e);
    }
}
