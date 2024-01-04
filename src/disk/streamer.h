#ifndef DISKSTREAMER_H
#define DISKSTREAMER_H


#include "disk.h"


struct disk_stream
{
    int pos;            //The current position in the disk from where the stream is reading. It keeps track of where you are in the disk as you read data. This streamer acts as if the entire storage medium is a array of bytes and the "pos" represents the index in that array to a character.
    struct disk* disk;  //This is a pointer to the disk structure, which represents the specific disk the stream is associated with.
};


struct disk_stream* diskstreamer_new(int disk_id);
int diskstreamer_seek(struct disk_stream* stream, int pos);
int diskstreamer_read(struct disk_stream* stream, void* out, int total);
void diskstreamer_close(struct disk_stream* stream);




#endif