#ifndef MODEOFOPERATIONS_H
#define MODEOFOPERATIONS_H	
#include <stdio.h>
typedef unsigned char u8;
void ecb_encrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey);
void ecb_decrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey);
void cbc_encrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey);
void cbc_decrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey);
void cfb_encrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey);
void cfb_decrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey);
void ofb(FILE* ifile, FILE *ofile, u8* ExpandedKey);
void ctr(FILE *ifile, FILE *ofile, u8* ExpandedKey);

#endif
