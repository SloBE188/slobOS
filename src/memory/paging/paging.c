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

int paging_map_to(uint32_t *directory, void *virt, void *phys, void *phys_end, int flags)
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
    res = paging_map_range(directory, virt, phys, total_pages, flags);

out:
    return res;

}


// Wechselt das aktuelle Page Directory.
void paging_switch(uint32_t* directory)
{
    // Lädt das neue Page Directory und aktualisiert die globale Variable.
    paging_load_directory(directory);
    current_directory = directory;
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
