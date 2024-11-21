#include "thread.h"
#include "pit.h"
#include "logger.h"
#include "interrupts.h"


template<typename F>
Thread* ThreadManager::create_thread(F entry_point, const char* arg) {
    Thread* thread = new Thread();
    if (!thread) {
        Logger::log(LogLevel::ERROR, "Failed to allocate thread structure");
        return nullptr;
    }

    // Create process with wrapper as entry point
    thread->pcb = create_process((void(*)())thread_wrapper);
    if (!thread->pcb) {
        delete thread;
        return nullptr;
    }

    // Determine the function type
    thread->has_arg = (arg != nullptr);
    
    // Check if the function has an argument and its return type
    if (thread->has_arg) {
        // Function with argument
        if (sizeof(entry_point) == sizeof(void(*)(const char*))) { 
            thread->entry_point.entry_void_arg = (void(*)(const char*))entry_point;
        } else {
            Logger::log(LogLevel::ERROR, "Invalid function signature for entry point with argument");
            delete thread;
            return nullptr;
        }
    } else {
        // Function without argument
        if (sizeof(entry_point) == sizeof(void(*)())) {
            thread->entry_point.entry_void = (void(*)())entry_point;
        } else {
            Logger::log(LogLevel::ERROR, "Invalid function signature for entry point without argument");
            delete thread;
            return nullptr;
        }
    }

    thread->arg = arg;
    thread->state = THREAD_READY;
    thread->wake_time = 0;
    thread->return_code = 0;
    thread->pcb->user_data = thread;

    Logger::log(LogLevel::DEBUG, "Created thread for PID %d", thread->pcb->pid);
    return thread;
}

void ThreadManager::thread_wrapper() {
    Thread* thread = (Thread*)current_process->user_data;
    if (!thread) {
        Logger::log(LogLevel::ERROR, "Invalid thread");
        ThreadManager::exit_thread(-1);
        return;
    }

    int32_t ret_code = 0;
    
    // Call appropriate entry point based on type
    if (thread->has_arg) {
        ret_code = thread->entry_point.entry_int_arg(thread->arg);
    } else {
        ret_code = thread->entry_point.entry_int();
    }

    thread->state = THREAD_TERMINATED;
    thread->return_code = ret_code;

    sys_schedule();
    
    // Should never reach here as scheduler will pick a new process
    while(1) { asm("hlt"); }
}

// Explicit template instantiations
template Thread* ThreadManager::create_thread<void(*)()>(void(*)(), const char*);
template Thread* ThreadManager::create_thread<void(*)(const char*)>(void(*)(const char*), const char*);
template Thread* ThreadManager::create_thread<int(*)()>(int(*)(), const char*);
template Thread* ThreadManager::create_thread<int(*)(const char*)>(int(*)(const char*), const char*);

void ThreadManager::exit_thread(int32_t return_code) {
    if (!current_process) return;

    Thread* thread = (Thread*)current_process->user_data;
    if (!thread) return;

    if (return_code != 0)
        thread->return_code = return_code;
    
    delete current_process->user_data;
    // Clear the user data before process termination
    current_process->user_data = nullptr;
    
    // Now terminate the process with return code
    terminate_current_process(thread->return_code);
}

void ThreadManager::sleep(uint32_t milliseconds) {
    sys_sleep(milliseconds);
}

void ThreadManager::update_sleeping_threads() {
    uint32_t current_time = get_current_time_ms();

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != TERMINATED) {
            Thread* thread = (Thread*)process_table[i].user_data;
            if (thread && thread->state == THREAD_SLEEPING) {
                if (current_time >= thread->wake_time) {
                    thread->state = THREAD_READY;
                    process_table[i].state = READY;
                }
            }
        }
    }
}

Thread* ThreadManager::get_current_thread() {
    if (!current_process) return nullptr;
    return (Thread*)current_process->user_data;
}

bool ThreadManager::is_thread_ready(Thread* thread) {
    if (!thread) return false;
    return thread->state == THREAD_READY || 
           (thread->state == THREAD_SLEEPING && 
            get_current_time_ms() >= thread->wake_time);
}

void thread_sleep(uint32_t milliseconds) {
    ThreadManager::sleep(milliseconds);
}