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

# Adapted from http://jk.ozlabs.org/blog/post/156/linked-list-debugging-gdb/

class LListPrintCommand(gdb.Command):
    MAX_DEPTH = 64

    def __init__(self):
        super(LListPrintCommand, self).__init__("llist-print",
                                                gdb.COMMAND_DATA, gdb.COMPLETE_SYMBOL)

    def invoke(self, argument, from_tty):
        args = gdb.string_to_argv(argument)
        if (len(args) < 1) or (len(args) > 3):
            print("USAGE: llist-print <llist> [max-depth] [data-type]")
            return
        
        expr      = args[0]
        max_depth = self.MAX_DEPTH
        data_type = ""
        if len(args) >= 2:
            max_depth = int(args[1])
        if len(args) >= 3:
            data_type = args[2]

        llist  = gdb.parse_and_eval(expr)
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

        while depth < max_depth:
            if len(data_type) > 0 and node['data']:
                data_expr = "*({} *){}".format(data_type, node['data'])
                data      = gdb.parse_and_eval(data_expr)
                print("llist[{}] {} -> {}".format(depth, node['data'], data))
            else:
                print("llist[{}]: {}".format(depth, node['data']))

            if node['next'] == head:
                break
            node = node['next']

            depth += 1


LListPrintCommand()
