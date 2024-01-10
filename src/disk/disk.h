#ifndef DISK_H
#define DISK_H


#include "config.h"
#include "fs/file.h"

typedef unsigned int CENTOS_DISK_TYPE;


//Represents a real physical hard disk
#define CENTOS_DISK_TYPE_REAL 0
struct disk
{
    CENTOS_DISK_TYPE type;
    int sector_size;
    struct filesystem* filesysem;

};





void disk_search_and_init();
struct disk* disk_get (int index);
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf);


#endif