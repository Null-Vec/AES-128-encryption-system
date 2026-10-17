#include "aes128.h"
#include "modes.h"
#include <stdio.h>
#include <stdint.h>

#define NK 4
#define Nr 10
#define Nb 4


    // Call the function defined in aes128.c
int main() {
    unsigned char key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    
    unsigned char ExpandedKey[176], ciphertext[16], decryptedtext[16];
    uint8_t i,j;
    // Key expansion
    key_expansion(key, ExpandedKey);
    // Print the expanded key
    printf("\nExpanded round key:\n");
    for (int i = 0; i < Nr + 1; i++) {
        for (int j = 0; j < 16; j++)
            printf("%2.2x%c", ExpandedKey[16 * i + j], (j != 15) ? ' ' : '\n');
    }

	//Open the respective files
    FILE *ifile = fopen("enc.pdf", "rb");
    FILE *ofile = fopen("dec.pdf", "wb");

//ecb_encrypt(ifile, ofile,  ExpandedKey);
//ecb_decrypt(ifile, ofile,  ExpandedKey);
//cbc_encrypt(ifile, ofile,  ExpandedKey);
//cbc_decrypt(ifile, ofile,  ExpandedKey);
//cfb_encrypt(ifile, ofile,  ExpandedKey);
//cfb_decrypt(ifile, ofile,  ExpandedKey);
//ofb(ifile, ofile,  ExpandedKey);
//ctr(ifile, ofile,  ExpandedKey); 
    fclose(ifile);
    fclose(ofile);
 
    return 0;
}
