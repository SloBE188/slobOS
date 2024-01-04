#include "streamer.h"
#include "memory/heap/kheap.h"
#include "config.h"




//This function creates a new disk stream for the disk with the given disk_id. It returns a pointer to the new disk_stream structure
struct disk_stream* diskstreamer_new(int disk_id)
{
    struct disk* disk = disk_get(disk_id);
    if (!disk)
    {
        return 0;
    }

    struct disk_stream* streamer = kzalloc(sizeof(struct disk_stream));
    streamer->pos = 0;
    streamer->disk = disk;

    return streamer;
    
}

//This function changes the current position in the disk stream to the given pos
int diskstreamer_seek(struct disk_stream* stream, int pos)
{
    stream->pos = pos;
    return 0;
}



//This function reads "total" bytes of data from the current position in the disk stream and stores it in the out buffer. It abstracts awa the need to manually load sectors from the disk.
int diskstreamer_read(struct disk_stream* stream, void* out, int total)
{
    int sector = stream->pos / CENTOS_SECTOR_SIZE;
    int offset = stream->pos % CENTOS_SECTOR_SIZE;
    char buf[CENTOS_SECTOR_SIZE];

    int res = disk_read_block(stream->disk, sector, 1, buf);
    if (res < 0)
    {
        goto out;
    }

    int total_to_read = total > CENTOS_SECTOR_SIZE ? CENTOS_SECTOR_SIZE : total;
    for (int i = 0; i < total_to_read; i++)
    {
        *(char*)out++ = buf[offset+i];
    }

    // Adjust the stream
    stream->pos += total_to_read;
    if (total > CENTOS_SECTOR_SIZE)
    {
        res = diskstreamer_read(stream, out, total-CENTOS_SECTOR_SIZE);
    }
out:
    return res;
}


//This function closes the disk stream, freeing up any ressources associated with it.
void diskstreamer_close(struct disk_stream* stream)
{
    kfree(stream);
}

