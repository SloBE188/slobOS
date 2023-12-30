#include "io/io.h"
#include "disk/disk.h"
#include "memory/memory.h"
#include "status.h"



struct disk disk;

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

void disk_search_and_init()
{

    memset(&disk, 0, sizeof(disk));
    disk.type = CENTOS_DISK_TYPE_REAL;
    disk.sector_size = CENTOS_SECTOR_SIZE;

}

struct disk* disk_get (int index)
{
    if (index != 0)
    {
        return 0;
    }
    return &disk;
    
}

int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf)
{
    if (idisk != &disk)
    {
        return -EIO;
    }

    return disk_read_sector(lba, total, buf);
    
}
