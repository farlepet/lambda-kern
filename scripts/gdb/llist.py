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
class LList():
    def __init__(self, _list):
        self.llist = _list
        
        fnames = [ f.name for f in self.llist.type.fields() ]

        if 'list' in fnames:
            # llist_t
            self.head = self.llist['list']
        elif 'next' in fnames:
            # llist_item_t
            self.head = self.llist
        else:
            print("Unknown llist type!")
            return
    
    def get(self, idx):
        depth = 0
        node  = self.head

        while depth < idx:
            if node['next'] == self.head:
                return None
            node = node['next']

            depth += 1
        
        return node['data']
    
    def iterate(self, callback, max = 0):
        depth = 0
        node  = self.head

        while (max == 0) or (depth < max):
            callback(depth, node['data'])

            if node['next'] == self.head:
                break
            node = node['next']

            depth += 1



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
        self.data_type = ""
        if len(args) >= 2:
            max_depth = int(args[1])
        if len(args) >= 3:
            self.data_type = args[2]

        llist = LList(gdb.parse_and_eval(expr))
        llist.iterate(self.printItemCallback, max_depth)

    def printItemCallback(self, idx, item):
        if len(self.data_type) > 0 and item:
            data_expr = "*({} *){}".format(self.data_type, item)
            data      = gdb.parse_and_eval(data_expr)
            print("llist[{}] {} -> {}".format(idx, item, data))
        else:
            print("llist[{}]: {}".format(idx, item))


LListPrintCommand()
