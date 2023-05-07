#include <algorithm>

template<class T>
void makeGaussianElimination(T *matrix, T *vector, size_t rows, size_t columns) {
    int i = 0;
    int j = 0;

    while (i < rows && j < columns) {
        int iMax = 0;
        for (int k = i; k < rows; ++k) {
            if (matrix[k * columns + j] == 1 || matrix[k * columns + j] == -1) {
                iMax = k;
                break;
            }
        }

        if (matrix[iMax * columns + j]) {
            if (iMax != i) {
                for (int k = 0; k < columns; ++k) {
                    std::swap(matrix[iMax * columns + k], matrix[i * columns + k]);
                }
                std::swap(vector[iMax], vector[i]);
            }
            for (int k = i + 1; k < rows; ++k) {
                int scale = matrix[k * columns + j] / matrix[i * columns + j];
                if (scale != 0) {
                    for (int m = j; m < columns; ++m) {
                        matrix[k * columns + m] -= scale * matrix[i * columns + m];
                    }
                    vector[k] -= scale * vector[i];
                }
            }
            ++i;
        }
        ++j;
    }
}
