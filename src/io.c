
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "io.h"
#include "../include/encode.h"
#include "../include/io.h"
#include "assert.h"

/*
次のようなデータ形式で出力します。
最初の8バイトに、FD8A5902 FDB97531 を付加し、これをfilename 形式の識別子としています。
その次の1バイトには、符号の個数が4バイトで書かれています。最大128個です。
そして次に、符号のリストが並びます。各1バイトです。
その次には、再び FD8A5902 FDB97531 という識別子が入ります。
次に、符号語の長さが並びます。各4バイトです。
次に、符号語が並びます。各8バイト (long) です。 000101 なら、 後ろから順に解釈していきます。64 + 0 + 16 + 0 + 0 + 0 で 80 が格納されます。
64以上のサイズを持つ暗号には対応できませんが、そういう状況は起こりません。理由は、圧縮後のデータサイズの限界が1MBと決めてあるのでその範囲内では起こりようがないからです。
こうすることで符号長と符号語を同時に取得することができるようになります。
その次には、再び FD8A5902 FDB97531 という識別子が入ります。
この次の8バイトには、圧縮データのサイズが入ります。この時に、1MB以上のサイズの場合はエラーが出て終了します。（そんな大容量を圧縮しようとしないでください）
その次からは、圧縮後のデータがサイズで指定された分入ります。

このデータの後にまた FD8A5902 FDB97531 という識別子が入ります。
このデータの後には何もないことになっています。
*/

void outputFile(int argc, char** argv, Cipher* codelist) {
    int dimInt = 256;
    int nOfSigns = 0;
    for (int i = 0; i < dimInt; i++) {
        if (codelist[i].used) nOfSigns++;  // 符号語の数を出す。
    }
    FILE* fp = fopen(argv[2], "wb");
    if (fp == NULL) {
        fprintf(stderr, "error: cannot open %s\n", argv[2]);
        exit(1);
    }
    int token[2] = { 0xFD8A5902, 0xFDB97531 };
    fwrite(&token, sizeof(int), 2, fp); // 識別子
    fwrite(&nOfSigns, sizeof(int), 1, fp); // 符号語の数
    nOfSigns = 0;
    unsigned char ci = 0;
    for (int i = 0; i < dimInt; i++) { //符号語があるかをチェック。1つずつ符号語を書き込む。
        if (codelist[i].used) {
            fwrite(&ci, sizeof(unsigned char), 1, fp);
            printf("Writing: %d(%c), No. %d\n", ci, ci, nOfSigns);
            nOfSigns++;
        }
        ci++;
    }
    fwrite(&token, sizeof(int), 2, fp); // 識別子

    int nOfSignsRegistered = 0; //長さを書き込む。
    for (int i = 0; nOfSignsRegistered < nOfSigns; i++) {
        if (codelist[i].used) {
            int len = codelist[i].length;
            fwrite(&len, sizeof(int), 1, fp);
            nOfSignsRegistered++;
        }
    }
    nOfSignsRegistered = 0;
    for (int i = 0; nOfSignsRegistered < nOfSigns; i++) { //1つずつ符号語を書きこむ。
        if (codelist[i].used) {
            long code = 0;
            int len = codelist[i].length;
            for (int j = 0; j < len; j++) {
                if (codelist[i].code[j] == '1') code += 1 << j; // 累乗みたいにして計算するときに便利な左シフト
            }
            fwrite(&code, sizeof(long), 1, fp);
            printf("Writing: %lx for %d(%c), len: %d, No. %d\n", code, i, i, len, nOfSignsRegistered);
            nOfSignsRegistered++;
        }
    }
    fwrite(&token, sizeof(int), 2, fp);

    unsigned char* buf = (unsigned char*)malloc(sizeof(unsigned char) * 1000000);
    size_t siz = 0;
    FILE* fpread = fopen(argv[3], "rb");

    for (siz = 0; siz < 1000000; siz++) {
        size_t rsize = fread(&buf[siz], sizeof(unsigned char), 1, fpread);
        if (rsize == 0) {
            buf[siz] = '\0';
            printf("%ld Byte read\n", siz);
            break;
        }
    }
    fwrite(&siz, sizeof(size_t), 1, fp);
    long fpz0 = ftell(fp);
    fwrite(&fpz0, sizeof(long), 1, fp); // この位置にファイルの長さを格納したいが、今は一旦保留する。
    //printf("%p\n", *fp);
    long fpz1 = ftell(fp);
    for (int i = 0; i < siz; i++) {
        fwrite(codelist[buf[i]].code, sizeof(unsigned char), codelist[buf[i]].length, fp);
    }
    long fpz2 = ftell(fp); // ファイルの長さを出すための材料を整える。
    //printf("%p\n", *fp);
    printf("%ld\n", fpz2 - fpz1);
    fwrite(&token, sizeof(int), 2, fp); // 最後の符号

    fseek(fp, fpz0, SEEK_SET); // 位置を保留した位置に戻す
    size_t lenStr = fpz2 - fpz1; //ファイルサイズの長さを取得する。
    fwrite(&lenStr, sizeof(size_t), 1, fp); // 圧縮後ファイルサイズを格納する
    fclose(fpread);
    fclose(fp);
}
void inputFile(int argc, char** argv) {
    int dimInt = 256;
    int nOfSigns = 0;
    FILE* fpread = fopen(argv[3], "rb");
    FILE* fp = fopen(argv[2], "w");

    int token[2] = { 0xFD8A5902, 0xFDB97531 };
    int tokenRead[2];
    size_t tokenResult = fread(&tokenRead, sizeof(int), 2, fpread); // 識別子
    assert(tokenResult == 2);
    assert(tokenRead[0] == token[0] && tokenRead[1] == token[1]);
    printf("token success\n");


    size_t success = fread(&nOfSigns, sizeof(int), 1, fpread); // 符号語の数
    assert(success == 1 && nOfSigns <= 256);
    printf("nOfSigns = %d\n", nOfSigns);

    Cipher codelist[256];
    for (int i = 0; i < dimInt; i++) codelist[i].used = 0;

    unsigned char ciToListNo[256]; // 実際に使用されるのは、 nOfSigns個のみ。存在する符号語の番号をここに書き込んでいく。256個毎回読み込むのは明らかな時間の無駄なので、これを導入した。

    unsigned char ci = 0;
    for (int i = 0; i < nOfSigns; i++) { //符号語があるかをチェック。1つずつ符号語を書き込む。
        size_t readed = fread(&ci, sizeof(unsigned char), 1, fpread);
        assert(readed == 1);
        codelist[ci].used = 1;
        ciToListNo[i] = ci;
    }

    tokenResult = fread(&tokenRead, sizeof(int), 2, fpread); // 識別子
    assert(tokenResult == 2);
    assert(tokenRead[0] == token[0] && tokenRead[1] == token[1]);

    int count = 0;
    long code;
    long code2;
    int len;
    int minLen = 128;
    int maxLen = 1;
    for (int i = 0; i < dimInt; i++) {
        if (codelist[i].used) { //表の長さを読み取り
            fread(&len, sizeof(int), 1, fpread);
            codelist[i].length = len;
            if (minLen > len) minLen = len;
            if (maxLen < len) maxLen = len;
        }

    }
    for (int i = 0; i < dimInt; i++) { // 表のコードを読み込み
        if (codelist[i].used) {
            fread(&code, sizeof(long), 1, fpread);
            code2 = code;
            int blen = 0; // len > blen の場合は、最後の何桁かを0で埋める必要が出てくる。
            while (code2) {
                code2 = code2 >> 1;
                blen++;
            }
            len = codelist[i].length;
            printf("Reading: %lx for %d(%c), len: %d, blen: %d, No. %d\n", code, i, i, len, blen, count);
            codelist[i].code = (char*)malloc(sizeof(char) * (len + 1));
            codelist[i].code[len] = '\0';
            for (int j = 0; j < len - blen; j++) { // len > blen の場合は、最後の何桁かを0で埋める必要が出てくる。
                codelist[i].code[len - 1 - j] = '0';
            }
            for (int j = 0; j < blen; j++) {
                codelist[i].code[j] = ((code >> j) & 1) ? '1' : '0'; // nビット右にずらしたときの値が1なら、 '1', そうでないとき '0'
            }  //これで表が完成したことになる。
            count++;
        }
    }

    tokenResult = fread(&tokenRead, sizeof(int), 2, fpread); // 識別子
    assert(tokenResult == 2);
    assert(tokenRead[0] == token[0] && tokenRead[1] == token[1]);


    size_t filesize; //ファイルサイズ
    tokenResult = fread(&filesize, sizeof(size_t), 1, fpread);
    assert(tokenResult && filesize <= 1000000);
    printf("Filesize = %ld\n", filesize);
    printf("u: %s\n", codelist['u'].code);

    tokenResult = fread(&filesize, sizeof(size_t), 1, fpread);
    assert(tokenResult && filesize <= 1000000);
    printf("Filesize (compressed) = %ld\n", filesize);
    //ファイルを1バイトずつ解釈して、解釈する。
    //最初に長さの最小を決めて、それ以降は1バイトずつ読む。
    //1バイトずつ読むごとに256種類の符号を全部調べる。
    //合致するものがあればそれを読む。
    //
    char* buf = (char*)calloc(sizeof(char), maxLen + 1);
    for (int filePos = 0; filePos < filesize;) {
        for (int i = 0; i < maxLen + 1; i++) buf[i] = 0; //初期化
        fread(buf, sizeof(char), minLen, fpread);
        filePos += minLen;
        int found = 0;
        for (int j = minLen; j <= maxLen; j++) {
            if (filePos >= filesize + 1) { //全部読んだ場合は終わり。
                printf("\nfinished reading (1)\n");
                exit(1);
            }
            for (int i = 0; i < nOfSigns; i++) { //合っているのを探すループ。
                if (codelist[ciToListNo[i]].length == j) { //長さ合わなければ意味がない
                //printf("%d: %s vs %s\n", j, buf, codelist[ciToListNo[i]].code);
                    if (!(strcmp(codelist[ciToListNo[i]].code, buf))) {
                        found = 1;
                        fwrite(&ciToListNo[i], sizeof(char), 1, fp); //最終出力ファイルに書き込み。
                        break;
                    }
                }
            }
            if (found == 1) break;
            fread(&buf[j], sizeof(char), 1, fpread); //追加で読み。
            filePos++;
        }
        if (!found) {
            printf("Not found\n"); 
            exit(1);    
        }
        printf("Reading... %d/%d\r", filePos, filesize);
    }
    printf("\nfinished reading\n");
    fclose(fpread);
    fclose(fp);

}