#ifndef LOGGER_H
#define LOGGER_H

#include "terminal.h"
#include "string_utils.h"
#include "io.h"

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static void setLogLevel(LogLevel level) {
        currentLogLevel = level;
    }

    template <typename... Args>
    static void log(LogLevel level, const char* format, Args... args) {
        if (level >= currentLogLevel) {
            uint8_t oldColor = get_terminal_color();
            set_log_color(level);

            char buffer[256];  // Adjust size as needed
            int length = format_string(buffer, sizeof(buffer), format, args...);

            term_print(getLogLevelPrefix(level));
            term_print(buffer);
            term_print("\n");

            restore_color(oldColor);
        }
    }

    template <typename... Args>
    static void logn(LogLevel level, const char* format, Args... args) {
        if (level >= currentLogLevel) {
            uint8_t oldColor = get_terminal_color();
            set_log_color(level);

            char buffer[256];  // Adjust size as needed
            int length = format_string(buffer, sizeof(buffer), format, args...);

            term_print(getLogLevelPrefix(level));
            term_print(buffer);

            restore_color(oldColor);
        }
    }

    template <typename... Args>
    static void debug(const char* format, Args... args) {
        log(DEBUG, format, args...);
    }

    template <typename... Args>
    static void info(const char* format, Args... args) {
        log(INFO, format, args...);
    }

    template <typename... Args>
    static void warning(const char* format, Args... args) {
        log(WARNING, format, args...);
    }

    template <typename... Args>
    static void error(const char* format, Args... args) {
        log(ERROR, format, args...);
    }

    template <typename... Args>
    static void critical(const char* format, Args... args) {
        log(CRITICAL, format, args...);
    }

    static void serial_log(const char* format, ...) {
        char buffer[512]; // Buffer size for formatted message
        va_list args; // Variable argument list

        va_start(args, format); // Initialize args to retrieve additional arguments
        int result = vformat_string(buffer, sizeof(buffer), format, args); // Format the message
        va_end(args); // Clean up the variable argument list


        const char* message = buffer;
        while (*message) {
            outb(0x3F8, *message++);
        }
    }   

private:
    static inline LogLevel currentLogLevel = INFO;

    static const char* getLogLevelPrefix(LogLevel level) {
        switch (level) {
            case DEBUG:   return "[DEBUG] ";
            case INFO:    return "[INFO] ";
            case WARNING: return "[WARNING] ";
            case ERROR:   return "[ERROR] ";
            case CRITICAL:return "[CRITICAL] ";
            default:      return "[UNKNOWN] ";
        }
    }

    static void set_log_color(LogLevel level) {
        switch (level) {
            case DEBUG:    set_text_color(VGA_LIGHT_GRAY); break;
            case INFO:     set_text_color(VGA_LIGHT_GREEN); break;
            case WARNING:  set_text_color(VGA_YELLOW); break;
            case ERROR:    set_text_color(VGA_LIGHT_RED); break;
            case CRITICAL: 
                set_text_color(VGA_WHITE); 
                set_text_bg_color(VGA_RED); 
                break;
            default:       set_text_color(VGA_WHITE); break; // Default case
        }
    }

    static void restore_color(uint8_t color) {
        set_text_color(color & 0x0F);
        set_text_bg_color((color >> 4) & 0x0F);
    }

};

#endif // LOGGER_H