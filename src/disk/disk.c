#include "io/io.h"
#include "disk/disk.h"
#include "memory/memory.h"
#include "status.h"



struct disk disk;


/*funktion liest eine bestimmte anzahl an sektoren (Parameter total) ab einer bestimmten LBA-Adresse (Parameter lba) 
von der Festplatte in den buffer (Parameter buf). 
funktion kommuniziert mit dem festplattencontroller direkt mit den I/O ports (asm labels insb/insw zum lesen von Bytes vom port & outb/outw zum schreiben von bytes an den port)
ox1F6, 0x1F2, 0x1F3, 0x1F4, 0x1F5, 0x1F7 sind standard Ports für die kommunikation mit dem primären ATA- Festplattencontroller.
Wenn beispielsweise die CPU einen Wert an den Port 0x1F6 schreibt (mit outb), legt sie Informationen im Festplattencontroller fest, 
etwa die ausgewählte Festplatte oder den Teil der LBA-Adresse. Wenn sie Daten aus Port 0x1F0 liest (mit insw), liest sie die Daten, 
die der Festplattencontroller bereitgestellt hat.
Festplattencontroller = Hardwarekomponente (Schnittstelle zwischen CPU & Festplatte)*/
int disk_read_sector(int lba, int total, void* buf)
{

    outb(0x1F6, (lba >> 24) | 0xE0);            //Select master drive and pass part of the LBA
    outb(0x1F2, total);                         //Send the total number of sectors i want to read         
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


/*funktion initialisiert die disk basierend auf der disk struktur & setzt ihre eigenschaften/sektorengrösse etc.*/
void disk_search_and_init()
{

    memset(&disk, 0, sizeof(disk));
    disk.type = SLOBOS_DISK_TYPE_REAL;
    disk.sector_size = SLOBOS_SECTOR_SIZE;
    disk.id = 0;
    disk.filesystem = fs_resolve(&disk);

}


/*funktion gibt eine referenz auf das disk struct basierend auf dem index Parameter zurück. Bei jedem index auser 0 wird ein nullzeiger zurückgegeben (geht nur index 0)*/
struct disk* disk_get (int index)
{
    if (index != 0)
    {
        return 0;
    }
    return &disk;
    
}

/*funktion liest aktiv teile der disk in form von blöcken (weil lba). dafür nutzt sie die zuvor implementierte disk_read_sector function. 
zusätzlich prüft sie noch, ob die angeforderte disk auch die tatsächlich vorhandene disk ist.*/
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf)
{
    if (idisk != &disk)
    {
        return -EIO;    //Error IO
    }

    return disk_read_sector(lba, total, buf);
    
}

    /*TESTING DISK IN KERNEL.C
    char buf[512];
    disk_read_sector(0, 1, buf);
    Da ich inzwischen den diskdriver angepasst habe (alles asgelagert)
    kann man auch so entwas von der disk lesen:
    disk_read_block(disk_get(0), 20, 4, buf) etc.
    */