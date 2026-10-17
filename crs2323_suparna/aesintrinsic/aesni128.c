#include <wmmintrin.h> // For AES-NI intrinsics
#include <stdio.h>
#include <immintrin.h>
#include <string.h>
#include <stdint.h>


unsigned char userkey[16] = {
        0x2b, 0x7e, 0x15, 0x16, 
        0x28, 0xae, 0xd2, 0xa6, 
        0xab, 0xf7, 0xcf, 0x9f, 
        0xfb, 0x9b, 0x6d, 0xd5
    }; // AES-128 key
    
    unsigned char key[176]; // Expanded key array for 11 round keys (16 bytes each)
    __m128i Key_Schedule[11];
    unsigned char ciphertext[16];
    unsigned char decryptedtext[16];

    unsigned char plaintext[16] = {
        0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
        0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
        };


// AES_128_ASSIST function definition
__m128i AES_128_ASSIST(__m128i temp1, __m128i temp2) {
    __m128i temp3;
    temp2 = _mm_shuffle_epi32(temp2, 0xff);
    temp3 = _mm_slli_si128(temp1, 0x4);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp3 = _mm_slli_si128(temp3, 0x4);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp3 = _mm_slli_si128(temp3, 0x4);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp1 = _mm_xor_si128(temp1, temp2);
    return temp1;
}

// AES_128_Key_Expansion function definition
void AES_128_Key_Expansion(const unsigned char *userkey, unsigned char *key) 
{
    __m128i temp1, temp2;
    __m128i *Key_Schedule = (__m128i*)key;
    
    // Load the first 16 bytes of user key
    temp1 = _mm_loadu_si128((__m128i*)userkey);
    Key_Schedule[0] = temp1;

    // First round key expansion
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x1);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[1] = temp1;

    // Second round key expansion
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x2);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[2] = temp1;

    // Third round key expansion
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x4);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[3] = temp1;

    // Fourth round key expansion
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x8);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[4] = temp1;

    // Fifth round key expansion
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x10);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[5] = temp1;

    // Sixth round key expansion
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x20);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[6] = temp1;

    // Seventh round key expansion
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x40);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[7] = temp1;

    // Eighth round key expansion
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x80);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[8] = temp1;

    // Ninth round key expansion
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x1b);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[9] = temp1;

    // Tenth and final round key expansion
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x36);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[10] = temp1;
}

/*unsigned char *Key: This is a pointer to the expanded AES key schedule,
 which consists of 11 round keys (each 16 bytes) for AES-128.
 
 unsigned char *state: This is a pointer to the 16-byte plaintext block
  that will be encrypted (commonly called the "state" in AES).
 
 unsigned char *output: This is a pointer to the 16-byte array
  where the encrypted output (ciphertext) will be stored.
 */


void aes_encrypt(unsigned char *Key, unsigned char *plaintext, unsigned char *ciphertext)
{
    int round, i;
    __m128i input = _mm_loadu_si128((__m128i *)plaintext); //loads the 16-byte plaintext block to input
    input = _mm_xor_si128(input, ((__m128i *)Key)[0]); //The first round of AES encryption is the "AddRoundKey" step, where the plaintext block is XORed with the first round key.
    for (round = 1; round < 10; round++) //middle 9 rounds
    {
        input = _mm_aesenc_si128(input, ((__m128i *)Key)[round]);
    }
    input = _mm_aesenclast_si128(input, ((__m128i *)Key)[10]); //last round(mix col is not happening here thats y use _aesenclast_)

    _mm_storeu_si128(((__m128i *)ciphertext), input); //storing the encrypted text to ciphertext
    
}


void aes_decrypt(unsigned char *key, unsigned char *ciphertext, unsigned char *decryptedtext)
{
    int round, i;

    __m128i input = _mm_loadu_si128((__m128i *)ciphertext);
    input = _mm_xor_si128(input, ((__m128i *)key)[10]);

    for (round = 9; round > 0; round--)
    {
        input = _mm_aesdec_si128(input, _mm_aesimc_si128(((__m128i *)key)[round]));
    }
    input = _mm_aesdeclast_si128(input, ((__m128i *)key)[0]);

    _mm_storeu_si128(((__m128i *)decryptedtext), input);
    

}




int main() {
    
     AES_128_Key_Expansion(userkey, key);

    // Print the expanded key
    printf("\nExpanded round key:\n");
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 16; j++) {
            printf("%2.2x%c", key[16 * i + j], (j != 15) ? ' ' : '\n');
        }
    }


//Encryption
    printf("\nPlaintext : ");
    for(int i = 0;i < 16;i++)
        printf("%2.2x",plaintext[i]);
    //aes_encrypt(key, plaintext,output);
    aes_encrypt(key, plaintext,ciphertext);
    printf("\nciphertext : ");
    for(uint8_t i =0 ;i< 16; i++){
        //printf("%2.2x",output[i]);
        printf("%2.2x",ciphertext[i]);
    }


//Decryption
    //aes_decrypt(key,output,decryptedtext);
    aes_decrypt(key,ciphertext,decryptedtext);
    printf("\nDecrypted text : ");
    for(int i =0 ;i< 16; i++){
        printf("%2.2x",decryptedtext[i]);
    }
    printf("\n\n"); 
    

    return 0;
}
