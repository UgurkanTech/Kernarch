#ifndef USER_MODE_H
#define USER_MODE_H

#include <stdint.h>

namespace UserMode {
    void switch_to_user_mode(uint32_t entry_point, uint32_t user_stack_top);
}

#endif // USER_MODE_H