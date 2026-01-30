#include "concat.h"

#include "stdio.h"

// Concatenates the boot tag with the chip model and returns a new dynamically allocated string.
char * concat(const char * f_part, const char * s_part)
{
    char * result;

    if(asprintf(&result, "%s%s", f_part, s_part) == -1)
    {
        return NULL;
    }
    
    return result;
}