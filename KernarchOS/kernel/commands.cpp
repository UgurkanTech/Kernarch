#include "commands.h"
#include "terminal.h"
#include "cstring.h"
#include "memory.h"
#include "io.h"
#include "interrupts.h"
#include "stack.h"
#include "thread.h"

using namespace std;

Command Commands::command_list[MAX_COMMANDS];
int Commands::command_count = 0;

void Commands::initialize() {
    add_command("echo", "[text]", "Print the given text", echo);
    add_command("help", "", "Display this help message", [](const char*) { help(); });
    add_command("clear", "", "Clear the screen", clear);
    add_command("meminfo", "", "Display memory information", meminfo);
    add_command("systeminfo", "", "Display system information", systeminfo);
    add_command("stack", "", "Display stack information", stack);
    add_command("shutdown", "", "Shut down the system", shutdown);
    add_command("test", "", "Starts Threading test", test);
    add_command("about", "", "About the OS", about);
}

void Commands::add_command(const char* name, const char* args, const char* description, void (*function)(const char*)) {
    if (command_count < MAX_COMMANDS) {
        command_list[command_count] = {name, args, description, function};
        command_count++;
    }
}

void Commands::execute(const char* command) {
    for (int i = 0; i < command_count; i++) {
        size_t cmd_len = strlen(command_list[i].name);
        if (strncmp(command, command_list[i].name, cmd_len) == 0 && (command[cmd_len] == ' ' || command[cmd_len] == '\0')) {
            sys_printf(">&f%s\n",command);
            command_list[i].function(command + cmd_len + 1);
            return;
        }
    }
    sys_printf("\n&4Unknown command: &f%s \n", command);
}

void Commands::help() {
    sys_printf("&aAvailable commands:\n");
    for (int i = 0; i < command_count; i++) {
        sys_printf("  &b%s", command_list[i].name);
        if (strlen(command_list[i].args) > 0) {
            sys_printf(" &c%s", command_list[i].args);
        }
        sys_printf(" &f- &7%s\n", command_list[i].description);
    }
}

// Implement your command functions here
void Commands::echo(const char* args) {
    sys_printf("&a%s \n", args);
}

void Commands::systeminfo(const char*) {
    sys_printf("&cUnknown\n");
}

void Commands::test(const char*) {
    sys_printf("&cStarting test...\n");
    sys_test();
}

void Commands::clear(const char* args) {
    (void)args;
    sys_clear();
}

void Commands::meminfo(const char* args) {
    (void)args;
    char buffer[256];
    sys_printf("%s\n", memory_info(buffer, sizeof(buffer)));
}

void Commands::stack(const char* args) {
    (void)args;
    uint32_t allocated = StackManager::get_total_allocated();
    uint32_t usage = StackManager::get_total_usage();

    sys_printf("&9Stack Allocated: &f%d bytes, &cUsed: &f%d bytes &e(%d%)\n", allocated, usage, (usage * 100) / allocated);
    
}

void Commands::about(const char* args) {
    (void)args;
    sys_printf(ascii_bytes);
    sys_printf("&b ===================================================================\n");
    sys_printf("&b = &eAbout:   &fA Simple OS Kernel                                     &b=\n");
    sys_printf("&b = &eVersion: &f0.1                                                    &b=\n");
    sys_printf("&b = &eAuthor:  &fUgurkan Hosgor                                         &b=\n");
    sys_printf("&b = &eLicense: &fCC BY-NC-ND 4.0                                        &b=\n");
    sys_printf("&b ===================================================================\n");
}

void Commands::shutdown(const char* args) {
    (void)args;
    sys_printf("&4Shutting down...\n");
    sys_sleep(500);
    // Try ACPI shutdown
    outw(0xB004, 0x2000);

    // QEMU-specific shutdown
    outw(0x604, 0x2000);

    // Try Bochs/Older QEMU versions
    for (const uint16_t* s = (const uint16_t*)"Shutdown"; *s; ++s) {
        outw(0x8900, *s);
    }

    // Try VirtualBox-specific shutdown
    outw(0x4004, 0x3400);

    // Try legacy keyboard controller reset
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);

    // If all else fails, halt the CPU
    sys_printf("&4Failed to shut down. Halting CPU.\n");
    for (;;) {
        asm volatile ("cli");
        asm volatile ("hlt");
    }
}
