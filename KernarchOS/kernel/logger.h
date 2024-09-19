#ifndef LOGGER_H
#define LOGGER_H

#include "terminal.h"
#include "string_utils.h"

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static void init() {
        set_log_level(INFO); // Default log level
    }

    static void set_log_level(LogLevel level) {
        current_log_level = level;
    }

    static void log(LogLevel level, const char* message) {
        if (level >= current_log_level) {
            uint8_t old_color = get_terminal_color();
            set_log_color(level);
            term_print(get_level_prefix(level));
            term_print(message);
            term_print("\n");
            restore_color(old_color);
        }
    }

    static void debug(const char* message) {
        log(DEBUG, message);
    }

    static void info(const char* message) {
        log(INFO, message);
    }

    template<typename... Args>
    static void info(const char* format, Args... args) {
        char buffer[256];  // Adjust size as needed
        format_string(buffer, sizeof(buffer), format, args...);
        log(INFO, buffer);
    }

    template<typename... Args>
    static void error(const char* format, Args... args) {
        char buffer[256];  // Adjust size as needed
        format_string(buffer, sizeof(buffer), format, args...);
        log(ERROR, buffer);
    }



    static void warning(const char* message) {
        log(WARNING, message);
    }

    static void error(const char* message) {
        log(ERROR, message);
    }

    static void critical(const char* message) {
        log(CRITICAL, message);
    }

private:
    static inline LogLevel current_log_level = INFO; 

    static const char* get_level_prefix(LogLevel level) {
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
            case DEBUG:
                set_text_color(VGA_LIGHT_GRAY);
                break;
            case INFO:
                set_text_color(VGA_LIGHT_GREEN);
                break;
            case WARNING:
                set_text_color(VGA_YELLOW);
                break;
            case ERROR:
                set_text_color(VGA_LIGHT_RED);
                break;
            case CRITICAL:
                set_text_color(VGA_WHITE);
                set_text_bg_color(VGA_RED);
                break;
        }
    }

    static void restore_color(uint8_t color) {
        set_text_color(color & 0x0F);
        set_text_bg_color((color >> 4) & 0x0F);
    }
};

#endif // LOGGER_H