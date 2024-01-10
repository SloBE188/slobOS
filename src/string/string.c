#include "string/string.h"

//This function will count the number of characters in a string
int strlen(const char* ptr)
{
    int i = 0;
    while(*ptr != 0)
    {
        i++;
        ptr += 1;
    }

    return i;
}


//This function will count the number of characters up to a maximum count, this is essential to prevent buffer overflows
int strnlen(const char* ptr, int max)
{
    int i = 0;
    for (i = 0; i < max; i++)
    {
        if (ptr[i] == 0)
            break;
    }

    return i;
}


//This function will return true if the given character is a digit character int the ASCII table
bool isdigit(char c)
{
    return c >= 48 && c <= 57;
}

//This function will convert a ASCII digit character into a actual number. For example "1" would be converted to a decimal 1 that can be used in math operations.
int tonumericdigit(char c)
{
    return c - 48;
}

char* strcpy(char* dest, const char* src)
{
    char* res = dest;
    while (*src != 0)
    {
        *dest = *src;
        src += 1;
        dest += 1;
    }

    *dest = 0x00;

    return res;
    
}
