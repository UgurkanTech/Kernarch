#ifndef COMMANDS_H
#define COMMANDS_H

#include "types.h"
#include "terminal.h"

struct Command {
    const char* name;
    const char* args;
    const char* description;
    void (*function)(const char*);
};

class Commands {
public:
    static void initialize();
    static void execute(const char* command);
    static void help();

private:
    static const int MAX_COMMANDS = 32;
    static Command command_list[MAX_COMMANDS];
    static int command_count;

    static void add_command(const char* name, const char* args, const char* description, void (*function)(const char*));
    
    // Command functions
    static void echo(const char* args);
    static void clear(const char* args);
    static void meminfo(const char* args);
    static void systeminfo(const char* args);
    static void stack(const char* args);
    static void shutdown(const char* args);
    static void test(const char* args);
};

#endif // COMMANDS_H