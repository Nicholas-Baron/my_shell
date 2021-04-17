#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "command.h"

class executor final {
  public:
    executor() = default;

    void execute(const simple_command &);

  private:
    int last_status{0};
};

#endif
