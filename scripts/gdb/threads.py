#!/usr/bin/env python
import gdb
import sys

sys.path.append(".")
from scripts.gdb.llist import LList

class ThreadPrintCommand(gdb.Command):
    MAX_DEPTH = 64

    def __init__(self):
        super(ThreadPrintCommand, self).__init__("list-threads",
                                                gdb.COMMAND_DATA, gdb.COMPLETE_SYMBOL)

    def invoke(self, argument, from_tty):
        args = gdb.string_to_argv(argument)
        if len(args) > 1:
            print("USAGE: list-threads [max-depth]")
            return
        
        # Determine target architecture
        frame     = gdb.selected_frame()
        self.arch = frame.architecture()
        
        thread_list = "_cpu_threads[0]"

        max_depth = self.MAX_DEPTH
        if len(args) >= 1:
            max_depth = int(args[0])

        llist = LList(gdb.parse_and_eval(thread_list))

        print("thread[IDX]: TID Name             Blocked Instruction  Time(s)")

        llist.iterate(self.printItemCallback, max_depth)

    def printItemCallback(self, idx, item):
        thread  = gdb.parse_and_eval("*(kthread_t *){}".format(item))
        # NOTE: Only supports x86 at the moment
        # NOTE: EIP does not currently work with kernel threads, as they do not
        # change stack on interrupt
        thread_tid     = int(thread['tid'])
        thread_name    = thread['name'].string().ljust(16)
        thread_blocked = str(thread['cond'] != 0)

        if self.arch.name() == "i386":
            thread_stack     = thread['arch']['stack_kern']['begin']
            thread_iret_eval = "*(arch_iret_regs_t *)({} - sizeof(arch_iret_regs_t))".format(thread_stack)
            thread_iret      = gdb.parse_and_eval(thread_iret_eval)
            thread_ip        = thread_iret['eip'].format_string(format='x').ljust(12)
        elif self.arch.name() == "armv7":
            thread_stack   = thread['arch']['stack_kern']['begin']
            thread_ip_eval = "((uint32_t *){})[-1]".format(thread_stack)
            thread_ip      = gdb.parse_and_eval(thread_ip_eval).format_string(format='x').ljust(12)
        else:
            # TODO
            thread_ip = 0

        thread_time    = float(thread['stats']['sched_time_accum']) / 1000000000.0

        print("thread[{: 3d}]: {: 3d} {} {:7s} {} {:03.3f}"
              .format(idx,
                      thread_tid,
                      thread_name,
                      thread_blocked,
                      thread_ip,
                      thread_time))

class ProcessPrintCommand(gdb.Command):
    MAX_DEPTH = 64

    def __init__(self):
        super(ProcessPrintCommand, self).__init__("list-procs",
                                                gdb.COMMAND_DATA, gdb.COMPLETE_SYMBOL)

    def invoke(self, argument, from_tty):
        args = gdb.string_to_argv(argument)
        if len(args) > 2:
            print("USAGE: list-procs [max-depth] [print-threads]")
            return
        
        thread_list = "procs"

        max_depth     = self.MAX_DEPTH
        self.print_threads = False
        if len(args) >= 1:
            max_depth = int(args[0])
        if len(args) >= 2:
            self.print_threads = int(args[1])

        llist = LList(gdb.parse_and_eval(thread_list))

        print("proc[IDX]: PID Name             Time(s)")

        llist.iterate(self.printItemCallback, max_depth)

    def printItemCallback(self, idx, item):
            proc = gdb.parse_and_eval("*(kproc_t *){}".format(item))

            proc_pid  = int(proc['pid'])
            proc_name = proc['name'].string().ljust(16)

            self.time_sum = 0
            threads = LList(proc['threads'])
            threads.iterate(self.threadTimeSumCallback)

            proc_time = float(self.time_sum) / 1000000000.0

            print("proc[{: 3d}]: {: 3d} {} {:03.3f}"
                  .format(idx,
                          proc_pid,
                          proc_name,
                          proc_time))
            
            if self.print_threads:
                threads.iterate(self.printThreadsCallback)
    
    def threadTimeSumCallback(self, idx, item):
        thread         = gdb.parse_and_eval("*(kthread_t *){}".format(item))
        thread_time    = int(thread['stats']['sched_time_accum'])
        self.time_sum += thread_time

    def printThreadsCallback(self, idx, item):
        thread  = gdb.parse_and_eval("*(kthread_t *){}".format(item))

        thread_tid     = int(thread['tid'])
        thread_name    = thread['name'].string().ljust(16)
        thread_time    = float(thread['stats']['sched_time_accum']) / 1000000000.0

        print("    <{: 3d}>: {: 3d} {} {:03.3f}"
              .format(idx,
                      thread_tid,
                      thread_name,
                      thread_time))


ThreadPrintCommand()
ProcessPrintCommand()
