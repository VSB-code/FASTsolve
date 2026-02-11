#ifndef CRAMER_COMPUTE_H
#define CRAMER_COMPUTE_H

// Runs the benchmark for a specific range of processors
void start_benchmark_suite(int n, int start_procs, int end_procs);

// Helper to get CPU count
int get_cpu_count();

#endif