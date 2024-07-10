#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_size_t.h> # TODO: size_t type is located in stdint.h
#include "zlib.h"
#include "png_gen_lib.h"

unsigned long crc(unsigned char *buf, int len); // TODO either make it static or move to header file

const int IHDR_LENGTH = 13; // TODO either make it static or move to header file
const int START_LEN = 100;
const int LEN_OF_BLOCK = 4;
const int HUGE_LEN = 300000;

#define GEN_BAD_PNG // TODO define things like that only with -D compiler option

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
    if (chunk->data != NULL) {                                                                                                                           // data // TODO remove this comment or make it meaningful
        memcpy(&(buffer->data[buffer->len]), chunk->data, chunk->length);
        buffer->len += chunk->length;       
    }                                                                               

    memcpy(hash_str, chunk->type, LEN_OF_BLOCK);                                         //
    memcpy(&(hash_str[4]), chunk->data, chunk->length);                                  // hash // TODO same as prev TODO
    unsigned long hash = crc((unsigned char *) hash_str, LEN_OF_BLOCK + chunk->length);  //
    print_number(hash, buffer);                                                          //
}

void make_png(struct png_buffer * buffer) {

    unsigned char data[HUGE_LEN] = "";
    buffer->data = data;

    unsigned long WIDTH =  rand() % 256;
    unsigned long HEIGHT = rand() % 256;

    // signature ---------------------------------------------------------
    #ifdef GEN_BAD_PNG      // TODO: maybe put in define, too many copies
        bool signature_flag = rand() % 2;
    #endif
    #ifndef GEN_BAD_PNG // TODO use `#else`
        bool signature_flag = 1;
    #endif

    unsigned char signature[8] = {}; // TODO 8 is a magical num. I'd better do it like 
    //                                          right_signature[] = {....}
    //                                          signature[sizeof(right_signature)]
    //                                                                                      
    unsigned char right_signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

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

    #ifdef GEN_BAD_PNG
        bool IHDR_flag = rand() % 2;
    #endif
    #ifndef GEN_BAD_PNG
        bool IHDR_flag = 1;
    #endif


    if (IHDR_flag) {
        strcpy(IHDR.type, "IHDR"); 
    } else {
        strcpy(IHDR.type, "SMTH");
    }

    IHDR.length = IHDR_LENGTH;     

    unsigned char data1[IHDR_LENGTH] = ""; // TODO poor naming.
    IHDR.data = data1;

    IHDR.data[3] = (char)  WIDTH;            // data
    IHDR.data[2] = (char) (WIDTH >> 8);
    IHDR.data[1] = (char) (WIDTH >> 16);
    IHDR.data[0] = (char) (WIDTH >> 24);

    IHDR.data[7] = (char) HEIGHT;
    IHDR.data[6] = (char) (HEIGHT >> 8);
    IHDR.data[5] = (char) (HEIGHT >> 16);
    IHDR.data[4] = (char) (HEIGHT >> 24);

    if (IHDR_flag) {
        IHDR.data[8] = 8;   // bit depth // 1 // TODO nice comments! Make other ones like those
        IHDR.data[9] = 2;   // colour type (RGB) // 0

        IHDR.data[10] = 0;  // weave method        (const)
        IHDR.data[11] = 0;  // compression method  (const)
        IHDR.data[12] = 0;  // filtration          (const)
    } else {
        IHDR.data[8] = rand() % 20;
        IHDR.data[9] = rand() % 10;

        IHDR.data[10] = rand() % 10;
        IHDR.data[11] = rand() % 10;
        IHDR.data[12] = rand() % 10;
    }

    write_chunk(&IHDR, buffer);
    // IHDR - main info about picture ------------------------------------
    // TODO too many spaces. Please, run your code through formatter or keep your eyes on codestyle


    // IDAT - data -------------------------------------------------------
    struct chunk IDAT = {};
        
    #ifdef GEN_BAD_PNG
        bool IDAT_flag = rand() % 2;
    #endif
    #ifndef GEN_BAD_PNG
        bool IDAT_flag = 1;
    #endif

    if (IDAT_flag) {
        strcpy(IDAT.type, "IDAT");
    } else {
        strcpy(IDAT.type, "SMTH");
    }


    IDAT.length = WIDTH * HEIGHT * 3 + HEIGHT; // (RGB) // TODO please, more comments to such forlmulas

    unsigned char data2[HUGE_LEN] = "";
    IDAT.data = data2;
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
            fprintf(stderr, "Ошибка выделения памяти\n"); // TODO please, use english in your error messages. Not all of terminals support russian
        }

        int zlib_result = compress(compressed_data, &compressed_size, (const unsigned char *)IDAT.data,  IDAT.length);
        if (zlib_result != Z_OK) {
            fprintf(stderr, "Ошибка сжатия: %d\n", zlib_result);
        }

        IDAT.data = compressed_data;
        write_chunk(&IDAT, buffer);

    } else {
        write_chunk(&IDAT, buffer);
    }
    // IDAT - data -------------------------------------------------------



    // IEND - end of pic -------------------------------------------------
    struct chunk IEND = {};
    #ifdef GEN_BAD_PNG
        bool IEND_flag = rand() % 2;
    #endif
    #ifndef GEN_BAD_PNG
        bool IEND_flag = 1;
    #endif

    if (IEND_flag) {
        strcpy(IEND.type, "IEND");
    } else {
        strcpy(IEND.type, "SMTH");
    }

    IEND.length = 0;

    write_chunk(&IEND, buffer);
    // IEND - end of pic -------------------------------------------------
}


/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256]; // TODO avoid global variables. At least, make them static or better use structures.
   
/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;
   
    /* Make the table for a fast CRC. */ // TODO formatting
void make_crc_table(void) {
    unsigned long c; // TODO poor naming
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
    
    #ifdef GEN_BAD_PNG
        bool CRC_flag = rand() % 2;
    #endif
    #ifndef GEN_BAD_PNG
        bool CRC_flag = 1;
    #endif

    if (CRC_flag) {
        return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
    } else {
        return rand() % 0xffffffff;
    }
}
