#ifndef FILE_H
#define FILE_H

#include "pparser.h"

typedef unsigned int FILE_SEEK_MODE;
enum
{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};


typedef unsigned int FILE_MODE;
enum
{ 
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};

struct disk;

//These are function pointers to be inherted by a particular filesystem driver.
typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);


struct filesystem
{
    /*The filesystem should return zero from resolve if the provided disk is using its filesystem.
    When the VFS éayer want to determine a filesystem, it will loop through all available registered filesystem and run resolve.
    If it returns zero, then the filesystem driver is indicating that it understands how to read the given disk.*/
    FS_RESOLVE_FUNCTION resolve;

    //This function pointer is called when we open a file in a given filesystem.
    FS_OPEN_FUNCTION open;

    //Name of the filesystem. Example: FAT16.
    char name[20];
};


    /*File descriptors represent open files. When you call "fopen", a file is opened and a file descriptor is created ro represent it.*/
struct file_descriptor
{
    // The descriptor index
    int index;
    //THe filesystem, this particular file is in.
    struct filesystem* filesystem;

    // Private data for internal file descriptor.
    void* private;

    // The disk that the file descriptor should be used on.
    struct disk* disk;
};

void fs_init();
int fopen(const char* filename, const char* mode_str);
void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);
#endif