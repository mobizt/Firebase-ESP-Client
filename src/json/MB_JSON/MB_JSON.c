/*
  MB_JSON.c v1.0.1 based on the modified version of cJSON.c v1.7.14 (Sept 3, 2020)

  All original static cJSON functions and static variables will be prefixed with MB_JSON_.
  
  Created December 20, 2021

  Copyright (c) 2022 Mobizt (K. Suwatchai)

  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/* cJSON */
/* JSON parser in C. */

#ifndef MB_JSON_C
#define MB_JSON_C

/* disable warnings about old C89 functions in MSVC */
#if !defined(_CRT_SECURE_NO_DEPRECATE) && defined(_MSC_VER)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
#if defined(_MSC_VER)
#pragma warning(push)
/* disable warning about single line comments in system headers */
#pragma warning(disable : 4001)
#endif

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <float.h>

#ifdef ENABLE_LOCALES
#include <locale.h>
#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

#include "MB_JSON.h"

/* define our own boolean type */
#ifdef true
#undef true
#endif
#define true ((MB_JSON_bool)1)

#ifdef false
#undef false
#endif
#define false ((MB_JSON_bool)0)

/* define isnan and isinf for ANSI C, if in C99 or above, isnan and isinf has been defined in math.h */
#ifndef isinf
#define isinf(d) (isnan((d - d)) && !isnan(d))
#endif
#ifndef isnan
#define isnan(d) (d != d)
#endif

#ifndef NAN
#ifdef _WIN32
#define NAN sqrt(-1.0)
#else
#define NAN 0.0 / 0.0
#endif
#endif

typedef struct
{
    const unsigned char *json;
    size_t position;
} MB_JSON_error;

static MB_JSON_error MB_JSON_global_error = {NULL, 0};

MB_JSON_PUBLIC(const char *)
MB_JSON_GetErrorPtr(void)
{
    return (const char *)(MB_JSON_global_error.json + MB_JSON_global_error.position);
}

MB_JSON_PUBLIC(char *)
MB_JSON_GetStringValue(const MB_JSON *const item)
{
    if (!MB_JSON_IsString(item))
    {
        return NULL;
    }

    return item->valuestring;
}

MB_JSON_PUBLIC(double)
MB_JSON_GetNumberValue(const MB_JSON *const item)
{
    if (!MB_JSON_IsNumber(item))
    {
        return (double)NAN;
    }

    return item->valuedouble;
}

/* This is a safeguard to prevent copy-pasters from using incompatible C and header files */
#if (MB_JSON_VERSION_MAJOR != 1) || (MB_JSON_VERSION_MINOR != 7) || (MB_JSON_VERSION_PATCH != 14)
#error MB_JSON.h and MB_JSON.c have different versions. Make sure that both have the same.
#endif

MB_JSON_PUBLIC(const char *)
MB_JSON_Version(void)
{
    static char version[15];
    sprintf(version, "%i.%i.%i", MB_JSON_VERSION_MAJOR, MB_JSON_VERSION_MINOR, MB_JSON_VERSION_PATCH);

    return version;
}

/* Case insensitive string comparison, doesn't consider two NULL pointers equal though */
static int MB_JSON_case_insensitive_strcmp(const unsigned char *string1, const unsigned char *string2)
{
    if ((string1 == NULL) || (string2 == NULL))
    {
        return 1;
    }

    if (string1 == string2)
    {
        return 0;
    }

    for (; tolower(*string1) == tolower(*string2); (void)string1++, string2++)
    {
        if (*string1 == '\0')
        {
            return 0;
        }
    }

    return tolower(*string1) - tolower(*string2);
}

typedef struct MB_JSON_internal_hooks
{
    void *(MB_JSON_CDECL *allocate)(size_t size);
    void(MB_JSON_CDECL *deallocate)(void *pointer);
    void *(MB_JSON_CDECL *reallocate)(void *pointer, size_t size);
} MB_JSON_internal_hooks;

#if defined(_MSC_VER)
/* work around MSVC error C2322: '...' address of dllimport '...' is not static */
static void *MB_JSON_CDECL internal_malloc(size_t size)
{
    return malloc(size);
}
static void MB_JSON_CDECL internal_free(void *pointer)
{
    free(pointer);
}
static void *MB_JSON_CDECL internal_realloc(void *pointer, size_t size)
{
    return realloc(pointer, size);
}
#else
#define MB_JSON_internal_malloc malloc
#define MB_JSON_internal_free free
#define MB_JSON_internal_realloc realloc
#endif

/* strlen of character literals resolved at compile time */
#define MB_JSON_static_strlen(string_literal) (sizeof(string_literal) - sizeof(""))

static MB_JSON_internal_hooks MB_JSON_global_hooks = {MB_JSON_internal_malloc, MB_JSON_internal_free, MB_JSON_internal_realloc};

static unsigned char *MB_JSON_strdup(const unsigned char *string, const MB_JSON_internal_hooks *const hooks)
{
    size_t length = 0;
    unsigned char *copy = NULL;

    if (string == NULL)
    {
        return NULL;
    }

    length = strlen((const char *)string) + sizeof("");
    copy = (unsigned char *)hooks->allocate(length);
    if (copy == NULL)
    {
        return NULL;
    }
    memcpy(copy, string, length);

    return copy;
}

MB_JSON_PUBLIC(void)
MB_JSON_InitHooks(MB_JSON_Hooks *hooks)
{
    if (hooks == NULL)
    {
        /* Reset hooks */
        MB_JSON_global_hooks.allocate = malloc;
        MB_JSON_global_hooks.deallocate = free;
        MB_JSON_global_hooks.reallocate = realloc;
        return;
    }

    MB_JSON_global_hooks.allocate = malloc;
    if (hooks->malloc_fn != NULL)
    {
        MB_JSON_global_hooks.allocate = hooks->malloc_fn;
    }

    MB_JSON_global_hooks.deallocate = free;
    if (hooks->free_fn != NULL)
    {
        MB_JSON_global_hooks.deallocate = hooks->free_fn;
    }

    /* use realloc only if both free and malloc are used */
    MB_JSON_global_hooks.reallocate = hooks->realloc_fn;
    if ((MB_JSON_global_hooks.allocate == malloc) && (MB_JSON_global_hooks.deallocate == free))
    {
        MB_JSON_global_hooks.reallocate = realloc;
    }
}

/* Internal constructor. */
static MB_JSON *MB_JSON_New_Item(const MB_JSON_internal_hooks *const hooks)
{
    MB_JSON *node = (MB_JSON *)hooks->allocate(sizeof(MB_JSON));
    if (node)
    {
        memset(node, '\0', sizeof(MB_JSON));
    }

    return node;
}

/* Delete a MB_JSON structure. */
MB_JSON_PUBLIC(void)
MB_JSON_Delete(MB_JSON *item)
{
    MB_JSON *next = NULL;
    while (item != NULL)
    {
        next = item->next;
        if (!(item->type & MB_JSON_IsReference) && (item->child != NULL))
        {
            MB_JSON_Delete(item->child);
        }
        if (!(item->type & MB_JSON_IsReference) && (item->valuestring != NULL))
        {
            MB_JSON_global_hooks.deallocate(item->valuestring);
        }
        if (!(item->type & MB_JSON_StringIsConst) && (item->string != NULL))
        {
            MB_JSON_global_hooks.deallocate(item->string);
        }
        MB_JSON_global_hooks.deallocate(item);
        item = next;
    }
}

/* get the decimal point character of the current locale */
static unsigned char MB_JSON_get_decimal_point(void)
{
#ifdef ENABLE_LOCALES
    struct lconv *lconv = localeconv();
    return (unsigned char)lconv->decimal_point[0];
#else
    return '.';
#endif
}

typedef struct
{
    const unsigned char *content;
    size_t length;
    size_t offset;
    size_t depth; /* How deeply nested (in arrays/objects) is the input at the current offset. */
    MB_JSON_internal_hooks hooks;
} MB_JSON_parse_buffer;

/* check if the given size is left to read in a given parse buffer (starting with 1) */
#define MB_JSON_can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
/* check if the buffer can be accessed at the given index (starting with 0) */
#define MB_JSON_can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define MB_JSON_cannot_access_at_index(buffer, index) (!MB_JSON_can_access_at_index(buffer, index))
/* get a pointer to the buffer at the position */
#define MB_JSON_buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)

/* Parse the input text to generate a number, and populate the result into item. */
static MB_JSON_bool MB_JSON_parse_number(MB_JSON *const item, MB_JSON_parse_buffer *const input_buffer)
{
    double number = 0;
    unsigned char *after_end = NULL;
    unsigned char number_c_string[64];
    unsigned char decimal_point = MB_JSON_get_decimal_point();
    size_t i = 0;

    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false;
    }

    /* copy the number into a temporary buffer and replace '.' with the decimal point
     * of the current locale (for strtod)
     * This also takes care of '\0' not necessarily being available for marking the end of the input */
    for (i = 0; (i < (sizeof(number_c_string) - 1)) && MB_JSON_can_access_at_index(input_buffer, i); i++)
    {
        switch (MB_JSON_buffer_at_offset(input_buffer)[i])
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '+':
        case '-':
        case 'e':
        case 'E':
            number_c_string[i] = MB_JSON_buffer_at_offset(input_buffer)[i];
            break;

        case '.':
            number_c_string[i] = decimal_point;
            break;

        default:
            goto loop_end;
        }
    }
loop_end:
    number_c_string[i] = '\0';

    number = strtod((const char *)number_c_string, (char **)&after_end);
    if (number_c_string == after_end)
    {
        return false; /* parse_error */
    }

    item->valuedouble = number;

    /* use saturation in case of overflow */
    if (number >= INT_MAX)
    {
        item->valueint = INT_MAX;
    }
    else if (number <= (double)INT_MIN)
    {
        item->valueint = INT_MIN;
    }
    else
    {
        item->valueint = (int)number;
    }

    item->type = MB_JSON_Number;

    input_buffer->offset += (size_t)(after_end - number_c_string);
    return true;
}

/* don't ask me, but the original MB_JSON_SetNumberValue returns an integer or double */
MB_JSON_PUBLIC(double)
MB_JSON_SetNumberHelper(MB_JSON *object, double number)
{
    if (number >= INT_MAX)
    {
        object->valueint = INT_MAX;
    }
    else if (number <= (double)INT_MIN)
    {
        object->valueint = INT_MIN;
    }
    else
    {
        object->valueint = (int)number;
    }

    return object->valuedouble = number;
}

MB_JSON_PUBLIC(char *)
MB_JSON_SetValuestring(MB_JSON *object, const char *valuestring)
{
    char *copy = NULL;
    /* if object's type is not MB_JSON_String or is MB_JSON_IsReference, it should not set valuestring */
    if (!(object->type & MB_JSON_String) || (object->type & MB_JSON_IsReference))
    {
        return NULL;
    }
    if (strlen(valuestring) <= strlen(object->valuestring))
    {
        strcpy(object->valuestring, valuestring);
        return object->valuestring;
    }
    copy = (char *)MB_JSON_strdup((const unsigned char *)valuestring, &MB_JSON_global_hooks);
    if (copy == NULL)
    {
        return NULL;
    }
    if (object->valuestring != NULL)
    {
        MB_JSON_free(object->valuestring);
    }
    object->valuestring = copy;

    return copy;
}

typedef struct
{
    unsigned char *buffer;
    size_t length;
    size_t offset;
    size_t depth; /* current nesting depth (for formatted printing) */
    MB_JSON_bool noalloc;
    MB_JSON_bool format; /* is this print a formatted print */
    MB_JSON_internal_hooks hooks;
} MB_JSON_printbuffer;

typedef struct
{
    size_t size;
    size_t depth;
    MB_JSON_bool format;
} MB_JSON_buffer_len_data_t;

/* realloc MB_JSON_printbuffer if necessary to have at least "needed" bytes more */
static unsigned char *MB_JSON_ensure(MB_JSON_printbuffer *const p, size_t needed)
{
    unsigned char *newbuffer = NULL;
    size_t newsize = 0;

    if ((p == NULL) || (p->buffer == NULL))
    {
        return NULL;
    }

    if ((p->length > 0) && (p->offset >= p->length))
    {
        /* make sure that offset is valid */
        return NULL;
    }

    if (needed > INT_MAX)
    {
        /* sizes bigger than INT_MAX are currently not supported */
        return NULL;
    }

    needed += p->offset + 1;
    if (needed <= p->length)
    {
        return p->buffer + p->offset;
    }

    if (p->noalloc)
    {
        return NULL;
    }

    /* calculate new buffer size */
    if (needed > (INT_MAX / 2))
    {
        /* overflow of int, use INT_MAX if possible */
        if (needed <= INT_MAX)
        {
            newsize = INT_MAX;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        newsize = needed * 2;
    }

    if (p->hooks.reallocate != NULL)
    {
        /* reallocate with realloc if available */
        newbuffer = (unsigned char *)p->hooks.reallocate(p->buffer, newsize);
        if (newbuffer == NULL)
        {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;

            return NULL;
        }
    }
    else
    {
        /* otherwise reallocate manually */
        newbuffer = (unsigned char *)p->hooks.allocate(newsize);
        if (!newbuffer)
        {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;

            return NULL;
        }

        memcpy(newbuffer, p->buffer, p->offset + 1);
        p->hooks.deallocate(p->buffer);
    }
    p->length = newsize;
    p->buffer = newbuffer;

    return newbuffer + p->offset;
}

/* calculate the new length of the string in a MB_JSON_printbuffer and update the offset */
static void MB_JSON_update_offset(MB_JSON_printbuffer *const buffer)
{
    const unsigned char *buffer_pointer = NULL;
    if ((buffer == NULL) || (buffer->buffer == NULL))
    {
        return;
    }
    buffer_pointer = buffer->buffer + buffer->offset;

    buffer->offset += strlen((const char *)buffer_pointer);
}

/* securely comparison of floating-point variables */
static MB_JSON_bool MB_JSON_compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

/* Render the number nicely from the given item into a string. */
static MB_JSON_bool MB_JSON_print_number(const MB_JSON *const item, MB_JSON_printbuffer *const output_buffer)
{
    unsigned char *output_pointer = NULL;
    double d = item->valuedouble;
    int length = 0;
    size_t i = 0;
    unsigned char number_buffer[26] = {0}; /* temporary buffer to print the number into */
    unsigned char decimal_point = MB_JSON_get_decimal_point();
    double test = 0.0;

    if (output_buffer == NULL)
    {
        return false;
    }

    /* This checks for NaN and Infinity */
    if (isnan(d) || isinf(d))
    {
        length = sprintf((char *)number_buffer, "null");
    }
    else
    {
        /* Try 15 decimal places of precision to avoid nonsignificant nonzero digits */
        length = sprintf((char *)number_buffer, "%1.15g", d);

        /* Check whether the original double can be recovered */
        if ((sscanf((char *)number_buffer, "%lg", &test) != 1) || !MB_JSON_compare_double((double)test, d))
        {
            /* If not, print with 17 decimal places of precision */
            length = sprintf((char *)number_buffer, "%1.17g", d);
        }
    }

    /* sprintf failed or buffer overrun occurred */
    if ((length < 0) || (length > (int)(sizeof(number_buffer) - 1)))
    {
        return false;
    }

    /* reserve appropriate space in the output */
    output_pointer = MB_JSON_ensure(output_buffer, (size_t)length + sizeof(""));
    if (output_pointer == NULL)
    {
        return false;
    }

    /* copy the printed number to the output and replace locale
     * dependent decimal point with '.' */
    for (i = 0; i < ((size_t)length); i++)
    {
        if (number_buffer[i] == decimal_point)
        {
            output_pointer[i] = '.';
            continue;
        }

        output_pointer[i] = number_buffer[i];
    }
    output_pointer[i] = '\0';

    output_buffer->offset += (size_t)length;

    return true;
}

/* parse 4 digit hexadecimal number */
static unsigned MB_JSON_parse_hex4(const unsigned char *const input)
{
    unsigned int h = 0;
    size_t i = 0;

    for (i = 0; i < 4; i++)
    {
        /* parse digit */
        if ((input[i] >= '0') && (input[i] <= '9'))
        {
            h += (unsigned int)input[i] - '0';
        }
        else if ((input[i] >= 'A') && (input[i] <= 'F'))
        {
            h += (unsigned int)10 + input[i] - 'A';
        }
        else if ((input[i] >= 'a') && (input[i] <= 'f'))
        {
            h += (unsigned int)10 + input[i] - 'a';
        }
        else /* invalid */
        {
            return 0;
        }

        if (i < 3)
        {
            /* shift left to make place for the next nibble */
            h = h << 4;
        }
    }

    return h;
}

/* converts a UTF-16 literal to UTF-8
 * A literal can be one or two sequences of the form \uXXXX */
static unsigned char MB_JSON_utf16_literal_to_utf8(const unsigned char *const input_pointer, const unsigned char *const input_end, unsigned char **output_pointer)
{
    long unsigned int codepoint = 0;
    unsigned int first_code = 0;
    const unsigned char *first_sequence = input_pointer;
    unsigned char utf8_length = 0;
    unsigned char utf8_position = 0;
    unsigned char sequence_length = 0;
    unsigned char first_byte_mark = 0;

    if ((input_end - first_sequence) < 6)
    {
        /* input ends unexpectedly */
        goto fail;
    }

    /* get the first utf16 sequence */
    first_code = MB_JSON_parse_hex4(first_sequence + 2);

    /* check that the code is valid */
    if (((first_code >= 0xDC00) && (first_code <= 0xDFFF)))
    {
        goto fail;
    }

    /* UTF16 surrogate pair */
    if ((first_code >= 0xD800) && (first_code <= 0xDBFF))
    {
        const unsigned char *second_sequence = first_sequence + 6;
        unsigned int second_code = 0;
        sequence_length = 12; /* \uXXXX\uXXXX */

        if ((input_end - second_sequence) < 6)
        {
            /* input ends unexpectedly */
            goto fail;
        }

        if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u'))
        {
            /* missing second half of the surrogate pair */
            goto fail;
        }

        /* get the second utf16 sequence */
        second_code = MB_JSON_parse_hex4(second_sequence + 2);
        /* check that the code is valid */
        if ((second_code < 0xDC00) || (second_code > 0xDFFF))
        {
            /* invalid second half of the surrogate pair */
            goto fail;
        }

        /* calculate the unicode codepoint from the surrogate pair */
        codepoint = 0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
    }
    else
    {
        sequence_length = 6; /* \uXXXX */
        codepoint = first_code;
    }

    /* encode as UTF-8
     * takes at maximum 4 bytes to encode:
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if (codepoint < 0x80)
    {
        /* normal ascii, encoding 0xxxxxxx */
        utf8_length = 1;
    }
    else if (codepoint < 0x800)
    {
        /* two bytes, encoding 110xxxxx 10xxxxxx */
        utf8_length = 2;
        first_byte_mark = 0xC0; /* 11000000 */
    }
    else if (codepoint < 0x10000)
    {
        /* three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
        utf8_length = 3;
        first_byte_mark = 0xE0; /* 11100000 */
    }
    else if (codepoint <= 0x10FFFF)
    {
        /* four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx */
        utf8_length = 4;
        first_byte_mark = 0xF0; /* 11110000 */
    }
    else
    {
        /* invalid unicode codepoint */
        goto fail;
    }

    /* encode as utf8 */
    for (utf8_position = (unsigned char)(utf8_length - 1); utf8_position > 0; utf8_position--)
    {
        /* 10xxxxxx */
        (*output_pointer)[utf8_position] = (unsigned char)((codepoint | 0x80) & 0xBF);
        codepoint >>= 6;
    }
    /* encode first byte */
    if (utf8_length > 1)
    {
        (*output_pointer)[0] = (unsigned char)((codepoint | first_byte_mark) & 0xFF);
    }
    else
    {
        (*output_pointer)[0] = (unsigned char)(codepoint & 0x7F);
    }

    *output_pointer += utf8_length;

    return sequence_length;

fail:
    return 0;
}

/* Parse the input text into an unescaped cinput, and populate item. */
static MB_JSON_bool MB_JSON_parse_string(MB_JSON *const item, MB_JSON_parse_buffer *const input_buffer)
{
    const unsigned char *input_pointer = MB_JSON_buffer_at_offset(input_buffer) + 1;
    const unsigned char *input_end = MB_JSON_buffer_at_offset(input_buffer) + 1;
    unsigned char *output_pointer = NULL;
    unsigned char *output = NULL;

    /* not a string */
    if (MB_JSON_buffer_at_offset(input_buffer)[0] != '\"')
    {
        goto fail;
    }

    {
        /* calculate approximate size of the output (overestimate) */
        size_t allocation_length = 0;
        size_t skipped_bytes = 0;
        while (((size_t)(input_end - input_buffer->content) < input_buffer->length) && (*input_end != '\"'))
        {
            /* is escape sequence */
            if (input_end[0] == '\\')
            {
                if ((size_t)(input_end + 1 - input_buffer->content) >= input_buffer->length)
                {
                    /* prevent buffer overflow when last input character is a backslash */
                    goto fail;
                }
                skipped_bytes++;
                input_end++;
            }
            input_end++;
        }
        if (((size_t)(input_end - input_buffer->content) >= input_buffer->length) || (*input_end != '\"'))
        {
            goto fail; /* string ended unexpectedly */
        }

        /* This is at most how much we need for the output */
        allocation_length = (size_t)(input_end - MB_JSON_buffer_at_offset(input_buffer)) - skipped_bytes;
        output = (unsigned char *)input_buffer->hooks.allocate(allocation_length + sizeof(""));
        if (output == NULL)
        {
            goto fail; /* allocation failure */
        }
    }

    output_pointer = output;
    /* loop through the string literal */
    while (input_pointer < input_end)
    {
        if (*input_pointer != '\\')
        {
            *output_pointer++ = *input_pointer++;
        }
        /* escape sequence */
        else
        {
            unsigned char sequence_length = 2;
            if ((input_end - input_pointer) < 1)
            {
                goto fail;
            }

            switch (input_pointer[1])
            {
            case 'b':
                *output_pointer++ = '\b';
                break;
            case 'f':
                *output_pointer++ = '\f';
                break;
            case 'n':
                *output_pointer++ = '\n';
                break;
            case 'r':
                *output_pointer++ = '\r';
                break;
            case 't':
                *output_pointer++ = '\t';
                break;
            case '\"':
            case '\\':
            case '/':
                *output_pointer++ = input_pointer[1];
                break;

            /* UTF-16 literal */
            case 'u':
                sequence_length = MB_JSON_utf16_literal_to_utf8(input_pointer, input_end, &output_pointer);
                if (sequence_length == 0)
                {
                    /* failed to convert UTF16-literal to UTF-8 */
                    goto fail;
                }
                break;

            default:
                goto fail;
            }
            input_pointer += sequence_length;
        }
    }

    /* zero terminate the output */
    *output_pointer = '\0';

    item->type = MB_JSON_String;
    item->valuestring = (char *)output;

    input_buffer->offset = (size_t)(input_end - input_buffer->content);
    input_buffer->offset++;

    return true;

fail:
    if (output != NULL)
    {
        input_buffer->hooks.deallocate(output);
    }

    if (input_pointer != NULL)
    {
        input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
    }

    return false;
}

static MB_JSON_bool MB_JSON_get_string_buffer_length_ptr(const unsigned char *const input, MB_JSON_buffer_len_data_t *const buf_len)
{
    const unsigned char *input_pointer = NULL;
    size_t output_length = 0;
    /* numbers of additional characters needed for escaping */
    size_t escape_characters = 0;

    if (input == NULL)
    {
        buf_len->size += 2;

        return true;
    }

    /* set "flag" to 1 if something needs to be escaped */
    for (input_pointer = input; *input_pointer; input_pointer++)
    {
        switch (*input_pointer)
        {
        case '\"':
        case '\\':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
            /* one character escape sequence */
            escape_characters++;
            break;
        default:
            if (*input_pointer < 32)
            {
                /* UTF-16 escape sequence uXXXX */
                escape_characters += 5;
            }
            break;
        }
    }
    output_length = (size_t)(input_pointer - input) + escape_characters + 2;

    buf_len->size += output_length;

    return true;
}

/* Render the cstring provided to an escaped version that can be printed. */
static MB_JSON_bool MB_JSON_print_string_ptr(const unsigned char *const input, MB_JSON_printbuffer *const output_buffer)
{
    const unsigned char *input_pointer = NULL;
    unsigned char *output = NULL;
    unsigned char *output_pointer = NULL;
    size_t output_length = 0;
    /* numbers of additional characters needed for escaping */
    size_t escape_characters = 0;

    if (output_buffer == NULL)
    {
        return false;
    }

    /* empty string */
    if (input == NULL)
    {
        output = MB_JSON_ensure(output_buffer, sizeof("\"\""));
        if (output == NULL)
        {
            return false;
        }
        strcpy((char *)output, "\"\"");

        return true;
    }

    /* set "flag" to 1 if something needs to be escaped */
    for (input_pointer = input; *input_pointer; input_pointer++)
    {
        switch (*input_pointer)
        {
        case '\"':
        case '\\':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
            /* one character escape sequence */
            escape_characters++;
            break;
        default:
            if (*input_pointer < 32)
            {
                /* UTF-16 escape sequence uXXXX */
                escape_characters += 5;
            }
            break;
        }
    }
    output_length = (size_t)(input_pointer - input) + escape_characters;

    output = MB_JSON_ensure(output_buffer, output_length + sizeof("\"\""));
    if (output == NULL)
    {
        return false;
    }

    /* no characters have to be escaped */
    if (escape_characters == 0)
    {
        output[0] = '\"';
        memcpy(output + 1, input, output_length);
        output[output_length + 1] = '\"';
        output[output_length + 2] = '\0';

        return true;
    }

    output[0] = '\"';
    output_pointer = output + 1;
    /* copy the string */
    for (input_pointer = input; *input_pointer != '\0'; (void)input_pointer++, output_pointer++)
    {
        if ((*input_pointer > 31) && (*input_pointer != '\"') && (*input_pointer != '\\'))
        {
            /* normal character, copy */
            *output_pointer = *input_pointer;
        }
        else
        {
            /* character needs to be escaped */
            *output_pointer++ = '\\';
            switch (*input_pointer)
            {
            case '\\':
                *output_pointer = '\\';
                break;
            case '\"':
                *output_pointer = '\"';
                break;
            case '\b':
                *output_pointer = 'b';
                break;
            case '\f':
                *output_pointer = 'f';
                break;
            case '\n':
                *output_pointer = 'n';
                break;
            case '\r':
                *output_pointer = 'r';
                break;
            case '\t':
                *output_pointer = 't';
                break;
            default:
                /* escape and print as unicode codepoint */
                sprintf((char *)output_pointer, "u%04x", *input_pointer);
                output_pointer += 4;
                break;
            }
        }
    }
    output[output_length + 1] = '\"';
    output[output_length + 2] = '\0';

    return true;
}

static MB_JSON_bool MB_JSON_get_string_buffer_length(const MB_JSON *const item, MB_JSON_buffer_len_data_t *const buf_len)
{
    return MB_JSON_get_string_buffer_length_ptr((unsigned char *)item->valuestring, buf_len);
}

/* Invoke print_string_ptr (which is useful) on an item. */
static MB_JSON_bool MB_JSON_print_string(const MB_JSON *const item, MB_JSON_printbuffer *const p)
{
    return MB_JSON_print_string_ptr((unsigned char *)item->valuestring, p);
}

/* Predeclare these prototypes. */
static MB_JSON_bool MB_JSON_parse_value(MB_JSON *const item, MB_JSON_parse_buffer *const input_buffer);
static MB_JSON_bool MB_JSON_print_value(const MB_JSON *const item, MB_JSON_printbuffer *const output_buffer);
static MB_JSON_bool MB_JSON_parse_array(MB_JSON *const item, MB_JSON_parse_buffer *const input_buffer);
static MB_JSON_bool MB_JSON_print_array(const MB_JSON *const item, MB_JSON_printbuffer *const output_buffer);
static MB_JSON_bool MB_JSON_parse_object(MB_JSON *const item, MB_JSON_parse_buffer *const input_buffer);
static MB_JSON_bool MB_JSON_print_object(const MB_JSON *const item, MB_JSON_printbuffer *const output_buffer);

static MB_JSON_bool MB_JSON_get_object_buffer_length(const MB_JSON *const item, MB_JSON_buffer_len_data_t *const buf_len);
static MB_JSON_bool MB_JSON_get_array_buffer_length(const MB_JSON *const item, MB_JSON_buffer_len_data_t *const buf_len);
static MB_JSON_bool MB_JSON_get_value_buffer_length(const MB_JSON *const item, MB_JSON_buffer_len_data_t *const buf_len);

/* Utility to jump whitespace and cr/lf */
static MB_JSON_parse_buffer *MB_JSON_buffer_skip_whitespace(MB_JSON_parse_buffer *const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL))
    {
        return NULL;
    }

    if (MB_JSON_cannot_access_at_index(buffer, 0))
    {
        return buffer;
    }

    while (MB_JSON_can_access_at_index(buffer, 0) && (MB_JSON_buffer_at_offset(buffer)[0] <= 32))
    {
        buffer->offset++;
    }

    if (buffer->offset == buffer->length)
    {
        buffer->offset--;
    }

    return buffer;
}

/* skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer */
static MB_JSON_parse_buffer *MB_JSON_skip_utf8_bom(MB_JSON_parse_buffer *const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL) || (buffer->offset != 0))
    {
        return NULL;
    }

    if (MB_JSON_can_access_at_index(buffer, 4) && (strncmp((const char *)MB_JSON_buffer_at_offset(buffer), "\xEF\xBB\xBF", 3) == 0))
    {
        buffer->offset += 3;
    }

    return buffer;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_ParseWithOpts(const char *value, const char **return_parse_end, MB_JSON_bool require_null_terminated)
{
    size_t buffer_length;

    if (NULL == value)
    {
        return NULL;
    }

    /* Adding null character size due to require_null_terminated. */
    buffer_length = strlen(value) + sizeof("");

    return MB_JSON_ParseWithLengthOpts(value, buffer_length, return_parse_end, require_null_terminated);
}

/* Parse an object - create a new root, and populate. */
MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, MB_JSON_bool require_null_terminated)
{
    MB_JSON_parse_buffer buffer = {0, 0, 0, 0, {0, 0, 0}};
    MB_JSON *item = NULL;

    /* reset error position */
    MB_JSON_global_error.json = NULL;
    MB_JSON_global_error.position = 0;

    if (value == NULL || 0 == buffer_length)
    {
        goto fail;
    }

    buffer.content = (const unsigned char *)value;
    buffer.length = buffer_length;
    buffer.offset = 0;
    buffer.hooks = MB_JSON_global_hooks;

    item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item == NULL) /* memory fail */
    {
        goto fail;
    }

    if (!MB_JSON_parse_value(item, MB_JSON_buffer_skip_whitespace(MB_JSON_skip_utf8_bom(&buffer))))
    {
        /* parse failure. ep is set. */
        goto fail;
    }

    /* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
    if (require_null_terminated)
    {
        MB_JSON_buffer_skip_whitespace(&buffer);
        if ((buffer.offset >= buffer.length) || MB_JSON_buffer_at_offset(&buffer)[0] != '\0')
        {
            goto fail;
        }
    }
    if (return_parse_end)
    {
        *return_parse_end = (const char *)MB_JSON_buffer_at_offset(&buffer);
    }

    return item;

fail:
    if (item != NULL)
    {
        MB_JSON_Delete(item);
    }

    if (value != NULL)
    {
        MB_JSON_error local_error;
        local_error.json = (const unsigned char *)value;
        local_error.position = 0;

        if (buffer.offset < buffer.length)
        {
            local_error.position = buffer.offset;
        }
        else if (buffer.length > 0)
        {
            local_error.position = buffer.length - 1;
        }

        if (return_parse_end != NULL)
        {
            *return_parse_end = (const char *)local_error.json + local_error.position;
        }

        MB_JSON_global_error = local_error;
    }

    return NULL;
}

/* Default options for MB_JSON_Parse */
MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_Parse(const char *value)
{
    return MB_JSON_ParseWithOpts(value, 0, 0);
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_ParseWithLength(const char *value, size_t buffer_length)
{
    return MB_JSON_ParseWithLengthOpts(value, buffer_length, 0, 0);
}

#define MB_JSON_min(a, b) (((a) < (b)) ? (a) : (b))

size_t MB_JSON_SerializedBufferLength(const MB_JSON *const item, MB_JSON_bool format)
{
    MB_JSON_buffer_len_data_t buf_len[1];
    buf_len->depth = 0;
    buf_len->format = format > 0 ? true : false;
    buf_len->size = 0;

    MB_JSON_get_value_buffer_length(item, buf_len);
    return buf_len->size;
}

static unsigned char *MB_JSON_print(const MB_JSON *const item, MB_JSON_bool format, const MB_JSON_internal_hooks *const hooks)
{
    static const size_t default_buffer_size = 256;
    MB_JSON_printbuffer buffer[1];
    unsigned char *printed = NULL;

    memset(buffer, 0, sizeof(buffer));

    /* create buffer */
    buffer->buffer = (unsigned char *)hooks->allocate(default_buffer_size);
    buffer->length = default_buffer_size;
    buffer->format = format;
    buffer->hooks = *hooks;
    if (buffer->buffer == NULL)
    {
        goto fail;
    }

    /* print the value */
    if (!MB_JSON_print_value(item, buffer))
    {
        goto fail;
    }
    MB_JSON_update_offset(buffer);

    /* check if reallocate is available */
    if (hooks->reallocate != NULL)
    {
        printed = (unsigned char *)hooks->reallocate(buffer->buffer, buffer->offset + 1);
        if (printed == NULL)
        {
            goto fail;
        }
        buffer->buffer = NULL;
    }
    else /* otherwise copy the JSON over to a new buffer */
    {
        printed = (unsigned char *)hooks->allocate(buffer->offset + 1);
        if (printed == NULL)
        {
            goto fail;
        }
        memcpy(printed, buffer->buffer, MB_JSON_min(buffer->length, buffer->offset + 1));
        printed[buffer->offset] = '\0'; /* just to be sure */

        /* free the buffer */
        hooks->deallocate(buffer->buffer);
    }

    return printed;

fail:
    if (buffer->buffer != NULL)
    {
        hooks->deallocate(buffer->buffer);
    }

    if (printed != NULL)
    {
        hooks->deallocate(printed);
    }

    return NULL;
}

/* Render a MB_JSON item/entity/structure to text. */
MB_JSON_PUBLIC(char *)
MB_JSON_Print(const MB_JSON *item)
{
    return (char *)MB_JSON_print(item, true, &MB_JSON_global_hooks);
}

MB_JSON_PUBLIC(char *)
MB_JSON_PrintUnformatted(const MB_JSON *item)
{
    return (char *)MB_JSON_print(item, false, &MB_JSON_global_hooks);
}

MB_JSON_PUBLIC(char *)
MB_JSON_PrintBuffered(const MB_JSON *item, int prebuffer, MB_JSON_bool fmt)
{
    MB_JSON_printbuffer p = {0, 0, 0, 0, 0, 0, {0, 0, 0}};

    if (prebuffer < 0)
    {
        return NULL;
    }

    p.buffer = (unsigned char *)MB_JSON_global_hooks.allocate((size_t)prebuffer);
    if (!p.buffer)
    {
        return NULL;
    }

    p.length = (size_t)prebuffer;
    p.offset = 0;
    p.noalloc = false;
    p.format = fmt;
    p.hooks = MB_JSON_global_hooks;

    if (!MB_JSON_print_value(item, &p))
    {
        MB_JSON_global_hooks.deallocate(p.buffer);
        return NULL;
    }

    return (char *)p.buffer;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_PrintPreallocated(MB_JSON *item, char *buffer, const int length, const MB_JSON_bool format)
{
    MB_JSON_printbuffer p = {0, 0, 0, 0, 0, 0, {0, 0, 0}};

    if ((length < 0) || (buffer == NULL))
    {
        return false;
    }

    p.buffer = (unsigned char *)buffer;
    p.length = (size_t)length;
    p.offset = 0;
    p.noalloc = true;
    p.format = format;
    p.hooks = MB_JSON_global_hooks;

    return MB_JSON_print_value(item, &p);
}

/* Parser core - when encountering text, process appropriately. */
static MB_JSON_bool MB_JSON_parse_value(MB_JSON *const item, MB_JSON_parse_buffer *const input_buffer)
{
    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false; /* no input */
    }

    /* parse the different types of values */
    /* null */
    if (MB_JSON_can_read(input_buffer, 4) && (strncmp((const char *)MB_JSON_buffer_at_offset(input_buffer), "null", 4) == 0))
    {
        item->type = MB_JSON_NULL;
        input_buffer->offset += 4;
        return true;
    }
    /* false */
    if (MB_JSON_can_read(input_buffer, 5) && (strncmp((const char *)MB_JSON_buffer_at_offset(input_buffer), "false", 5) == 0))
    {
        item->type = MB_JSON_False;
        input_buffer->offset += 5;
        return true;
    }
    /* true */
    if (MB_JSON_can_read(input_buffer, 4) && (strncmp((const char *)MB_JSON_buffer_at_offset(input_buffer), "true", 4) == 0))
    {
        item->type = MB_JSON_True;
        item->valueint = 1;
        input_buffer->offset += 4;
        return true;
    }
    /* string */
    if (MB_JSON_can_access_at_index(input_buffer, 0) && (MB_JSON_buffer_at_offset(input_buffer)[0] == '\"'))
    {
        return MB_JSON_parse_string(item, input_buffer);
    }
    /* number */
    if (MB_JSON_can_access_at_index(input_buffer, 0) && ((MB_JSON_buffer_at_offset(input_buffer)[0] == '-') || ((MB_JSON_buffer_at_offset(input_buffer)[0] >= '0') && (MB_JSON_buffer_at_offset(input_buffer)[0] <= '9'))))
    {
        return MB_JSON_parse_number(item, input_buffer);
    }
    /* array */
    if (MB_JSON_can_access_at_index(input_buffer, 0) && (MB_JSON_buffer_at_offset(input_buffer)[0] == '['))
    {
        return MB_JSON_parse_array(item, input_buffer);
    }
    /* object */
    if (MB_JSON_can_access_at_index(input_buffer, 0) && (MB_JSON_buffer_at_offset(input_buffer)[0] == '{'))
    {
        return MB_JSON_parse_object(item, input_buffer);
    }

    return false;
}

static MB_JSON_bool MB_JSON_get_value_buffer_length(const MB_JSON *const item, MB_JSON_buffer_len_data_t *const buf_len)
{

    switch ((item->type) & 0xFF)
    {
    case MB_JSON_NULL:
        buf_len->size += 4;
        return true;

    case MB_JSON_False:
        buf_len->size += 5;
        return true;

    case MB_JSON_True:
        buf_len->size += 4;
        return true;

    case MB_JSON_Raw:
    {

        if (item->valuestring == NULL)
        {
            return false;
        }

        buf_len->size += strlen(item->valuestring);
        return true;
    }

    case MB_JSON_String:
        return MB_JSON_get_string_buffer_length(item, buf_len);

    case MB_JSON_Array:
        return MB_JSON_get_array_buffer_length(item, buf_len);

    case MB_JSON_Object:
        return MB_JSON_get_object_buffer_length(item, buf_len);

    default:
        return false;
    }
}

/* Render a value to text. */
static MB_JSON_bool MB_JSON_print_value(const MB_JSON *const item, MB_JSON_printbuffer *const output_buffer)
{
    unsigned char *output = NULL;

    if ((item == NULL) || (output_buffer == NULL))
    {
        return false;
    }

    switch ((item->type) & 0xFF)
    {
    case MB_JSON_NULL:
        output = MB_JSON_ensure(output_buffer, 5);
        if (output == NULL)
        {
            return false;
        }
        strcpy((char *)output, "null");
        return true;

    case MB_JSON_False:
        output = MB_JSON_ensure(output_buffer, 6);
        if (output == NULL)
        {
            return false;
        }
        strcpy((char *)output, "false");
        return true;

    case MB_JSON_True:
        output = MB_JSON_ensure(output_buffer, 5);
        if (output == NULL)
        {
            return false;
        }
        strcpy((char *)output, "true");
        return true;

    case MB_JSON_Number:
        return MB_JSON_print_number(item, output_buffer);

    case MB_JSON_Raw:
    {
        size_t raw_length = 0;
        if (item->valuestring == NULL)
        {
            return false;
        }

        raw_length = strlen(item->valuestring) + sizeof("");
        output = MB_JSON_ensure(output_buffer, raw_length);
        if (output == NULL)
        {
            return false;
        }
        memcpy(output, item->valuestring, raw_length);
        return true;
    }

    case MB_JSON_String:
        return MB_JSON_print_string(item, output_buffer);

    case MB_JSON_Array:
        return MB_JSON_print_array(item, output_buffer);

    case MB_JSON_Object:
        return MB_JSON_print_object(item, output_buffer);

    default:
        return false;
    }
}

/* Build an array from input text. */
static MB_JSON_bool MB_JSON_parse_array(MB_JSON *const item, MB_JSON_parse_buffer *const input_buffer)
{
    MB_JSON *head = NULL; /* head of the linked list */
    MB_JSON *current_item = NULL;

    if (input_buffer->depth >= MB_JSON_NESTING_LIMIT)
    {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;

    if (MB_JSON_buffer_at_offset(input_buffer)[0] != '[')
    {
        /* not an array */
        goto fail;
    }

    input_buffer->offset++;
    MB_JSON_buffer_skip_whitespace(input_buffer);
    if (MB_JSON_can_access_at_index(input_buffer, 0) && (MB_JSON_buffer_at_offset(input_buffer)[0] == ']'))
    {
        /* empty array */
        goto success;
    }

    /* check if we skipped to the end of the buffer */
    if (MB_JSON_cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        MB_JSON *new_item = MB_JSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse next value */
        input_buffer->offset++;
        MB_JSON_buffer_skip_whitespace(input_buffer);
        if (!MB_JSON_parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        MB_JSON_buffer_skip_whitespace(input_buffer);
    } while (MB_JSON_can_access_at_index(input_buffer, 0) && (MB_JSON_buffer_at_offset(input_buffer)[0] == ','));

    if (MB_JSON_cannot_access_at_index(input_buffer, 0) || MB_JSON_buffer_at_offset(input_buffer)[0] != ']')
    {
        goto fail; /* expected end of array */
    }

success:
    input_buffer->depth--;

    if (head != NULL)
    {
        head->prev = current_item;
    }

    item->type = MB_JSON_Array;
    item->child = head;

    input_buffer->offset++;

    return true;

fail:
    if (head != NULL)
    {
        MB_JSON_Delete(head);
    }

    return false;
}

MB_JSON_bool MB_JSON_get_array_buffer_length(const MB_JSON *const item, MB_JSON_buffer_len_data_t *const buf_len)
{
    MB_JSON *current_element = item->child;

    //'[';
    buf_len->size++;

    buf_len->depth++;

    //add newline and tab after [
    if (buf_len->format)
    {
        // '\n';
        buf_len->size+= 1;
        //'\t';
        buf_len->size += buf_len->depth;
    }

    while (current_element != NULL)
    {
        if (!MB_JSON_get_value_buffer_length(current_element, buf_len))
            return false;

        if (current_element->next)
        {
            // ',' or ",\n"
            buf_len->size += (size_t)(buf_len->format ? 2 : 1);

            //'\t'
            if (buf_len->format)
                buf_len->size += buf_len->depth;
        }

        current_element = current_element->next;
    }

    buf_len->depth--;

    //add newline and tab before ]
    if (buf_len->format)
    {
        //'\n';
        buf_len->size += 1;

        //'\t'
        buf_len->size += buf_len->depth;
    }

    //']';
    buf_len->size++;

    return true;
}

/* Render an array to text */
static MB_JSON_bool MB_JSON_print_array(const MB_JSON *const item, MB_JSON_printbuffer *const output_buffer)
{
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    MB_JSON *current_element = item->child;

    if (output_buffer == NULL)
    {
        return false;
    }

    /* Compose the output array. */
    /* opening square bracket */
    output_pointer = MB_JSON_ensure(output_buffer, 1);
    if (output_pointer == NULL)
    {
        return false;
    }

    *output_pointer = '[';
    output_buffer->offset++;
    output_buffer->depth++;

    //add newline and tab after [
    if (output_buffer->format)
    {
        output_pointer = MB_JSON_ensure(output_buffer, 2);
        if (output_pointer == NULL)
        {
            return false;
        }
        *output_pointer++ = '\n';
        output_buffer->offset += 1;

        size_t i;
        output_pointer = MB_JSON_ensure(output_buffer, output_buffer->depth);
        if (output_pointer == NULL)
        {
            return false;
        }
        for (i = 0; i < output_buffer->depth; i++)
        {
            *output_pointer++ = '\t';
        }
        output_buffer->offset += output_buffer->depth;
    }

    while (current_element != NULL)
    {
        if (!MB_JSON_print_value(current_element, output_buffer))
        {
            return false;
        }
        MB_JSON_update_offset(output_buffer);
        if (current_element->next)
        {
            length = (size_t)(output_buffer->format ? 2 : 1);
            output_pointer = MB_JSON_ensure(output_buffer, length + 1);
            if (output_pointer == NULL)
            {
                return false;
            }
            *output_pointer++ = ',';
            //add newline and tab after ,
            if (output_buffer->format)
            {
                *output_pointer++ = '\n';
            }
            output_buffer->offset += length;

            if (output_buffer->format)
            {
                size_t i;
                output_pointer = MB_JSON_ensure(output_buffer, output_buffer->depth + 1);
                if (output_pointer == NULL)
                {
                    return false;
                }
                for (i = 0; i < output_buffer->depth; i++)
                {
                    *output_pointer++ = '\t';
                }
                output_buffer->offset += output_buffer->depth;
            }

            *output_pointer = '\0';
        }
        current_element = current_element->next;
    }

    output_pointer = MB_JSON_ensure(output_buffer, 2);
    if (output_pointer == NULL)
    {
        return false;
    }
    output_buffer->depth--;

    //add newline and tab before ]
    if (output_buffer->format)
    {
        output_pointer = MB_JSON_ensure(output_buffer, 2);
        if (output_pointer == NULL)
        {
            return false;
        }
        *output_pointer++ = '\n';
        output_buffer->offset += 1;

        size_t i;
        output_pointer = MB_JSON_ensure(output_buffer, output_buffer->depth + 1);
        if (output_pointer == NULL)
        {
            return false;
        }
        for (i = 0; i < output_buffer->depth; i++)
        {
            *output_pointer++ = '\t';
        }
        output_buffer->offset += output_buffer->depth;
    }

    *output_pointer++ = ']';
    *output_pointer = '\0';

    return true;
}

/* Build an object from the text. */
static MB_JSON_bool MB_JSON_parse_object(MB_JSON *const item, MB_JSON_parse_buffer *const input_buffer)
{
    MB_JSON *head = NULL; /* linked list head */
    MB_JSON *current_item = NULL;

    if (input_buffer->depth >= MB_JSON_NESTING_LIMIT)
    {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;

    if (MB_JSON_cannot_access_at_index(input_buffer, 0) || (MB_JSON_buffer_at_offset(input_buffer)[0] != '{'))
    {
        goto fail; /* not an object */
    }

    input_buffer->offset++;
    MB_JSON_buffer_skip_whitespace(input_buffer);
    if (MB_JSON_can_access_at_index(input_buffer, 0) && (MB_JSON_buffer_at_offset(input_buffer)[0] == '}'))
    {
        goto success; /* empty object */
    }

    /* check if we skipped to the end of the buffer */
    if (MB_JSON_cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        MB_JSON *new_item = MB_JSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse the name of the child */
        input_buffer->offset++;
        MB_JSON_buffer_skip_whitespace(input_buffer);
        if (!MB_JSON_parse_string(current_item, input_buffer))
        {
            goto fail; /* failed to parse name */
        }
        MB_JSON_buffer_skip_whitespace(input_buffer);

        /* swap valuestring and string, because we parsed the name */
        current_item->string = current_item->valuestring;
        current_item->valuestring = NULL;

        if (MB_JSON_cannot_access_at_index(input_buffer, 0) || (MB_JSON_buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto fail; /* invalid object */
        }

        /* parse the value */
        input_buffer->offset++;
        MB_JSON_buffer_skip_whitespace(input_buffer);
        if (!MB_JSON_parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        MB_JSON_buffer_skip_whitespace(input_buffer);
    } while (MB_JSON_can_access_at_index(input_buffer, 0) && (MB_JSON_buffer_at_offset(input_buffer)[0] == ','));

    if (MB_JSON_cannot_access_at_index(input_buffer, 0) || (MB_JSON_buffer_at_offset(input_buffer)[0] != '}'))
    {
        goto fail; /* expected end of object */
    }

success:
    input_buffer->depth--;

    if (head != NULL)
    {
        head->prev = current_item;
    }

    item->type = MB_JSON_Object;
    item->child = head;

    input_buffer->offset++;
    return true;

fail:
    if (head != NULL)
    {
        MB_JSON_Delete(head);
    }

    return false;
}

static MB_JSON_bool MB_JSON_get_object_buffer_length(const MB_JSON *const item, MB_JSON_buffer_len_data_t *const buf_len)
{
    size_t length = 0;
    MB_JSON *current_item = item->child;

    //'{' or "{\n"
    length = (size_t)(buf_len->format && current_item != NULL ? 2 : 1); 

    //do nothing for empty object
    if (current_item != NULL)
    {
        buf_len->depth++;

        buf_len->size += length;

        while (current_item)
        {
            //'\t'
            if (buf_len->format)
                buf_len->size += buf_len->depth;

            /* get key length */
            if (!MB_JSON_get_string_buffer_length_ptr((unsigned char *)current_item->string, buf_len))
                return false;

            length = (size_t)(buf_len->format ? 2 : 1);

            buf_len->size += length;

            /* get value length */
            if (!MB_JSON_get_value_buffer_length(current_item, buf_len))
                return false;

            /* add comma length if not last */
            length = ((size_t)(buf_len->format ? 1 : 0) + (size_t)(current_item->next ? 1 : 0));

            buf_len->size += length;

            current_item = current_item->next;
        }

        //'\t'
        if (buf_len->format)
            buf_len->size += buf_len->depth - 1;

        buf_len->depth--;
    }

    //'}'
    buf_len->size++;

    return true;
}

/* Render an object to text. */
static MB_JSON_bool MB_JSON_print_object(const MB_JSON *const item, MB_JSON_printbuffer *const output_buffer)
{
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    MB_JSON *current_item = item->child;

    if (output_buffer == NULL)
    {
        return false;
    }

    /* Compose the output: */
    length = (size_t)(output_buffer->format && current_item != NULL ? 2 : 1); /* fmt: {\n */
    output_pointer = MB_JSON_ensure(output_buffer, length + 1);
    if (output_pointer == NULL)
    {
        return false;
    }

    *output_pointer++ = '{';

    //do nothing for empty object
    if (current_item != NULL)
    {
        output_buffer->depth++;
        if (output_buffer->format)
        {
            *output_pointer++ = '\n';
        }
        output_buffer->offset += length;

        while (current_item)
        {
            if (output_buffer->format)
            {
                size_t i;
                output_pointer = MB_JSON_ensure(output_buffer, output_buffer->depth);
                if (output_pointer == NULL)
                {
                    return false;
                }
                for (i = 0; i < output_buffer->depth; i++)
                {
                    *output_pointer++ = '\t';
                }
                output_buffer->offset += output_buffer->depth;
            }

            /* print key */
            if (!MB_JSON_print_string_ptr((unsigned char *)current_item->string, output_buffer))
            {
                return false;
            }
            MB_JSON_update_offset(output_buffer);

            length = (size_t)(output_buffer->format ? 2 : 1);
            output_pointer = MB_JSON_ensure(output_buffer, length);
            if (output_pointer == NULL)
            {
                return false;
            }
            *output_pointer++ = ':';
            if (output_buffer->format)
            {
                *output_pointer++ = ' ';
            }
            output_buffer->offset += length;

            /* print value */
            if (!MB_JSON_print_value(current_item, output_buffer))
            {
                return false;
            }
            MB_JSON_update_offset(output_buffer);

            /* print comma if not last */
            length = ((size_t)(output_buffer->format ? 1 : 0) + (size_t)(current_item->next ? 1 : 0));
            output_pointer = MB_JSON_ensure(output_buffer, length + 1);
            if (output_pointer == NULL)
            {
                return false;
            }
            if (current_item->next)
            {
                *output_pointer++ = ',';
            }

            if (output_buffer->format)
            {
                *output_pointer++ = '\n';
            }
            *output_pointer = '\0';
            output_buffer->offset += length;

            current_item = current_item->next;
        }

        output_pointer = MB_JSON_ensure(output_buffer, output_buffer->format ? (output_buffer->depth + 1) : 2);
        if (output_pointer == NULL)
        {
            return false;
        }
        if (output_buffer->format)
        {
            size_t i;
            for (i = 0; i < (output_buffer->depth - 1); i++)
            {
                *output_pointer++ = '\t';
            }
        }

        output_buffer->depth--;
    }

    *output_pointer++ = '}';
    *output_pointer = '\0';

    return true;
}

/* Get Array size/item / object item. */
MB_JSON_PUBLIC(int)
MB_JSON_GetArraySize(const MB_JSON *array)
{
    MB_JSON *child = NULL;
    size_t size = 0;

    if (array == NULL)
    {
        return 0;
    }

    child = array->child;

    while (child != NULL)
    {
        size++;
        child = child->next;
    }

    /* FIXME: Can overflow here. Cannot be fixed without breaking the API */

    return (int)size;
}

static MB_JSON *MB_JSON_get_array_item(const MB_JSON *array, size_t index)
{
    MB_JSON *current_child = NULL;

    if (array == NULL)
    {
        return NULL;
    }

    current_child = array->child;
    while ((current_child != NULL) && (index > 0))
    {
        index--;
        current_child = current_child->next;
    }

    return current_child;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_GetArrayItem(const MB_JSON *array, int index)
{
    if (index < 0)
    {
        return NULL;
    }

    return MB_JSON_get_array_item(array, (size_t)index);
}

static MB_JSON *MB_JSON_get_object_item(const MB_JSON *const object, const char *const name, const MB_JSON_bool case_sensitive)
{
    MB_JSON *current_element = NULL;

    if ((object == NULL) || (name == NULL))
    {
        return NULL;
    }

    current_element = object->child;
    if (case_sensitive)
    {
        while ((current_element != NULL) && (current_element->string != NULL) && (strcmp(name, current_element->string) != 0))
        {
            current_element = current_element->next;
        }
    }
    else
    {
        while ((current_element != NULL) && (MB_JSON_case_insensitive_strcmp((const unsigned char *)name, (const unsigned char *)(current_element->string)) != 0))
        {
            current_element = current_element->next;
        }
    }

    if ((current_element == NULL) || (current_element->string == NULL))
    {
        return NULL;
    }

    return current_element;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_GetObjectItem(const MB_JSON *const object, const char *const string)
{
    return MB_JSON_get_object_item(object, string, false);
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_GetObjectItemCaseSensitive(const MB_JSON *const object, const char *const string)
{
    return MB_JSON_get_object_item(object, string, true);
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_HasObjectItem(const MB_JSON *object, const char *string)
{
    return MB_JSON_GetObjectItem(object, string) ? 1 : 0;
}

/* Utility for array list handling. */
static void MB_JSON_suffix_object(MB_JSON *prev, MB_JSON *item)
{
    prev->next = item;
    item->prev = prev;
}

/* Utility for handling references. */
static MB_JSON *MB_JSON_create_reference(const MB_JSON *item, const MB_JSON_internal_hooks *const hooks)
{
    MB_JSON *reference = NULL;
    if (item == NULL)
    {
        return NULL;
    }

    reference = MB_JSON_New_Item(hooks);
    if (reference == NULL)
    {
        return NULL;
    }

    memcpy(reference, item, sizeof(MB_JSON));
    reference->string = NULL;
    reference->type |= MB_JSON_IsReference;
    reference->next = reference->prev = NULL;
    return reference;
}

static MB_JSON_bool MB_JSON_add_item_to_array(MB_JSON *array, MB_JSON *item)
{
    MB_JSON *child = NULL;

    if ((item == NULL) || (array == NULL) || (array == item))
    {
        return false;
    }

    child = array->child;
    /*
     * To find the last item in array quickly, we use prev in array
     */
    if (child == NULL)
    {
        /* list is empty, start new one */
        array->child = item;
        item->prev = item;
        item->next = NULL;
    }
    else
    {
        /* append to the end */
        if (child->prev)
        {
            MB_JSON_suffix_object(child->prev, item);
            array->child->prev = item;
        }
    }

    return true;
}

/* Add item to array/object. */
MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_AddItemToArray(MB_JSON *array, MB_JSON *item)
{
    return MB_JSON_add_item_to_array(array, item);
}

#if defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
#pragma GCC diagnostic push
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
/* helper function to cast away const */
static void *cast_away_const(const void *string)
{
    return (void *)string;
}
#if defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
#pragma GCC diagnostic pop
#endif

static MB_JSON_bool MB_JSON_add_item_to_object(MB_JSON *const object, const char *const string, MB_JSON *const item, const MB_JSON_internal_hooks *const hooks, const MB_JSON_bool constant_key)
{
    char *new_key = NULL;
    int new_type = MB_JSON_Invalid;

    if ((object == NULL) || (string == NULL) || (item == NULL) || (object == item))
    {
        return false;
    }

    if (constant_key)
    {
        new_key = (char *)cast_away_const(string);
        new_type = item->type | MB_JSON_StringIsConst;
    }
    else
    {
        new_key = (char *)MB_JSON_strdup((const unsigned char *)string, hooks);
        if (new_key == NULL)
        {
            return false;
        }

        new_type = item->type & ~MB_JSON_StringIsConst;
    }

    if (!(item->type & MB_JSON_StringIsConst) && (item->string != NULL))
    {
        hooks->deallocate(item->string);
    }

    item->string = new_key;
    item->type = new_type;

    return MB_JSON_add_item_to_array(object, item);
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_AddItemToObject(MB_JSON *object, const char *string, MB_JSON *item)
{
    return MB_JSON_add_item_to_object(object, string, item, &MB_JSON_global_hooks, false);
}

/* Add an item to an object with constant string as key */
MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_AddItemToObjectCS(MB_JSON *object, const char *string, MB_JSON *item)
{
    return MB_JSON_add_item_to_object(object, string, item, &MB_JSON_global_hooks, true);
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_AddItemReferenceToArray(MB_JSON *array, MB_JSON *item)
{
    if (array == NULL)
    {
        return false;
    }

    return MB_JSON_add_item_to_array(array, MB_JSON_create_reference(item, &MB_JSON_global_hooks));
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_AddItemReferenceToObject(MB_JSON *object, const char *string, MB_JSON *item)
{
    if ((object == NULL) || (string == NULL))
    {
        return false;
    }

    return MB_JSON_add_item_to_object(object, string, MB_JSON_create_reference(item, &MB_JSON_global_hooks), &MB_JSON_global_hooks, false);
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_AddNullToObject(MB_JSON *const object, const char *const name)
{
    MB_JSON *null = MB_JSON_CreateNull();
    if (MB_JSON_add_item_to_object(object, name, null, &MB_JSON_global_hooks, false))
    {
        return null;
    }

    MB_JSON_Delete(null);
    return NULL;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_AddTrueToObject(MB_JSON *const object, const char *const name)
{
    MB_JSON *true_item = MB_JSON_CreateTrue();
    if (MB_JSON_add_item_to_object(object, name, true_item, &MB_JSON_global_hooks, false))
    {
        return true_item;
    }

    MB_JSON_Delete(true_item);
    return NULL;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_AddFalseToObject(MB_JSON *const object, const char *const name)
{
    MB_JSON *false_item = MB_JSON_CreateFalse();
    if (MB_JSON_add_item_to_object(object, name, false_item, &MB_JSON_global_hooks, false))
    {
        return false_item;
    }

    MB_JSON_Delete(false_item);
    return NULL;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_AddBoolToObject(MB_JSON *const object, const char *const name, const MB_JSON_bool boolean)
{
    MB_JSON *bool_item = MB_JSON_CreateBool(boolean);
    if (MB_JSON_add_item_to_object(object, name, bool_item, &MB_JSON_global_hooks, false))
    {
        return bool_item;
    }

    MB_JSON_Delete(bool_item);
    return NULL;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_AddNumberToObject(MB_JSON *const object, const char *const name, const double number)
{
    MB_JSON *number_item = MB_JSON_CreateNumber(number);
    if (MB_JSON_add_item_to_object(object, name, number_item, &MB_JSON_global_hooks, false))
    {
        return number_item;
    }

    MB_JSON_Delete(number_item);
    return NULL;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_AddStringToObject(MB_JSON *const object, const char *const name, const char *const string)
{
    MB_JSON *string_item = MB_JSON_CreateString(string);
    if (MB_JSON_add_item_to_object(object, name, string_item, &MB_JSON_global_hooks, false))
    {
        return string_item;
    }

    MB_JSON_Delete(string_item);
    return NULL;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_AddRawToObject(MB_JSON *const object, const char *const name, const char *const raw)
{
    MB_JSON *raw_item = MB_JSON_CreateRaw(raw);
    if (MB_JSON_add_item_to_object(object, name, raw_item, &MB_JSON_global_hooks, false))
    {
        return raw_item;
    }

    MB_JSON_Delete(raw_item);
    return NULL;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_AddObjectToObject(MB_JSON *const object, const char *const name)
{
    MB_JSON *object_item = MB_JSON_CreateObject();
    if (MB_JSON_add_item_to_object(object, name, object_item, &MB_JSON_global_hooks, false))
    {
        return object_item;
    }

    MB_JSON_Delete(object_item);
    return NULL;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_AddArrayToObject(MB_JSON *const object, const char *const name)
{
    MB_JSON *array = MB_JSON_CreateArray();
    if (MB_JSON_add_item_to_object(object, name, array, &MB_JSON_global_hooks, false))
    {
        return array;
    }

    MB_JSON_Delete(array);
    return NULL;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_DetachItemViaPointer(MB_JSON *parent, MB_JSON *const item)
{
    if ((parent == NULL) || (item == NULL))
    {
        return NULL;
    }

    if (item != parent->child)
    {
        /* not the first element */
        item->prev->next = item->next;
    }
    if (item->next != NULL)
    {
        /* not the last element */
        item->next->prev = item->prev;
    }

    if (item == parent->child)
    {
        /* first element */
        parent->child = item->next;
    }
    else if (item->next == NULL)
    {
        /* last element */
        parent->child->prev = item->prev;
    }

    /* make sure the detached item doesn't point anywhere anymore */
    item->prev = NULL;
    item->next = NULL;

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_DetachItemFromArray(MB_JSON *array, int which)
{
    if (which < 0)
    {
        return NULL;
    }

    return MB_JSON_DetachItemViaPointer(array, MB_JSON_get_array_item(array, (size_t)which));
}

MB_JSON_PUBLIC(void)
MB_JSON_DeleteItemFromArray(MB_JSON *array, int which)
{
    MB_JSON_Delete(MB_JSON_DetachItemFromArray(array, which));
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_DetachItemFromObject(MB_JSON *object, const char *string)
{
    MB_JSON *to_detach = MB_JSON_GetObjectItem(object, string);

    return MB_JSON_DetachItemViaPointer(object, to_detach);
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_DetachItemFromObjectCaseSensitive(MB_JSON *object, const char *string)
{
    MB_JSON *to_detach = MB_JSON_GetObjectItemCaseSensitive(object, string);

    return MB_JSON_DetachItemViaPointer(object, to_detach);
}

MB_JSON_PUBLIC(void)
MB_JSON_DeleteItemFromObject(MB_JSON *object, const char *string)
{
    MB_JSON_Delete(MB_JSON_DetachItemFromObject(object, string));
}

MB_JSON_PUBLIC(void)
MB_JSON_DeleteItemFromObjectCaseSensitive(MB_JSON *object, const char *string)
{
    MB_JSON_Delete(MB_JSON_DetachItemFromObjectCaseSensitive(object, string));
}

/* Replace array/object items with new ones. */
MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_InsertItemInArray(MB_JSON *array, int which, MB_JSON *newitem)
{
    MB_JSON *after_inserted = NULL;

    if (which < 0)
    {
        return false;
    }

    after_inserted = MB_JSON_get_array_item(array, (size_t)which);
    if (after_inserted == NULL)
    {
        return MB_JSON_add_item_to_array(array, newitem);
    }

    newitem->next = after_inserted;
    newitem->prev = after_inserted->prev;
    after_inserted->prev = newitem;
    if (after_inserted == array->child)
    {
        array->child = newitem;
    }
    else
    {
        newitem->prev->next = newitem;
    }
    return true;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_ReplaceItemViaPointer(MB_JSON *const parent, MB_JSON *const item, MB_JSON *replacement)
{
    if ((parent == NULL) || (replacement == NULL) || (item == NULL))
    {
        return false;
    }

    if (replacement == item)
    {
        return true;
    }

    replacement->next = item->next;
    replacement->prev = item->prev;

    if (replacement->next != NULL)
    {
        replacement->next->prev = replacement;
    }
    if (parent->child == item)
    {
        if (parent->child->prev == parent->child)
        {
            replacement->prev = replacement;
        }
        parent->child = replacement;
    }
    else
    { /*
         * To find the last item in array quickly, we use prev in array.
         * We can't modify the last item's next pointer where this item was the parent's child
         */
        if (replacement->prev != NULL)
        {
            replacement->prev->next = replacement;
        }
        if (replacement->next == NULL)
        {
            parent->child->prev = replacement;
        }
    }

    item->next = NULL;
    item->prev = NULL;
    MB_JSON_Delete(item);

    return true;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_ReplaceItemInArray(MB_JSON *array, int which, MB_JSON *newitem)
{
    if (which < 0)
    {
        return false;
    }

    return MB_JSON_ReplaceItemViaPointer(array, MB_JSON_get_array_item(array, (size_t)which), newitem);
}

static MB_JSON_bool MB_JSON_replace_item_in_object(MB_JSON *object, const char *string, MB_JSON *replacement, MB_JSON_bool case_sensitive)
{
    if ((replacement == NULL) || (string == NULL))
    {
        return false;
    }

    /* replace the name in the replacement */
    if (!(replacement->type & MB_JSON_StringIsConst) && (replacement->string != NULL))
    {
        MB_JSON_free(replacement->string);
    }
    replacement->string = (char *)MB_JSON_strdup((const unsigned char *)string, &MB_JSON_global_hooks);
    replacement->type &= ~MB_JSON_StringIsConst;

    return MB_JSON_ReplaceItemViaPointer(object, MB_JSON_get_object_item(object, string, case_sensitive), replacement);
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_ReplaceItemInObject(MB_JSON *object, const char *string, MB_JSON *newitem)
{
    return MB_JSON_replace_item_in_object(object, string, newitem, false);
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_ReplaceItemInObjectCaseSensitive(MB_JSON *object, const char *string, MB_JSON *newitem)
{
    return MB_JSON_replace_item_in_object(object, string, newitem, true);
}

/* Create basic types: */
MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateNull(void)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item)
    {
        item->type = MB_JSON_NULL;
    }

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateTrue(void)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item)
    {
        item->type = MB_JSON_True;
    }

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateFalse(void)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item)
    {
        item->type = MB_JSON_False;
    }

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateBool(MB_JSON_bool boolean)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item)
    {
        item->type = boolean ? MB_JSON_True : MB_JSON_False;
    }

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateNumber(double num)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item)
    {
        item->type = MB_JSON_Number;
        item->valuedouble = num;

        /* use saturation in case of overflow */
        if (num >= INT_MAX)
        {
            item->valueint = INT_MAX;
        }
        else if (num <= (double)INT_MIN)
        {
            item->valueint = INT_MIN;
        }
        else
        {
            item->valueint = (int)num;
        }
    }

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateString(const char *string)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item)
    {
        item->type = MB_JSON_String;
        item->valuestring = (char *)MB_JSON_strdup((const unsigned char *)string, &MB_JSON_global_hooks);
        if (!item->valuestring)
        {
            MB_JSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateStringReference(const char *string)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item != NULL)
    {
        item->type = MB_JSON_String | MB_JSON_IsReference;
        item->valuestring = (char *)cast_away_const(string);
    }

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateObjectReference(const MB_JSON *child)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item != NULL)
    {
        item->type = MB_JSON_Object | MB_JSON_IsReference;
        item->child = (MB_JSON *)cast_away_const(child);
    }

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateArrayReference(const MB_JSON *child)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item != NULL)
    {
        item->type = MB_JSON_Array | MB_JSON_IsReference;
        item->child = (MB_JSON *)cast_away_const(child);
    }

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateRaw(const char *raw)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item)
    {
        item->type = MB_JSON_Raw;
        item->valuestring = (char *)MB_JSON_strdup((const unsigned char *)raw, &MB_JSON_global_hooks);
        if (!item->valuestring)
        {
            MB_JSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateArray(void)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item)
    {
        item->type = MB_JSON_Array;
    }

    return item;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateObject(void)
{
    MB_JSON *item = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (item)
    {
        item->type = MB_JSON_Object;
    }

    return item;
}

/* Create Arrays: */
MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateIntArray(const int *numbers, int count)
{
    size_t i = 0;
    MB_JSON *n = NULL;
    MB_JSON *p = NULL;
    MB_JSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = MB_JSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = MB_JSON_CreateNumber(numbers[i]);
        if (!n)
        {
            MB_JSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            MB_JSON_suffix_object(p, n);
        }
        p = n;
    }

    if (a && a->child)
    {
        a->child->prev = n;
    }

    return a;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateFloatArray(const float *numbers, int count)
{
    size_t i = 0;
    MB_JSON *n = NULL;
    MB_JSON *p = NULL;
    MB_JSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = MB_JSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = MB_JSON_CreateNumber((double)numbers[i]);
        if (!n)
        {
            MB_JSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            MB_JSON_suffix_object(p, n);
        }
        p = n;
    }

    if (a && a->child)
    {
        a->child->prev = n;
    }

    return a;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateDoubleArray(const double *numbers, int count)
{
    size_t i = 0;
    MB_JSON *n = NULL;
    MB_JSON *p = NULL;
    MB_JSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = MB_JSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = MB_JSON_CreateNumber(numbers[i]);
        if (!n)
        {
            MB_JSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            MB_JSON_suffix_object(p, n);
        }
        p = n;
    }

    if (a && a->child)
    {
        a->child->prev = n;
    }

    return a;
}

MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_CreateStringArray(const char *const *strings, int count)
{
    size_t i = 0;
    MB_JSON *n = NULL;
    MB_JSON *p = NULL;
    MB_JSON *a = NULL;

    if ((count < 0) || (strings == NULL))
    {
        return NULL;
    }

    a = MB_JSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = MB_JSON_CreateString(strings[i]);
        if (!n)
        {
            MB_JSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            MB_JSON_suffix_object(p, n);
        }
        p = n;
    }

    if (a && a->child)
    {
        a->child->prev = n;
    }

    return a;
}

/* Duplication */
MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_Duplicate(const MB_JSON *item, MB_JSON_bool recurse)
{
    MB_JSON *newitem = NULL;
    MB_JSON *child = NULL;
    MB_JSON *next = NULL;
    MB_JSON *newchild = NULL;

    /* Bail on bad ptr */
    if (!item)
    {
        goto fail;
    }
    /* Create new item */
    newitem = MB_JSON_New_Item(&MB_JSON_global_hooks);
    if (!newitem)
    {
        goto fail;
    }
    /* Copy over all vars */
    newitem->type = item->type & (~MB_JSON_IsReference);
    newitem->valueint = item->valueint;
    newitem->valuedouble = item->valuedouble;
    if (item->valuestring)
    {
        newitem->valuestring = (char *)MB_JSON_strdup((unsigned char *)item->valuestring, &MB_JSON_global_hooks);
        if (!newitem->valuestring)
        {
            goto fail;
        }
    }
    if (item->string)
    {
        newitem->string = (item->type & MB_JSON_StringIsConst) ? item->string : (char *)MB_JSON_strdup((unsigned char *)item->string, &MB_JSON_global_hooks);
        if (!newitem->string)
        {
            goto fail;
        }
    }
    /* If non-recursive, then we're done! */
    if (!recurse)
    {
        return newitem;
    }
    /* Walk the ->next chain for the child. */
    child = item->child;
    while (child != NULL)
    {
        newchild = MB_JSON_Duplicate(child, true); /* Duplicate (with recurse) each item in the ->next chain */
        if (!newchild)
        {
            goto fail;
        }
        if (next != NULL)
        {
            /* If newitem->child already set, then crosswire ->prev and ->next and move on */
            next->next = newchild;
            newchild->prev = next;
            next = newchild;
        }
        else
        {
            /* Set newitem->child and move to it */
            newitem->child = newchild;
            next = newchild;
        }
        child = child->next;
    }
    if (newitem && newitem->child)
    {
        newitem->child->prev = newchild;
    }

    return newitem;

fail:
    if (newitem != NULL)
    {
        MB_JSON_Delete(newitem);
    }

    return NULL;
}

static void MB_JSON_skip_oneline_comment(char **input)
{
    *input += MB_JSON_static_strlen("//");

    for (; (*input)[0] != '\0'; ++(*input))
    {
        if ((*input)[0] == '\n')
        {
            *input += MB_JSON_static_strlen("\n");
            return;
        }
    }
}

static void MB_JSON_skip_multiline_comment(char **input)
{
    *input += MB_JSON_static_strlen("/*");

    for (; (*input)[0] != '\0'; ++(*input))
    {
        if (((*input)[0] == '*') && ((*input)[1] == '/'))
        {
            *input += MB_JSON_static_strlen("*/");
            return;
        }
    }
}

static void MB_JSON_minify_string(char **input, char **output)
{
    (*output)[0] = (*input)[0];
    *input += MB_JSON_static_strlen("\"");
    *output += MB_JSON_static_strlen("\"");

    for (; (*input)[0] != '\0'; (void)++(*input), ++(*output))
    {
        (*output)[0] = (*input)[0];

        if ((*input)[0] == '\"')
        {
            (*output)[0] = '\"';
            *input += MB_JSON_static_strlen("\"");
            *output += MB_JSON_static_strlen("\"");
            return;
        }
        else if (((*input)[0] == '\\') && ((*input)[1] == '\"'))
        {
            (*output)[1] = (*input)[1];
            *input += MB_JSON_static_strlen("\"");
            *output += MB_JSON_static_strlen("\"");
        }
    }
}

MB_JSON_PUBLIC(void)
MB_JSON_Minify(char *json)
{
    char *into = json;

    if (json == NULL)
    {
        return;
    }

    while (json[0] != '\0')
    {
        switch (json[0])
        {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            json++;
            break;

        case '/':
            if (json[1] == '/')
            {
                MB_JSON_skip_oneline_comment(&json);
            }
            else if (json[1] == '*')
            {
                MB_JSON_skip_multiline_comment(&json);
            }
            else
            {
                json++;
            }
            break;

        case '\"':
            MB_JSON_minify_string(&json, (char **)&into);
            break;

        default:
            into[0] = json[0];
            json++;
            into++;
        }
    }

    /* and null-terminate. */
    *into = '\0';
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_IsInvalid(const MB_JSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == MB_JSON_Invalid;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_IsFalse(const MB_JSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == MB_JSON_False;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_IsTrue(const MB_JSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xff) == MB_JSON_True;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_IsBool(const MB_JSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & (MB_JSON_True | MB_JSON_False)) != 0;
}
MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_IsNull(const MB_JSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == MB_JSON_NULL;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_IsNumber(const MB_JSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == MB_JSON_Number;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_IsString(const MB_JSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == MB_JSON_String;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_IsArray(const MB_JSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == MB_JSON_Array;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_IsObject(const MB_JSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == MB_JSON_Object;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_IsRaw(const MB_JSON *const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == MB_JSON_Raw;
}

MB_JSON_PUBLIC(MB_JSON_bool)
MB_JSON_Compare(const MB_JSON *const a, const MB_JSON *const b, const MB_JSON_bool case_sensitive)
{
    if ((a == NULL) || (b == NULL) || ((a->type & 0xFF) != (b->type & 0xFF)) || MB_JSON_IsInvalid(a))
    {
        return false;
    }

    /* check if type is valid */
    switch (a->type & 0xFF)
    {
    case MB_JSON_False:
    case MB_JSON_True:
    case MB_JSON_NULL:
    case MB_JSON_Number:
    case MB_JSON_String:
    case MB_JSON_Raw:
    case MB_JSON_Array:
    case MB_JSON_Object:
        break;

    default:
        return false;
    }

    /* identical objects are equal */
    if (a == b)
    {
        return true;
    }

    switch (a->type & 0xFF)
    {
    /* in these cases and equal type is enough */
    case MB_JSON_False:
    case MB_JSON_True:
    case MB_JSON_NULL:
        return true;

    case MB_JSON_Number:
        if (MB_JSON_compare_double(a->valuedouble, b->valuedouble))
        {
            return true;
        }
        return false;

    case MB_JSON_String:
    case MB_JSON_Raw:
        if ((a->valuestring == NULL) || (b->valuestring == NULL))
        {
            return false;
        }
        if (strcmp(a->valuestring, b->valuestring) == 0)
        {
            return true;
        }

        return false;

    case MB_JSON_Array:
    {
        MB_JSON *a_element = a->child;
        MB_JSON *b_element = b->child;

        for (; (a_element != NULL) && (b_element != NULL);)
        {
            if (!MB_JSON_Compare(a_element, b_element, case_sensitive))
            {
                return false;
            }

            a_element = a_element->next;
            b_element = b_element->next;
        }

        /* one of the arrays is longer than the other */
        if (a_element != b_element)
        {
            return false;
        }

        return true;
    }

    case MB_JSON_Object:
    {
        MB_JSON *a_element = NULL;
        MB_JSON *b_element = NULL;
        MB_JSON_ArrayForEach(a_element, a)
        {
            /* TODO This has O(n^2) runtime, which is horrible! */
            b_element = MB_JSON_get_object_item(b, a_element->string, case_sensitive);
            if (b_element == NULL)
            {
                return false;
            }

            if (!MB_JSON_Compare(a_element, b_element, case_sensitive))
            {
                return false;
            }
        }

        /* doing this twice, once on a and b to prevent true comparison if a subset of b
             * TODO: Do this the proper way, this is just a fix for now */
        MB_JSON_ArrayForEach(b_element, b)
        {
            a_element = MB_JSON_get_object_item(a, b_element->string, case_sensitive);
            if (a_element == NULL)
            {
                return false;
            }

            if (!MB_JSON_Compare(b_element, a_element, case_sensitive))
            {
                return false;
            }
        }

        return true;
    }

    default:
        return false;
    }
}

MB_JSON_PUBLIC(void *)
MB_JSON_malloc(size_t size)
{
    return MB_JSON_global_hooks.allocate(size);
}

MB_JSON_PUBLIC(void)
MB_JSON_free(void *object)
{
    MB_JSON_global_hooks.deallocate(object);
}

#endif //MB_JSON_C