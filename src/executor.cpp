#include "executor.h"

#include <array>
#include <cassert>
#include <cstdlib> // exit
#include <cstring> // strcpy

#include <fcntl.h>    // open
#include <stdio.h>    // perror
#include <sys/wait.h> // waitpid
#include <unistd.h>   // fork, exec

static char * allocate_c_str(const std::string & str) {
    return strcpy(new char[str.size() + 1], str.c_str());
}

void executor::wait_on_running_command() {

    auto pid = current_command.value();

    auto iter = active_commands.find(pid);
    assert(iter != active_commands.end());
    auto parent_read = iter->second.parent_read;

    for (std::array<char, 256> buf;;) {
        buf.fill(0);

        const auto rc = waitpid(pid, &last_status, WNOHANG);
        if (rc == pid) break;

        if (rc < 0) {
            perror("waitpid");
            last_status = -1;
            return;
        }

        while (true) {

            const auto nread = read(parent_read, buf.data(), buf.size());
            if (nread < 0) {
                perror("read");
                last_status = -1;
                return;
            }

            if (nread == 0) break;

            for (auto i = 0; i < nread; ++i) {
                putchar(buf[i]);
                buf[i] = 0;
            }
        }
    }

    current_command.reset();
}

void executor::execute(const simple_command & cmd) {

    if (auto iter = builtins.find(cmd.executable); iter != builtins.end()) {
        (this->*(iter->second))(cmd);
        return;
    }

    int to_parent[2]{0, 0};
    int to_child[2]{0, 0};

    if (pipe(to_parent) < 0 or pipe(to_child) < 0) {
        perror("pipe");
        return;
    }

    auto child_read = to_child[0];
    auto parent_read = to_parent[0];

    auto child_write = to_parent[1];
    auto parent_write = to_child[1];

    if (auto pid = fork(); pid < 0) {
        perror("fork");
    } else if (pid == 0) {
        // Child process launches the actual program.

        close(parent_read);
        close(parent_write);

        auto input_fd = child_read;
        // TODO: Avoid the `pipe` syscall when creating pipes
        if (current_connect == connection::pipe) {

            assert(current_command.has_value());
            auto parent_pid = current_command.value();

            // Remount the pipes to be the ones from the previous command
            // One way only
            close(input_fd);

            auto iter = active_commands.find(parent_pid);
            assert(iter != active_commands.end());

            input_fd = iter->second.parent_read;
        } else if (auto iter = cmd.redirects.find(redirect::stdin); iter != cmd.redirects.end()) {
            input_fd = open(iter->second.c_str(), O_RDONLY | O_CLOEXEC);
            if (input_fd == -1) {
                perror("open");
                exit(1);
            }
        }

        auto output_fd = child_write;
        if (auto iter = cmd.redirects.find(redirect::stdout); iter != cmd.redirects.end()) {
            // Create a file with rw-r--r-- permissions
            output_fd = open(iter->second.c_str(), O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
            if (output_fd == -1) {
                perror("open");
                exit(1);
            }
        }

        if (dup2(input_fd, STDIN_FILENO) < 0 or dup2(output_fd, STDOUT_FILENO) < 0) {
            perror("dup2");
            return;
        }

        auto * executable = allocate_c_str(cmd.executable);

        std::vector<char *> args;
        args.push_back(executable);
        for (auto & arg : cmd.arguments) args.push_back(allocate_c_str(arg));
        args.push_back(nullptr);

        // TODO: Remove `p` and do our own path traversal?
        // TODO: Add `e` to pass different environment
        if (execvp(executable, args.data()) < 0) {
            perror("execvp");
            return;
        }

    } else {
        // Parent waits for child.

        close(child_read);
        close(child_write);

        current_command = pid;
        active_commands.emplace(pid, command_data{parent_read, parent_write});
    }
}

void executor::shell_exit(const simple_command &) { std::exit(0); }
