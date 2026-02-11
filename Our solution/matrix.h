#ifndef MATRIX_H
#define MATRIX_H

typedef struct
{
    int n;
    double **data;
}Matrix;

Matrix* create_matrix(int n);
void free_matrix(Matrix *m);
void fill_random(Matrix *m);
Matrix* copy_matrix(Matrix *src);
void replace_column(Matrix *m, double *vec, int col_idx);

#endif
