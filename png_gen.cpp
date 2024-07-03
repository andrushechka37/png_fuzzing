#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long crc(unsigned char *buf, int len);

// free
// code clean 

const unsigned long WIDTH = 200;
const unsigned long HEIGHT = 200;
const int IHDR_LENGTH = 13;
const int START_LEN = 100;
const int LEN_OF_BLOCK = 4;

struct chunk {
    unsigned long length;
    char type[4];
    char * data; 
};

void print_number(unsigned long number, FILE * file) {

    unsigned char array[4];
    array[0] = (number >> 24) & 0xFF;
    array[1] = (number >> 16) & 0xFF;
    array[2] = (number >> 8) & 0xFF;
    array[3] = number & 0xFF;

    for (int i = 0; i < 4; i++) {
        fprintf(file, "%c", array[i]);
    }
}

void write_chunk(FILE * pfile, chunk * chunk) {

    print_number(chunk->length, pfile);              // len
    fwrite(chunk->type, sizeof(char), 4, pfile);     // type

    char * hash_str = NULL;                                                         //
    if (chunk->data == NULL) {                                                      //
        hash_str = (char *)calloc(START_LEN, sizeof(char));                         //
    } else {                                                                        // data
        fwrite(chunk->data, sizeof(char), chunk->length, pfile);                    //
        hash_str = (char *)calloc(chunk->length + START_LEN, sizeof(char));         //
    }                                                                               //

    memcpy(hash_str, chunk->type, LEN_OF_BLOCK);                                        //
    memcpy(&(hash_str[4]), chunk->data, chunk->length);                                 // hash
    unsigned long hash = crc((unsigned char *)hash_str, LEN_OF_BLOCK + chunk->length);  //
    print_number(hash, pfile);                                                          //

    free(hash_str);
}

void make_png(FILE * file) {

    // signature ---------------------------------------------------------
    unsigned char signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    fwrite(signature, sizeof(char), 8, file);
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

    write_chunk(file, &IHDR);
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

    write_chunk(file, &IDAT);

    free(IDAT.data);
    // IDAT - data -------------------------------------------------------



    // IEND - end of pic -------------------------------------------------
    chunk IEND = {};
    strcpy(IEND.type, "IEND");
    IEND.length = 0;
    write_chunk(file, &IEND);
    // IEND - end of pic -------------------------------------------------

    fclose(file);
}


int main() {
    FILE * pfile = fopen("test_pic.png", "wb");
    make_png(pfile);
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
        is the 1's complement of the final running CRC (see the
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

