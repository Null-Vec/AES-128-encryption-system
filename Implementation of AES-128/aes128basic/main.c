#include "aes128.h"
#include <stdio.h>
#include <stdint.h>

#define NK 4
#define Nr 10
#define Nb 4


    // Call the function defined in aes128.c
int main() {

    unsigned char plaintext[16] = {
        0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
        0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34

    };

    // unsigned char plaintext[16];
    // printf("Give Input 16 char:");
    // for(int i=0;i<16;i++){
    //     scanf("%2c",&plaintext[i]);
    // }

    unsigned char key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    unsigned char expandedkey[176];
    unsigned char ciphertext[16];
    unsigned char decryptedtext[16];
    uint8_t i,j;

    // Key expansion
    key_expansion(key, expandedkey);
    // Print the expanded key
    printf("\nExpanded round key:\n");
    for (int i = 0; i < Nr + 1; i++) {
        for (int j = 0; j < 16; j++) {
            printf("%2.2x%c", expandedkey[16 * i + j], (j != 15) ? ' ' : '\n');
        }
    }

    //Encryption
    printf("\nPlaintext : ");
    for(i = 0;i < 16;i++)
        printf("%2.2x",plaintext[i]);
    aes_encrypt(expandedkey, plaintext, ciphertext);
    printf("\nciphertext : ");
    for(uint8_t i =0 ;i< 16; i++){
        printf("%2.2x",ciphertext[i]);
    }

   //Decryption
    aes_decrypt(expandedkey, ciphertext, decryptedtext);
    printf("\nDecrypted text : ");
    for(uint8_t i =0 ;i< 16; i++){
        printf("%2.2x",decryptedtext[i]);
    }
    printf("\n\n"); 
    return 0;
}
