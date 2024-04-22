#include "vbe.h"



void main()
{
    vbe_mode_info_structure *vbe_info = mode_info;
    // Zugriff auf die Framebuffer-Adresse
    unsigned int *framebuffer_addr = mode_info->framebuffer;
}