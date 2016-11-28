#pragma once

typedef struct
{
    unsigned int count[2];
    unsigned int state[4];
    unsigned char buffer[64];
}MD5_CTX;
void MD5Transform(unsigned int state[4], const unsigned char block[64]);

void MD5Decode(unsigned int *output, const unsigned char *input,unsigned int len);

void MD5Encode(unsigned char *output, const unsigned int *input,unsigned int len);

void MD5Final(MD5_CTX *context, unsigned char digest[16]);

void MD5Update(MD5_CTX *context, const unsigned char *input, unsigned int inputlen);

void MD5Init(MD5_CTX *context);

void MD5Transform(unsigned int state[4], const unsigned char block[64]);
