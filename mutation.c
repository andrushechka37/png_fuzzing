

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_size_t.h>
#include "zlib.h"

unsigned long crc(unsigned char *buf, int len);

const int IHDR_LENGTH = 13;
const int START_LEN = 100;
const int LEN_OF_BLOCK = 4;

struct chunk {
    unsigned long length;
    char type[4];
    unsigned char * data; 
};

struct png_buffer {
    unsigned char * data;
    size_t len;
};

void print_number(unsigned long number, struct png_buffer * buffer) {

    unsigned char array[4];
    array[0] = (number >> 24) & 0xFF;
    array[1] = (number >> 16) & 0xFF;
    array[2] = (number >>  8) & 0xFF;
    array[3] =  number        & 0xFF;

    for (int i = 0; i < 4; i++) {
        buffer->data[buffer->len] = array[i];
        buffer->len++;
    }
}

void write_chunk(struct chunk * chunk, struct png_buffer * buffer) {

    buffer->data = (unsigned char *) realloc(buffer->data, buffer->len + chunk->length + 12);

    print_number(chunk->length, buffer);             
    
    memcpy(&(buffer->data[buffer->len]), chunk->type, 4);   
    buffer->len += 4;                                      

    char * hash_str = NULL;                                                         
    if (chunk->data == NULL) {                                                      
        hash_str = (char *) calloc(START_LEN, sizeof(char));                         
    } else {                                                                        // data
        memcpy(&(buffer->data[buffer->len]), chunk->data, chunk->length);
        buffer->len += chunk->length;
        hash_str = (char *) calloc(chunk->length + START_LEN, sizeof(char));         
    }                                                                               

    memcpy(hash_str, chunk->type, LEN_OF_BLOCK);                                        //
    memcpy(&(hash_str[4]), chunk->data, chunk->length);                                 // hash
    unsigned long hash = crc((unsigned char *)hash_str, LEN_OF_BLOCK + chunk->length);  //
    print_number(hash, buffer);                                                         //

    free(hash_str);
}

void make_png(struct png_buffer * buffer) {

    buffer->data = (unsigned char *) calloc(START_LEN, sizeof(char));

    unsigned long WIDTH =  rand() % 256;
    unsigned long HEIGHT = rand() % 256;

    // signature ---------------------------------------------------------
    unsigned char signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    memcpy(buffer->data, signature, 8);
    buffer->len = 8;
    // signature ---------------------------------------------------------


    // IHDR - main info about picture ------------------------------------
    struct chunk IHDR = {};
    IHDR.length = IHDR_LENGTH;     
    strcpy(IHDR.type, "IHDR");  

    IHDR.data = (unsigned char *)calloc(IHDR_LENGTH, sizeof(char));

    IHDR.data[3] = (char)  WIDTH;            // data
    IHDR.data[2] = (char) (WIDTH >> 8);
    IHDR.data[1] = (char) (WIDTH >> 16);
    IHDR.data[0] = (char) (WIDTH >> 24);

    IHDR.data[7] = (char) HEIGHT;
    IHDR.data[6] = (char) (HEIGHT >> 8);
    IHDR.data[5] = (char) (HEIGHT >> 16);
    IHDR.data[4] = (char) (HEIGHT >> 24);

    IHDR.data[8] = 8;   // bit depth // 1
    IHDR.data[9] = 2;   // colour type (RGB) // 0

    IHDR.data[10] = 0;  // weave method        (const)
    IHDR.data[11] = 0;  // compression method  (const)
    IHDR.data[12] = 0;  // filtration          (const)

    write_chunk(&IHDR, buffer);
    free(IHDR.data);
    // IHDR - main info about picture ------------------------------------



    // IDAT - data -------------------------------------------------------
    struct chunk IDAT = {};
    strcpy(IDAT.type, "IDAT");

    IDAT.length = WIDTH * HEIGHT * 3 + HEIGHT; // (RGB)

    IDAT.data = (unsigned char *) calloc(IDAT.length + 1, sizeof(char));
    for (int i = 0; i < IDAT.length; i++) {
        IDAT.data[i] = rand() % 256;
    }

    for (int y = 0; y < HEIGHT; y++) {
        // Сдвигаем данные каждой строки на 1 байт вперед
        memmove(&IDAT.data[y * (WIDTH * 3 + 1)], &IDAT.data[y * WIDTH * 3], WIDTH * 3);  
        // Устанавливаем байт фильтра (тип 0 - None) в начало строки
        IDAT.data[y * (WIDTH * 3 + 1)] = 0; 
    }

   // Сжатие данных с помощью zlib
    uLongf compressed_size = compressBound(IDAT.length);
    unsigned char * compressed_data = (unsigned char *) calloc(compressed_size,1);

    if (compressed_data == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
    }

    int zlib_result = compress(compressed_data, &compressed_size, (const unsigned char *)IDAT.data,  IDAT.length);
    if (zlib_result != Z_OK) {
        fprintf(stderr, "Ошибка сжатия: %d\n", zlib_result);
    }

    free(IDAT.data);
    IDAT.data = compressed_data;

    write_chunk(&IDAT, buffer);

    free(compressed_data);
    // IDAT - data -------------------------------------------------------



    // IEND - end of pic -------------------------------------------------
    struct chunk IEND = {};
    strcpy(IEND.type, "IEND");
    IEND.length = 0;
    write_chunk(&IEND, buffer);
    // IEND - end of pic -------------------------------------------------
}


// int main() {
//     struct png_buffer buffer = {};
//     make_png(&buffer);

//     FILE * hui = fopen("txt.png", "wb");
//     fwrite(buffer.data, buffer.len, sizeof(char), hui);
//     return 0;
// }

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];
   
/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;
   
    /* Make the table for a fast CRC. */
void make_crc_table(void) {
    unsigned long c;
    int n, k;
   
    for (n = 0; n < 256; n++) {
        c = (unsigned long) n;
        for (k = 0; k < 8; k++) {
        if (c & 1)
           c = 0xedb88320L ^ (c >> 1);
        else
           c = c >> 1;
        }
        crc_table[n] = c;
    }
    crc_table_computed = 1;
}
   
/* Update a running CRC with the bytes buf[0..len-1]--the CRC
    should be initialized to all 1's, and the transmitted value
    s the 1's complement of the final running CRC (see the
    crc() routine below)). */
   
unsigned long update_crc(unsigned long crc, unsigned char *buf, int len) {
    unsigned long c = crc;
    int n;
   
    if (!crc_table_computed)
        make_crc_table();
    for (n = 0; n < len; n++) {
       c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c;
}
   
/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char *buf, int len) {
    return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}







    







// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------











// You need to use -I/path/to/AFLplusplus/include -I.
#include "AFLplusplus/include/afl-fuzz.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DATA_SIZE (10000)

typedef struct my_mutator {

  afl_state_t *afl;

  // any additional data here!
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
my_mutator_t *afl_custom_init(afl_state_t *afl, unsigned int seed) {

  srand(seed);  // needed also by surgical_havoc_mutate()

  my_mutator_t *data = calloc(1, sizeof(my_mutator_t));
  if (!data) {

    perror("afl_custom_init alloc");
    return NULL;

  }

  if ((data->mutated_out = (u8 *)malloc(MAX_FILE)) == NULL) {

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

/**
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

size_t afl_custom_fuzz(my_mutator_t *data, uint8_t *buf, size_t buf_size,
                       u8 **out_buf, uint8_t *add_buf,
                       size_t add_buf_size,  // add_buf can be NULL
                       size_t max_size) {

  // Make sure that the packet size does not exceed the maximum size expected by
  // the fuzzer
  size_t mutated_size = DATA_SIZE <= max_size ? DATA_SIZE : max_size;

  printf(" 1 ");

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


/**
 * Deinitialize everything
 *
 * @param data The data ptr from afl_custom_init
 */
void afl_custom_deinit(my_mutator_t *data) {

  free(data->post_process_buf);
  free(data->mutated_out);
  free(data->trim_buf);
  free(data->buffer.data);
  free(data);

}


