/// imageBW - A simple image processing module for BW images/
///           represented using run-length encoding (RLE)
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// The AED Team <jmadeira@ua.pt, jmr@ua.pt, ...>
/// 2024

// Student authors (fill in below):
// NMec:119234
// Name:Eduardo José Farinha do Rosário
// NMec:119954
// Name:Henrique Marques Lopes
//
// Date: 4/12/24
//

#include "imageBW.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instrumentation.h"

// The data structure
//
// A BW image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the pointers
// to the RLE compressed image rows.
//
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Constant value --- Use them throughout your code
// const uint8 BLACK = 1;  // Black pixel value, defined on .h
// const uint8 WHITE = 0;  // White pixel value, defined on .h
const int EOR = -1; // Stored as the last element of a RLE row

// Internal structure for storing RLE BW images
struct image {
    uint32 width;
    uint32 height;
    int *
        *row; // pointer to an array of pointers referencing the compressed rows
};

// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or
// file (I/O) operations use defensive techniques.
// When one of these functions fails,
// it immediately prints an error and exits the program.
// This fail-fast approach to error handling is simpler for the programmer.

// Use the following function to check a condition
// and exit if it fails.

// Check a condition and if false, print failmsg and exit.
static void check(int condition, const char *failmsg) {
    if (!condition) {
        perror(failmsg);
        exit(errno || 255);
    }
}

/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) { ///
    InstrCalibrate();
    InstrName[0] = "pixmem"; // InstrCount[0] will count pixel array acesses
    InstrName[1] = "bol_ops";
    InstrName[2] = "rlemem";
    // Name other counters here...
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
#define BOL_OPS InstrCount[1]
#define RLEMEM InstrCount[2]
// Add more macros here...

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!

/// Auxiliary (static) functions

/// Create the header of an image data structure
/// And allocate the array of pointers to RLE rows
static Image AllocateImageHeader(uint32 width, uint32 height) {
    assert(width > 0 && height > 0);
    Image newHeader = malloc(sizeof(struct image));
    check(newHeader != NULL, "malloc");

    newHeader->width = width;
    newHeader->height = height;

    // Allocating the array of pointers to RLE rows
    newHeader->row = malloc(height * sizeof(int *));
    check(newHeader->row != NULL, "malloc");

    return newHeader;
}

/// Allocate an array to store a RLE row with n elements
static int *AllocateRLERowArray(uint32 n) {
    assert(n > 2);
    int *newArray = malloc(n * sizeof(int));
    check(newArray != NULL, "malloc");

    return newArray;
}

/// Compute the number of runs of a non-compressed (RAW) image row
static uint32 GetNumRunsInRAWRow(uint32 image_width, const uint8 *RAW_row) {
    assert(image_width > 0);
    assert(RAW_row != NULL);

    // How many runs?
    uint32 num_runs = 1;
    for (uint32 i = 1; i < image_width; i++) {
        if (RAW_row[i] != RAW_row[i - 1]) {
            num_runs++;
        }
    }

    return num_runs;
}

/// Get the number of runs of a compressed RLE image row
static uint32 GetNumRunsInRLERow(const int *RLE_row) {
    assert(RLE_row != NULL);

    // Go through the RLE_row until EOR is found
    // Discard RLE_row[0], since it is a pixel color

    uint32 num_runs = 0;
    uint32 i = 1;
    while (RLE_row[i] != EOR) {
        num_runs++;
        i++;
    }

    return num_runs;
}

/// Get the number of elements of an array storing a compressed RLE image row
static uint32 GetSizeRLERowArray(const int *RLE_row) {
    assert(RLE_row != NULL);

    // Go through the array until EOR is found
    uint32 i = 0;
    while (RLE_row[i] != EOR) {
        i++;
    }

    return (i + 1);
}

/// Compress into RLE format a RAW image row
/// Allocates and returns the array storing the image row in RLE format
static int *CompressRow(uint32 image_width, const uint8 *RAW_row) {
    assert(image_width > 0);
    assert(RAW_row != NULL);

    // How many runs?
    uint32 num_runs = GetNumRunsInRAWRow(image_width, RAW_row);

    // Allocate the RLE row array
    int *RLE_row = malloc((num_runs + 2) * sizeof(int));
    check(RLE_row != NULL, "malloc");

    // Go through the RAW_row
    RLE_row[0] = (int)RAW_row[0]; // Initial pixel value
    uint32 index = 1;
    int num_pixels = 1;
    for (uint32 i = 1; i < image_width; i++) {
        if (RAW_row[i] != RAW_row[i - 1]) {
            RLE_row[index++] = num_pixels;
            num_pixels = 0;
        }
        num_pixels++;
    }
    RLE_row[index++] = num_pixels;
    RLE_row[index] = EOR; // Reached the end of the row

    return RLE_row;
}

static uint8 *UncompressRow(uint32 image_width, const int *RLE_row) {
    assert(image_width > 0);
    assert(RLE_row != NULL);

    // The uncompressed row
    uint8 *row = (uint8 *)malloc(image_width * sizeof(uint8));
    check(row != NULL, "malloc");

    // Go through the RLE_row until EOR is found
    int pixel_value = RLE_row[0];
    uint32 i = 1;
    uint32 dest_i = 0;
    while (RLE_row[i] != EOR) {
        // For each run
        for (int aux = 0; aux < RLE_row[i]; aux++) {
            row[dest_i++] = (uint8)pixel_value;
        }
        // Next run
        i++;
        pixel_value ^= 1;
    }

    return row;
}

// Add your auxiliary functions here...

/// Image management functions

/// Create a new BW image, either BLACK or WHITE.
///   width, height : the dimensions of the new image.
///   val: the pixel color (BLACK or WHITE).
/// Requires: width and height must be non-negative, val is either BLACK or
/// WHITE.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageCreate(uint32 width, uint32 height, uint8 val) {
    assert(width > 0 && height > 0);
    assert(val == WHITE || val == BLACK);

    Image newImage = AllocateImageHeader(width, height);

    // All image pixels have the same value
    int pixel_value = (int)val;

    // Creating the image rows, each row has just 1 run of pixels
    // Each row is represented by an array of 3 elements [value,length,EOR]
    for (uint32 i = 0; i < height; i++) {
        newImage->row[i] = AllocateRLERowArray(3);
        newImage->row[i][0] = pixel_value;
        newImage->row[i][1] = (int)width;
        newImage->row[i][2] = EOR;
    }

    return newImage;
}

/// Create a new BW image, with a perfect CHESSBOARD pattern.
///   width, height : the dimensions of the new image.
///   square_edge : the lenght of the edges of the sqares making up the
///   chessboard pattern.
///   first_value: the pixel color (BLACK or WHITE) of the
///   first image pixel.
/// Requires: width and height must be non-negative, val is either BLACK or
/// WHITE.
/// Requires: for the squares, width and height must be multiples of the
/// edge lenght of the squares
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageCreateChessboard(uint32 width, uint32 height, uint32 square_edge,
                            uint8 first_value) {
    assert(width > 0 && height > 0);
    assert(width % square_edge == 0 && height % square_edge == 0);

    // determina o número de colunas e linhas do tabuleiro
    uint32 num_cols = width / square_edge;

    Image chessboard = AllocateImageHeader(width, height);

    size_t board_size = sizeof(chessboard->row); // espaço que um quadrado ocupa

    for (uint32_t i = 0; i < height; i++) {
        chessboard->row[i] = AllocateRLERowArray(num_cols + 2);
        chessboard->row[i][num_cols + 1] = EOR;
        board_size += sizeof(chessboard->row[i]);
        board_size += sizeof(chessboard->row[i][0]);
        board_size += sizeof(chessboard->row[i][num_cols + 1]);
        // inicializa o primeiro elemento pixel
        if (i == 0)
            chessboard->row[0][0] = first_value;

        // verifica se faz o primeiro pixel da linha faz parte da quadrado
        // anterior, se fizer fica com o mesmo valor, se não fizer fica com o
        // valor contrário
        else
            chessboard->row[i][0] = (i % square_edge == 0)
                                        ? !chessboard->row[i - 1][0]
                                        : chessboard->row[i - 1][0];

        // adiciona o adiciona as runs à linha
        for (uint32 j = 1; j < num_cols + 1; j++) {
            chessboard->row[i][j] = square_edge;
            board_size += sizeof(chessboard->row[i][j]);
        }
    }
    // descomente para usar na função ChessTable()
    /*printf("|%13zu|%10d|%18d|%17d|\n", board_size, num_cols * height, width,*/
    /*       square_edge);*/

    return chessboard;
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail.
void ImageDestroy(Image *imgp) {
    assert(imgp != NULL);

    Image img = *imgp;

    for (uint32 i = 0; i < img->height; i++) {
        free(img->row[i]);
    }
    free(img->row);
    free(img);

    *imgp = NULL;
}

/// Printing on the console

/// Output the raw BW image
void ImageRAWPrint(const Image img) {
    assert(img != NULL);

    printf("width = %d height = %d\n", img->width, img->height);
    printf("RAW image:\n");

    // Print the pixels of each image row
    for (uint32 i = 0; i < img->height; i++) {
        // The value of the first pixel in the current row
        int pixel_value = img->row[i][0];
        for (uint32 j = 1; img->row[i][j] != EOR; j++) {
            // Print the current run of pixels
            for (int k = 0; k < img->row[i][j]; k++) {
                printf("%d", pixel_value);
            }
            // Switch (XOR) to the pixel value for the next run, if any
            pixel_value ^= 1;
        }
        // At current row end
        printf("\n");
    }
    printf("\n");
}

/// Output the compressed RLE image
void ImageRLEPrint(const Image img) {
    assert(img != NULL);

    printf("width = %d height = %d\n", img->width, img->height);
    printf("RLE encoding:\n");

    // Print the compressed rows information
    for (uint32 i = 0; i < img->height; i++) {
        uint32 j;
        for (j = 0; img->row[i][j] != EOR; j++) {
            printf("%d ", img->row[i][j]);
        }
        printf("%d\n", img->row[i][j]);
    }
    printf("\n");
}

/// PBM BW file operations

// See PBM format specification: http://netpbm.sourceforge.net/doc/pbm.html

// Auxiliary function
static void unpackBits(int nbytes, const uint8 bytes[], uint8 raw_row[]) {
    // bitmask starts at top bit
    int offset = 0;
    uint8 mask = 1 << (7 - offset);
    while (offset < 8) { // or (mask > 0)
        for (int b = 0; b < nbytes; b++) {
            raw_row[8 * b + offset] = (bytes[b] & mask) != 0;
        }
        mask >>= 1;
        offset++;
    }
}

// Auxiliary function
static void packBits(int nbytes, uint8 bytes[], const uint8 raw_row[]) {
    // bitmask starts at top bit
    int offset = 0;
    uint8 mask = 1 << (7 - offset);
    while (offset < 8) { // or (mask > 0)
        for (int b = 0; b < nbytes; b++) {
            if (offset == 0)
                bytes[b] = 0;
            bytes[b] |= raw_row[8 * b + offset] ? mask : 0;
        }
        mask >>= 1;
        offset++;
    }
}

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE *f) {
    char c;
    int i = 0;
    while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
        i++;
    }
    return i;
}

/// Load a raw PBM file.
/// Only binary PBM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageLoad(const char *filename) { ///
    int w, h;
    char c;
    FILE *f = NULL;
    Image img = NULL;

    check((f = fopen(filename, "rb")) != NULL, "Open failed");
    // Parse PBM header
    check(fscanf(f, "P%c ", &c) == 1 && c == '4', "Invalid file format");
    skipComments(f);
    check(fscanf(f, "%d ", &w) == 1 && w >= 0, "Invalid width");
    skipComments(f);
    check(fscanf(f, "%d", &h) == 1 && h >= 0, "Invalid height");
    check(fscanf(f, "%c", &c) == 1 && isspace(c), "Whitespace expected");

    // Allocate image
    img = AllocateImageHeader(w, h);

    // Read pixels
    int nbytes = (w + 8 - 1) / 8; // number of bytes for each row
    // using VLAs...
    uint8 bytes[nbytes];
    uint8 raw_row[nbytes * 8];
    for (uint32 i = 0; i < img->height; i++) {
        check(fread(bytes, sizeof(uint8), nbytes, f) == (size_t)nbytes,
              "Reading pixels");
        unpackBits(nbytes, bytes, raw_row);
        img->row[i] = CompressRow(w, raw_row);
    }

    fclose(f);
    return img;
}

/// Save image to PBM file.
/// On success, returns unspecified integer. (No need to check!)
/// On failure, does not return, EXITS program!
int ImageSave(const Image img, const char *filename) { ///
    assert(img != NULL);
    int w = img->width;
    int h = img->height;
    FILE *f = NULL;

    check((f = fopen(filename, "wb")) != NULL, "Open failed");
    check(fprintf(f, "P4\n%d %d\n", w, h) > 0, "Writing header failed");

    // Write pixels
    int nbytes = (w + 8 - 1) / 8; // number of bytes for each row
    // using VLAs...
    uint8 bytes[nbytes];
    // unit8 raw_row[nbytes*8];
    for (uint32 i = 0; i < img->height; i++) {
        // UncompressRow...
        uint8 *raw_row = UncompressRow(nbytes * 8, img->row[i]);
        // Fill padding pixels with WHITE
        memset(raw_row + w, WHITE, nbytes * 8 - w);
        packBits(nbytes, bytes, raw_row);
        size_t written = fwrite(bytes, sizeof(uint8), nbytes, f);
        check(written == (size_t)nbytes, "Writing pixels failed");
        free(raw_row);
    }

    // Cleanup
    fclose(f);
    return 0;
}

/// Information queries

/// Get image width
int ImageWidth(const Image img) {
    assert(img != NULL);
    return img->width;
}

/// Get image height
int ImageHeight(const Image img) {
    assert(img != NULL);
    return img->height;
}

/// Image comparison

int ImageIsEqual(const Image img1, const Image img2) {
    assert(img1 != NULL && img2 != NULL);

    // verificar se dimensões das imagens são diferentes
    if (img1->width != img2->width || img1->height != img2->height) {
        return 0; // imagens diferentes
    }

    // itera pelas linhas (pixels em altura) das imagens
    for (uint32 i = 0; i < img1->height; i++) {
        // arrays das linhas codificadas de ambas as imagens
        int *row1 = img1->row[i];
        int *row2 = img2->row[i];

        // verificar se tamanho dos arrays RLE das linhas é diferente
        if (GetSizeRLERowArray(row1) != GetSizeRLERowArray(row2)) {
            return 0; // diferentes
        }

        // itera pelos elementos (pixels codificados) nas linhas até encontrar o
        // marcador EOR (end of row)
        for (uint32 j = 0; row1[j] != EOR; j++) {
            if (row1[j] != row2[j]) {
                return 0; // diferentes
            }
        }
    }

    // se não houver diferenças, retorna imagens iguais
    return 1;
}

int ImageIsDifferent(const Image img1, const Image img2) {
    assert(img1 != NULL && img2 != NULL);
    return !ImageIsEqual(img1, img2);
}

/// Boolean Operations on image pixels

/// These functions apply boolean operations to images,
/// returning a new image as a result.
///
/// Operand images are left untouched and must be of the same size.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)

Image ImageNEG(const Image img) {
    assert(img != NULL);

    uint32 width = img->width;
    uint32 height = img->height;

    Image newImage = AllocateImageHeader(width, height);

    // Directly copying the rows, one by one
    // And changing the value of row[i][0]

    for (uint32 i = 0; i < height; i++) {
        uint32 num_elems = GetSizeRLERowArray(img->row[i]);
        newImage->row[i] = AllocateRLERowArray(num_elems);
        memcpy(newImage->row[i], img->row[i], num_elems * sizeof(int));
        newImage->row[i][0] ^=
            1; // Just negate the value of the first pixel run
    }

    return newImage;
}

Image ImageAND_Uncompressed(const Image img1, const Image img2) {
    assert(img1 != NULL && img2 != NULL);
    check(img1->height == img2->height && img1->width == img2->width, "size");

    // primeiro método: verificar pixel a pixel
    int width = ImageWidth(img1);
    int height = ImageHeight(img1);
    Image new_image = AllocateImageHeader(width, height);

    // inicializar as linhas necessárias
    uint8 *row1;
    uint8 *row2;
    uint8 *new_row = (uint8 *)malloc(sizeof(uint8) * width);
    int *comp_row;
    InstrReset();
    for (int i = 0; i < height; i++) {
        // descomprimir as linhas para poder iterar pelos pixeis dessa linha
        row1 = UncompressRow(width, img1->row[i]);
        row2 = UncompressRow(width, img2->row[i]);

        // o pixel da nova imagem fica com valor = 1 se ambos os valores
        // forem 1, caso contrário fica com valor = 0
        for (int j = 0; j < width; j++) {
            new_row[j] = (row1[j] & row2[j]);
            PIXMEM += sizeof(new_row[j]);
            BOL_OPS++;
        }

        // comprimir para poder adicionar à nova imagem
        comp_row = CompressRow(width, new_row);
        new_image->row[i] = comp_row;
        PIXMEM += sizeof(new_image->row[i]);
        // liberta o espaço alocado para cada linha
        free(row1);
        free(row2);
    }
    PIXMEM += sizeof(new_image->row);
    free(new_row);

    // descomentar para dar os prints da tabela da função ANDTable()
    /*printf("|%19lu|%12d|%11d|\n", BOL_OPS, height, width);*/

    return new_image;
}

Image ImageAND_Without_Uncompress(const Image img1, const Image img2) {
    assert(img1 != NULL && img2 != NULL);
    check(img1->height == img2->height && img1->width == img2->width, "size");

    // segundo método: comparar usando as runs
    int width = ImageWidth(img1);
    int height = ImageHeight(img1);
    Image new_image = AllocateImageHeader(width, height);

    int new_val;
    int run1 = 0, run2 = 0, min_run;
    int num_runs1 = 0;
    int num_runs2 = 0;
    int *temp_row = (int *)malloc(
        sizeof(int) * (width + 2)); // run temporária com o tamanho do pior caso

    if (temp_row == NULL) {
        printf("Error creating row\n");
        return NULL;
    }

    // reseta os contadores
    InstrReset();

    for (int i = 0; i < height; i++) {

        // inicializar as variáveis
        int size = 1;
        int j = 1, k = 1;
        int val1 = img1->row[i][0];
        int val2 = img2->row[i][0];
        int current_val = val2 & val1;
        temp_row[0] = current_val;
        int current_run = 0;
        num_runs1 += GetNumRunsInRLERow(img1->row[i]);
        num_runs2 += GetNumRunsInRLERow(img2->row[i]);
        while (img1->row[i][j] != EOR || img2->row[i][k] != EOR) {

            if (run1 == 0)
                run1 = img1->row[i][j];

            if (run2 == 0)
                run2 = img2->row[i][k];

            // ver qual é a run mais pequena
            min_run = (run1 < run2) ? run1 : run2;

            // valor do "and" das duas runs
            new_val = (val1 ^ (j % 2 == 0)) & (val2 ^ (k % 2 == 0));
            BOL_OPS++;

            // começar uma nova run
            if (new_val != current_val) {
                if (current_run > 0)
                    temp_row[size++] = current_run;
                current_run = min_run;
                current_val = new_val;

            } else // aumentar a run atual
                current_run += min_run;

            run1 -= min_run;
            run2 -= min_run;

            if (run1 == 0)
                j++;

            if (run2 == 0)
                k++;
        }

        if (current_run > 0)
            temp_row[size++] = current_run;

        temp_row[size++] = -1;

        // alocar apenas o espaço necessário
        new_image->row[i] = (int *)malloc(sizeof(int) * size);
        if (new_image->row[i] == NULL) {
            printf("Error creating row\n");
            return NULL;
        }

        // copiar a linha para a nova imagem
        memcpy(new_image->row[i], temp_row, size * sizeof(int));
    }

    // calcular a memoria ocupada
    RLEMEM += sizeof(new_image->row);
    for (int i = 0; i < height; i++) {
        RLEMEM += sizeof(new_image->row[i]);
        int runs = GetSizeRLERowArray(new_image->row[i]);
        for (int j = 0; j < runs; j++)
            RLEMEM += sizeof(new_image->row[i][j]);
    }
    free(temp_row);

    // descomentar para dar os prints da tabela da função ANDTable()

    // tabela para o Melhor e pior caso
    /*printf("|%14lu|%11d|%11d|%8d|%7d|\n", BOL_OPS, num_runs1, num_runs2, height,*/
    /*       width);*/

    // tabela para o caso médio
    printf("|%14lu|%11d|%11d|%8.3f|%7d|\n", BOL_OPS, num_runs1, num_runs2,
           (double)num_runs2 / height, width);

    return new_image;
}

Image ImageAND(const Image img1,
               const Image img2) { // Comentar a função que não se quer usar,
                                   // pois não podemos modificar o imageBW.h
    /*return ImageAND_Uncompressed(img1, img2);*/
    return ImageAND_Without_Uncompress(img1, img2);
}

Image ImageOR(const Image img1, const Image img2) {
    assert(img1 != NULL && img2 != NULL);
    assert(img1->height == img2->height && img1->width == img2->width);

    // obter dimensões das imagens
    uint32 width = img1->width;
    uint32 height = img1->height;

    // criar uma nova imagem para armazenar o resultado
    Image result = AllocateImageHeader(width, height);

    // itera pelas linhas (altura)
    for (uint32 i = 0; i < height; i++) {
        // Descomprimir as linhas de ambas as imagens para arrays "raw_row1" e
        // "raw_row2"
        uint8 *raw_row1 = UncompressRow(width, img1->row[i]);
        uint8 *raw_row2 = UncompressRow(width, img2->row[i]);

        // aloca memória para armazenar a linha resultante da operação OR
        uint8 *raw_result = malloc(width * sizeof(uint8));
        check(raw_result != NULL, "malloc");

        // iterar por cada pixel na linha descomprimida
        for (uint8 j = 0; j < width; j++) {
            // operação OR bit a bit entre os pixels correspondentes das imagens
            // e armazena no array raw_result
            raw_result[j] = raw_row1[j] | raw_row2[j];
        }

        // comprime raw_result de novo para o formato RLE
        result->row[i] = CompressRow(width, raw_result);

        // liberta a memória
        free(raw_row1);
        free(raw_row2);
        free(raw_result);
    }

    return result;
}

Image ImageXOR(Image img1, Image img2) {
    assert(img1 != NULL && img2 != NULL);
    check(img1->height == img2->height && img1->width == img2->width, "size");
    // COMPLETE THE CODE
    // You might consider using the UncompressRow and CompressRow auxiliary
    // files Or devise a more efficient alternative

    int width = ImageWidth(img1);
    int height = ImageHeight(img1);

    // inicializar as linhas necessárias
    uint8 *row1;
    uint8 *row2;
    uint8 *new_row = (uint8 *)malloc(sizeof(uint8) * width);
    int *comp_row;

    Image new_image = AllocateImageHeader(width, height);
    for (int i = 0; i < height; i++) {
        // descomprimir as linhas para poder iterar pelos pixeis dessa linha
        row1 = UncompressRow(width, img1->row[i]);
        row2 = UncompressRow(width, img2->row[i]);

        // O pixel da nova imagem fica com valor = 1 se ambos os valores
        // forem 1, caso contrário fica com valor = 0
        for (int j = 0; j < width; j++) {
            new_row[j] = (row1[j] ^ row2[j]);
        }

        // comprimir para poder adicionar à nova imagem
        comp_row = CompressRow(width, new_row);
        new_image->row[i] = comp_row;

        // liberta o espaço alocado para cada linha
        free(row1);
        free(row2);
    }
    free(new_row);
    return new_image;
}

/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)

/// Mirror an image = flip top-bottom.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageHorizontalMirror(const Image img) {
    assert(img != NULL);

    uint32 width = img->width;
    uint32 height = img->height;

    Image newImage = AllocateImageHeader(width, height);

    for (uint32 i = 0; i < height; i++) {
        // index da linha corresponde à linha i da imagem invertida na imagem
        // normal
        int index = height - 1 - i;
        int size = GetSizeRLERowArray(img->row[index]);
        newImage->row[i] = AllocateRLERowArray(size);

        // copia a linha da imagem original para a linha da imagem
        // invertida
        for (int j = 0; j < size; j++) {
            newImage->row[i][j] = img->row[index][j];
        }
    }

    return newImage;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageVerticalMirror(const Image img) {
    assert(img != NULL);

    uint32 width = img->width;
    uint32 height = img->height;

    Image newImage = AllocateImageHeader(width, height);

    // itera sobre cada linha da imagem original
    for (uint32 i = 0; i < height; i++) {
        // descomprimir a linha
        uint8 *raw_row = UncompressRow(width, img->row[i]);

        // alocar memória para a imagem espelhada
        uint8 *mirror_row = malloc(width * sizeof(uint8));
        check(mirror_row != NULL, "malloc");

        // espelhar a linha copiando os pixels na ordem inversa
        for (uint32 j = 0; j < width; j++) {
            mirror_row[j] = raw_row[width - 1 - j];
        }

        // comprime a linha espelhada e adiciona à nova imagem
        newImage->row[i] = CompressRow(width, mirror_row);

        // libertar a memória
        free(raw_row);
        free(mirror_row);
    }

    return newImage;
}

/// Replicate img2 at the bottom of imag1, creating a larger image
/// Requires: the width of the two images must be the same.
/// Returns the new larger image.
/// Ensures: The original images are not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageReplicateAtBottom(const Image img1, const Image img2) {
    assert(img1 != NULL && img2 != NULL);
    assert(img1->width == img2->width);

    uint32 new_width = img1->width;
    uint32 new_height = img1->height + img2->height;

    Image newImage = AllocateImageHeader(new_width, new_height);

    uint32 height1 = img1->height;
    int size;
    for (uint32 i = 0; i < new_height; i++) {
        // verifica se a linha i faz parte da img1 ou da img2
        if (i < height1) {

            size = GetSizeRLERowArray(img1->row[i]);
            newImage->row[i] = AllocateRLERowArray(size);
            // copia a linha da img1 para a nova imagem
            for (int j = 0; j < size; j++)
                newImage->row[i][j] = img1->row[i][j];

        } else {

            size = GetSizeRLERowArray(img2->row[i - height1]);
            newImage->row[i] = AllocateRLERowArray(size);
            // copia a linha da img2 para a nova imagem
            for (int j = 0; j < size; j++)
                newImage->row[i][j] = img2->row[i - height1][j];
        }
    }

    return newImage;
}

/// Replicate img2 to the right of imag1, creating a larger image
/// Requires: the height of the two images must be the same.
/// Returns the new larger image.
/// Ensures: The original images are not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageReplicateAtRight(const Image img1, const Image img2) {
    assert(img1 != NULL && img2 != NULL);
    assert(img1->height == img2->height);

    uint32 new_width = img1->width + img2->width;
    uint32 new_height = img1->height;

    Image newImage = AllocateImageHeader(new_width, new_height);

    for (uint32 i = 0; i < new_height; i++) {
        int *rle_row1 = img1->row[i];
        int *rle_row2 = img2->row[i];

        uint32 num_runs1 = GetNumRunsInRLERow(rle_row1);
        uint32 num_runs2 = GetNumRunsInRLERow(rle_row2);

        int initial_pixel1 = rle_row1[0];
        int initial_pixel2 = rle_row2[0];

        int last_pixel1 =
            (num_runs1 % 2 == 1) ? initial_pixel1 : !initial_pixel1;
        // se ímpar -> last_pixel1 = inicial_pixel1
        // se par -> last_pixel1 = contrário do inicial_pixel1

        int merge_runs = (last_pixel1 ==
                          initial_pixel2); // ver se a segunda imagem começa com
                                           // a mesma "cor" que acaba a primeira

        // alocar memoria para o novo rle row
        uint32 new_max_size =
            num_runs1 + num_runs2 +
            2; //+2 porque é o bit inicial e o bit terminador (-1)
        int *new_rle_row = malloc(new_max_size * sizeof(int));
        check(new_rle_row != NULL, "malloc");

        uint32 new_index = 0;
        new_rle_row[new_index++] = initial_pixel1;

        //
        uint32 index1 = 1;
        while (rle_row1[index1] != EOR) { // EOR -> end of row == -1
            new_rle_row[new_index++] = rle_row1[index1++];
        }

        uint32 index2 = 1;

        if (merge_runs) {
            new_rle_row[new_index - 1] +=
                rle_row2[index2++]; // unir (merge) ultima run da img1 com a
                                    // primeira da img2
        }

        // copiar os restantes runs da row da img2
        while (rle_row2[index2] != EOR) {
            new_rle_row[new_index++] = rle_row2[index2++];
        }

        new_rle_row[new_index++] = EOR;

        new_rle_row = realloc(new_rle_row, new_index * sizeof(int));
        check(new_rle_row != NULL, "realloc");

        newImage->row[i] = new_rle_row;
    }

    return newImage;
}
