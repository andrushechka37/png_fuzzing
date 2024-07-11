#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_size_t.h> // TODO: size_t type is located in stdint.h
#include "zlib.h"
#include "png_gen_lib.h"

static unsigned long crc(unsigned char *buf, int len); 

static const int IHDR_LENGTH = 13; 
static const int START_LEN = 100;
static const int LEN_OF_BLOCK = 4;
static const int HUGE_LEN = 300000;

#define GEN_BAD_PNG // TODO define things like that only with -D compiler option

#ifdef GEN_BAD_PNG
#define FLAG_GEN(flag)          \
    bool flag = rand() % 2;

#else 
    #define FLAG_GEN(flag)       \
        bool flag = 1;
#endif

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

    print_number(chunk->length, buffer);             
    
    memcpy(&(buffer->data[buffer->len]), chunk->type, 4);   
    buffer->len += 4;                                      

    char hash_str[HUGE_LEN] = "";                                                         
    if (chunk->data != NULL) {                                                                                                                        
        memcpy(&(buffer->data[buffer->len]), chunk->data, chunk->length);
        buffer->len += chunk->length;       
    }                                                                               

    memcpy(hash_str, chunk->type, LEN_OF_BLOCK);                                         
    memcpy(&(hash_str[4]), chunk->data, chunk->length);                                  
    unsigned long hash = crc((unsigned char *) hash_str, LEN_OF_BLOCK + chunk->length);  
    print_number(hash, buffer);                                                          
}

void make_png(struct png_buffer * buffer) {

    unsigned char data_for_buffer[HUGE_LEN] = "";
    buffer->data = data_for_buffer;

    unsigned long WIDTH =  rand() % 256;
    unsigned long HEIGHT = rand() % 256;

    // signature ---------------------------------------------------------
    FLAG_GEN(signature_flag);
                                                                                   
    unsigned char right_signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    unsigned char signature[sizeof(right_signature)] = {};

    if (signature_flag) {
        memcpy(signature, right_signature, 8);
    } else {
        for (int i = 0; i < 8; i++) {
            signature[i] = rand() % 0xAA;
        }
    }

    memcpy(buffer->data, signature, 8);
    buffer->len = 8;
    // signature ---------------------------------------------------------

    // IHDR - main info about picture ------------------------------------
    struct chunk IHDR = {};

    FLAG_GEN(IHDR_flag);

    if (IHDR_flag) {
        strcpy(IHDR.type, "IHDR"); 
    } else {
        strcpy(IHDR.type, "SMTH");
    }

    IHDR.length = IHDR_LENGTH;     

    unsigned char data_for_IHDR[IHDR_LENGTH] = "";
    IHDR.data = data_for_IHDR;

    IHDR.data[3] = (char)  WIDTH;           
    IHDR.data[2] = (char) (WIDTH >> 8);
    IHDR.data[1] = (char) (WIDTH >> 16);
    IHDR.data[0] = (char) (WIDTH >> 24);

    IHDR.data[7] = (char) HEIGHT;
    IHDR.data[6] = (char) (HEIGHT >> 8);
    IHDR.data[5] = (char) (HEIGHT >> 16);
    IHDR.data[4] = (char) (HEIGHT >> 24);

    if (IHDR_flag) {
        IHDR.data[8]  = 8;  // bit depth    // TODO nice comments! Make other ones like those
        IHDR.data[9]  = 2;  // colour type (RGB)

        IHDR.data[10] = 0;  // weave method        (const)
        IHDR.data[11] = 0;  // compression method  (const)
        IHDR.data[12] = 0;  // filtration          (const)
    } else {
        IHDR.data[8]  = rand() % 20;
        IHDR.data[9]  = rand() % 10;

        IHDR.data[10] = rand() % 10;
        IHDR.data[11] = rand() % 10;
        IHDR.data[12] = rand() % 10;
    }

    write_chunk(&IHDR, buffer);
    // IHDR - main info about picture ------------------------------------

    // IDAT - data -------------------------------------------------------
    struct chunk IDAT = {};
        
    FLAG_GEN(IDAT_flag);

    if (IDAT_flag) {
        strcpy(IDAT.type, "IDAT");
    } else {
        strcpy(IDAT.type, "SMTH");
    }

    IDAT.length = WIDTH * HEIGHT * 3 + HEIGHT; // 3 bytes on each pixel(RGB) + bytes of filter for each string

    unsigned char data_for_IDAT[HUGE_LEN] = "";
    IDAT.data = data_for_IDAT;
    for (int i = 0; i < IDAT.length; i++) {
        IDAT.data[i] = rand() % 256;
    }

    for (int y = 0; y < HEIGHT; y++) {
        memmove(&IDAT.data[y * (WIDTH * 3 + 1)], &IDAT.data[y * WIDTH * 3], WIDTH * 3);  

        if (IDAT_flag) {
            IDAT.data[y * (WIDTH * 3 + 1)] = 0; 
        } else {
            IDAT.data[y * (WIDTH * 3 + 1)] = rand() % 10; 
        }
    }

    if (IDAT_flag) {
        // zlib
        uLongf compressed_size = compressBound(IDAT.length);
        unsigned char compressed_data[HUGE_LEN] = "";

        if (compressed_data == NULL) {
            fprintf(stderr, "compressed_data ptr is NULL\n");
        }

        int zlib_result = compress(compressed_data, &compressed_size, (const unsigned char *)IDAT.data,  IDAT.length);
        if (zlib_result != Z_OK) {
            fprintf(stderr, "Error of compression %d\n", zlib_result);
        }

        IDAT.data = compressed_data;
        write_chunk(&IDAT, buffer);

    } else {
        write_chunk(&IDAT, buffer);
    }
    // IDAT - data -------------------------------------------------------

    // IEND - end of pic -------------------------------------------------
    struct chunk IEND = {};

    FLAG_GEN(IEND_flag);

    if (IEND_flag) {
        strcpy(IEND.type, "IEND");
    } else {
        strcpy(IEND.type, "SMTH");
    }

    IEND.length = 0;

    write_chunk(&IEND, buffer);
    // IEND - end of pic -------------------------------------------------
}



unsigned long crc_table[256]; 
   
int crc_table_computed = 0;
   
void make_crc_table(void) {
    unsigned long hash;
    int i, j;
   
    for (i = 0; i < 256; i++) {
        hash = (unsigned long) i;

        for (j = 0; j < 8; j++) {
        if (hash & 1)
           hash = 0xedb88320L ^ (hash >> 1);
        else
           hash = hash >> 1;
        }
        crc_table[i] = hash;
    }
    crc_table_computed = 1;
}
   
static unsigned long update_crc(unsigned long crc, unsigned char *buf, int len) {
    unsigned long hash = crc;
    int i;
   
    if (!crc_table_computed)
        make_crc_table();
    for (i = 0; i < len; i++) {
       hash = crc_table[(hash ^ buf[i]) & 0xff] ^ (hash >> 8);
    }
    return hash;
}
   
unsigned long crc(unsigned char *buf, int len) {
    
    FLAG_GEN(CRC_flag);

    if (CRC_flag) {
        return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
    } else {
        return rand() % 0xffffffff;
    }
}