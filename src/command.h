#ifndef COMMAND_H
#define COMMAND_H

#include <iosfwd>
#include <map>
#include <memory> // std::unique_ptr
#include <string>
#include <vector>

class executor;

class command {
  public:
    constexpr command() = default;

    // Base classes should not be copied or moved.
    command(const command &) = delete;
    command & operator=(const command &) = delete;

    command(command &&) = default;
    command & operator=(command &&) = default;

    virtual ~command() noexcept = default;

    virtual void execute(executor &) const = 0;

  protected:
    virtual void print_command(std::ostream &) const = 0;

  private:
    friend std::ostream & operator<<(std::ostream & lhs, const command & rhs) {
        rhs.print_command(lhs);
        return lhs;
    }
};

using command_ptr = std::unique_ptr<command>;

// { commands... }
class command_block final : public command {
  public:
    command_block() = default;

    command_block(command_block &&) = default;
    command_block & operator=(command_block &&) = default;

    ~command_block() noexcept final = default;

    void append_command(command_ptr cmd) { commands.push_back(std::move(cmd)); }

    void execute(executor & e) const;

  protected:
    void print_command(std::ostream &) const final;

  private:
    std::vector<command_ptr> commands{};
};

enum class redirect {
    stdin,
    stdout,
    stderr,
};

class simple_command final : public command {
  public:
    explicit simple_command(std::string && name)
        : executable{std::move(name)} {}

    simple_command(simple_command &&) = default;
    simple_command & operator=(simple_command &&) = default;

    ~simple_command() noexcept final = default;

    // ^command
    void execute(executor &) const;

    void append_arg(std::string && arg) { arguments.push_back(std::move(arg)); }
    void append_redirect(redirect, std::string &&);

  protected:
    void print_command(std::ostream &) const final;

  private:
    friend executor; // TODO: Reconsider this.
    std::string executable{};
    std::vector<std::string> arguments{};
    std::map<redirect, std::string> redirects{};
};

#endif
