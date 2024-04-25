#include "vbe.h"

extern vbe_mode_info_structure *mode_info;


void put_pixel(int x, int y, uint32_t color) 
{
    vbe_mode_info_structure *vbe_info = mode_info;
    uint32_t *buffer = vbe_info->framebuffer;
    buffer[y * (vbe_info.pitch / 4) + x] = color;
}

void draw_rectangle(int x, int y, int width, int height, uint32_t color) 
{

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            put_pixel(x + j, y + i, color);
        }
    }
}


void draw_window(int x, int y, int width, int height, uint32_t border_color, uint32_t title_color, uint32_t background_color, const char* title) 
{

    // Zeichne den Fensterhintergrund
    draw_rectangle(x, y, width, height, background_color);

    // Zeichne den Titelbalken
    int title_height = 20; // Höhe des Titelbalkens festlegen
    draw_rectangle(x, y, width, title_height, title_color);

    // Zeichne den Fensterrahmen
    draw_rectangle(x, y, width, 2, border_color); // Oberer Rahmen
    draw_rectangle(x, y + height - 2, width, 2, border_color); // Unterer Rahmen
    draw_rectangle(x, y, 2, height, border_color); // Linker Rahmen
    draw_rectangle(x + width - 2, y, 2, height, border_color); // Rechter Rahmen

    // Setze den Titeltext
    int text_x = x + 5; // Ein wenig Rand von links
    int text_y = y + 2; // Ein wenig Rand von oben
    while (*title) {
        draw_char(text_x, text_y, *title, 0xFFFFFFFF); // Weiß für den Text
        title++;
        text_x += 8; // Verschiebe den Textcursor für 8x8 Zeichen
        if (text_x > (x + width - 8)) break; // Verhindert das Schreiben außerhalb des Titelbalkens
    }
}

/*Verwendung aus dem kernel.c file könnte so aussehen: draw_window(50, 50, 300, 200, 0xFF0000, 0x0000FF, 0xCCCCCC, "Mein Fenster");
*/
