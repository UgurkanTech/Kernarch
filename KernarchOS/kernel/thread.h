#ifndef THREAD_H
#define THREAD_H

#include "types.h"
#include "process.h"

enum ThreadState {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_SLEEPING,
    THREAD_TERMINATED
};

struct Thread {
    PCB* pcb;                    // Process Control Block
    union {
        void (*entry_void)();           // void func()
        void (*entry_void_arg)(const char*); // void func(const char*)
        int32_t  (*entry_int)();            // int func()
        int32_t  (*entry_int_arg)(const char*);  // int func(const char*)
    } entry_point;
    const char* arg;             // Optional string argument
    uint32_t wake_time;          // Time to wake up (for sleep)
    ThreadState state;           // Thread state
    bool has_arg;
    int32_t return_code;
};

class ThreadManager {
public:    
    // Specialized thread creation functions
    template<typename F>
    static Thread* create_thread(F entry_point, const char* arg = nullptr);
    
    static void exit_thread(int32_t return_code = 0);
    static void sleep(uint32_t milliseconds);
    static void update_sleeping_threads();
    
    static Thread* get_current_thread();
    static bool is_thread_ready(Thread* thread);

private:
    static void thread_wrapper();
};

void thread_sleep(uint32_t milliseconds);

#endif // THREAD_H