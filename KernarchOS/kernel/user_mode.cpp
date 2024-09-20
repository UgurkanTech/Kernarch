#include "user_mode.h"
#include "logger.h"

// Declare the assembly function
extern "C" void jump_to_user_mode(uint32_t entry_point, uint32_t user_stack_top);

namespace UserMode {

void switch_to_user_mode(uint32_t entry_point, uint32_t user_stack_top) {
    Logger::info("Switching to user mode");

    // Call the assembly function to switch to user mode
    jump_to_user_mode(entry_point, user_stack_top);

    // We should never reach here
    Logger::error("Returned from user mode unexpectedly");
}

} // namespace UserMode