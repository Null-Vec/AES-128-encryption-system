#ifndef AES128_H
#define AES128_H

#include <stdint.h>
// Function declaration
// Function prototypes
void key_expansion(unsigned char *key, unsigned char *expandedkey);
void subword(unsigned char *input);
void rotate_word(unsigned char *input);
void aes_encrypt(unsigned char *expandedkey, unsigned char *plaintext, unsigned char *ciphertext);
void aes_encrypt_rounds(unsigned char *state, unsigned char *ciphertext,unsigned char *expandedkey);
void generate_roundkey(unsigned char* roundkey, unsigned char *expandedkey);
void Add_roundkey(unsigned char *state, unsigned char *roundkey);
void subbytes(unsigned char *state);
void shift_row(unsigned char  *state);
void left_shift(unsigned char *row, uint8_t nbr);
void mixcolumn(unsigned char *state);
void ret_column(unsigned char *column);
unsigned char galois_mult(unsigned char a, unsigned char b);
void aes_decrypt(unsigned char *expandedkey, unsigned char *ciphertext, unsigned char *decryptedtext);
void aes_decrypt_rounds(unsigned char *state, unsigned char *DecryptedText, unsigned char *expandedkey);
void invsubbytes(unsigned char *state);
void invshift_row(unsigned char  *state);
void right_shift(unsigned char *row, uint8_t nbr);
void invmixcolumn(unsigned char *state);
void invret_column(unsigned char *column);


#endif // AES128_H

