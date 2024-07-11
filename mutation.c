#include "../AFLplusplus/include/afl-fuzz.h"    // TODO never write relative pathes in include
                                                // just leave it as `#include "afl-fuzz.h"` and use -I compiler option

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "png_gen_lib.h"

static const int  DATA_SIZE = 10000;

typedef struct my_mutator {

    afl_state_t *afl;

    struct png_buffer buffer;
    size_t trim_size_current;
    int    trimmming_steps;
    int    cur_step;

    u8 *mutated_out, *post_process_buf, *trim_buf;

} my_mutator_t;

#define PTR_CHECK(ptr, message)     \
    if (ptr == NULL) {              \
        perror(message);            \
        return 0;                   \
    }

my_mutator_t * afl_custom_init(afl_state_t *afl, unsigned int seed) {

    srand(seed);  // needed also by surgical_havoc_mutate()

    my_mutator_t *data = calloc(1, sizeof(my_mutator_t));

    PTR_CHECK(data, "afl_custom_init alloc");
    PTR_CHECK((data->mutated_out = (u8 *)malloc(MAX_FILE)), "afl_custom_init malloc");
    PTR_CHECK((data->post_process_buf = (u8 *)malloc(MAX_FILE)), "afl_custom_init malloc");
    PTR_CHECK((data->trim_buf = (u8 *)malloc(MAX_FILE)), "afl_custom_init malloc");

    data->afl = afl;

    return data;
}

#undef PTR_CHECK

size_t afl_custom_fuzz(my_mutator_t * data, uint8_t * buf, size_t buf_size,
                       u8 ** out_buf, uint8_t * add_buf,
                       size_t add_buf_size,  // add_buf can be NULL
                       size_t max_size) {

    size_t mutated_size = DATA_SIZE <= max_size ? DATA_SIZE : max_size;

    make_png(&(data->buffer));

    if (max_size < data->buffer.len) {
        memcpy(data->mutated_out, data->buffer.data, max_size);
        mutated_size = max_size;
    } else {
        memcpy(data->mutated_out, data->buffer.data, data->buffer.len);
        mutated_size = data->buffer.len;
    }
    
    *out_buf = data->mutated_out;
    return mutated_size;
}

void afl_custom_deinit(my_mutator_t *data) {

    free(data->post_process_buf);
    free(data->mutated_out);
    free(data->trim_buf);
    free(data->buffer.data);
    free(data);
}


