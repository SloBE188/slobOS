#ifndef FILE_H
#define FILE_H

#include "pparser.h"
#include <stdint.h>

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

enum
{
    FILE_STAT_READ_ONLY = 0b00000001
};

typedef unsigned int FILE_STAT_FLAGS;



struct disk;

//These are function pointers to be inherted by a particular filesystem driver.
typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int (*FS_READ_FUNCTION)(struct disk* disk, void* private, uint32_t size, uint32_t nmemb, char* out);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);
typedef int (*FS_CLOSE_FUNCTION)(void* private);
typedef int (*FS_SEEK_FUNCTION)(void* private, uint32_t offset, FILE_SEEK_MODE seek_mode);

struct file_stat
{
    FILE_STAT_FLAGS flags;
    uint32_t filesize;
};

typedef int (*FS_STAT_FUNCTION)(struct disk* disk, void* private, struct file_stat* stat);

struct filesystem
{
    /*The filesystem should return zero from resolve if the provided disk is using its filesystem.
    When the VFS éayer want to determine a filesystem, it will loop through all available registered filesystem and run resolve.
    If it returns zero, then the filesystem driver is indicating that it understands how to read the given disk.*/
    FS_RESOLVE_FUNCTION resolve;

    //This function pointer is called when we open a file in a given filesystem.
    FS_OPEN_FUNCTION open;


    FS_READ_FUNCTION read;

    FS_SEEK_FUNCTION seek;
    
    FS_STAT_FUNCTION stat;

    FS_CLOSE_FUNCTION close;

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
int fseek(int fd, int offset, FILE_SEEK_MODE whence);
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd);
int fstat(int fd, struct file_stat* stat);
int fclose(int fd);

void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);
#endif