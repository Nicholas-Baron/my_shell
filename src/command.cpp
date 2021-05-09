#include "command.h"

#include "executor.h"

#include <iostream>

void simple_command::print_command(std::ostream & lhs) const {
    lhs << executable;
    for (auto & arg : arguments) lhs << ' ' << arg;
    for (auto & redir : redirects) switch (redir.first) {
        case redirect::stdin:
            lhs << " < " << redir.second;
            break;
        case redirect::stdout:
            lhs << " > " << redir.second;
            break;
        case redirect::stderr:
            lhs << " 2> " << redir.second;
            break;
        }
}

void simple_command::append_redirect(redirect redir, std::string && file) {

    if (auto iter = redirects.find(redir); iter != redirects.end()) {
        std::cout << "Ignoring redirect about file " << iter->second << "\nNow using " << file
                  << " instead\n";
        iter->second = std::move(file);
    } else {
        redirects.emplace(redir, std::move(file));
    }
}

void simple_command::execute(executor & executor) const { executor.execute(*this); }

void pipeline::print_command(std::ostream & lhs) const {
    bool first = true;
    for (auto & cmd : commands) {
        if (first) first = false;
        else
            lhs << " | ";

        lhs << *cmd;
    }
}

void pipeline::execute(executor & e) const {
    for (auto i = 0u; i < commands.size(); ++i) {
        if (i != 0u) e.set_connect_state(executor::connection::pipe);

        commands.at(i)->execute(e);
    }
    e.set_connect_state(executor::connection::separate);
}

void command_block::print_command(std::ostream & lhs) const {
    lhs << "{\n";
    for (auto & cmd : commands) lhs << *cmd << '\n';
    lhs << '}' << std::endl;
}

void command_block::execute(executor & e) const {
    for (auto & cmd : commands) {
        cmd->execute(e);
        e.wait_on_running_command();
    }
}
