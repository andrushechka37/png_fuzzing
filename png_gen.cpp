#include <cstdio>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_size_t.h>

unsigned long crc(unsigned char *buf, int len);

const int IHDR_LENGTH = 13;
const int START_LEN = 100;
const int LEN_OF_BLOCK = 4;

struct chunk {
    unsigned long length;
    char type[4];
    char * data; 
};

struct png_buffer {
     unsigned char * data;
    size_t len;
};

void print_number(unsigned long number, png_buffer * buffer) {

    unsigned char array[4];
    array[0] = (number >> 24) & 0xFF;
    array[1] = (number >> 16) & 0xFF;
    array[2] = (number >> 8) & 0xFF;
    array[3] = number & 0xFF;

    for (int i = 0; i < 4; i++) {
        buffer->data[buffer->len] = array[i];
        buffer->len++;
    }


}

void write_chunk(chunk * chunk, png_buffer * buffer) {

    buffer->data = (unsigned char *) realloc(buffer->data, buffer->len + chunk->length + 15);
    print_number(chunk->length, buffer);              // len
    memcpy(&(buffer->data[buffer->len]), chunk->type, 4);   //
    buffer->len += 4;                                       // type

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

void make_png(png_buffer * buffer) {

    buffer->data = (unsigned char *) calloc(8 + 30, sizeof(char));

    unsigned long WIDTH =  rand() % 512;
    unsigned long HEIGHT = rand() % 512;

    // signature ---------------------------------------------------------
    unsigned char signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    memcpy(buffer->data, signature, 8);
    buffer->len = 8;
    // signature ---------------------------------------------------------


    // IHDR - main info about picture ------------------------------------
    chunk IHDR = {};
    IHDR.length = IHDR_LENGTH;     
    strcpy(IHDR.type, "IHDR");  

    IHDR.data = (char *)calloc(IHDR_LENGTH, sizeof(char));

    IHDR.data[3] = (char) WIDTH;            // data
    IHDR.data[2] = (char) (WIDTH >> 8);
    IHDR.data[1] = (char) (WIDTH >> 16);
    IHDR.data[0] = (char) (WIDTH >> 24);

    IHDR.data[7] = (char) HEIGHT;
    IHDR.data[6] = (char) (HEIGHT >> 8);
    IHDR.data[5] = (char) (HEIGHT >> 16);
    IHDR.data[4] = (char) (HEIGHT >> 24);

    IHDR.data[8] = 1;   // bit depth
    IHDR.data[9] = 0;   // colour type (RGB)

    IHDR.data[10] = 0;  // weave method        (const)
    IHDR.data[11] = 0;  // compression method  (const)
    IHDR.data[12] = 0;  // filtration          (const)

    write_chunk(&IHDR, buffer);
    free(IHDR.data);
    // IHDR - main info about picture ------------------------------------



    // IDAT - data -------------------------------------------------------
    chunk IDAT = {};
    strcpy(IDAT.type, "IDAT");

    IDAT.length = WIDTH * HEIGHT * 3; // (RGB)

    IDAT.data = (char *)calloc(IDAT.length + 1, sizeof(char));
    for (int i = 0; i < IDAT.length; i++) {
        IDAT.data[i] = rand() % 256;
    }

    write_chunk(&IDAT, buffer);

    free(IDAT.data);
    // IDAT - data -------------------------------------------------------



    // IEND - end of pic -------------------------------------------------
    chunk IEND = {};
    strcpy(IEND.type, "IEND");
    IEND.length = 0;
    write_chunk(&IEND, buffer);
    // IEND - end of pic -------------------------------------------------
}


int main() {
    png_buffer buffer = {};
    make_png(&buffer);

    FILE * hui = fopen("txt.png", "wb");
    fwrite(buffer.data, buffer.len, sizeof(char), hui);
    return 0;
}

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


