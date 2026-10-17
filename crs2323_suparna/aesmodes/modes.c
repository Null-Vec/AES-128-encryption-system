#include <stdio.h>
#include <unistd.h>  // For ftruncate on POSIX systems
#include "aes128.h"
#include "modes.h"

// Declare a global IV of size 16 bytes (128 bits)
unsigned char IV[16] = {0x01,0x02,0x01,0x02,0x01,0x02,0x01,0x02,0x01,0x02,0x01,0x02,
						0x01,0x02,0x01,0x02,};

void ecb_encrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey){
    u8 in_block[16],out_block[16];
    u8 bytes_read;

    // Read the input file in 16-byte chunks, encrypt, and write to output file
     while ((bytes_read = fread(in_block, 1, 16, ifile)) == 16) {
        aes_encrypt(ExpandedKey, in_block, out_block);
        fwrite(out_block, 1, 16, ofile);
    }

    // Apply PKCS#7 padding
    u8 padding_value = 16 - bytes_read;
    for (u8 i = bytes_read; i < 16; ++i)
        in_block[i] = padding_value;
    aes_encrypt(ExpandedKey, in_block, out_block);
    fwrite(out_block, 1, 16, ofile);
}

void ecb_decrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey){
    u8 in_block[16], out_block[16];
    u8 bytes_read;

    // Read the input file in 16-byte chunks, decrypt, and write to output file
    while (fread(in_block, 1, 16, ifile) > 0) {
        aes_decrypt(ExpandedKey, in_block, out_block);
        fwrite(out_block, 1, 16, ofile);
    }
    // Handle the last block with padding removal
    fseek(ofile, -out_block[15],SEEK_END);
    // Get the current position in the file
    long current_pos = ftell(ofile);
     // Truncate the file to the new size
    ftruncate(fileno(ofile), current_pos);
}

void cbc_encrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey){
    u8 plaintext[16], out_block[16];
    u8 bytes_read;
    // Write the IV to the outfile first
    fwrite(IV, 1, 16, ofile);

    // Read from the file by 16 bytes and operate on CBC mode
    while ((bytes_read = fread(plaintext, 1, 16, ifile)) == 16) {
        //XOR plaintext with the IV
        for (u8 i = 0; i < 16; ++i)
            out_block[i] = IV[i] ^ plaintext[i] ; 
        
        aes_encrypt(ExpandedKey, out_block, IV);
        // Write the encrypted plaintext to the output file
        fwrite(IV, 1, 16, ofile);
    }

    // PKSC#7 padding
    u8 padding_value = 16 - bytes_read;
    // Fill the remaining part with the padding value
    for (u8 i = bytes_read; i < 16; ++i)
        plaintext[i] = padding_value;
    //XOR plaintext with the IV
    for (u8 i = 0; i < 16; ++i)
        out_block[i] = IV[i] ^ plaintext[i] ; 
    aes_encrypt(ExpandedKey, out_block, IV);        
    fwrite(IV, 1, 16, ofile);
}

void cbc_decrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey){
    u8 input[16], output[16];
    u8 bytes_read;

    // Get the IV first
    fread(IV, 1, 16, ifile);
    
    // Read from the file by 16 bytes and operate on CBC decryption mode
    while (fread(input, 1, 16, ifile) > 0) {
        
        aes_decrypt(ExpandedKey, input, output);
        //XOR plaintext with the IV
        for (u8 i = 0; i < 16; ++i)
            output[i] ^= IV[i];// Get the original plaintext
        
        for (u8 i = 0; i < 16; i++)
            IV[i] = input[i];

        fwrite(output, 1, 16, ofile);
    }

    //Get the last plaintext of the input and validate PKCS padding
    fseek(ofile, -input[15],SEEK_END);
    // Get the current position in the file
    long current_pos = ftell(ofile);
     // Truncate the file to the new size
    ftruncate(fileno(ofile), current_pos);
}

void cfb_encrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey){
    u8 in_block[16], out_block[16];
    u8 bytes_read;

    // Write the IV to the outfile first
    fwrite(IV, 1, 16, ofile);

    // Read from the file by 16 bytes and operate on CFB mode
    while ((bytes_read = fread(in_block, 1, 16, ifile)) > 0) {
        aes_encrypt(ExpandedKey, IV, out_block);
        //XOR plaintext with the IV
        for (u8 i = 0; i < 16; i++)
            IV[i] ^= out_block[i];
        
        // Write the encrypted plaintext to the output file
        fwrite(IV, 1, bytes_read, ofile);
    }
}

void cfb_decrypt(FILE* ifile, FILE *ofile, u8* ExpandedKey){
    u8 in_block[16], out_block[16];
    u8 bytes_read;

    // Write the IV to the outfile first
    fread(IV, 1, 16, ifile);

    // Read from the file by 16 bytes and operate on CFB mode
    while ((bytes_read = fread(in_block, 1, 16, ifile)) > 0) {
        aes_encrypt(ExpandedKey, IV, out_block);
        for (u8 i = 0; i < 16; i++)
            IV[i] = in_block[i] ^ out_block[i];
        fwrite(IV, 1, bytes_read, ofile);// Write the decrypted plaintext to the output file

        //Get the ciphertext as IV for next block
        for (u8 i = 0; i < 16; i++)
            IV[i] = in_block[i];
    }
}

void ofb(FILE* ifile, FILE *ofile, u8* ExpandedKey){
    u8 in_block[16], out_block[16];
    u8 bytes_read;
    
    // Read from the file by 16 bytes and operate on OFB mode
    while ((bytes_read = fread(in_block, 1, 16, ifile)) > 0) {
        aes_encrypt(ExpandedKey, IV, out_block);
        for (u8 i = 0; i < 16; i++)
            in_block[i] ^= out_block[i]; //XOR input with the output from encryption block
        for (u8 i = 0; i < 16; i++)
            IV[i] = out_block[i];
        // Write the encrypted plaintext to the output file
        fwrite(in_block, 1, bytes_read, ofile);
    }
}

void ctr(FILE *ifile, FILE *ofile, u8* ExpandedKey){

    //Set the least significant 32 bits to 0
    IV[0]=IV[1]=IV[2]=IV[3]=0x00;

    u8 output[16], in_block[16];
    u8 bytes_read;

    // Read the input file in 16-byte chunks, encrypt, and write to output file
    while ((bytes_read = fread(in_block, 1, 16, ifile)) > 0) {
        
        aes_encrypt(ExpandedKey, IV, output);
        for (u8 i = 0; i < 16; i++)
            output[i] ^= in_block[i];
        fwrite(in_block, 1, bytes_read, ofile);
        // Increament least sinificant 32 bits
        for (u8 i = 0; i < 4; ++i)
            if (++IV[i] != 0) break;
    }
}
