# Our Solution: Modular Parallel Cramer's Rule Benchmark

This directory contains a modular C implementation of a benchmark for Cramer's Rule using process-based parallelism.

## 📂 Codebase Structure

| File | Description |
| :--- | :--- |
| **`interface.c`** | **Entry Point (`main`)**. Handles user input, detects CPU cores (via `lscpu`), and orchestrates the high-level benchmarking logic. |
| **`cramer.c` / `.h`** | **Parallel Logic**. Contains `start_benchmark_suite` and `run_single_benchmark`. implementation of `fork()` to spawn child processes for computing sub-problems. |
| **`matrix.c` / `.h`** | **Memory Management**. dynamic allocation (`malloc`/`free`) for matrices, deep copying, and column replacement ops. |
| **`det.c` / `.h`** | **Math Logic**. Computes the determinant of a matrix using **Gaussian Elimination** (Row Echelon Form) to achieve $O(N^3)$ complexity instead of factorial time. |

## 🚀 How It Works

1.  **Initialization**: The program asks for a matrix size $N$ (e.g., 1000).
2.  **Core Detection**: It queries the system for the number of available logical cores.
3.  **Data Preparation**: Generates a random $N \times N$ matrix $A$ and a random vector $B$.
4.  **Benchmarking Loop**:
    -   Iterates from 1 process up to the maximum detected cores.
    -   **Forking**: For each iteration with $P$ processes, it calls `fork()` $P$ times.
    -   **Work Distribution**: Each child process is assigned a "chunk" of columns (computation of determinants $det(A_i)$).
    -   **Computation**: Children compute determinants using Gaussian Elimination and write temporary results to disk.
    -   **Synchronization**: The parent process waits for all children (`wait(NULL)`).
    -   **Timing**: The total wall-clock time is measured and reported.

## 🛠️ Build & Run Instructions

### Prerequisites
-   **Linux Environment** (WSL, Ubuntu, Debian, etc.)
-   **GCC**

### Compilation
Since the project is modular, you must compile all `.c` files together.

```bash
gcc interface.c cramer.c matrix.c det.c -o cramer_benchmark
```

### Execution
```bash
./cramer_benchmark
```

### Usage Example
```text
$ ./cramer_benchmark
==========================================
         CRAMER'S RULE BENCHMARK          
==========================================
Enter Matrix Size (N) [e.g. 1000]: 500

[System] Detected 12 Logical Cores.
[Info] System has many cores (>8).
[Info] Skipping 1-7 processes to save time.

Pre-computing Det(A)...

Starting Benchmark (8 to 12 Processes)...
------------------------------------------
Running with 8 Process(es)... Time: 2.1432 s
Running with 9 Process(es)... Time: 1.9811 s
...
```

## 🔍 Implementation Details

### Parallel Speedup
The program demonstrates that calculating $N+1$ determinants (required for Cramer's Rule) is an "embarrassingly parallel" problem. By splitting the $N$ column replacements across $P$ processes, we can achieve significant speedup on multi-core systems.

### Memory Management
-   **Dynamic Allocation**: `create_matrix` (in `matrix.c`) uses `malloc` to allocate an array of pointers, then allocates rows.
-   **Deep Copies**: To prevent race conditions or data corruption, child processes operate on *copies* of the matrix (`copy_matrix`).
-   **Cleanup**: All allocated memory is freed using `free_matrix` and `free` to prevent memory leaks.

### Gaussian Elimination for Determinants
Calculating a determinant recursively is $O(N!)$. This implementation uses **Gaussian Elimination** to convert the matrix to an upper triangular form, reducing complexity to $O(N^3)$. This allows the program to handle $N=1000$ in a reasonable time.
