/*
  MB_JSON.h v1.0.1 based on the modified version of cJSON.h v1.7.14 (Sept 3, 2020)

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

#ifndef MB_JSON_H
#define MB_JSON_H

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

/* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention.  For windows you have 3 define options:

MB_JSON_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
MB_JSON_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
MB_JSON_IMPORT_SYMBOLS - Define this if you want to dllimport symbol

For *nix builds that support visibility attribute, you can define similar behavior by

setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS

then using the MB_JSON_API_VISIBILITY flag to "export" the same symbols the way MB_JSON_EXPORT_SYMBOLS does

*/

#define MB_JSON_CDECL __cdecl
#define MB_JSON_STDCALL __stdcall

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(MB_JSON_HIDE_SYMBOLS) && !defined(MB_JSON_IMPORT_SYMBOLS) && !defined(MB_JSON_EXPORT_SYMBOLS)
#define MB_JSON_EXPORT_SYMBOLS
#endif

#if defined(MB_JSON_HIDE_SYMBOLS)
#define MB_JSON_PUBLIC(type)   type MB_JSON_STDCALL
#elif defined(MB_JSON_EXPORT_SYMBOLS)
#define MB_JSON_PUBLIC(type)   __declspec(dllexport) type MB_JSON_STDCALL
#elif defined(MB_JSON_IMPORT_SYMBOLS)
#define MB_JSON_PUBLIC(type)   __declspec(dllimport) type MB_JSON_STDCALL
#endif
#else /* !__WINDOWS__ */
#define MB_JSON_CDECL
#define MB_JSON_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(MB_JSON_API_VISIBILITY)
#define MB_JSON_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
#define MB_JSON_PUBLIC(type) type
#endif
#endif

/* project version */
#define MB_JSON_VERSION_MAJOR 1
#define MB_JSON_VERSION_MINOR 7
#define MB_JSON_VERSION_PATCH 14

#include <stddef.h>

/* MB_JSON Types: */
#define MB_JSON_Invalid (0)
#define MB_JSON_False  (1 << 0)
#define MB_JSON_True   (1 << 1)
#define MB_JSON_NULL   (1 << 2)
#define MB_JSON_Number (1 << 3)
#define MB_JSON_String (1 << 4)
#define MB_JSON_Array  (1 << 5)
#define MB_JSON_Object (1 << 6)
#define MB_JSON_Raw    (1 << 7) /* raw json */

#define MB_JSON_IsReference 256
#define MB_JSON_StringIsConst 512

/* The MB_JSON structure: */
typedef struct MB_JSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct MB_JSON *next;
    struct MB_JSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct MB_JSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==MB_JSON_String  and type == MB_JSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use MB_JSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==MB_JSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} MB_JSON;

typedef struct MB_JSON_Hooks
{
      /* malloc/free are CDECL on Windows regardless of the default calling convention of the compiler, so ensure the hooks allow passing those functions directly. */
      void *(MB_JSON_CDECL *malloc_fn)(size_t sz);
      void (MB_JSON_CDECL *free_fn)(void *ptr);
      void *(MB_JSON_CDECL *realloc_fn)(void *ptr, size_t sz);
} MB_JSON_Hooks;

typedef int MB_JSON_bool;

/* Limits how deeply nested arrays/objects can be before MB_JSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef MB_JSON_NESTING_LIMIT
#define MB_JSON_NESTING_LIMIT 1000
#endif

/* returns the version of MB_JSON as a string */
MB_JSON_PUBLIC(const char*) MB_JSON_Version(void);

/* Supply malloc, realloc and free functions to MB_JSON */
MB_JSON_PUBLIC(void) MB_JSON_InitHooks(MB_JSON_Hooks* hooks);

size_t MB_JSON_SerializedBufferLength(const MB_JSON *const item, MB_JSON_bool format);

/* Memory Management: the caller is always responsible to free the results from all variants of MB_JSON_Parse (with MB_JSON_Delete) and MB_JSON_Print (with stdlib free, MB_JSON_Hooks.free_fn, or MB_JSON_free as appropriate). The exception is MB_JSON_PrintPreallocated, where the caller has full responsibility of the buffer. */
/* Supply a block of JSON, and this returns a MB_JSON object you can interrogate. */
MB_JSON_PUBLIC(MB_JSON *)
MB_JSON_Parse(const char *value);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_ParseWithLength(const char *value, size_t buffer_length);
/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will match MB_JSON_GetErrorPtr(). */
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_ParseWithOpts(const char *value, const char **return_parse_end, MB_JSON_bool require_null_terminated);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, MB_JSON_bool require_null_terminated);

/* Render a MB_JSON entity to text for transfer/storage. */
MB_JSON_PUBLIC(char *) MB_JSON_Print(const MB_JSON *item);
/* Render a MB_JSON entity to text for transfer/storage without any formatting. */
MB_JSON_PUBLIC(char *) MB_JSON_PrintUnformatted(const MB_JSON *item);
/* Render a MB_JSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
MB_JSON_PUBLIC(char *) MB_JSON_PrintBuffered(const MB_JSON *item, int prebuffer, MB_JSON_bool fmt);
/* Render a MB_JSON entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on failure. */
/* NOTE: MB_JSON is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you actually need */
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_PrintPreallocated(MB_JSON *item, char *buffer, const int length, const MB_JSON_bool format);
/* Delete a MB_JSON entity and all subentities. */
MB_JSON_PUBLIC(void) MB_JSON_Delete(MB_JSON *item);

/* Returns the number of items in an array (or object). */
MB_JSON_PUBLIC(int) MB_JSON_GetArraySize(const MB_JSON *array);
/* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_GetArrayItem(const MB_JSON *array, int index);
/* Get item "string" from object. Case insensitive. */
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_GetObjectItem(const MB_JSON * const object, const char * const string);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_GetObjectItemCaseSensitive(const MB_JSON * const object, const char * const string);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_HasObjectItem(const MB_JSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when MB_JSON_Parse() returns 0. 0 when MB_JSON_Parse() succeeds. */
MB_JSON_PUBLIC(const char *) MB_JSON_GetErrorPtr(void);

/* Check item type and return its value */
MB_JSON_PUBLIC(char *) MB_JSON_GetStringValue(const MB_JSON * const item);
MB_JSON_PUBLIC(double) MB_JSON_GetNumberValue(const MB_JSON * const item);

/* These functions check the type of an item */
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_IsInvalid(const MB_JSON * const item);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_IsFalse(const MB_JSON * const item);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_IsTrue(const MB_JSON * const item);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_IsBool(const MB_JSON * const item);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_IsNull(const MB_JSON * const item);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_IsNumber(const MB_JSON * const item);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_IsString(const MB_JSON * const item);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_IsArray(const MB_JSON * const item);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_IsObject(const MB_JSON * const item);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_IsRaw(const MB_JSON * const item);

/* These calls create a MB_JSON item of the appropriate type. */
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateNull(void);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateTrue(void);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateFalse(void);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateBool(MB_JSON_bool boolean);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateNumber(double num);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateString(const char *string);
/* raw json */
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateRaw(const char *raw);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateArray(void);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateObject(void);

/* Create a string where valuestring references a string so
 * it will not be freed by MB_JSON_Delete */
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateStringReference(const char *string);
/* Create an object/array that only references it's elements so
 * they will not be freed by MB_JSON_Delete */
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateObjectReference(const MB_JSON *child);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateArrayReference(const MB_JSON *child);

/* These utilities create an Array of count items.
 * The parameter count cannot be greater than the number of elements in the number array, otherwise array access will be out of bounds.*/
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateIntArray(const int *numbers, int count);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateFloatArray(const float *numbers, int count);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateDoubleArray(const double *numbers, int count);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_CreateStringArray(const char *const *strings, int count);

/* Append item to the specified array/object. */
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_AddItemToArray(MB_JSON *array, MB_JSON *item);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_AddItemToObject(MB_JSON *object, const char *string, MB_JSON *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the MB_JSON object.
 * WARNING: When this function was used, make sure to always check that (item->type & MB_JSON_StringIsConst) is zero before
 * writing to `item->string` */
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_AddItemToObjectCS(MB_JSON *object, const char *string, MB_JSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing MB_JSON to a new MB_JSON, but don't want to corrupt your existing MB_JSON. */
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_AddItemReferenceToArray(MB_JSON *array, MB_JSON *item);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_AddItemReferenceToObject(MB_JSON *object, const char *string, MB_JSON *item);

/* Remove/Detach items from Arrays/Objects. */
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_DetachItemViaPointer(MB_JSON *parent, MB_JSON * const item);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_DetachItemFromArray(MB_JSON *array, int which);
MB_JSON_PUBLIC(void) MB_JSON_DeleteItemFromArray(MB_JSON *array, int which);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_DetachItemFromObject(MB_JSON *object, const char *string);
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_DetachItemFromObjectCaseSensitive(MB_JSON *object, const char *string);
MB_JSON_PUBLIC(void) MB_JSON_DeleteItemFromObject(MB_JSON *object, const char *string);
MB_JSON_PUBLIC(void) MB_JSON_DeleteItemFromObjectCaseSensitive(MB_JSON *object, const char *string);

/* Update array items. */
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_InsertItemInArray(MB_JSON *array, int which, MB_JSON *newitem); /* Shifts pre-existing items to the right. */
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_ReplaceItemViaPointer(MB_JSON * const parent, MB_JSON * const item, MB_JSON * replacement);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_ReplaceItemInArray(MB_JSON *array, int which, MB_JSON *newitem);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_ReplaceItemInObject(MB_JSON *object,const char *string,MB_JSON *newitem);
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_ReplaceItemInObjectCaseSensitive(MB_JSON *object,const char *string,MB_JSON *newitem);

/* Duplicate a MB_JSON item */
MB_JSON_PUBLIC(MB_JSON *) MB_JSON_Duplicate(const MB_JSON *item, MB_JSON_bool recurse);
/* Duplicate will create a new, identical MB_JSON item to the one you pass, in new memory that will
 * need to be released. With recurse!=0, it will duplicate any children connected to the item.
 * The item->next and ->prev pointers are always zero on return from Duplicate. */
/* Recursively compare two MB_JSON items for equality. If either a or b is NULL or invalid, they will be considered unequal.
 * case_sensitive determines if object keys are treated case sensitive (1) or case insensitive (0) */
MB_JSON_PUBLIC(MB_JSON_bool) MB_JSON_Compare(const MB_JSON * const a, const MB_JSON * const b, const MB_JSON_bool case_sensitive);

/* Minify a strings, remove blank characters(such as ' ', '\t', '\r', '\n') from strings.
 * The input pointer json cannot point to a read-only address area, such as a string constant, 
 * but should point to a readable and writable adress area. */
MB_JSON_PUBLIC(void) MB_JSON_Minify(char *json);

/* Helper functions for creating and adding items to an object at the same time.
 * They return the added item or NULL on failure. */
MB_JSON_PUBLIC(MB_JSON*) MB_JSON_AddNullToObject(MB_JSON * const object, const char * const name);
MB_JSON_PUBLIC(MB_JSON*) MB_JSON_AddTrueToObject(MB_JSON * const object, const char * const name);
MB_JSON_PUBLIC(MB_JSON*) MB_JSON_AddFalseToObject(MB_JSON * const object, const char * const name);
MB_JSON_PUBLIC(MB_JSON*) MB_JSON_AddBoolToObject(MB_JSON * const object, const char * const name, const MB_JSON_bool boolean);
MB_JSON_PUBLIC(MB_JSON*) MB_JSON_AddNumberToObject(MB_JSON * const object, const char * const name, const double number);
MB_JSON_PUBLIC(MB_JSON*) MB_JSON_AddStringToObject(MB_JSON * const object, const char * const name, const char * const string);
MB_JSON_PUBLIC(MB_JSON*) MB_JSON_AddRawToObject(MB_JSON * const object, const char * const name, const char * const raw);
MB_JSON_PUBLIC(MB_JSON*) MB_JSON_AddObjectToObject(MB_JSON * const object, const char * const name);
MB_JSON_PUBLIC(MB_JSON*) MB_JSON_AddArrayToObject(MB_JSON * const object, const char * const name);

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define MB_JSON_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
/* helper for the MB_JSON_SetNumberValue macro */
MB_JSON_PUBLIC(double) MB_JSON_SetNumberHelper(MB_JSON *object, double number);
#define MB_JSON_SetNumberValue(object, number) ((object != NULL) ? MB_JSON_SetNumberHelper(object, (double)number) : (number))
/* Change the valuestring of a MB_JSON_String object, only takes effect when type of object is MB_JSON_String */
MB_JSON_PUBLIC(char*) MB_JSON_SetValuestring(MB_JSON *object, const char *valuestring);

/* Macro for iterating over an array or object */
#define MB_JSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* malloc/free objects using the malloc/free functions that have been set with MB_JSON_InitHooks */
MB_JSON_PUBLIC(void *) MB_JSON_malloc(size_t size);
MB_JSON_PUBLIC(void) MB_JSON_free(void *object);

#ifdef __cplusplus
}
#endif

#endif //FBJS_MB_JSON_H
