#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_MATRIX_SIZE 2
#define MAX_MATRIX_SIZE 10
#define MAX_INTENSITY 255
#define INTENSITY_RANGE (MAX_INTENSITY + 1)
#define NEIGHBOR_RADIUS 1
#define PRINT_FIELD_WIDTH 3

typedef struct {
    int **cells;
    int *values;
    int size;
} SonarMatrix;

int readMatrixSize(int *matrixSize) {
    printf("Enter matrix size (%d-%d): ", MIN_MATRIX_SIZE, MAX_MATRIX_SIZE);
    if(scanf("%d", matrixSize) != 1) {
        return 0;
    }
    return (*matrixSize >= MIN_MATRIX_SIZE && *matrixSize <= MAX_MATRIX_SIZE);
}

SonarMatrix createMatrix(int matrixSize) {
    SonarMatrix matrix;

    int **currentCell;
    int cellIndex;
    int totalCells = matrixSize * matrixSize;
    matrix.size = matrixSize;
    matrix.cells = malloc((size_t)totalCells * sizeof(int *));
    matrix.values = malloc((size_t)totalCells * sizeof(int));

    if(matrix.cells == NULL || matrix.values == NULL) {
        free(matrix.cells);
        free(matrix.values);
        matrix.cells = NULL;
        matrix.values = NULL;
        return matrix;
    }

    for(currentCell = matrix.cells, cellIndex = 0; currentCell < matrix.cells + totalCells; currentCell++, cellIndex++) {
        *currentCell = matrix.values + cellIndex;
    }
    return matrix;
}

void generateRandomMatrix(SonarMatrix *matrix) {
    int **currentCell;
    int **matrixEnd = matrix->cells + matrix->size * matrix->size;

    for(currentCell = matrix->cells; currentCell < matrixEnd; currentCell++) {
        **currentCell = rand() % INTENSITY_RANGE;
    }
}

void printMatrix(SonarMatrix *matrix) {
    int **currentCell;
    int **matrixEnd = matrix->cells + matrix->size * matrix->size;

    int cellsPrintedInRow = 0;
    for(currentCell = matrix->cells; currentCell < matrixEnd; currentCell++) {
        printf("%*d ", PRINT_FIELD_WIDTH, **currentCell);
        cellsPrintedInRow++;
        if(cellsPrintedInRow == matrix->size) {
            printf("\n");
            cellsPrintedInRow = 0;
        }
    }
    printf("\n");
}

void printStage(const char *title, SonarMatrix *matrix) {
    printf("%s\n", title);
    printMatrix(matrix);
}

void freeMatrix(SonarMatrix *matrix) {
    free(matrix->values);
    free(matrix->cells);

    matrix->values = NULL;
    matrix->cells = NULL;
}

void swapAddresses(int **first, int **second) {
    int *temporaryAddress = *first;
    *first = *second;
    *second = temporaryAddress;
}

void transposeMatrix(SonarMatrix *matrix) {
    int row;
    int column;
    int matrixSize = matrix->size;

    for(row = 0; row < matrixSize; row++) {
        for(column = row+1; column < matrixSize; column++) {
            swapAddresses(matrix->cells + row * matrixSize + column, matrix->cells + column * matrixSize + row);
        }
    }
}

void reverseEachRow(SonarMatrix *matrix) {
    int row;
    int matrixSize = matrix->size;
    int **leftCell;
    int **rightCell;

    for(row = 0; row < matrixSize; row++) {
        leftCell = matrix->cells + row * matrixSize;
        rightCell = leftCell + matrixSize - 1;

        while(leftCell < rightCell) {
            swapAddresses(leftCell, rightCell);
            leftCell++;
            rightCell--;
        }
    }
}

void rotateMatrixClockwise(SonarMatrix *matrix) {
    transposeMatrix(matrix);
    reverseEachRow(matrix);
}

int isInsideMatrix(int row, int column, int matrixSize) {
    return (row >= 0 && row< matrixSize && column >=0 && column < matrixSize);
}

int averageAroundCell(SonarMatrix *matrix, int row, int column) {
    int rowOffset;
    int columnOffset;
    int neighborRow;
    int neighborColumn;
    int neighborSum = 0;
    int neighborCount = 0;
    int matrixSize =matrix->size;

    for(rowOffset = -NEIGHBOR_RADIUS; rowOffset <= NEIGHBOR_RADIUS; rowOffset++) {
        for(columnOffset = -NEIGHBOR_RADIUS; columnOffset <= NEIGHBOR_RADIUS; columnOffset++) {
            neighborRow = row + rowOffset;
            neighborColumn = column + columnOffset;

            if(isInsideMatrix(neighborRow, neighborColumn, matrixSize)) {
                int **neighborCell = matrix->cells + neighborRow * matrixSize + neighborColumn;
                neighborSum += **neighborCell % INTENSITY_RANGE;
                neighborCount++;
            }
        }
    }
    return neighborSum / neighborCount;
}

void encodeAveragesInCells(SonarMatrix *matrix) {
    int row;
    int column;
    int matrixSize = matrix->size;
    int **currentCell;

    for(row = 0; row < matrixSize; row++) {
        for(column = 0; column < matrixSize; column++) {
            currentCell = matrix->cells + row * matrixSize + column;

            **currentCell = (**currentCell % INTENSITY_RANGE) + averageAroundCell(matrix, row, column) * INTENSITY_RANGE;
        }
    }
}

void keepOnlyAverages(SonarMatrix *matrix) {
    int **currentCell;
    int **matrixEnd = matrix->cells + matrix->size * matrix->size;
    for (currentCell = matrix->cells; currentCell < matrixEnd; currentCell++) {
        **currentCell /= INTENSITY_RANGE;
    }
}

void applySmoothingFilter(SonarMatrix *matrix) {
    encodeAveragesInCells(matrix);
    keepOnlyAverages(matrix);
}

int main(void) {
    int matrixSize;
    SonarMatrix sonarMatrix;

    srand((unsigned int)time(NULL));
    if(!readMatrixSize(&matrixSize)) {
        printf("Invalid matrix size.\n");
        return 1;
    }

    sonarMatrix = createMatrix(matrixSize);
    if(sonarMatrix.cells == NULL || sonarMatrix.values == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }
    generateRandomMatrix(&sonarMatrix);
    printStage("Original Randomly Generated Matrix:", &sonarMatrix);

    rotateMatrixClockwise(&sonarMatrix);
    printStage("Matrix after 90-degree Clockwise Rotation:", &sonarMatrix);

    applySmoothingFilter(&sonarMatrix);
    printStage("Matrix after Applying 3x3 Smoothing Filter:", &sonarMatrix);

    freeMatrix(&sonarMatrix);
    return 0;
}