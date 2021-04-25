#ifndef X86_PROC_H
#define X86_PROC_H

extern void enter_ring(uint8_t ring, void *address);
extern void enter_ring_newstack(uint8_t ring, void *address, void *esp);

extern void return_from_fork();

#endif
