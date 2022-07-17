#!/usr/bin/env python
import gdb
import sys

class KallocPrintCommand(gdb.Command):
    MAX_DEPTH = 64

    def __init__(self):
        super(KallocPrintCommand, self).__init__("list-allocs",
                                                gdb.COMMAND_DATA, gdb.COMPLETE_SYMBOL)

    def invoke(self, argument, from_tty):
        args = gdb.string_to_argv(argument)
        if len(args) > 2:
            print("USAGE: list-allocs [addr] [end_addr]")
            return
 
        alloc_array = "allocs"

        # @todo Base on architecture
        start_addr = 0
        end_addr   = 0xffffffff
        if len(args) == 1:
            start_addr = end_addr = int(args[0], base=16)
        elif len(args) == 2:
            start_addr = int(args[0], base=16)
            end_addr   = int(args[1], base=16)


        print("BLK  IDX: Used Address      Size")
        for blk in range(0,512):
            alloc_block_eval = "{}[{}]".format(alloc_array, blk)
            alloc_block      = gdb.parse_and_eval(alloc_block_eval)
            if alloc_block == 0:
                continue
            for idx in range(0,1024):
                alloc = alloc_block[idx]
                if alloc['valid'] == 0:
                    continue
                if (start_addr >= (alloc['addr'] + alloc['size'])) or (end_addr < alloc['addr']):
                   continue

                used = int(alloc['used'])
                addr = alloc['addr'].format_string(format='x').ljust(12)
                size = alloc['size'].format_string(format='x').ljust(12)
                print("{: 3d} {: 4d}:    {} {} {}".format(blk, idx, used, addr, size))

KallocPrintCommand()
