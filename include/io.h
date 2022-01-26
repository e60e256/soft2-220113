#pragma once

#ifndef Cipherdefined
#define Cipherdefined

typedef struct leafcipher Cipher;

struct leafcipher{
    char* code;
    int length;
    int used;
};

#endif

void outputFile(int argc, char** argv, Cipher* codelist);
void inputFile(int argc, char** argv);