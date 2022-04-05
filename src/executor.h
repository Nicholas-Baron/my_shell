#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "command.h"

#include <map>
#include <optional>
#include <string>

#include <sys/types.h>

class executor final {
  public:
    executor() = default;

    void execute(const simple_command &);

    void wait_on_running_command();

    enum class connection { pipe, separate };
    void set_connect_state(connection conn) noexcept { current_connect = conn; }

  private:
    // builtins
    void shell_exit(const simple_command &);

    inline static const std::map<std::string, void (executor::*)(const simple_command &)> builtins{
        std::pair{std::string{"exit"}, &executor::shell_exit}};

    struct command_data {
        int parent_read;
        int parent_write;
    };

    std::map<pid_t, command_data> active_commands;
    std::optional<pid_t> current_command;

    connection current_connect{connection::separate};
    int last_status{0};
};

#endif
