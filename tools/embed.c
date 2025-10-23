#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <libgen.h>

#define COMMONLIB_REMOVE_PREFIX
#define COMMONLIB_IMPLEMENTATION
#include <commonlib.h>

typedef struct {
    unsigned char* data;
    size_t size;
} Data;

#define defer(ret_val) \
    result = ret_val;\
    goto defer


bool is_char_valid_var_name(char ch){
    return ((('a' <= ch) && (ch <= 'z')) || (('A' <= ch) && (ch <= 'Z')) || (ch == '_'));
}

bool validate_c_name(const char *name, size_t name_size, char *new_name, size_t new_name_size) {
    size_t n = 0;
    for (size_t i = 0; i < name_size; ++i) {
        char ch = name[i];
        if (!is_char_valid_var_name(ch)) {
            if (isdigit(ch)) {
                if (i == 0) {
                    new_name[n++] = '_';
                }
                new_name[n++] = ch;
            }
        } else {
            new_name[n++] = ch;
            if (n >= new_name_size) {
                log_error("new_name_size `%zu` is too small for the new name!", new_name_size);
                return false;
            }
        }
    }
    return true;
}

Data* slurp_file(const char* filename){
    FILE* f = fopen(filename, "rb");
    Data* result = (Data*)malloc(sizeof(Data));

    if (f == NULL){
        fprintf(stderr, "ERROR: slurp_file::fopen(\"%s\", \"rb\") -> %s\n", filename, strerror(errno));
        defer(NULL);
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: slurp_file(%s)::fseek(f, 0, SEEK_END) -> %s\n", filename, strerror(errno));
        defer(NULL);
    }

    int fsize = ftell(f);

    if (fsize == -1){
        fprintf(stderr, "ERROR: slurp_file(%s)::ftell(f) -> %s\n", filename, strerror(errno));
        defer(NULL);
    }

    result->size = (size_t)fsize;

    result->data = (unsigned char*)malloc(sizeof(unsigned char)*fsize);

    if (result == NULL){
        fprintf(stderr, "ERROR: slurp_file::malloc(%zu) -> %s\n", sizeof(char)*fsize, strerror(errno));
        defer(NULL);
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: slurp_file(%s)::fseek(f, 0, SEEK_SET) -> %s\n", filename, strerror(errno));
        defer(NULL);
    }

    if (fread(result->data, sizeof(char), result->size, f) != result->size){
        fprintf(stderr, "ERROR: slurp_file::fread(result, %zu, 1, f) -> %s\n", result->size, strerror(errno));
        defer(NULL);
    }

 defer:
    fclose(f);
    return result;
}

bool write_data_to_cfile(Data* data, const char* _out){
    char buff[1024] = {0};
    snprintf(buff, 1024, "%s.c", _out);
    const char* filename = buff;
    FILE* f = fopen(filename, "wb");
    bool result = true;
    char* out = (char*)malloc(sizeof(char)*strlen(_out));
    char *og_out = out;

    size_t _out_len = strlen(_out);
    memcpy(out, _out, _out_len);
    out[_out_len] = '\0';

    out = basename(out);

    size_t out_len = strlen(out);

    if (f == NULL){
        fprintf(stderr, "ERROR: write_data_to_cfile::fopen(\"%s\", \"wb\") -> %s\n", filename, strerror(errno));
        /* fclose(f); */
        return false;
        /* defer(false); */
    }

#define NEW_NAME_SIZE (1024*2)
    char new_name[NEW_NAME_SIZE] = {0};

    if (!validate_c_name(out, out_len, new_name, NEW_NAME_SIZE)) return false;

    fprintf(f, "// file created from `embed` program.\n");
    fprintf(f, "\n");
    fprintf(f, "#define %s_size (%zu)\n", new_name, data->size);
    fprintf(f, "static unsigned char %s_bytes[%s_size] = {\n", new_name, new_name);

    int per_line = 20;
    int n = 0;
    for (int i = 0; i < data->size; ++i){
        fprintf(f, "0x%X, ", (uint8_t)data->data[i]);
        if (++n >= per_line){
            n = 0;
            fprintf(f, "\n");
        }
    }

    fprintf(f, "};\n");

    printf("INFO: Created file `%s`\n", filename);

    free(og_out);
    return result;
}

int main(int argc, char** argv){
    const char* program = shift_args(argv, argc);

    if (argc <= 0){
        fprintf(stderr, "Usage: %s <file>\n", program);
        fprintf(stderr, "Error: No input file is provided!\n");
        exit(1);
    }

    const char* filename = shift_args(argv, argc);

    Data* data = slurp_file(filename);

    if (data == NULL) return 1;

    String_view sv = sv_from_cstr(filename);

    sv_rremove_until_char_after(&sv, '.');

    if (!write_data_to_cfile(data, sv_to_cstr(sv))) return 1;

    fflush(stdout);

    return 0;
}
