
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "io.h"
#include "../include/encode.h"
#include "../include/io.h"


void outputFile(int argc, char** argv, Cipher* codelist) {
    int dimInt = 128;
    int nOfSigns = 0;
    for (int i = 0; i < dimInt; i++) {
        if (codelist[i].used) nOfSigns++;  // 符号語の数を出す。
    }
    FILE *fp = fopen(argv[1], "wb");
    if (fp == NULL) {
	fprintf(stderr, "error: cannot open %s\n", argv[1]);
	exit(1);
    }
    int token[2] = {0xFD8A5902, 0xFDB97531};
    fwrite(token, sizeof(int), 2, fp); // 識別子
    fwrite(nOfSigns, sizeof(int), 1, fp); // 符号語の数
    nOfSigns = 0;
    char ci = 0;
    for (int i = 0; i < dimInt; i++) { //符号語があるかをチェック。1つずつ符号語を書き込む。
        if (codelist[i].used) {
            fwrite(ci, sizeof(char), 1, fp);
            nOfSigns++;
        }
        ci++;
    }
    fwrite(token, sizeof(int), 2, fp);
    int nOfSignsRegistered = 0;
    for (int i = 0; nOfSignsRegistered < nOfSigns; i++) { //1つずつ符号語を書きこむ。
        if (codelist[i].used) {
            long code = 0;
            int len = codelist[i].length;
            for (int j = 0; j < len; j++) {
                if (codelist[i].code[len-1-j] == '1') code += 1 << j; // 累乗みたいにして計算するときに便利な左シフト
            }
            fwrite(code, sizeof(long), 1, fp);
            nOfSignsRegistered++;
        }
    }
    fwrite(token, sizeof(int), 2, fp);
    
    char* buf = (char*)malloc(sizeof(char) * 1000000);
    size_t siz = 0;
    FILE *fpread = fopen(argv[2], "rb");
    
    for (siz = 0; siz < 1000000; siz++) {
        size_t rsize = fread(&buf[siz], sizeof(char), 1, fpread);
        if (rsize == 0) {
            buf[siz] = '\0';
            printf("%d Byte read\n", siz);
            break;
        }
    }
    fwrite(siz, sizeof(size_t), 1, fp);

    for (int i = 0; i < siz; i++) {
        
    }
    
    fwrite(token, sizeof(int), 2, fp);
/*
次のようなデータ形式で出力します。
最初の8バイトに、FD8A5902 FDB97531 を付加し、これをfilename 形式の識別子としています。
その次の1バイトには、符号の個数が4バイトで書かれています。最大128個です。
そして次に、符号のリストが並びます。各1バイトです。
その次には、再び FD8A5902 FDB97531 という識別子が入ります。
次に、符号語が並びます。各8バイト (long) です。 000101 なら、 後ろから順に解釈していきます。64 + 0 + 16 + 0 + 0 + 0 で 80 が格納されます。
64以上のサイズを持つ暗号には対応できませんが、そういう状況は起こりません。理由は、圧縮後のデータサイズの限界が1MBと決めてあるのでその範囲内では起こりようがないからです。
こうすることで符号長と符号語を同時に取得することができるようになります。
その次には、再び FD8A5902 FDB97531 という識別子が入ります。
この次の8バイトには、圧縮データのサイズが入ります。この時に、1MB以上のサイズの場合はエラーが出て終了します。（そんな大容量を圧縮しようとしないでください）
その次からは、圧縮後のデータがサイズで指定された分入ります。
このデータの後にまた FD8A5902 FDB97531 という識別子が入ります。
このデータの後には何もないことになっています。
*/

    fclose(fp);
}
void inputFile(int argc, char** argv, Cipher* codelist) {


}