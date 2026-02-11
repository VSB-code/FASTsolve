/*
 * Solution for Cramer's Rule with Multiprocessing
 * * Description:
 * This program solves a system of linear equations using Cramer's Rule.
 * It parallelizes the computation of determinants using fork() to utilize
 * multiple CPU cores. It benchmarks performance from 1 to N cores.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <sys/wait.h>
 #include <time.h>
 #include <math.h>
 #include <string.h>
 
 // --- Matrix Structure ---
 typedef struct {
     int size;
     double **data;
 } Matrix;
 
 // --- Helper Functions ---
 
 // Allocate memory for a matrix
 Matrix* create_matrix(int size) {
     Matrix *m = (Matrix*)malloc(sizeof(Matrix));
     m->size = size;
     m->data = (double**)malloc(size * sizeof(double*));
     for (int i = 0; i < size; i++) {
         m->data[i] = (double*)malloc(size * sizeof(double));
     }
     return m;
 }
 
 // Free memory
 void free_matrix(Matrix *m) {
     for (int i = 0; i < m->size; i++) {
         free(m->data[i]);
     }
     free(m->data);
     free(m);
 }
 
 // Copy matrix
 Matrix* copy_matrix(Matrix *src) {
     Matrix *dst = create_matrix(src->size);
     for (int i = 0; i < src->size; i++) {
         for (int j = 0; j < src->size; j++) {
             dst->data[i][j] = src->data[i][j];
         }
     }
     return dst;
 }
 
 // Compute Determinant using Gaussian Elimination (O(N^3))
 double determinant(Matrix *m) {
     Matrix *temp = copy_matrix(m);
     double det = 1.0;
     int n = temp->size;
 
     for (int i = 0; i < n; i++) {
         int pivot = i;
         while (pivot < n && fabs(temp->data[pivot][i]) < 1e-9) pivot++;
         
         if (pivot == n) {
             free_matrix(temp);
             return 0.0;
         }
 
         if (pivot != i) {
             double *swap = temp->data[i];
             temp->data[i] = temp->data[pivot];
             temp->data[pivot] = swap;
             det = -det;
         }
 
         det *= temp->data[i][i];
 
         for (int j = i + 1; j < n; j++) {
             double factor = temp->data[j][i] / temp->data[i][i];
             for (int k = i; k < n; k++) {
                 temp->data[j][k] -= factor * temp->data[i][k];
             }
         }
     }
     free_matrix(temp);
     return det;
 }
 
 // Replace a column with vector B
 void replace_column(Matrix *m, double *b, int col) {
     for (int i = 0; i < m->size; i++) {
         m->data[i][col] = b[i];
     }
 }
 
 // Helper to get CPU cores via shell command
 int get_logical_cores() {
     FILE *fp = popen("nproc", "r");
     if (fp == NULL) return 1;
     int cores;
     fscanf(fp, "%d", &cores);
     pclose(fp);
     return cores > 0 ? cores : 1;
 }
 
 // --- Main Parallel Logic ---
 int main(int argc, char *argv[]) {
     int N = 300; // Default size
     if (argc > 1) N = atoi(argv[1]);
 
     int max_cores = get_logical_cores();
     
     printf("AI-Generated Cramer's Rule Benchmark\n");
     printf("Matrix Size: %dx%d\n", N, N);
     printf("Detected Cores: %d\n", max_cores);
     printf("------------------------------------\n");
 
     // Initialize Random Seed
     srand(time(NULL));
 
     // Create Matrix A and Vector B
     Matrix *A = create_matrix(N);
     double *B = (double*)malloc(N * sizeof(double));
 
     // Fill with random data
     for (int i = 0; i < N; i++) {
         B[i] = ((double)rand() / RAND_MAX) * 10.0;
         for (int j = 0; j < N; j++) {
             A->data[i][j] = ((double)rand() / RAND_MAX) * 10.0;
         }
     }
 
     // Pre-compute Main Determinant
     double detA = determinant(A);
     if (fabs(detA) < 1e-9) {
         printf("Matrix is singular. No unique solution.\n");
         return 1;
     }
 
     FILE *outfile = fopen("ai_benchmark_results.txt", "w");
 
     // Benchmark Loop (1 to Max Cores)
     for (int cores = 1; cores <= max_cores; cores++) {
         printf("Running with %d cores... ", cores);
         fflush(stdout);
 
         clock_t start_time = clock();
 
         // Fork Processes
         for (int p = 0; p < cores; p++) {
             pid_t pid = fork();
             if (pid == 0) { // Child Process
                 int chunk_size = (N + cores - 1) / cores;
                 int start_col = p * chunk_size;
                 int end_col = (p + 1) * chunk_size;
                 if (end_col > N) end_col = N;
 
                 Matrix *local_A = copy_matrix(A);
                 
                 // Temp file for output
                 char fname[50];
                 sprintf(fname, "temp_%d.dat", p);
                 FILE *fp = fopen(fname, "w");
 
                 for (int j = start_col; j < end_col; j++) {
                     double *original_col = (double*)malloc(N * sizeof(double));
                     for(int r=0; r<N; r++) original_col[r] = local_A->data[r][j];
 
                     replace_column(local_A, B, j);
                     double d = determinant(local_A);
                     fprintf(fp, "%d %lf\n", j, d);
 
                     // Restore
                     for(int r=0; r<N; r++) local_A->data[r][j] = original_col[r];
                     free(original_col);
                 }
                 
                 fclose(fp);
                 free_matrix(local_A);
                 exit(0);
             }
         }
 
         // Wait for all children
         for (int p = 0; p < cores; p++) {
             wait(NULL);
         }
 
         double time_taken = (double)(clock() - start_time) / CLOCKS_PER_SEC;
         printf("Time: %.4f s\n", time_taken);
         fprintf(outfile, "%d %.4f\n", cores, time_taken);
         
         // Clean up temp files
         system("rm temp_*.dat");
     }
 
     fclose(outfile);
     free_matrix(A);
     free(B);
     
     printf("------------------------------------\n");
     printf("Benchmark complete. Results saved to ai_benchmark_results.txt\n");
     return 0;
 }