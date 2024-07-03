// free
// code clean 




#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


unsigned long crc(unsigned char *buf, int len);
#define IS_NULL_PTR(ptr)    \
    if (prt == NULL) {      \
        printf("null prt"); \
        return 0;           \
    }


struct chunk{
   unsigned long length;
   char type[4];
   char * data; 
};

const unsigned long width = 1;
const unsigned long height = 1;
const int IHDR_length = 13;

void write_chunk(FILE *fp, chunk *chunk) {

    unsigned char array[4];
    array[0] = (chunk->length >> 24) & 0xFF;
    array[1] = (chunk->length >> 16) & 0xFF;
    array[2] = (chunk->length >> 8) & 0xFF;
    array[3] = chunk->length & 0xFF;

    for (int i = 0; i < 4; i++) {
        fprintf(fp, "%c", array[i]);
    }

    fwrite(chunk->type, sizeof(char), 4, fp);
    char * hash_str = NULL;
    if (chunk->data == NULL) {
        hash_str = (char *)calloc(100, sizeof(char));
    } else {
        fwrite(chunk->data, sizeof(char), chunk->length, fp);
        hash_str = (char *)calloc(strlen(chunk->data) + 100, sizeof(char));
    }


    memcpy(hash_str, chunk->type, 4);

    memcpy(&(hash_str[4]), chunk->data, chunk->length); // !!!!!!!1

    unsigned long hash = crc((unsigned char *)hash_str, 4 + chunk->length);
    array[0] = (hash >> 24) & 0xFF;
    array[1] = (hash >> 16) & 0xFF;
    array[2] = (hash >> 8) & 0xFF;
    array[3] = hash & 0xFF;

    for (int i = 0; i < 4; i++) {
        fprintf(fp, "%c", array[i]);
    }
}

void make_png(FILE * file) {



    // signature ---------------------------------------------------------
    unsigned char signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    fwrite(signature, sizeof(char), 8, file);
    // signature ---------------------------------------------------------



    // IHDR - main info about picture ------------------------------------
    chunk IHDR = {};
    IHDR.length = IHDR_length;              // length
    strcpy(IHDR.type, "IHDR");     // type

    IHDR.data = (char *)calloc(IHDR_length, sizeof(char));

    IHDR.data[3] = (char) width;            // data
    IHDR.data[2] = (char) (width >> 8);
    IHDR.data[1] = (char) (width >> 16);
    IHDR.data[0] = (char) (width >> 24);

    IHDR.data[7] = (char) height;
    IHDR.data[6] = (char) (height >> 8);
    IHDR.data[5] = (char) (height >> 16);
    IHDR.data[4] = (char) (height >> 24);

    IHDR.data[8] = 1;   // bit depth
    IHDR.data[9] = 0;   // colour type (RGB)
    IHDR.data[11] = 0;  // compression method 
    IHDR.data[12] = 0;  // filtration
    IHDR.data[10] = 0;  // weave method

    write_chunk(file, &IHDR);
    free(IHDR.data);
    // IHDR - main info about picture ------------------------------------



    // IDAT - data -------------------------------------------------------
    chunk IDAT = {};
    strcpy(IDAT.type, "IDAT");

    IDAT.length = width * height * 3; // 3 байта на пиксель (RGB)
    char idata_chunk[] = {0x00, 0x00, 0x00, 0x00, // Длина чанка
                        0x49, 0x44, 0x41, 0x54, // Тип чанка
                        0x08, 0x00, // Флаг компрессии и фильтра
                        0x00, 0x00, 0x00, 0x00, // CRC
                        0x00, 0x00, 0x00, 0x00}; // CRC

    char *data = (char *)calloc(IDAT.length + 10, sizeof(char));
    for (int i = 0; i < IDAT.length; i++) {
        data[i] = rand() % 256;
    }
    IDAT.data = data;
    write_chunk(file, &IDAT);
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
   void make_crc_table(void)
   {
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
   
   unsigned long update_crc(unsigned long crc, unsigned char *buf,
                            int len)
   {
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
   unsigned long crc(unsigned char *buf, int len)
   {
     return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
   }


