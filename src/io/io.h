#ifndef IO_H
#define IO_H


unsigned char insb(unsigned short port);    //reading 1 byte from the port
unsigned short insw(unsigned short port);   //reading 2 bytes from the port


void outb(unsigned short port, unsigned char val);  //outputing 1 byte to the port
void outw(unsigned short port, unsigned short val); //outputing 2 bytes to the port
#endif