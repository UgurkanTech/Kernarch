#ifndef LOGGER_H
#define LOGGER_H

#include "terminal.h"

enum LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
};

class Logger {
public:
    static void init() {
        set_log_level(LOG_INFO); // Default log level
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
        log(LOG_DEBUG, message);
    }

    static void info(const char* message) {
        log(LOG_INFO, message);
    }

    static void warning(const char* message) {
        log(LOG_WARNING, message);
    }

    static void error(const char* message) {
        log(LOG_ERROR, message);
    }

    static void critical(const char* message) {
        log(LOG_CRITICAL, message);
    }

private:
    static LogLevel current_log_level;

    static const char* get_level_prefix(LogLevel level) {
        switch (level) {
            case LOG_DEBUG:   return "[DEBUG] ";
            case LOG_INFO:    return "[INFO] ";
            case LOG_WARNING: return "[WARNING] ";
            case LOG_ERROR:   return "[ERROR] ";
            case LOG_CRITICAL:return "[CRITICAL] ";
            default:          return "[UNKNOWN] ";
        }
    }

    static void set_log_color(LogLevel level) {
        switch (level) {
            case LOG_DEBUG:
                set_text_color(VGA_LIGHT_GRAY);
                break;
            case LOG_INFO:
                set_text_color(VGA_LIGHT_GREEN);
                break;
            case LOG_WARNING:
                set_text_color(VGA_YELLOW);
                break;
            case LOG_ERROR:
                set_text_color(VGA_LIGHT_RED);
                break;
            case LOG_CRITICAL:
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

// Define the static member variables
LogLevel Logger::current_log_level = LOG_INFO;

#endif // LOGGER_H