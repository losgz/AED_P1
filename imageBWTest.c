// imageBWTest - A program that performs some image processing.
//
// This program is an example use of the imageBW module,
// a programming project for the course AED, DETI / UA.PT
//
// You may freely use and modify this code, NO WARRANTY, blah blah,
// as long as you give proper credit to the original and subsequent authors.
//
// The AED Team <jmadeira@ua.pt, jmr@ua.pt, ...>
// 2024

#include "imageBW.h"
#include "instrumentation.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PIXMEM InstrCount[0]
#define BOL_OPS InstrCount[1]
#define RLEMEM InstrCount[2]

void ChessTable() {
    Image chessboard;
    int i, j;
    printf("|   Size(B)   |   Runs   |   Height/Width   |   Square Edge   |\n");
    for (i = 2; i <= 16384; i *= 2) {
        printf("|             |          |                  |                 "
               "|\n");
        for (j = 1; j != i; j *= 2) {
            // Certeficar que os prints estão descomentados na função
            // ImageCreateChessboard
            chessboard = ImageCreateChessboard(i, i, j, 1);
            ImageDestroy(&chessboard);
        }
        printf("|_____________|__________|__________________|_________________|"
               "\n");
    }
}

void ANDTable() {
    // Ter em conta que os prints da imageAND para o melhor e pior caso são
    // diferenetes do caso médio. Logo, a tabela do caso médio NÃO pode ser
    // impressa ao mesmo tempo que a tabela do melhor e pior caso
    Image chessboard1, black_image, white_image, image_and;
    int i, j;
    printf("\nBest case\n");
    printf("| Num of Calls | Runs Img1 | Runs Img2 | Height | Width |\n"); // ImageAND_Without_Uncompress()
    /*printf("|   Num of Pixels   |   Height   |   Width   |\n");*/        // ImageAND_Uncompressed()
    for (i = 2; i <= 1028; i *= 2) {
        printf("|              |           |           |        |       |\n");// ImageAND_Without_Uncompress()
        /*printf("|                   |            |           |\n");*/       // ImageAND_Uncompressed()
        for (j = i / 2; j <= i; j *= 2) {
            black_image = ImageCreate(j, i, 1);
            white_image = ImageCreate(j, i, 0);
            void InstrReset();
            // Certefique que os prints estão ativos na função ImageAND
            image_and = ImageAND(black_image, white_image);
            ImageDestroy(&white_image);
            ImageDestroy(&black_image);
            ImageDestroy(&image_and);
        }
        printf("|______________|___________|___________|________|_______|\n");// ImageAND_Without_Uncompress()
        /*printf("|___________________|____________|___________|\n");*/       // ImageAND_Uncompressed()
    }
    // SE FOR TESTAR O IMAGEAND BÁSICO COMENTAR O RESTO DA FUNÇÃO
    printf("\nWorst case\n");
    printf("| Num of Calls | Runs Img1 | Runs Img2 | Height | Width |\n");
    for (i = 2; i <= 1028; i *= 2) {
        printf("|              |           |           |        |       |\n");
        for (j = i / 2; j <= i; j *= 2) {
            // Certefique que os prints estão NÂO ativos na função ImageCreateChessboard
            chessboard1 = ImageCreateChessboard(j, i, 1, 1);
            black_image = ImageCreate(j, i, 1);
            void InstrReset();
            // Certefique que os prints estão ativos na função ImageAND
            image_and = ImageAND(chessboard1, black_image);
            ImageDestroy(&chessboard1);
            ImageDestroy(&black_image);
            ImageDestroy(&image_and);
        }
        printf("|______________|___________|___________|________|_______|\n");
    }
    printf("\nAverage case\n");
    // Certefique que os prints estão ativos na função ImageAND
    Image image1 = ImageLoad("binary_pbm_images/random_case_image_6.pbm");
    Image image2 = ImageLoad("binary_pbm_images/random_case_image_7.pbm");
    printf("| Num of Calls | Runs Img1 | Runs Img2 | R_Line | Width |\n");
    printf("|              |           |           |        |       |\n");

    image_and = ImageAND(image1, image2);
    ImageDestroy(&image_and);
    ImageDestroy(&image1);
    ImageDestroy(&image2);
    printf("|______________|___________|___________|________|_______|\n");

    image1 = ImageLoad("binary_pbm_images/random_case_image_11.pbm");
    image2 = ImageLoad("binary_pbm_images/random_case_image_13.pbm");
    image_and = ImageAND(image1, image2);
    ImageDestroy(&image_and);
    ImageDestroy(&image1);
    ImageDestroy(&image2);
    printf("|______________|___________|___________|________|_______|\n");

    image1 = ImageLoad("binary_pbm_images/random_case_image_17.pbm");
    image2 = ImageLoad("binary_pbm_images/random_case_image_19.pbm");
    image_and = ImageAND(image1, image2);
    ImageDestroy(&image_and);
    ImageDestroy(&image1);
    ImageDestroy(&image2);
    printf("|______________|___________|___________|________|_______|\n");

    image1 = ImageLoad("binary_pbm_images/random_case_image_24.pbm");
    image2 = ImageLoad("binary_pbm_images/random_case_image_26.pbm");
    image_and = ImageAND(image1, image2);
    ImageDestroy(&image_and);
    ImageDestroy(&image1);
    ImageDestroy(&image2);
    printf("|______________|___________|___________|________|_______|\n");

    image1 = ImageLoad("binary_pbm_images/random_case_image_33.pbm");
    image2 = ImageLoad("binary_pbm_images/random_case_image_35.pbm");
    image_and = ImageAND(image1, image2);
    ImageDestroy(&image_and);
    ImageDestroy(&image1);
    ImageDestroy(&image2);
    printf("|______________|___________|___________|________|_______|\n");
}

void AnalyzeRuns(uint32 width, uint32 height) {
    uint32 min_square_edge =
        (width < height) ? width : height; // menor entre largura e altura
    uint32 max_square_edge = 1; // sempre 1 para o maior número de runs

    // menor número de runs
    uint32 min_runs =
        height; // cada linha tem 1 run, total é igual ao número de linhas
    printf("Padrão com menos runs:\n");
    printf("Aresta: %d, Número de runs: %d\n", min_square_edge, min_runs);

    // Maior número de runs
    uint32 max_runs = width * height; // cada pixel é um run
    printf("Padrão com mais runs:\n");
    printf("Aresta: %d, Número de runs: %d\n", max_square_edge, max_runs);
}

void CompareANDPerformance() {
    Image img1, img2, result;
    int width, height;
    clock_t start, end;
    double exec_time;
    size_t memory_used;
    int num_ops;

    // Certefique que os prints NÂO estão ativos na função ImageAND
    
    printf("\nComparison of ImageAND Implementations\n");
    printf("|   Width   |   Height   | Num of OPS | Exec Time (s) | Mem "
           "(bytes) |\n");
    printf("|-----------|------------|------------|---------------|------------"
           "-|\n");

    for (width = 128, height = 128; width <= 2048; width *= 2, height *= 2) {
        // Criar imagens de teste
        img1 = ImageCreate(width, height, BLACK);
        img2 = ImageCreate(width, height, WHITE);

        // Medir tempo e memória para Uncompressed
        InstrReset(); // Resetar contadores
        start = clock();
        result = ImageAND(img1, img2);
        end = clock();
        exec_time = (double)(end - start) / CLOCKS_PER_SEC;
        /*memory_used = PIXMEM; // para o ImageAND_Uncompressed()*/
        memory_used = RLEMEM; // para o ImageAND_Without_Uncompress()
        num_ops = BOL_OPS;
        ImageDestroy(&result); // Limpar imagem resultante

        // Imprimir resultados na tabela
        printf("|%11d|%12d|%12d|%15.6f|%13zu|\n", width, height, num_ops,
               exec_time, memory_used);

        // Limpar imagens originais
        ImageDestroy(&img1);
        ImageDestroy(&img2);
    }
    printf("|-----------|------------|------------|---------------|------------"
           "-|\n");
}

// função para o por os resulatdos num ficheiro csv para fazer os plots
void ANDCompareRandomDirForPlots(const char *input_dir,
                                 const char *output_file) {
    FILE *output;
    DIR *dir;
    struct dirent *entry;
    Image img1, img2, result;
    size_t num_operations;

    output = fopen(output_file, "w");
    if (!output) {
        perror("Failed to open output file");
        return;
    }

    dir = opendir(input_dir);
    if (!dir) {
        perror("Failed to open input directory");
        fclose(output);
        return;
    }

    fprintf(output, "Image,Width,Ops\n");

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char file1_path[256], file2_path[256];

            // fazer o and de uma imagem com ela mesma
            snprintf(file1_path, sizeof(file1_path), "%s/%s", input_dir,
                     entry->d_name);

            snprintf(file2_path, sizeof(file2_path), "%s/%s", input_dir,
                     entry->d_name);

            img1 = ImageLoad(file1_path);
            img2 = ImageLoad(file2_path);

            if (!img1 || !img2) {
                fprintf(stderr, "Failed to load images: %s, %s\n", file1_path,
                        file2_path);
                continue;
            }

            int width = ImageWidth(img1);

            InstrReset();
            result = ImageAND(img1, img2);
            num_operations = BOL_OPS;

            fprintf(output, "%s,%d,%zu\n", entry->d_name, width,
                    num_operations);

            ImageDestroy(&result);
            ImageDestroy(&img1);
            ImageDestroy(&img2);
        }
    }

    closedir(dir);
    fclose(output);
    printf("Results written to %s\n", output_file);
}

int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(stderr, "Usage: %s  # no arguments required (for now)\n",
                argv[0]);
        exit(1);
    }

    // To initalize operation counters
    ImageInit();

    // Creating and displaying some images
    Image white_image = ImageCreate(8, 8, WHITE);
    ImageRAWPrint(white_image);

    Image black_image = ImageCreate(8, 8, BLACK);
    ImageRAWPrint(black_image);

    Image image_1 = ImageNEG(white_image);
    ImageRAWPrint(image_1);

    Image chessboard = ImageCreateChessboard(8, 8, 2, BLACK);
    ImageRAWPrint(chessboard);

    printf("Chess AND Chess\n");
    Image image_2 = ImageAND(chessboard, chessboard);
    ImageRAWPrint(image_2);

    ImageDestroy(&image_2);
    printf("Chess AND Black_Image\n");
    image_2 = ImageAND(chessboard, black_image);
    ImageRAWPrint(image_2);

    ImageDestroy(&image_2);
    printf("Chess AND White_Image\n");
    image_2 = ImageAND(chessboard, white_image);
    ImageRAWPrint(image_2);

    printf("Chess XOR Black_Image\n");
    Image image_3 = ImageXOR(chessboard, black_image);
    ImageRAWPrint(image_3);

    ImageDestroy(&image_3);
    printf("Chess XOR White_Image\n");
    image_3 = ImageXOR(chessboard, white_image);
    ImageRAWPrint(image_3);

    printf("Chess XOR Chess\n");
    Image image_4 = ImageXOR(chessboard, chessboard);
    ImageRAWPrint(image_4);

    printf("Image  Original\n");
    Image image_5 = ImageLoad("pbm/washington.pbm");
    ImageRAWPrint(image_5);

    printf("Image Horizontal Mirror\n");
    Image image_6 = ImageHorizontalMirror(image_5);
    ImageRAWPrint(image_6);

    printf("Image Horizontal Mirror on Bottom of the Original\n");
    Image image_7 = ImageReplicateAtBottom(image_5, image_6);
    ImageRAWPrint(image_7);

    printf("Image Vertical\n");
    Image image_8 = ImageVerticalMirror(image_5);
    ImageRAWPrint(image_8);

    printf("Image duplicate right without Uncompress\n");
    Image image_9 = ImageReplicateAtRight(image_5, image_5);
    ImageRAWPrint(image_9);

    printf("Image OR\n");
    Image image_10 = ImageOR(chessboard, black_image);
    ImageRAWPrint(image_10);

    printf("Image OR\n");
    Image image_11 = ImageOR(chessboard, white_image);
    ImageRAWPrint(image_11);

    printf("Image Is Equal\n");
    ImageIsEqual(image_5, image_6);

    ImageDestroy(&chessboard);
    printf("Chess 16 16 2\n");
    chessboard = ImageCreateChessboard(16, 16, 2, BLACK);
    ImageRAWPrint(chessboard);

    ImageDestroy(&chessboard);
    printf("Chess 16 16 8\n");
    chessboard = ImageCreateChessboard(16, 16, 8, BLACK);
    ImageRAWPrint(chessboard);

    ImageDestroy(&chessboard);
    printf("Chess 32 32 8\n");
    chessboard = ImageCreateChessboard(32, 32, 8, BLACK);
    ImageRAWPrint(chessboard);

    uint32 width = 32;
    uint32 height = 32;
    // analisar os padrões de xadrez
    AnalyzeRuns(width, height);
    /**/
    ChessTable();
    /**/
    ANDTable();
    /**/
    CompareANDPerformance();
    /**/
    ANDCompareRandomDirForPlots("binary_pbm_images", "results.csv");

    /*** UNCOMMENT TO TEST THE NEXT FUNCTIONS

    Image image_2 = ImageReplicateAtBottom(white_image, black_image);
    ImageRAWPrint(image_2);


    printf("image_1 AND image_1\n");
    Image image_3 = ImageAND(image_1, image_1);
    ImageRAWPrint(image_3);

    printf("image_1 AND image_2\n");
    Image image_4 = ImageAND(image_1, image_2);
    ImageRAWPrint(image_4);

    printf("image_1 OR image_2\n");
    Image image_5 = ImageOR(image_1, image_2);
    ImageRAWPrint(image_5);

    printf("image_1 XOR image_1\n");
    Image image_6 = ImageXOR(image_1, image_1);
    ImageRAWPrint(image_6);

    printf("image_1 XOR image_2\n");
    Image image_7 = ImageXOR(image_1, image_2);
    ImageRAWPrint(image_7);

    Image image_8 = ImageReplicateAtRight(image_6, image_7);
    ImageRAWPrint(image_8);

    Image image_9 = ImageReplicateAtRight(image_6, image_6);
    ImageRAWPrint(image_9);

    Image image_10 = ImageHorizontalMirror(image_1);
    ImageRAWPrint(image_10);

    Image image_11 = ImageVerticalMirror(image_8);
    ImageRAWPrint(image_11);

    // Saving in PBM format
    ImageSave(image_7, "image7.pbm");
    ImageSave(image_8, "image8.pbm");

     ***/

    // Housekeeping
    ImageDestroy(&white_image);
    ImageDestroy(&black_image);
    ImageDestroy(&image_1);
    ImageDestroy(&chessboard);
    /*** UNCOMMENT IF YOU CREATE THOSE IMAGES
     ***/

    ImageDestroy(&image_2);
    ImageDestroy(&image_3);
    ImageDestroy(&image_4);
    ImageDestroy(&image_5);
    ImageDestroy(&image_6);
    ImageDestroy(&image_7);
    ImageDestroy(&image_8);
    ImageDestroy(&image_9);
    ImageDestroy(&image_10);
    ImageDestroy(&image_11);
    /*ImageDestroy(&image_12);*/

    return 0;
}
