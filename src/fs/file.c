#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "kernel.h"


//Here is the maximum of total filesystem as specified in config.h
struct filesystem* filesystems[CENTOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[CENTOS_MAX_FILE_DESCRIPTORS];



/*This function gets a free available filesystem slot from the filesystems array that can be used
by someone who wants to register a new filesystem in the VFS Layer.*/
static struct filesystem** fs_get_free_filesystem()
{
    int i = 0;
    for (i = 0; i < CENTOS_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] == 0)
        {
            return &filesystems[i];
        }
    }

    return 0;
}


/*When a filesystem driver that we will implement such as FAT16 wants to register its self in our VFS system it
will call this funciton and pass a filesystem structure
that implements all its pointers to its underlying open, write and read functions.
Additionally the filesystem name would be provided. This function then clones the data and stores
it in a free filesystem array index.*/
void fs_insert_filesystem(struct filesystem* filesystem)
{
    struct filesystem** fs;
    fs = fs_get_free_filesystem();
    if (!fs)
    {
        print("Problem inserting filesystem"); 
        while(1) {}
    }

    *fs = filesystem;
}


static void fs_static_load()
{
    //fs_insert_filesystem(fat16_init());
}


//THis function will load all the filesystem implementations into memory.
void fs_load()
{
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

void fs_init()
{
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}


//This function will be used to create a new descriptor for a give file
static int file_new_descriptor(struct file_descriptor** desc_out)
{
    int res = -ENOMEM;
    for (int i = 0; i < CENTOS_MAX_FILE_DESCRIPTORS; i++)
    {
        if (file_descriptors[i] == 0)
        {
            struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));
            // Descriptors start at 1
            desc->index = i + 1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }

    return res;
}

/*This function returns a file descriptor based on a index number. In linux the "open" function opens a file and returns a descriptor number.
THis here works similary. We identify open file descriptors by numbers, we can get the descriptor information if we know the number. The descriptor describes the open file*/
static struct file_descriptor* file_get_descriptor(int fd)
{
    if (fd <= 0 || fd >= CENTOS_MAX_FILE_DESCRIPTORS)
    {
        return 0;
    }

    // Descriptors start at 1
    int index = fd - 1;
    return file_descriptors[index];
}


/*This function will return the filesystemm that can read the provided disk. If no loaded filesystem can read the disk
then NULL is returned.*/
struct filesystem* fs_resolve(struct disk* disk)
{
    struct filesystem* fs = 0;
    for (int i = 0; i < CENTOS_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0)
        {
            fs = filesystems[i];
            break;
        }
    }

    return fs;
}

//Start for the fopen function.
int fopen(const char* filename, const char* mode)
{
    return -EIO;
}