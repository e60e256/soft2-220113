#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "../include/encode.h"
#include "io.h"
#include "../include/io.h"
#include "assert.h"

int main(int argc, char **argv)
{
    if (argc != 4) {
	fprintf(stderr, "usage: %s <mode> <outputfilename> <inputfilename>\n",argv[0]);
	exit(1);
    }
    
    if (!(strcmp(argv[1], "cp_basic"))) {
    Node *root = encode(argv[3]);
    Cipher codelist[256];
    for (int i = 0; i < 256; i++) codelist[i].used = 0;
    traverse_tree(0,root,codelist);
    printf("%s\n", codelist['x'].code);
    outputFile(argc, argv, codelist);
    return EXIT_SUCCESS;
    } else if ((!(strcmp(argv[1], "ex_basic")))) {
        inputFile(argc, argv);
    }
}
