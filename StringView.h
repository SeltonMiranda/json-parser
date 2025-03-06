#ifndef __STRING_VIEW__
#define __STRING_VIEW__

#include <stdio.h>
#include <string.h>

#define sv_null (String_View){ .data = NULL, .size = 0 }
#define sv(c_str) (String_View){ .data = c_str, .size = (c_str) ? strlen(c_str) : 0 }
#define sv_args(str) (int)(str.size), (str.data)
#define sv_fmt "%.*s"

typedef struct String_View {
    char* data;
    size_t size;
} String_View;

void r_trim(String_View *str);
void l_trim(String_View *str);
void trim(String_View *str);
int string_view_equals(String_View a, String_View b);
int sv_to_owned(String_View sv, char **c_str);

#endif // __STRING_VIEW__

#ifdef __STRING_VIEW_IMPLEMENTATION__

#include <ctype.h>
#include <stdlib.h>

void r_trim(String_View *str)
{
    while (str->size > 0 && isspace(str->data[str->size - 1]))
    {
        str->size--;
    }
}

void l_trim(String_View *str)
{
    while (str->size > 0 && isspace(str->data[0]))
    {
        str->data++;
        str->size--;
    }
}

void trim(String_View *str)
{
    l_trim(str);
    r_trim(str);
}

int sv_cmp(String_View a, String_View b)
{
    return (a.size == b.size) && (strncmp(a.data, b.data, a.size) == 0);
}

int sv_to_owned(String_View sv, char **c_str)
{
    *c_str = malloc(sizeof(char) * (sv.size + 1));
    if (!(*c_str))
    {
        printf("ERROR! Couldn't alloc memory for c_str\n");
        return 0;
    }
    strncpy((*c_str), sv.data, sv.size);
    (*c_str)[sv.size] = '\0';
    return 1;
}

#endif // __STRING_VIEW_IMPLEMENTATION__