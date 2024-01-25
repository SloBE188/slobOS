#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"
void paging_load_directory(uint32_t* directory);

static uint32_t* current_directory = 0;

// Funktion zum Erstellen eines 4GB großen Paging-Bereichs.
struct paging_4gb_chunk* paging_new_4gb(uint8_t flags)
{
    // Erstellt Speicher für das Page Directory. Jeder Eintrag im Verzeichnis zeigt auf eine Page Table.
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        // Erstellt Speicher für alle Page Tables im oben erstellten Verzeichnis.
        uint32_t* entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        for (int b = 0; b < PAGING_TOTAL_ENTRIES_PER_TABLE; b++)
        {
            // Jeder Eintrag in der Tabelle wird mit der Basisadresse der Seite und den übergebenen Flags konfiguriert.
            entry[b] = (offset + (b * PAGING_PAGE_SIZE)) | flags;
        }
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
        // Der Verzeichniseintrag verweist auf die Page Table und wird als beschreibbar markiert.
        directory[i] = (uint32_t)entry | flags | PAGING_IS_WRITEABLE;
    }

    // Erstellt einen Chunk für die 4GB große Paging-Struktur und speichert das Verzeichnis darin.
    struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
}

// Funktion zum Wechseln des aktuellen Page Directories.
void paging_switch(uint32_t* directory)
{
    paging_load_directory(directory);
    current_directory = directory;
}

void paging_free_4gb(struct paging_4gb_chunk* chunk)
{
    for (int i = 0; i < 1024; i++)
    {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t* table = (uint32_t*)(entry & 0xfffff000);
        kfree(table);
    }

    kfree(chunk->directory_entry);
    kfree(chunk);
}

// Gibt das Page Directory des übergebenen 4GB Paging-Chunks zurück.
uint32_t* paging_4gb_chunk_get_directory(struct paging_4gb_chunk* chunk)
{
    return chunk->directory_entry;
}

// Überprüft, ob eine Adresse page-aligned ist.
bool paging_is_aligned(void* addr)
{
    return ((uint32_t)addr % PAGING_PAGE_SIZE) == 0;
} 

// Ermittelt die Indexe für das Directory und die Tabelle basierend auf einer virtuellen Adresse.
int paging_get_indexes(void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out)
{
    int res = 0;
    if (!paging_is_aligned(virtual_address))
    {
        res = -EINVARG;
        goto out;
    }  

    // Berechnet den Index im Page Directory und in der Page Table.
    *directory_index_out = ((uint32_t)virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out = ((uint32_t)virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);
out:
    return res;
}

// Setzt einen Wert in der Page Table für eine gegebene virtuelle Adresse.
int paging_set(uint32_t* directory, void* virt, uint32_t val)
{
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

    // Zugriff auf den entsprechenden Eintrag in der Page Table und Setzen des Wertes.
    uint32_t entry = directory[directory_index];
    uint32_t* table = (uint32_t*)(entry & 0xfffff000);
    table[table_index] = val;

    return 0;
}
