#include "executor.h"

#include <sys/wait.h> // waitpid

#include <array>
#include <cstring>  // strcpy
#include <fcntl.h>  // open
#include <stdio.h>  // perror
#include <unistd.h> // fork, exec

static char * allocate_c_str(const std::string & str) {
    return strcpy(new char[str.size() + 1], str.c_str());
}

void executor::execute(const simple_command & cmd) {

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

        // TODO: Support redirects
        if (dup2(child_read, STDIN_FILENO) < 0 or dup2(child_write, STDOUT_FILENO) < 0) {
            perror("dup2");
            return;
        }

        auto * executable = allocate_c_str(cmd.executable);

        auto ** args = new char *[cmd.arguments.size() + 2];
        {
            auto i = 0;
            args[i++] = executable;
            for (auto & arg : cmd.arguments) args[i++] = allocate_c_str(arg);
            args[i] = nullptr;
        }

        // TODO: Remove `p` and do our own path traversal?
        // TODO: Add `e` to pass different environment
        if (execvp(executable, args) < 0) {
            perror("execvp");
            return;
        }

    } else {
        // Parent waits for child.

        close(child_read);
        close(child_write);

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
    }
}
