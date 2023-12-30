#include "io/io.h"


int disk_read_sector(int lba, int total, void* buf)
{

    outb(0x1F6, (lba >> 24) | 0xE0);            //Select master drive and pass part of the LBA
    outb(0x1F2, total);                         //Send the total number of sectors we want to read         
    outb(0x1F3,(unsigned char)(lba & 0xff));    //Send more of the LBA
    outb(0x1F4, (unsigned char)(lba >> 8));     //Send more of the LBA
    outb(0x1F5, (unsigned char)(lba >> 24));    //Send more of the LBA
    outb(0x1F7, 0x20);

    unsigned short* ptr = (unsigned short*) buf;
    for (int b = 0; b < total; b++)
    {
        //Wait for the buffer to be ready
        char c = insb(0x1F7);
        while (!(c & 0x08))
        {
            c = insb(0x1F7);
        }

        //Copy from hard disk to memory
        for (int i = 0; i < 256; i++)
        {
            *ptr = insw(0x1F0);
            ptr++;
        }
        
        
    }
    
    return 0;
}