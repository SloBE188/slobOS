#ifndef ISR80H_PROCESS_H
#define ISR80H_PROCESS_H

struct interrupt_frame;

void *isr80h_command_process_load_from_shell(struct interrupt_frame *frame);





#endif