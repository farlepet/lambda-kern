#ifndef KTASK_KBUG_H
#define KTASK_KBUG_H

#include <types.h>

#define KBUG_IDEBUG_SUCCESS 0x00000DEB //!< IDEBUG successfully started

#ifdef DEBUGGER

enum kbug_types
{
    KBUG_PROCINFO = 0, //!< Request info about a process
    KBUG_CPUINFO  = 1, //!< Request info about the CPU
    KBUG_MEMINFO  = 2, //!< Request info about memory
    KBUG_IDEBUG   = 3  //!< Start the interactive debugger
};

enum kbug_proc_type
{
    KBUG_PROC_NPROCS  = 0, //!< Number of currently running processes
    KBUG_PROC_PROCPID = 1, //!< The PID of the selected process
    KBUG_PROC_UPROC   = 2  //!< Request a user version of the task structure
};

struct kbug_type_msg //!< Message containing type of next message
{
    uint8_t type; //!< Type of request
};

struct kbug_proc_msg //!< Message containing type of process info it requests
{
    int      pid;  //!< PID of the process whose information is wanted (not always used)
    uint8_t  type; //!< Type of information it requests
    uint32_t info; //!< Extra required information (not always used)
};

struct kbug_mem_msg //!< Message containing memory request
{
    uintptr_t mem_addr; //!< Address of memory to request
    uint32_t  mem_len;  //!< Length of memory request
};



// All-in-one structures, use these:
struct kbug_type_proc_msg
{
    struct kbug_type_msg ktm;
    struct kbug_proc_msg kpm;
};

struct kbug_type_mem_msg
{
    struct kbug_type_msg ktm;
    struct kbug_mem_msg kmm;
};

__noreturn void kbug_task(void);

#endif // DEBUGGER

#endif // KTASK_KBUG_H
