#pragma once

typedef struct node Node;
   
struct node{
    int symbol;
    int count;
    Node *left;
    Node *right;
};

#ifndef Cipherdefined
#define Cipherdefined
typedef struct leafcipher Cipher;

struct leafcipher{
    char* code;
    int length;
    int used;
};
#endif

// ファイルをエンコードし木のrootへのポインタを返す
Node *encode(const char *filename);
// Treeを走査して表示する
void traverse_tree(const int depth, const Node *root, Cipher* codelist);

