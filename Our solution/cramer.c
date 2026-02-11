#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cramer.h"
#include "matrix.h"
#include "det.h"

int get_cpu_count()
{
    system("lscpu | grep '^CPU(s):' | awk '{print $2}' > core_config.tmp");
    FILE *f = fopen("core_config.tmp", "r");
    if(!f) return 1;
    int cores;
    if(fscanf(f,"%d",&cores) != 1) cores = 1;
    fclose(f);
    system("rm core_config.tmp");
    return (cores > 0) ? cores : 1;
}

// Internal function to run one specific test
double run_single_benchmark(int n,int procs,Matrix *A_orig,double *B)
{
    clock_t start = clock();

    for(int p=0;p<procs;p++)
    {
        if (fork() == 0)
        { // child
            int chunk = (n+procs-1) / procs;
            int start_col = p*chunk;
            int end_col = (p+1) * chunk;
            if (end_col > n) end_col = n;
            char fname[32];
            sprintf(fname, "res_%d_%d.tmp", getpid(), p);
            FILE *f = fopen(fname, "w");
            Matrix *A_local = copy_matrix(A_orig);

            for (int j=start_col;j<end_col;j++)
            {
                double *orig_col = malloc(n*sizeof(double));
                for(int r=0;r<n;r++) orig_col[r] = A_local->data[r][j];

                replace_column(A_local,B,j);
                double det_Aj = compute_determinant(A_local);

                fprintf(f,"%d %e\n",j,det_Aj);
                
                for(int r=0; r<n; r++) A_local->data[r][j] = orig_col[r];
                free(orig_col);
            }
            fclose(f);
            free_matrix(A_local);
            exit(0);
        }
    }

    for (int p=0;p<procs;p++) wait(NULL); // parent waits

    clock_t end = clock();
    system("rm res_*.tmp"); 
    return ((double)(end-start)) / CLOCKS_PER_SEC;
}

void start_benchmark_suite(int n,int start_procs,int end_procs)
{
    srand(time(NULL));
    Matrix *A = create_matrix(n);
    fill_random(A);
    double *B = malloc(n * sizeof(double));
    for (int i=0;i<n;i++) B[i] = ((double)rand() / RAND_MAX) * 10.0;

    printf("\nPre-computing Det(A)...\n");
    double det_A = compute_determinant(A);
    if (det_A == 0)
    { 
        printf("Error: Det(A) is 0. No unique solution.\n");
        exit(1);
    }

    printf("\nStarting Benchmark (%d to %d Processes)...\n",start_procs,end_procs);
    printf("------------------------------------------\n");
    
    FILE *outfile = fopen("cramer_results.txt", "w");
    fprintf(outfile, "Procs Time\n");

    for (int procs=start_procs;procs<=end_procs;procs++)
    {
        printf("Running with %d Process(es)... ", procs);
        fflush(stdout);

        double t = run_single_benchmark(n,procs,A,B);
        
        printf("Time: %.4f s\n", t);
        fprintf(outfile, "%d %.4f\n", procs, t);
    }

    printf("------------------------------------------\n");
    printf("[Success] Benchmark complete. Saved to 'cramer_results.txt'\n");

    fclose(outfile);
    free(B);
    free_matrix(A);
}