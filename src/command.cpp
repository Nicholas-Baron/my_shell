#include "command.h"

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
