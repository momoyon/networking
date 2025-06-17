#ifndef _COMMONLIB_H_
#define _COMMONLIB_H_
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

#if defined(_WIN32) && defined(_MSC_VER)
// NOTE: Don't include unwanted files to speed up compilation
#define WIN32_LEAN_AND_MEAN
#define NOCOMM
#include <windows.h>
#undef C_ASSERT // Bruh
#endif

// Remove Prefix
#ifdef COMMONLIB_REMOVE_PREFIX
#define ASSERT C_ASSERT
#define ARRAY_LEN C_ARRAY_LEN

#define darr_append c_darr_append
#define darr_free c_darr_free
#define darr_shift c_darr_shift
#define darr_remove c_darr_remove
#define darr_delete c_darr_delete
#define DYNAMIC_ARRAY_INITIAL_CAPACITY c_DYNAMIC_ARRAY_INITIAL_CAPACITY
// Deprecated Dynamic-array API
#define da_append c_da_append
#define da_delete c_da_delete
#define da_remove c_da_remove
// #define c_DYNAMIC_ARRAY_INITIAL_CAPACITY
#define arr_stack_init c_arr_stack_init
#define arr_heap_init c_arr_heap_init
#define arr_append c_arr_append
#define arr_remove c_arr_remove
#define arr_delete c_arr_delete
#define arr_free   c_arr_free

#define os_get_timedate c_os_get_timedate
#define os_file_exists c_os_file_exists

#define log_error c_log_error
#define log_info c_log_info
#define log_warning c_log_warning
#define log_debug c_log_debug

#define read_file c_read_file
#define touch_file_if_doesnt_exist c_touch_file_if_doesnt_exist

#define Arena c_Arena
#define arena_make c_arena_make
#define arena_alloc c_arena_alloc
#define arena_reset c_arena_reset
#define arena_free c_arena_free
#define arena_alloc_str c_arena_alloc_str
#define arena_alloc_wstr c_arena_alloc_wstr

#define clampi c_clampi
#define clampf c_clampf
#define randomi c_randomi
#define randomf c_randomf

#define String_view c_String_view

#define shift_args c_shift_args

#define SV_FMT c_SV_FMT
#define SV_ARG c_SV_ARG

#define SV c_SV

#define sv_print_dumb c_sv_print_dumb
#define sv_from_cstr c_sv_from_cstr
#define sv_lpop c_sv_lpop
#define sv_lpop_until_predicate c_sv_lpop_until_predicate
#define sv_lpop_until_string c_sv_lpop_until_string
#define sv_rpop_until_predicate c_sv_rpop_until_predicate
#define sv_lpop_until_char c_sv_lpop_until_char
#define sv_rpop_until_char c_sv_rpop_until_char
#define sv_lremove c_sv_lremove
#define sv_rremove c_sv_rremove
#define sv_lremove_until_char c_sv_lremove_until_char
#define sv_rremove_until_char c_sv_rremove_until_char
#define sv_lremove_until_char_after c_sv_lremove_until_char_after
#define sv_rremove_until_char_after c_sv_rremove_until_char_after
#define sv_ltrim c_sv_ltrim
#define sv_rtrim c_sv_rtrim
#define sv_trim c_sv_trim
#define sv_to_cstr c_sv_to_cstr
#define sv_to_int c_sv_to_int
#define sv_to_uint c_sv_to_uint
#define sv_to_uint8_hex c_sv_to_uint8_hex
#define sv_to_float c_sv_to_float
#define sv_contains_char c_sv_contains_char
#define sv_is_hex_numbers c_sv_is_hex_numbers
#define sv_equals c_sv_equals
#define sv_get_part c_sv_get_part


#endif // COMMONLIB_REMOVE_PREFIX

// Memory allocation
#ifndef C_MALLOC
#define C_MALLOC malloc
#endif
#ifndef C_CALLOC
#define C_CALLOC calloc
#endif
#ifndef C_FREE
#define C_FREE free
#endif
#ifndef C_REALLOC
#define C_REALLOC realloc
#endif
#ifndef C_MEMCPY
#define C_MEMCPY memcpy
#endif


// typedefs
typedef unsigned int uint;
typedef uint8_t      uint8;
typedef uint16_t     uint16;
typedef uint32_t     uint32;
typedef uint64_t     uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float  float32;
typedef double float64;

typedef wchar_t wchar;

typedef const char*  cstr;
typedef const wchar* wstr;


// Macros
#if defined(_MSC_VER) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#define C_ASSERT(condition, msg) do {\
                if (!(condition)) {\
                        fprintf(stderr, "%s:%d:0 [ASSERTION FAILED] %s: %s", __FILE__, __LINE__, #condition, msg);\
                        DebugBreak();\
                }\
        } while (0)
#else
#define C_ASSERT(condition, msg) do {\
                if (!(condition)) {\
                        fprintf(stderr, "%s:%d:0 [ASSERTION FAILED] %s: %s", __FILE__, __LINE__, #condition, msg);\
                        exit(1);\
                }\
        } while (0)
#endif // defined(_MSC_VER) && !defined(__clang__) && !defined(__INTEL_COMPILER)

#define C_ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define c_shift(xs, xsz) (assert(xsz > 0 && "Array is empty"), xsz--, *xs++)
#define c_shift_args c_shift

//
// Math
//

int   c_clampi(int v, int min, int max);
float c_clampf(float v, float min, float max);
float c_randomf(float from, float to);
int   c_randomi(int from, int to);

//
// Struct pre-decls
//

typedef struct c_Arena c_Arena;

//
// ## Data Structures
//

//
// Dynamic-Array
//

// NOTE: To use c_darr_append() the Dynamic-Array struct should be defined using
// DEFINE_DYNAMIC_ARRAY or have the same members as below!
// NOTE!!!: We actually don't want this since this makes the user want to
// use this macro to define dynamic arrays. But the user might want extra fields
// in the struct; So we recommend defining da structs manually like:
// ```C
// typedef struct {
//    <item-type> items;
//    size_t count;
//    size_t capacity;
//    [extra fields...];
// }
// ```
// #define DEFINE_DYNAMIC_ARRAY(struct_name, elm_type) typedef struct {
//         elm_type *items;
//         size_t count;
//         size_t capacity;
//     }

#define c_DYNAMIC_ARRAY_INITIAL_CAPACITY (sizeof(size_t))

#define c_darr_append(da, elm) c_darr_append_impl(da, elm, c_darr_append)
#define c_darr_append_impl(da, elm, api) do {\
        if (strcmp(#api, "c_darr_append") != 0) {\
            c_log_warning("%s is deprecated please use the newer api!", #api);\
        }\
		if ((da).items == NULL) {\
			(da).capacity = c_DYNAMIC_ARRAY_INITIAL_CAPACITY;\
			(da).count = 0;\
			(da).items = C_MALLOC(sizeof(elm) * (da).capacity);\
		}\
		if ((da).count >= (da).capacity) {\
			(da).capacity *= 2;\
                        (da).items = C_REALLOC((da).items, (da).capacity * sizeof(elm));\
			C_ASSERT((da).items != NULL, "TODO: Log error instead of asserting");\
		}\
		(da).items[(da).count++] = elm;\
	} while (0)

// NOTE: We cant do C_ASSERT() here because it aint one expression...
// NOTE: darr_shift will make the da loose its ptr, so store the ptr elsewhere if you want to free it later!!!
#define c_darr_shift(da) (assert((da).count > 0 && "Array is empty"), (da).count--, *(da).items++)
#define c_darr_free(da) C_FREE((da).items)

#define c_darr_delete(da, type, idx) c_darr_delete_impl(da, type, idx, c_darr_delete)
#define c_darr_delete_impl(da, type, idx, api) do {\
        if (strcmp(#api, "c_darr_delete") != 0) {\
            c_log_warning("%s is deprecated please use the newer api!", #api);\
        }\
        if ((idx) >= 0 && (idx) <= (da).count-1) {\
            type temp = (da).items[(idx)];\
            (da).items[(idx)] = (da).items[(da).count-1];\
            (da).items[--(da).count] = temp;\
        } else {\
            c_log_error("%s:%d: Trying to remove from outofbounds! %zu != (0 ~ %zu)", __FILE__, __LINE__, (size_t)idx, (size_t)(da).count);\
            exit(1);\
        }\
    } while (0)

#define c_darr_remove(da, type, elm_ptr, idx) c_darr_remove_impl(da, type, elm_ptr, idx, c_darr_remove)
#define c_darr_remove_impl(da, type, elm_ptr, idx, api) do {\
        if (strcmp(#api, "c_darr_remove") != 0) {\
            c_log_warning("%s is deprecated please use the newer api!", #api);\
        }\
        if ((idx) >= 0 && (idx) <= (da).count-1) {\
            type temp = (da).items[(idx)];\
            (da).items[(idx)] = (da).items[(da).count-1];\
            (da).items[--(da).count] = temp;\
            if ((elm_ptr) == NULL) {\
                c_log_error("%s:%d: You cant pass NULL as the elm_ptr! please use c_darr_delete to not get the element removed!", __FILE__, __LINE__);\
                exit(1);\
            }\
            type *temp_ptr = elm_ptr; \
            *temp_ptr = temp; \
        } else {\
            c_log_error("%s:%d: Trying to remove from outofbounds! %zu != (0 ~ %zu)", __FILE__, __LINE__, (size_t)idx, (size_t)(da).count);\
            exit(1);\
        }\
    } while (0)

// Deprecated API
#define c_da_append(da, elm) c_darr_append_impl(da, elm, c_da_append)
#define c_da_delete(da, type, idx) c_darr_delete_impl(da, type, idx, c_da_delete)
#define c_da_remove(da, type, elm_ptr, idx) c_darr_remove_impl(da, type, elm_ptr, idx, c_da_remove)

//
// Static-Array
//

// #define DEFINE_DYNAMIC_ARRAY(struct_name, elm_type) typedef struct {
//         elm_type *items;
//         size_t count;
//         size_t capacity;
//     }

// NOTE: Initializes the static-array on the heap
#define c_arr_heap_init(arr, cap) do {\
    if ((arr).items != NULL) {\
        log_error("%s:%d: This static-array is already initialized!", __FILE__, __LINE__);\
        exit(1);\
    }\
	(arr).count = 0;\
    (arr).capacity = cap;\
    (arr).items = C_CALLOC((arr).capacity, sizeof(*(arr).items));\
    if ((arr).items == NULL) {\
        log_error("%s:%d: calloc failed while trying init this static-array!", __FILE__, __LINE__);\
        exit(1);\
    }\
    } while (0)

// NOTE: Initializes the static-array on the stack
// NOTE: You are supposed to pass a struct that ateast has the same members as Dynamic-Arrays.
// NOTE: This just sets the `capacity` member of the struct to the capacity inferred by the stack-allocated items
// eg: ```C
//     #define ARR_CAP 1024
//     typedef struct {
//          int items[ARR_CAP];
//          size_t count;
//          size_t capacity;
//     } Array;
//     ```
#define c_arr_stack_init(arr) do {\
        if ((arr).items == NULL) {\
            log_error("%s:%d: Please pass an static-array that has it's `items` alloced on the stack!", __FILE__, __LINE__);\
            exit(1);\
        }\
        (arr).capacity = C_ARRAY_LEN((arr).items);\
        if ((arr).capacity == 0) {\
            log_error("%s:%d: Failed to set the capacity of the static-array!", __FILE__, __LINE__);\
            exit(1);\
        }\
    } while (0)

#define c_arr_append(arr, elm) do {\
    if ((arr).items == NULL) {\
        log_error("%s:%d: Please initialize the static array!", __FILE__, __LINE__);\
        exit(1);\
    }\
    if ((arr).count <= (arr).capacity-1) {\
        (arr).items[(arr).count++] = elm;\
    } else {\
		log_warning("%s:%d: Trying to add element outofbounds!", __FILE__, __LINE__);\
	}\
    } while (0)

#define c_arr_remove c_darr_remove
#define c_arr_delete c_darr_delete
#define c_arr_free c_darr_free

//
// OS
//

#if defined(__linux__)
#include <sys/stat.h>
#endif

void c_os_get_timedate(c_Arena* a);
bool c_os_file_exists(cstr filename);

//
// Logging
//

#define c_log_error(fmt, ...) do {\
		fprintf(stderr, "%s"fmt"\n", "[ERROR] ", ##__VA_ARGS__);\
	} while (0)
#define c_log_info(fmt, ...) do {\
		fprintf(stdout, "%s"fmt"\n", "[INFO] ", ##__VA_ARGS__);\
	} while (0)
#define c_log_warning(fmt, ...) do {\
		fprintf(stdout, "%s"fmt"\n", "[WARNING] ", ##__VA_ARGS__);\
	} while (0)
#ifdef DEBUG
#define c_log_debug(fmt, ...) do {\
		fprintf(stdout, "%s"fmt"\n", "[DEBUG] ", ##__VA_ARGS__);\
	} while (0)
#else
#define c_log_debug(...)
#endif // DEBUG
//
// File
//

// reads entire file and gives back the file content and filesize in bytes. (caller must be responsible for freeing the string!)
const char* c_read_file(const char* filename, int *file_size);
void c_touch_file_if_doesnt_exist(cstr file);

//
// ### Allocators ###
//

//
// c_Arena
//

#define ARENA_BUFF_INITIAL_SIZE (1024*4)

struct c_Arena {
    void* buff;
    uint64 buff_size;
    void* ptr;
};

// pass size 0 to get ARENA_BUFF_INITIAL_SIZE
c_Arena c_arena_make(size_t size);
void* c_arena_alloc(c_Arena* a, size_t size);
void c_arena_reset(c_Arena* a);
void c_arena_free(c_Arena* a);

// TODO: Do we embed stb_snprintf to use stbsp_snprintf?
#define c_arena_alloc_str(a, fmt, ...)    c_arena_alloc(&(a), sizeof(char)*snprintf((a).ptr, (int)((a).buff_size - ((uint8*)(a).ptr - (uint8*)(a).buff)), (fmt), __VA_ARGS__)+1)
#define c_arena_alloc_wstr(a, fmt, ...) c_arena_alloc(&a, sizeof(char)*wprintf(a.ptr, a.buff_size - ((uint8*)a.ptr - (uint8*)a.buff), (fmt), __VA_ARGS__)+1)

//
// String Builder
//

typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} c_String_builder;

void c_sb_append(c_String_builder* sb, char* data);

//
// String view
//

typedef struct {
    char *data;
    size_t count;
} c_String_view;

#define c_SV_FMT "%.*s"
#define c_SV_ARG(sv) (int)(sv).count, (sv).data

#define c_SV(cstr) (c_String_view){.data = (char*)cstr, strlen(cstr)}

void c_sv_print_dumb(c_String_view sv);
c_String_view c_sv_from_cstr(const char* cstr); // Actually just use SV(cstr) macro...
c_String_view c_sv_lpop(c_String_view* sv, uint32 n);
c_String_view c_sv_lpop_until_predicate(c_String_view* sv, int(*predicate)(int));
c_String_view c_sv_lpop_until_string(c_String_view* sv, const char *string);
c_String_view c_sv_rpop_until_predicate(c_String_view* sv, int(*predicate)(int));
c_String_view c_sv_lpop_until_char(c_String_view* sv, char ch);
c_String_view c_sv_rpop_until_char(c_String_view* sv, char ch);
void c_sv_lremove(c_String_view* sv, size_t n);
void c_sv_rremove(c_String_view* sv, size_t n);
void c_sv_lremove_until_char(c_String_view* sv, char ch);
void c_sv_rremove_until_char(c_String_view* sv, char ch);
void c_sv_lremove_until_char_after(c_String_view* sv, char ch);
void c_sv_rremove_until_char_after(c_String_view* sv, char ch);
void c_sv_ltrim(c_String_view* sv);
void c_sv_rtrim(c_String_view* sv);
void c_sv_trim(c_String_view* sv);
char* c_sv_to_cstr(c_String_view sv);
int64 c_sv_to_int(c_String_view sv, int *count, int base);
uint64 c_sv_to_uint(c_String_view sv, int *count, int base);
float64 c_sv_to_float(c_String_view sv, int *count);
bool c_sv_contains_char(c_String_view sv, char ch);
bool c_sv_is_hex_numbers(c_String_view sv);
bool c_sv_equals(c_String_view sv1, c_String_view sv2);
c_String_view c_sv_get_part(c_String_view sv, int from, int to);

#endif /* _COMMONLIB_H_ */

//////////////////////////////////////////////////
#ifdef COMMONLIB_IMPLEMENTATION
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

// My things implementation:

//
// Math
//

int c_clampi(int v, int min, int max) {
    v = v < min ? min : v;
    v = v > max ? max : v;
    return v;
}

float c_clampf(float v, float min, float max) {
    v = v < min ? min : v;
    v = v > max ? max : v;
    return v;
}

float c_randomf(float from, float to) {
	float r = rand() / RAND_MAX;
	return from + (to - from) * r;
}

int c_randomi(int from, int to) {
	return randomf((float)from, (float)to);
}

//
// OS
//

#if defined(_WIN32) || defined(__CYGWIN__)
void c_os_get_timedate(c_Arena* a) {
        (void)a;
        C_ASSERT(false, "Unimplemented!");
}

bool c_os_file_exists(cstr filename) {
        (void) filename;
        C_ASSERT(false, "Unimplemented!");
        return false;
}

#elif defined(__linux__)
void c_os_get_timedate(c_Arena* a) {
        (void)a;
        C_ASSERT(false, "Unimplemented!");
}

bool c_os_file_exists(cstr filename) {
        struct stat buf;
        return stat(filename, &buf) == 0;
}
#endif

// simple and dirty way to have defering in C (not recommended to use!)
#define defer(ret_val) \
    result = ret_val;\
    goto defer

const char *c_read_file(const char* filename, int *file_size) {
    FILE* f = fopen(filename, "r");
    char* result = NULL;

    if (f == NULL){
        // TODO: Replace every strerror with strerror_s
        c_log_error("'%s': %s", filename, strerror(errno));
        defer(NULL);
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        c_log_error("'%s': %s", filename, strerror(errno));
        defer(NULL);
    }

    size_t fsize = ftell(f);

    if (fsize == (size_t)-1){
        c_log_error("'%s': %s", filename, strerror(errno));
        defer(NULL);
    }

    result = C_MALLOC(sizeof(char)*(fsize+1));

    if (result == NULL){
        c_log_error("'%s': %s", filename, strerror(errno));
        defer(NULL);
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        c_log_error("'%s': %s", filename, strerror(errno));
        defer(NULL);
    }

    size_t read = fread((char*)result, sizeof(char), fsize, f);

    // Process the result to remove '\r' characters
    char* cleaned_result = malloc(read + 1); // Allocate memory for cleaned result
    if (cleaned_result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(result);
        return NULL;
    }

    size_t j = 0; // Index for cleaned_result
    for (size_t i = 0; i < read; i++) {
        if (result[i] != '\r') { // Skip '\r' characters
            cleaned_result[j++] = result[i];
        }
    }
    cleaned_result[j] = '\0'; // Null-terminate the cleaned result

    free(result); // Free the original result
    *file_size = (int)j; // Update the file size without '\r'
    return cleaned_result; // Return the cleaned result

    *file_size = (int)read;

 defer:
    if (f) fclose(f);
    if (result == NULL) *file_size = -1;
    return result;
}

void c_touch_file_if_doesnt_exist(cstr filename) {
        if (c_os_file_exists(filename)) return;
        FILE* file = fopen(filename, "w");
        if (file) fclose(file);
}

//
// ### Allocators ###
//

// c_Arena

c_Arena c_arena_make(size_t size) {
    c_Arena res = {0};
    res.buff_size = size == 0 ? ARENA_BUFF_INITIAL_SIZE : size;
    res.buff = C_MALLOC(res.buff_size);
    res.ptr = res.buff;

    C_ASSERT(res.buff, "Malloc failed?");

    return res;
}

void* c_arena_alloc(c_Arena* a, size_t size) {
    C_ASSERT(a->buff, "Bro pass an initialized arena!");

    void* res = a->ptr;
    a->ptr = (uint8*)a->ptr + size;

    size_t diff = (size_t)((uint8*)a->ptr - (uint8*)a->buff);
    if (diff > a->buff_size) {
        c_log_info("c_Arena resized from %zu to %zu", a->buff_size, a->buff_size*2);
        a->buff_size *= 2;
        a->buff = C_REALLOC(a->buff, a->buff_size);
        a->ptr = (uint8*)a->buff + diff;
        res = a->ptr;
        a->ptr = (uint8*)a->ptr + size;
    }
    /* C_ASSERT((size_t)((uint8*)a->ptr - (uint8*)a->buff) <= a->buff_size); */

    return res;
}

void c_arena_reset(c_Arena* a) {
    a->ptr = a->buff;
}

void c_arena_free(c_Arena* a) {
    C_FREE(a->buff);
}

//
// String Builder
//

void c_sb_append(c_String_builder* sb, char* data) {
    size_t data_size = strlen(data);
    if (sb->size + data_size > sb->capacity) {
        sb->capacity *= 2;
        sb->data = C_REALLOC(sb->data, sb->capacity);
    }

    // void *memcpy(void dest[restrict .n], const void src[restrict .n],
    C_MEMCPY((void *)((uintptr_t)sb->data + (uintptr_t)sb->data), data, data_size);
    sb->size += data_size;
}

//
// String view
//

void c_sv_print_dumb(c_String_view sv){
    for (size_t i = 0; i < (size_t)sv.count; ++i){
        putc(*(sv.data+i), stdout);
    }
}

c_String_view c_sv_from_cstr(const char* cstr){
    return (c_String_view){
        .data = (char *)cstr,
        .count = strlen(cstr),
    };
}

c_String_view c_sv_lpop(c_String_view* sv, uint32 n) {
    c_String_view res = {0};
    if (sv->count < n) return res;
    res.data = sv->data;
    res.count = n;

    sv->data += n;
    sv->count -= n;

    return res;
}

c_String_view c_sv_lpop_until_predicate(c_String_view* sv, int(*predicate)(int)){
    const char* old_sv_data = sv->data;
    while (sv->count > 0 && !predicate(*sv->data)){
        sv->data++;
        sv->count--;
    }

    return (c_String_view){
        .data = sv->data - (sv->data - old_sv_data),
        .count = (sv->data - old_sv_data),
    };
}

c_String_view c_sv_lpop_until_string(c_String_view* sv, const char *string) {
    size_t string_len = strlen(string);

    char *old_sv_data = sv->data;

    while (sv->count > string_len) {
        bool matched = true;
        for (size_t i = 0; i < string_len; ++i) {
            if (sv->data[i] != string[i]) matched = false;
        }
        if (matched) break;
        sv->data++;
        sv->count--;
    }

    return (c_String_view) {
        .data = old_sv_data,
        .count = (sv->data - old_sv_data),
    };
}

c_String_view c_sv_rpop_until_predicate(c_String_view* sv, int(*predicate)(int)){
    size_t old_sv_count = sv->count;
    while (sv->count > 0 && !predicate(*(sv->data+sv->count-1))){
        sv->count--;
    }

    return (c_String_view){
        .data = sv->data + sv->count,
        .count = old_sv_count - sv->count,
    };
}

c_String_view c_sv_lpop_until_char(c_String_view* sv, char ch){
    const char* old_sv_data = sv->data;
    while (sv->count > 0 && *sv->data != ch){
        sv->data++;
        sv->count--;
    }

    return (c_String_view){
        .data = sv->data - (sv->data - old_sv_data),
        .count = (sv->data - old_sv_data),
    };
}

c_String_view c_sv_rpop_until_char(c_String_view* sv, char ch){
    size_t old_sv_count = sv->count;
    while (sv->count > 0 && *(sv->data+sv->count-1) != ch){
        sv->count--;
    }

    return (c_String_view){
        .data = sv->data + sv->count,
        .count = old_sv_count - sv->count,
    };
}

void c_sv_lremove(c_String_view* sv, size_t n){
    if (n > sv->count) n = sv->count;

    sv->data += n;
    sv->count -= n;
}

void c_sv_rremove(c_String_view* sv, size_t n){
    if (n > sv->count)
        sv->count = 0;
    else
        sv->count -= n;
}

void c_sv_lremove_until_char(c_String_view* sv, char ch){
    while (sv->count > 0 && *sv->data != ch){
        sv->data++;
        sv->count--;
    }
}

void c_sv_rremove_until_char(c_String_view* sv, char ch){
    while (sv->count > 0 && *(sv->data+sv->count-1) != ch){
        sv->count--;
    }
}

void c_sv_lremove_until_char_after(c_String_view* sv, char ch){
    while (sv->count > 0 && *(sv->data-1) != ch){
        sv->data++;
        sv->count--;
    }
}

void c_sv_rremove_until_char_after(c_String_view* sv, char ch){
    while (sv->count > 0 && *(sv->data+sv->count) != ch){
        sv->count--;
    }
}

void c_sv_ltrim(c_String_view* sv){
    while (sv->count > 0 && isspace(*sv->data)){
        sv->data++;
        sv->count--;
    }
}

void c_sv_rtrim(c_String_view* sv){
    while (sv->count > 0 && isspace(*(sv->data+sv->count-1))){
        sv->count--;
    }
}

void c_sv_trim(c_String_view* sv){
    c_sv_ltrim(sv);
    c_sv_rtrim(sv);
}

char* c_sv_to_cstr(c_String_view sv){
    char* res = (char*)malloc(sizeof(char)*(sv.count + 1));
    if (res == NULL) {
        C_ASSERT(false, "Buy more RAM bruh");
    }
    C_MEMCPY(res, sv.data, sv.count);
    res[sv.count] = '\0';
    return res;
}

int64 c_sv_to_int(c_String_view sv, int *count_out, int base) {
    char *str = c_sv_to_cstr(sv);

    char *endptr = NULL;

    int64 res = strtol(str, &endptr, base);

    if (endptr == str) {
        *count_out = -1;
        return res;
    }

    *count_out = (int)(endptr - str);

    C_FREE(str);
    return res;
}

uint64 c_sv_to_uint(c_String_view sv, int *count, int base) {
    char* str = c_sv_to_cstr(sv);

    char *endptr = NULL;
    uint64 res = strtoul(str, &endptr, base);

    if (endptr == str) {
        *count = -1;
        return res;
    }

    *count = (int)(endptr - str);

    C_FREE(str);
    return res;
}

float64 c_sv_to_float(c_String_view sv, int *count) {
    char* str = c_sv_to_cstr(sv);

    char *endptr = NULL;
    float64 res = strtod(str, &endptr);

    if (endptr == str) {
        *count = -1;
        return res;
    }

    *count = (int)(endptr - str);

    C_FREE(str);
    return res;
}

bool c_sv_contains_char(c_String_view sv, char ch){
    for (size_t i = 0; i < sv.count; ++i){
        if (sv.data[i] == ch) return true;
    }
    return false;
}

bool c_sv_is_hex_numbers(c_String_view sv) {
    char hex[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '0', 'a', 'b', 'c', 'd', 'e', 'f',
        'A', 'B', 'C', 'D', 'E', 'F'
    };
    bool found = false;
    for (size_t i = 0; i < sv.count; ++i) {
        char c = sv.data[i];
        for (size_t j = 0; j < C_ARRAY_LEN(hex); ++j) {
            if (hex[j] == c) {
	found = true;
            }
        }
    }

    return found;
}

bool c_sv_equals(c_String_view sv1, c_String_view sv2) {
    if (sv1.count != sv2.count) return false;
    for (size_t i = 0; i < sv1.count; ++i) {
        if (sv1.data[i] != sv2.data[i]) {
            return false;
        }
    }

    return true;
}

c_String_view c_sv_get_part(c_String_view sv, int from, int to) {
    from = clampi(from, 0, sv.count);
    to   = clampi(to, from, sv.count);

    c_String_view range = {
        .data = (char*)(sv.data + from),
        .count = (size_t)(to - from),
    };

    return range;
}
#endif
