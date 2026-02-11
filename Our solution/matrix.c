#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

Matrix* create_matrix(int n)
{
    Matrix *m = malloc(sizeof(Matrix));
    m->n = n;
    m->data = malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++)
    m->data[i] = malloc(n * sizeof(double));
    return m;
}

void free_matrix(Matrix *m)
{
    for(int i=0;i<m->n;i++) free(m->data[i]);
    free(m->data);
    free(m);
}

void fill_random(Matrix *m)
{
    for(int i=0;i<m->n;i++)
    {
        for(int j=0;j<m->n;j++)
        m->data[i][j] = ((double)rand() / RAND_MAX)*10.0 - 5.0;
    }
}

Matrix* copy_matrix(Matrix *src)
{
    Matrix *dest = create_matrix(src->n);
    for(int i=0;i<src->n;i++)
    {
        for(int j=0;j<src->n;j++)
        dest->data[i][j] = src->data[i][j];
    }
    return dest;
}

void replace_column(Matrix *m, double *vec, int col_idx)
{
    for (int i = 0; i < m->n; i++)
    m->data[i][col_idx] = vec[i];
}
