#ifndef X86_PROC_H
#define X86_PROC_H

extern void enter_ring(int ring, void *address, int argc, const char **argv, const char **envp);
extern void enter_ring_newstack(int ring, void *address, int argc, const char **argv, const char **envp, void *esp);

#define enter_ring_noargs(RING, ADDRESS) enter_ring(RING, ADDRESS, 0, 0, 0)

extern void return_from_fork();

#endif