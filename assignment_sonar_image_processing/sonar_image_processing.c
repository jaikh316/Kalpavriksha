#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_MATRIX_SIZE 2
#define MAX_MATRIX_SIZE 10
#define MAX_INTENSITY 255
#define INTENSITY_RANGE (MAX_INTENSITY + 1) 
#define NEIGHBOR_RADIUS 1                    
#define PRINT_FIELD_WIDTH 3

void releaseMatrix(int **matrix, int **cellsEnd)
{
    int **cellPointer;
    for(cellPointer=matrix; cellPointer < cellsEnd; cellPointer++) {
      free(*cellPointer);
    }
    free(matrix);
}

void freeMatrix(int **matrix, int size) {
  releaseMatrix(matrix, matrix + size * size);
}

int readMatrixSize(int *size)
{
    printf("Enter matrix size(%d-%d): ", MIN_MATRIX_SIZE, MAX_MATRIX_SIZE);
    if (scanf("%d", size) != 1)
        return 0;
    return (*size >= MIN_MATRIX_SIZE && *size <= MAX_MATRIX_SIZE);
}

int **createMatrix(int size)
{
    int **cellPointer;
    int **matrix = malloc((size_t)size * size * sizeof(int *));
    int **matrixEnd;

    if (matrix == NULL)
        return NULL;

    matrixEnd = matrix + size * size;
    for (cellPointer = matrix; cellPointer < matrixEnd; cellPointer++)
    {
        *cellPointer = malloc(sizeof(int));
        if(*cellPointer== NULL)
        {
            releaseMatrix(matrix, cellPointer); 
            return NULL;
        }
        **cellPointer = rand() % INTENSITY_RANGE;
    }
    return matrix;
}


void printMatrix(int **matrix, int size)
{
    int **cellPointer;
    int **matrixEnd = matrix + size * size;
    int cellsPrintedInRow = 0;

    for (cellPointer = matrix; cellPointer < matrixEnd; cellPointer++)
    {
        printf("%*d ", PRINT_FIELD_WIDTH, **cellPointer);
        cellsPrintedInRow++;
        if (cellsPrintedInRow == size)
        {
            printf("\n");
            cellsPrintedInRow = 0;
        }
    }
}

void printStage(const char *title, int **matrix, int size)
{
    printf("%s\n", title);
    printMatrix(matrix, size);
}


void swapAddresses(int **first, int **second)
{
    int *swapAddress = *first;  
    *first  = *second;
    *second = swapAddress;
}

void transposeMatrix(int **matrix, int size)
{
    int row, column;
    for (row = 0; row < size; row++)
    {
        for (column = row + 1; column < size; column++)
        {
            swapAddresses(matrix + row * size + column, matrix + column * size + row);
        }
    }
}

void reverseEachRow(int **matrix, int size)
{
    int **leftSlot, **rightSlot;
    int row;
    for (row = 0; row < size; row++)
    {
        leftSlot = matrix + row * size;
        rightSlot = leftSlot + size - 1;

        while (leftSlot < rightSlot)
        {
            swapAddresses(leftSlot, rightSlot);
            leftSlot++;
            rightSlot--;
        }
    }
}

void rotateMatrixClockwise(int **matrix, int size) {
    transposeMatrix(matrix, size);
    reverseEachRow(matrix, size);
}


int isInsideMatrix(int row, int column, int size) {
  return (row >= 0 && row < size && column >= 0 && column < size);
}

int averageAroundCell(int **matrix, int size, int row, int column)
{
    int rowOffset, columnOffset;
    int neighborRow, neighborColumn;
    int neighborSum = 0;
    int neighborCount = 0;

    for (rowOffset = -NEIGHBOR_RADIUS; rowOffset <= NEIGHBOR_RADIUS; rowOffset++)
    {
        for (columnOffset = -NEIGHBOR_RADIUS; columnOffset <= NEIGHBOR_RADIUS; columnOffset++)
        {
            neighborRow = row + rowOffset;
            neighborColumn = column + columnOffset;

            if (isInsideMatrix(neighborRow, neighborColumn, size))
            {
                neighborSum += **(matrix + neighborRow * size + neighborColumn) % INTENSITY_RANGE;
                neighborCount++;
            }
        }
    }
    return neighborSum / neighborCount;
}

void encodeAveragesInCells(int **matrix, int size)
{
    int row, column;
    int **currentCell;

    for (row = 0; row < size; row++)
    {
        for (column = 0; column < size; column++)
        {
            currentCell = matrix + row * size + column;
            **currentCell = (**currentCell % INTENSITY_RANGE)
                          + averageAroundCell(matrix, size, row, column) * INTENSITY_RANGE;
        }
    }
}

void keepOnlyAverages(int **matrix, int size)
{
    int **currentCell;
    int **matrixEnd = matrix + size * size;

    for (currentCell = matrix; currentCell < matrixEnd; currentCell++)
        **currentCell = **currentCell / INTENSITY_RANGE;
}

void applySmoothingFilter(int **matrix, int size) {
  encodeAveragesInCells(matrix, size);
  keepOnlyAverages(matrix, size);
}

int main(void)
{
    int matrixSize;
    int **matrix;
    if (!readMatrixSize(&matrixSize))
    {
        printf("Invalid matrix size.\n");
        return 1;
    }

    srand((unsigned int)time(NULL));
    matrix = createMatrix(matrixSize);
    if (matrix == NULL)
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    printStage("Original Randomly Generated Matrix:", matrix, matrixSize);
    rotateMatrixClockwise(matrix, matrixSize);
    printStage("Matrix after 90° Clockwise Rotation:", matrix, matrixSize);
    applySmoothingFilter(matrix, matrixSize);
    printStage("Matrix after Applying 3×3 Smoothing Filter:", matrix, matrixSize);

    freeMatrix(matrix, matrixSize);
    return 0;
}