#include "commands.h"
#include "terminal.h"
#include "cstring.h"
#include "memory.h"
#include "io.h"

using namespace std;

Command Commands::command_list[MAX_COMMANDS];
int Commands::command_count = 0;

void Commands::initialize() {
    add_command("echo", "[text]", "Print the given text", echo);
    add_command("help", "", "Display this help message", [](const char*) { help(); });
    add_command("clear", "", "Clear the screen", clear);
    add_command("meminfo", "", "Display memory information", meminfo);
    add_command("systeminfo", "", "Display system information", systeminfo);
    add_command("shutdown", "", "Shut down the system", shutdown);
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
            command_list[i].function(command + cmd_len + 1);
            return;
        }
    }
    term_print_colored("Unknown command: ", VGA_LIGHT_RED);
    term_print(command);
    term_print("\n");
}

void Commands::help() {
    term_print_colored("Available commands:\n", VGA_YELLOW);
    for (int i = 0; i < command_count; i++) {
        term_print("  ");
        term_print_colored(command_list[i].name, VGA_LIGHT_GREEN);
        if (strlen(command_list[i].args) > 0) {
            term_print(" ");
            term_print_colored(command_list[i].args, VGA_LIGHT_CYAN);
        }
        term_print(" - ");
        term_print_colored(command_list[i].description, VGA_WHITE);
        term_print("\n");
    }
}

// Implement your command functions here
void Commands::echo(const char* args) {
    term_print(args);
    term_print("\n");
}


void Commands::systeminfo(const char*) {
    term_print("Unknown\n");
}

void Commands::clear(const char* args) {
    (void)args;
    term_clear();
}

void Commands::meminfo(const char* args) {
    (void)args;
    print_memory_info();
}

void Commands::shutdown(const char* args) {
    (void)args;
    term_print("Shutting down...\n");

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
    term_print("Failed to shut down. Halting CPU.\n");
    for (;;) {
        asm volatile ("cli");
        asm volatile ("hlt");
    }
}