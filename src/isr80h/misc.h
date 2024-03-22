#ifndef ISR80H_MISC_H
#define ISR80H_MISC_H

/*This misc files (.c & .h) are simply responsuible for holding miscellaneous command prototypes that do not fit into
any particular category (like the test command isr80h_commando_sum)*/
struct interrupt_frame;

void* isr80h_commando_sum(struct interrupt_frame* frame);

#endif