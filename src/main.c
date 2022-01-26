#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "../include/encode.h"
#include "io.h"
#include "../include/io.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
	fprintf(stderr, "usage: %s <filename>\n",argv[0]);
	exit(1);
    }
    
    Node *root = encode(argv[1]);
    Cipher codelist[128];
    for (int i = 0; i < 128; i++) codelist[i].used = 0;
    traverse_tree(0,root,codelist);
    printf("%s\n", codelist['x'].code)
    ;
    return EXIT_SUCCESS;
}
