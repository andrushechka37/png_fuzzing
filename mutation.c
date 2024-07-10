#include "../AFLplusplus/include/afl-fuzz.h"    // TODO never write relative pathes in include
                                                // just leave it as `#include "afl-fuzz.h"` and use -I compiler option

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "png_gen_lib.h"

const int  DATA_SIZE = 10000; // TODO make it static. Avoid global constants

typedef struct my_mutator {

    afl_state_t *afl;

    struct png_buffer buffer;
    size_t trim_size_current;
    int    trimmming_steps;
    int    cur_step;

    u8 *mutated_out, *post_process_buf, *trim_buf;

} my_mutator_t;

/**
 * Initialize this custom mutator
 *
 * @param[in] afl a pointer to the internal state object. Can be ignored for
 * now.
 * @param[in] seed A seed for this mutator - the same seed should always mutate
 * in the same way.
 * @return Pointer to the data object this custom mutator instance should use.
 *         There may be multiple instances of this mutator in one afl-fuzz run!
 *         Return NULL on error.
 */
my_mutator_t * afl_custom_init(afl_state_t *afl, unsigned int seed) {

    srand(seed);  // needed also by surgical_havoc_mutate()

    my_mutator_t *data = calloc(1, sizeof(my_mutator_t));
    if (!data) {
        perror("afl_custom_init alloc");
        return NULL;
    }

    if ((data->mutated_out = (u8 *)malloc(MAX_FILE)) == NULL) { // TODO its better to write function / define to check errors instead of copy-pasting
        perror("afl_custom_init malloc");
        return NULL;
    }

    if ((data->post_process_buf = (u8 *)malloc(MAX_FILE)) == NULL) {
        perror("afl_custom_init malloc");
        return NULL;
    }

    if ((data->trim_buf = (u8 *)malloc(MAX_FILE)) == NULL) {
        perror("afl_custom_init malloc");
        return NULL;
    }

    data->afl = afl;

    return data;
}

/** // TODO either remove this or write docstrings to every function
 * Perform custom mutations on a given input
 *
 * (Optional for now. Required in the future)
 *
 * @param[in] data pointer returned in afl_custom_init for this fuzz case
 * @param[in] buf Pointer to input data to be mutated
 * @param[in] buf_size Size of input data
 * @param[out] out_buf the buffer we will work on. we can reuse *buf. NULL on
 * error.
 * @param[in] add_buf Buffer containing the additional test case
 * @param[in] add_buf_size Size of the additional test case
 * @param[in] max_size Maximum size of the mutated output. The mutation must not
 *     produce data larger than max_size.
 * @return Size of the mutated output.
 */

size_t afl_custom_fuzz(my_mutator_t * data, uint8_t * buf, size_t buf_size,
                       u8 ** out_buf, uint8_t * add_buf,
                       size_t add_buf_size,  // add_buf can be NULL
                       size_t max_size) {


    size_t mutated_size = DATA_SIZE <= max_size ? DATA_SIZE : max_size;

    make_png(&(data->buffer));
    if (max_size < data->buffer.len) {
        // TODO formatting. Remove empty lines in the begin of `if-body` and add them before `if`
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


