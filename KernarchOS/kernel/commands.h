#ifndef COMMANDS_H
#define COMMANDS_H

#include <stddef.h>

#define MAX_COMMAND_LENGTH 256

class Commands {
public:
    static void execute(const char* command);

private:
    static void echo(const char* args);
    static void help();
    static void clear();
    static void meminfo();
    static void shutdown();
    static void systeminfo();
};

#endif // COMMANDS_H