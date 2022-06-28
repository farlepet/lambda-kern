#!/usr/bin/env python
import gdb

"""
struct llist_item {
    void         *data; /** Data represented by list item */
    llist_item_t *prev; /** Previous item in list */
    llist_item_t *next; /** Next item in list */
};

struct llist {
    llist_item_t *list; /** Pointer to first item in list */
    lock_t        lock; /** Lock to ensure only a single thread touches the list at a time */
};
"""

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
        
        thread_list = "_cpu_threads[0]"

        max_depth = self.MAX_DEPTH
        if len(args) >= 1:
            max_depth = int(args[0])

        llist  = gdb.parse_and_eval(thread_list)
        fnames = [ f.name for f in llist.type.fields() ]

        if 'list' in fnames:
            # llist_t
            head = llist['list']
        elif 'next' in fnames:
            # llist_item_t
            head = llist
        else:
            print("Unknown llist type!")
            return

        depth = 0
        node  = head

        print("thread[IDX]: TID Name             Blocked    Instruction  Time(s)")

        while depth < max_depth:
            thread_expr = "*(kthread_t *){}".format(node['data'])
            thread      = gdb.parse_and_eval(thread_expr)
            # NOTE: Only supports x86 at the moment
            # NOTE: Does not currently work with kernel threads, as they do not
            # change stack on interrupt
            thread_tid     = int(thread['tid'])
            thread_name    = thread['name'].string().ljust(16)
            thread_blocked = thread['blocked'].format_string(format='x').ljust(10)

            thread_stack     = thread['arch']['stack_kern']['begin']
            thread_iret_eval = "*(arch_iret_regs_t *)({} - sizeof(arch_iret_regs_t))".format(thread_stack)
            thread_iret      = gdb.parse_and_eval(thread_iret_eval)
            thread_eip       = thread_iret['eip'].format_string(format='x').ljust(12)
            thread_time    = float(thread['stats']['sched_time_accum']) / 1000000000.0

            print("thread[{: 3d}]: {: 3d} {} {} {} {:03.3f}"
                  .format(depth,
                          thread_tid,
                          thread_name,
                          thread_blocked,
                          thread_eip,
                          thread_time))

            if node['next'] == head:
                break
            node = node['next']

            depth += 1


ThreadPrintCommand()
