#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"


void paging_load_directory(uint32_t* directory);

// Globale Variable, die das aktuell geladene Page Directory verfolgt.
static uint32_t* current_directory = 0;

// Funktion zum Initialisieren eines 4-GB großen Paging-Bereichs.
struct paging_4gb_chunk* paging_new_4gb(uint8_t flags)
{
    // Reserviert Speicher für das Page Directory. Jeder Eintrag wird später auf eine Page Table zeigen.
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);

    // Initialisiert den Offset für die Adressberechnung.
    int offset = 0;

    // Iteriert über alle Einträge des Page Directories.
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        // Reserviert Speicher für eine Page Table, die 1024 Einträge haben wird.
        uint32_t* entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);

        // Konfiguriert jeden Eintrag in der Page Table.
        for (int b = 0; b < PAGING_TOTAL_ENTRIES_PER_TABLE; b++)
        {
            // Setzt die Basisadresse für die Seite und fügt die Flags hinzu.
            entry[b] = (offset + (b * PAGING_PAGE_SIZE)) | flags;
        }

        // Erhöht den Offset um die Größe des durch diese Table abgedeckten Speichers.
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);

        // Verknüpft den Page Directory-Eintrag mit der Page Table und setzt die Flags, einschließlich des Schreibschutzes.
        directory[i] = (uint32_t)entry | flags | PAGING_IS_WRITEABLE;
    }

    // Erstellt den Paging-Chunk und weist ihm das Page Directory zu.
    struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;

    // Gibt den initialisierten 4-GB-Paging-Chunk zurück.
    return chunk_4gb;
}



/*
uint32_t *directory: the page directory that the funcion will modify
the function first checks if the virtual and physical addresses are page aligned
if yes, it proceeds t set the mapping by calling paging_set
it maps a single page of virtual memory to a single page of physical memory
*/
int paging_map(struct paging_4gb_chunk* directory, void* virt, void* phys, int flags)
{

    if ((unsigned int)virt % PAGING_PAGE_SIZE || ((unsigned int)phys % PAGING_PAGE_SIZE))
    {
        return -EINVARG;
    }
    

    return paging_set(directory->directory_entry, virt, (uint32_t) phys | flags);
}


/*param ptr: address to be aligned
the function first checks if the address is already aligned
if the address is not aligned, the function calculates the nearest higher address that is aligned.
the newly calculated address is the returned as a void pointer*/
void* paging_align_address(void* ptr)
{
    if ((uint32_t)ptr % PAGING_PAGE_SIZE)
    {
        return (void*)((uint32_t)ptr + PAGING_PAGE_SIZE - ((uint32_t)ptr % PAGING_PAGE_SIZE));
    }

    return ptr;
    
}

//This function alignes the value to the lowest page for the value provided
void* paging_align_to_lower_page(void* addr)
{
    uint32_t _addr = (uint32_t) addr;
    _addr -= (_addr % PAGING_PAGE_SIZE);
    return (void*) _addr;
}

// Die Funktion paging_map_to ordnet eine physische Adressbereich einer virtuellen Adresse im Paging-Verzeichnis zu.
// Sie nimmt ein Verzeichnis (Page Directory), einen virtuellen Startpunkt, einen physischen Startpunkt,
// ein physisches Endpunkt und Flaggen für die Seiteneigenschaften.
int paging_map_to(struct paging_4gb_chunk* directory, void* virt, void* phys, void* phys_end, int flags)
{

    int res = 0;

    //checks the allignment (must be page aligned) of the phys, virt & phys end variable
    if((uint32_t)virt % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }

    if((uint32_t)phys % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }

    if((uint32_t) phys_end % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }

    //checks if the phys end address is smaller than the phys address wich shouldn't be.
    if((uint32_t) phys_end < (uint32_t) phys)
    {
        res = -EINVARG;
        goto out;
    }

    //calculates total bytes between the phys and phys end adresses and converts this in number of pages
    uint32_t total_bytes = phys_end - phys;
    int total_pages = total_bytes / PAGING_PAGE_SIZE;

    // Ruft eine andere Funktion auf, um den Bereich im Verzeichnis tatsächlich zuzuordnen.
    // Diese Funktion würde das Page Directory aktualisieren, um die virtuelle Adresse
    // auf den physischen Bereich zu mappen mit der gegebenen Anzahl von Seiten und Flags.
     res = paging_map_range(directory, virt, phys, total_pages, flags);

out:
    return res;

}


// Wechselt das aktuelle Page Directory.
void paging_switch(struct paging_4gb_chunk *directory)
{
    // Lädt das neue Page Directory und aktualisiert die globale Variable.
    paging_load_directory(directory->directory_entry);
    current_directory = directory->directory_entry;
}

// Gibt den Speicher eines 4-GB Paging-Chunks frei.
void paging_free_4gb(struct paging_4gb_chunk* chunk)
{
    // Durchläuft alle Page Tables im Directory und gibt sie frei.
    for (int i = 0; i < 1024; i++)
    {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t* table = (uint32_t*)(entry & 0xfffff000);
        kfree(table);
    }

    // Gibt das Page Directory und die Chunk-Struktur frei.
    kfree(chunk->directory_entry);
    kfree(chunk);
}

// Gibt das Page Directory eines 4-GB Chunks zurück.
uint32_t* paging_4gb_chunk_get_directory(struct paging_4gb_chunk* chunk)
{
    return chunk->directory_entry;
}

// Überprüft, ob eine Adresse korrekt ausgerichtet ist.
bool paging_is_aligned(void* addr)
{
    return ((uint32_t)addr % PAGING_PAGE_SIZE) == 0;
}

// Berechnet die Indexe für Directory und Table basierend auf einer virtuellen Adresse.
int paging_get_indexes(void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out)
{
    // Überprüft, ob die Adresse ausgerichtet ist und berechnet die Indexe.
    int res = 0;
    if (!paging_is_aligned(virtual_address))
    {
        res = -EINVARG;
        goto out;
    }

    *directory_index_out = ((uint32_t)virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out = ((uint32_t)virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);

out:
    return res;
}

// Aktualisiert einen Eintrag in einer Page Table.
int paging_set(uint32_t* directory, void* virt, uint32_t val)
{
    // Überprüft die Ausrichtung und setzt den Wert im entsprechenden Table-Eintrag.
    if (!paging_is_aligned(virt))
    {
        return -EINVARG;
    }

    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    int res = paging_get_indexes(virt, &directory_index, &table_index);
    if (res < 0)
    {
        return res;
    }

    uint32_t entry = directory[directory_index];
    uint32_t* table = (uint32_t*)(entry & 0xfffff000);
    table[table_index] = val;

    return 0;
}

/*
uint32_t *directoy: the page directory to be modofied with the new mappings
void* virt: the starting virtual address that needs to be mapped
void* phys: the starting physical address to which the starting virtual address will be mapped
int count: the number of contiguous pages to map
int flags: flags to set properties of the pages like read only or read write etc.*/
int paging_map_range(struct paging_4gb_chunk* directory, void* virt, void* phys, int count, int flags)
{
    int res = 0;

    //for loop for the cound(number of pages to map)
    for (int i = 0; i < count; i++)
    {
        res = paging_map(directory, virt, phys, flags);     //Maps as many single pages of virtual memory to single pages of physical memory as "counr" says 
        if (res < 0)
        {
            break;              //If the value returned by paging map is below 0, it will break out of the loop.
        }                                          
        virt += PAGING_PAGE_SIZE;
        phys += PAGING_PAGE_SIZE;
        
    }


    return res;
}

/*For copying strings from a user process to into kernel memory i have to implement the ability to get a page table entry
from a given page directory. This shit is what this function does.*/
uint32_t paging_get(uint32_t* directory, void* virt)
{
    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    paging_get_indexes(virt, &directory_index, &table_index);
    uint32_t entry = directory[directory_index];
    uint32_t* table = (uint32_t*)(entry & 0xffff000);
    return table[table_index];
}
