#ifndef AES128_H
#define AES128_H

#define NK 4
#define Nr 10
#define Nb 4
// Function declaration
// Function prototypes
void key_expansion(unsigned char *key, unsigned char *expandedkey);
void aes_encrypt(unsigned char *expandedkey, unsigned char *plaintext, unsigned char *ciphertext);
void aes_decrypt(unsigned char *expandedkey, unsigned char *ciphertext, unsigned char *decryptedtext);

#endif // AES128_H

