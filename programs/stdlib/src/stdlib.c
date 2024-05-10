#include "stdlib.h"
#include "slobos.h"


char* itoa(int i)
{
    static char text[12];       // Ein statischer Puffer von 12 Zeichen, der global und nicht im Stack gespeichert wird wegen static. 
                                // Groß genug für alle 32-Bit Ganzzahlen und das Nullzeichen.

    int loc = 11;               // loc wird verwendet, um die aktuelle Position im Puffer 'text' zu speichern.
    text[11] = 0;               // Setzt das terminierende Nullzeichen für den C-String.

    char neg = 1;               // Flag für negative Zahlen. Startet mit 1 (wahr), was bedeutet, dass die Zahl als negativ angenommen wird.

    if (i >= 0)
    {
        neg = 0;                // Wenn die Zahl nicht negativ ist, setze das Flag auf 0 (falsch).
        i = -i;                 // Konvertiere die Zahl in ihre negative Form, um Modulo-Operationen zu vereinfachen.
    }

    while(i)                    // Schleife, die durchläuft, solange i nicht 0 ist.
    {
        text[--loc] = '0' - (i % 10); // Berechnet die letzte Ziffer, subtrahiert diese von '0', um das entsprechende ASCII-Zeichen zu erhalten,
                                     // und fügt sie an der aktuellen Position ein. Dekrementiert loc vor der Zuweisung.
        i /= 10;                      // Entfernt die letzte Ziffer von i.
    }

    if (loc == 11)              // Falls keine Ziffern verarbeitet wurden (i war 0).
        text[--loc] = '0';      // Fügt eine '0' hinzu, da i ursprünglich 0 war.

    if (neg)                    // Wenn die Zahl negativ ist.
        text[--loc] = '-';      // Fügt das Minuszeichen am Anfang des Strings hinzu.

    return &text[loc];          // Gibt einen Zeiger auf den Anfang des generierten Strings zurück.
}


void *malloc(size_t size)
{
    return slobos_malloc(size);
}


void free(void *ptr)
{
    slobos_free(ptr);
}