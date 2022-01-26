#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "encode.h"
#include "../include/encode.h"
#include "assert.h"
#define NSYMBOLS 256



static int symbol_count[NSYMBOLS];

// 以下このソースで有効なstatic関数のプロトタイプ宣言

// ファイルを読み込み、static配列の値を更新する関数
static void count_symbols(const char *filename);

// symbol_count をリセットする関数
static void reset_count(void);

// 与えられた引数でNode構造体を作成し、そのアドレスを返す関数
static Node *create_node(int symbol, int count, Node *left, Node *right);

// Node構造体へのポインタが並んだ配列から、最小カウントを持つ構造体をポップしてくる関数
// n は 配列の実効的な長さを格納する変数を指している（popするたびに更新される）
static Node *pop_min(int *n, Node *nodep[]);

// ハフマン木を構成する関数
static Node *build_tree(void);

//リーフ同士の統合 や ダミーノード同士の統合 

// 以下 static関数の実装
static void count_symbols(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
	fprintf(stderr, "error: cannot open %s\n", filename);
	exit(1);
    }
    
    // 1Byteずつ読み込み、カウントする
    /*
      write a code for counting
    */
    unsigned char* buf = (unsigned char*)malloc(sizeof(unsigned char) * 1000000);
    for (int i = 0; i < 1000000; i++) {
        size_t rsize = fread(&buf[i], sizeof(unsigned char), 1, fp);
        if (rsize == 0) {
            buf[i] = '\0';
            printf("%d Byte read\n", i);
            break;
        }
        symbol_count[(int)buf[i]]++;
        
    }

    fclose(fp);
}
static void reset_count(void)
{
    for (int i = 0 ; i < NSYMBOLS ; i++)
	symbol_count[i] = 0;
}

static Node *create_node(int symbol, int count, Node *left, Node *right)
{
    Node *ret = (Node *)malloc(sizeof(Node));
    *ret = (Node){ .symbol = symbol, .count = count, .left = left, .right = right};
    return ret;
}

static Node *pop_min(int *n, Node *nodep[])
{
    // Find the node with the smallest count
    // カウントが最小のノードを見つけてくる
    int argmin = 0;
    for (int i = 0; i < *n; i++) {
	if (nodep[i]->count < nodep[argmin]->count) {
	    argmin = i;
	}
    }
    
    Node *node_min = nodep[argmin];
    
    // Remove the node pointer from nodep[]
    // 見つかったノード以降の配列を前につめていく
    for (int i = argmin; i < (*n) - 1; i++) {
	nodep[i] = nodep[i + 1];
    }
    // 合計ノード数を一つ減らす
    (*n)--;
    
    return node_min;
}

static Node *build_tree(void)
{
    int n = 0;
    Node *nodep[NSYMBOLS];
    
    for (int i = 0; i < NSYMBOLS; i++) {
	// カウントの存在しなかったシンボルには何もしない
	if (symbol_count[i] == 0) continue;
	
	nodep[n] = create_node(i, symbol_count[i], NULL, NULL);
	n++;
    }

    const int dummy = -1; // ダミー用のsymbol を用意しておく
    while (n >= 2) {
	Node *node1 = pop_min(&n, nodep);
	Node *node2 = pop_min(&n, nodep);
	
	// Create a new node
	// 選ばれた2つのノードを元に統合ノードを新規作成
	// 作成したノードはnodep にどうすればよいか?
	//nodep[n++] = create_node(dummy, symbol_count[node1->symbol] + symbol_count[node2->symbol], node1, node2); // 一番後ろにくっつける。
	nodep[n++] = create_node(dummy, node1->count + node2->count, node1, node2); // symbol-countだとダミーが入っているとだめ。
	
    if (node1->symbol == -1) {
        if (node2->symbol == -1) {
            printf("Merging -1(%d) + -1(%d), creating a new dummy(%d)\n", node1->count,  node2->count, nodep[n-1]->count);
        } else {
            printf("Merging -1(%d) + %c(%d), creating a new dummy(%d)\n", node1->count, node2->symbol, node2->count, nodep[n-1]->count);
        }
    } else if (node2->symbol == -1) {
        printf("Merging %c(%d) + -1(%d), creating a new dummy(%d)\n", node1->symbol, node1->count, node2->count, nodep[n-1]->count);
    } else {
        printf("Merging %c(%d) + %c(%d), creating a new dummy(%d)\n", node1->symbol, node1->count, node2->symbol, node2->count, nodep[n-1]->count);
    
    }
    }
    

    // なぜ以下のコードで木を返したことになるか少し考えてみよう
    return (n==0)?NULL:nodep[0];
    // 0個 -> そもそも存在しない
    // 1個 -> 形成された。
}



// Perform depth-first traversal of the tree
// 深さ優先で木を走査する
// 現状は何もしていない（再帰してたどっているだけ）
void traverse_tree(const int depth, const Node *np, Cipher* codelist)
{
    // 今のところ1回しか呼び出せない。
    static int hwordlengthSoFar = 0;
    static int wordlengthSoFar = 0;
    static int wordKindSoFar = 0;
    static int counterPart = 0;
    static char encoded[64] = "0";
    

    if (np->left == NULL && np->right == NULL) {
        hwordlengthSoFar += depth * (np->count);
        wordlengthSoFar += np->count;
        wordKindSoFar++;
        
        counterPart = 5 * wordlengthSoFar;
        

        int last1Index = -1;
        for (int i = 0; i < strlen(encoded); i++) {
            if (encoded[i] == '1') last1Index = i;
        }
        if (1 == 1) {   
            for (int i = 0; i < last1Index; i++) { // +--を表示しない。
                if (encoded[i] == '0') {
                    printf("|     ");
                } else {
                    printf("      ");
                }
            }
            if (last1Index == -1) last1Index = 0;
            for (int i = last1Index; i < strlen(encoded); i++) {
                if (encoded[i] == '0') {
                    printf("+--0--");
                } else {
                    printf("+--1--");
                }
            }
        }
        if (np->symbol != '\n') {
            printf("\'%c\' (%d times)", np->symbol, np->count);
            //printf("Depth: %d, Symbol: %c, Times: %d", depth, np->symbol, np->count);
            //printf("Depth: %d, Symbol: %c, Times: %d, Length: %d, HuffmanSoFar: %d, wordlengthSoFar: %d, TypesSoFar: %d, EqualBitSoFar: %d", depth, np->symbol, np->count, depth * (np->count), hwordlengthSoFar, wordlengthSoFar, wordKindSoFar, counterPart);
        } else {
            printf("\'\\n\' (%d times)", np->count);
            //printf("Depth: %d, Symbol: \\n, Times: %d", depth,  np->count);
            //printf("Depth: %d, Symbol: \\n, Times: %d, Length: %d, HuffmanSoFar: %d, wordlengthSoFar: %d, TypesSoFar: %d, EqualBitSoFar: %d", depth, np->count, depth * (np->count), hwordlengthSoFar, wordlengthSoFar, wordKindSoFar, counterPart);
        }
        printf(", Cipher: %s\n", encoded);
        codelist[np->symbol].length = depth;
        codelist[np->symbol].code = (char*)malloc(sizeof(char) * (depth + 1));
        memcpy((codelist[np->symbol].code), encoded, strlen(encoded) + 1);
        codelist[np->symbol].used = 1; // 使用フラグを立てる
        
        return;
    }
    encoded[depth] = '0';
    traverse_tree(depth + 1, np->left, codelist);
    encoded[depth] = '1';
    traverse_tree(depth + 1, np->right, codelist);
    encoded[depth] = '\0';
}

// この関数は外部 (main) で使用される (staticがついていない)
Node *encode(const char *filename)
{
    reset_count();
    count_symbols(filename);
    Node *root = build_tree();
    
    if (root == NULL){
	fprintf(stderr,"A tree has not been constructed.\n");
    }

    return root;
}
