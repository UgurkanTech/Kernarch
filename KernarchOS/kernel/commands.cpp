#include "commands.h"
#include "terminal.h"
#include "cstring.h"
#include "memory.h"
#include "acpi.h"
#include "io.h"

using namespace std;

void Commands::execute(const char* command) {
    if (strncmp(command, "echo ", 5) == 0) {
        echo(command + 5);
    } else if (strcmp(command, "help") == 0) {
        help();
    } else if (strcmp(command, "clear") == 0) {
        clear();
    } else if (strcmp(command, "meminfo") == 0) {
        meminfo();
    } else if (strcmp(command, "shutdown") == 0) {
        shutdown();
    } else if (strcmp(command, "systeminfo") == 0) {
        systeminfo();
    } else {
        term_print("Unknown command: ");
        term_print(command);
        term_print("\n");
    }
}


void Commands::echo(const char* args) {
    term_print(args);
    term_print("\n");
}

void Commands::help() {
    term_print("Available commands:\n");
    term_print("  echo [text] - Print the given text\n");
    term_print("  help - Display this help message\n");
    term_print("  clear - Clear the screen\n");
    term_print("  meminfo - Display memory information\n");
    term_print("  systeminfo - Display system information\n");
    term_print("  shutdown - Shut down the system\n");
}

void Commands::systeminfo() {
    ACPI acpi;
    if (acpi.initialize()) {
        acpi.print_system_info();
    } else {
        term_print("Failed to initialize ACPI\n");
    }
}

void Commands::clear() {
    term_clear();
}

void Commands::meminfo() {
    print_memory_info();
}

void Commands::shutdown() {
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