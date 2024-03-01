#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"


void paging_load_directory(uint32_t* directory);

// Globale Variable, die das aktuell geladene Page Directory verfolgt.
static uint32_t* current_directory = 0;

// Funktion zum Initialisieren eines 4-GB großen Paging-Bereichs.
struct paging_4gb_chunk* paging_new_4gb(uint8_t flags)
{
    // Reserviert Speicher für die Page Directorys. Jeder Eintrag zeigt auf eine Page Table.
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        // Reserviert Speicher für die 1024 Page Tables.
        uint32_t* entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        for (int b = 0; b < PAGING_TOTAL_ENTRIES_PER_TABLE; b++)
        {
            // Konfiguriert jeden entry in der Table mit der Basisadresse der page und den Flags.
            entry[b] = (offset + (b * PAGING_PAGE_SIZE)) | flags;
        }
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
        // Konfiguriert die Directory entrys, um auf die Page Tables zu verweisen.
        directory[i] = (uint32_t)entry | flags | PAGING_IS_WRITEABLE;
    }

    // Erstellt und gibt einen Chunk für die 4 GB Paging-Struktur zurück.
    struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
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
