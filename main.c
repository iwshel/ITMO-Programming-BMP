#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct{
    char smth1[18];
    char width[4];
    char height[4];
    char smth2[28];
} header;


typedef struct {
    int height;
    int width;
    int** color;
} BMP;


unsigned int calcSynchSafe(unsigned char length[4]) {
    unsigned int result = 0;
    for (int i = 3; i >= 0; i--){
        result += length[i] << (i * 8);
    }

    return result;
}


void freePic(BMP picture) {
    for (int i = 0; i < picture.width; i++){
        free(picture.color[i]);
    }
    free(picture.color);
}


BMP readBMP(FILE* inputFile, int height, int width, header mainHeader) {
    BMP picture;
    picture.height = height;
    picture.width = width;
    picture.color = (int**) malloc(width * sizeof(int*));
    for (int i = 0; i < width; i++) {
        picture.color[i] = (int*) malloc(height * sizeof(int));
    }
    int red, green, blue;
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            blue = getc(inputFile);
            green = getc(inputFile);
            red = getc(inputFile);
            if (blue == 0 && green == 0 && red == 0){
                picture.color[x][y] = 1;
            } else {
                picture.color[x][y] = 0;
            }
        }
        if (width % 4 == 1) {
            getc(inputFile);
        }
        if (width % 4 == 2) {
            getc(inputFile);
            getc(inputFile);
        }
        if (width % 4 == 3) {
            getc(inputFile);
            getc(inputFile);
            getc(inputFile);
        }
    }

    return picture;
}


BMP getNextLife(BMP board) {
    BMP newBoard;
    newBoard.height = board.height;
    newBoard.width = board.width;
    newBoard.color = (int**) malloc(board.width * sizeof(int*));

    for (int i = 0; i < board.width; ++i){
        newBoard.color[i] = (int*) malloc(board.height * sizeof(int));
    }

    int neighbours;
    for (int y = 0; y < board.height; y++) {
        for (int x = 0; x < board.width; x++) {
            neighbours = 0;
            for (int i = x - 1; i < x + 2; i++){
                for (int j = y - 1;  j < y + 2; j++){
                    if (x == i && y == j) break;
                    int newI = i;
                    int newJ = j;
                    if (newI > 0) {
                        newI = newI % board.width;
                    } else if (newI < 0) {
                        newI += board.width;
                    }
                    if (newJ > 0) {
                        newJ = newJ % board.height;
                    } else if (newJ < 0) {
                        newJ += board.height;
                    }
                    neighbours += board.color[newI][newJ];
                }
            }

            if (board.color[x][y] == 1) {
                if (neighbours < 2 ||neighbours > 3){
                    newBoard.color[x][y] = 0;
                } else {
                    newBoard.color[x][y] = 1;
                }
            } else {
                if (neighbours == 3){
                    newBoard.color[x][y] = 1;
                } else {
                    newBoard.color[x][y] = 0;
                }
            }
        }
    }
    for (int i = 0; i < board.height; i++){
        for (int j = 0; j < board.width; j++){
            board.color[j][i] = newBoard.color[j][i];
        }
    }
    freePic(newBoard);

    return board;
}


int createBMP(char* dirname, int iter, header mainHeader, BMP picture) {
    char *path = (char *) malloc(sizeof(dirname) + 100000);
    sprintf(path, "%s%d.bmp", dirname, iter);
    FILE* outputFile = fopen(path, "wb");
    if (outputFile == NULL){
        printf("can not create new bmp file:( ");
        return 1;
    }
    fwrite(&mainHeader, 54, 1, outputFile);
    unsigned char pixel;
    for (int y = picture.height - 1; y >= 0; y--) {
        for (int x = 0; x < picture.width; x++) {
                if (picture.color[x][y] == 0){
                    pixel = 255;
                } else {
                    pixel = 0;
                }
                putc(pixel, outputFile);
                putc(pixel, outputFile);
                putc(pixel, outputFile);
        }
        if (picture.width % 4 == 1) {
                putc(0, outputFile);
        }
        if (picture.width % 4 == 2) {
                putc(0, outputFile);
                putc(0, outputFile);
        }
        if (picture.width % 4 == 3) {
                putc(0, outputFile);
                putc(0, outputFile);
                putc(0, outputFile);
        }
    }
    printf("bmp %d created in: %s\n", iter, path);

    free(path);
    fclose(outputFile);
    return 0;
}


int gameLife(char* filename, char* dirName, int maxIter, int dumpFreq) {
    FILE* inputFile = fopen(filename, "rb");
    if (inputFile == NULL){
        printf("can not open input bmp file:( \n");
        return 1;
    }
    header info;
    fread(&info, 1, 54, inputFile);

    unsigned int propHeight = calcSynchSafe(info.height);
    unsigned int propWidth = calcSynchSafe(info.width);
    BMP picture = readBMP(inputFile, propHeight, propWidth, info);
    BMP nextMove;
    for (int i = 1; i <= maxIter; i++) {
        for (int j = 0; j < dumpFreq; j++) {
            nextMove = getNextLife(picture);
        }
        int resultBMP;
        resultBMP = createBMP(dirName, i, info, nextMove);
        if (resultBMP == 1) {
            return 2;
        }
    }

    freePic(picture);
    fclose(inputFile);
    return 0;
}


int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("wrong input");
        return 1;
    }
    char* inputFile;
    char* outputDir;
    int maxIter = 100000;
    int dumpFreq = 1;
    for (int i = 1; i < argc; i+=2) {
        if (!strcmp(argv[i], "--input"))
            inputFile = argv[i + 1];
        else if (!strcmp(argv[i], "--output"))
            outputDir = argv[i + 1];
        else if (!strcmp(argv[i], "--max_iter"))
            maxIter = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "--dump_freq"))
            dumpFreq = atoi(argv[i + 1]);
    }
    int resultOfGame;
    resultOfGame = gameLife(inputFile, outputDir, maxIter, dumpFreq);
    if (resultOfGame == 1) {
        printf ("game error with input file \n");
    } else if (resultOfGame == 2) {
        printf ("bmp creating error \n");
    } else {
        printf("game successfully ended! :) ");
    }
    return 0;
}
