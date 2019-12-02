#ifndef X86_PROC_H
#define X86_PROC_H

extern void enter_ring(int ring, void *address, int argc, const char **argv, const char **envp);

#define enter_ring_noargs(RING, ADDRESS) enter_ring(RING, ADDRESS, 0, 0, 0)

#endif