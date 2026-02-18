#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t u32;
typedef float f32;

typedef struct
{
    u32 rows, cols;

    // row-major
    f32* data;

    // We store one big array and index it instead of multiple arrays and have
    // each being the rows
    /*
     * Logically
     * [1,2]
     * [3,4]
     * [5,6]
     *
     * "Physically"
     * [ 1, 2,  3, 4, 5, 6]
     */
} matrix;

// Rows first
f32 mat_sum0(const matrix* mat)
{
    f32 sum = 0.0f;

    for (u32 row = 0; row < mat->rows; row++)
    {
        for (u32 col = 0; col < mat->cols; col++)
        {
            sum += mat->data[col + row * mat->cols];
        }
    }
    return sum;
}

// Columns first
f32 mat_sum1(const matrix* mat)
{
    f32 sum = 0.0f;

    for (u32 col = 0; col < mat->cols; col++)
    {
        for (u32 row = 0; row < mat->rows; row++)
        {
            sum += mat->data[col + row * mat->cols];
        }
    }
    return sum;
}

/*
i, j, k -> ~1700 ms mid
i, k, j -> ~110 ms fastest
j, k, i -> ~5000 ms slowest
*/

void mat_mul(matrix* out, const matrix* a, const matrix* b)
{
    out->rows = a->rows;
    out->cols = b->cols;
}

#define SIZE 1024
int main(void)
{
    matrix a = {SIZE, SIZE};
    a.data = (f32*)malloc(SIZE * SIZE * sizeof(f32));

    for (u32 i = 0; i < 1000; i++)
    {
        a.data[i] = (f32)rand() / (f32)RAND_MAX;
    }

    f32 s = 0.0f;

    for (u32 i = 0; i < 100; i++)
    {
        s += mat_sum1(&a);
    }

    printf("%f\n", s);

    free(a.data);
    return 0;
}
