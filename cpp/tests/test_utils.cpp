#include <iostream>
#include "utils.h"

template<class T>
void printSystem(T *matrix, T *vector, int rows, int columns) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            std::cout << matrix[i * columns + j] << " ";
        }
        std::cout << "| " << vector[i] << std::endl;
    }
}

void testGaussianElimination() {
    int mat1[5][5] = {{1, 1, 0, 0, 0},
                        {1, 1, 1, 0, 0},
                        {0, 1, 1, 1, 0},
                        {0, 0, 1, 1, 1},
                        {0, 0, 0, 1, 1}};
    int vec1[5] = {1, 2, 2, 2, 1};
    makeGaussianElimination<int>(&mat1[0][0], vec1, 5, 5);
    printSystem(&mat1[0][0], vec1, 5, 5);

    int mat2[4][4] = {{1, 1, 0, 0}, {1, 1, 1, 0}, {0, 1, 1, 1}, {0, 0, 1, 1}};
    int vec2[4] = {1, 1, 2, 1};
    makeGaussianElimination(&mat2[0][0], vec2, 4, 4);
    printSystem(&mat2[0][0], vec2, 4, 4);

    int mat3[3][3] = {{1, 0, 1}, {0, 1, 1}, {1, 1, 0}};
    int vec3[3] = {0, 0, 0};
    makeGaussianElimination(&mat3[0][0], vec3, 3, 3);
    printSystem(&mat3[0][0], vec3, 3, 3);
}

int main() {
    testGaussianElimination();
    return 0;
}