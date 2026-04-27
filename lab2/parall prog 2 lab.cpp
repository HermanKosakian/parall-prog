#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <omp.h>

using namespace std;
using namespace std::chrono;


static int** readMatrixFromFile(ifstream& file, int n) {
    int** matrix = new int* [n];
    for (int i = 0; i < n; ++i) {
        matrix[i] = new int[n];
        for (int j = 0; j < n; ++j) {
            file >> matrix[i][j];
        }
    }
    return matrix;
}

static bool seekToHeader(ifstream& file, const string& header) {
    string line;
    while (getline(file, line)) {
        if (line.find(header) != string::npos) {
            return true;
        }
    }
    return false;
}

static int** multiplyMatricesParallel(int** A, int** B, int n, int num_threads) {
    int** C = new int* [n];
    for (int i = 0; i < n; ++i) {
        C[i] = new int[n]();
    }

    omp_set_num_threads(num_threads);

#pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            long long sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += static_cast<long long>(A[i][k]) * B[k][j];
            }
            C[i][j] = static_cast<int>(sum);
        }
    }
    return C;
}

static int** multiplyMatricesSequential(int** A, int** B, int n) {
    int** C = new int* [n];
    for (int i = 0; i < n; ++i) {
        C[i] = new int[n]();
    }

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            long long sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += static_cast<long long>(A[i][k]) * B[k][j];
            }
            C[i][j] = static_cast<int>(sum);
        }
    }
    return C;
}

static void writeMatrixToFile(int** matrix, int n, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Cannot open file for writing: " << filename << endl;
        return;
    }
    file << "SIZE: " << n << "x" << n << "\n\n";
    file << "--- Result Matrix (" << n << "x" << n << ") ---\n";
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            file << matrix[i][j];
            if (j < n - 1) file << " ";
        }
        file << "\n";
    }
    file.close();
}

static void appendTimeToFile(int n, double seconds, int num_threads, const string& filename) {
    ofstream file(filename, ios::app);
    if (!file.is_open()) {
        cerr << "Cannot open time file: " << filename << endl;
        return;
    }
    file << "Size " << n << "x" << n << " with " << num_threads
        << " threads: " << seconds << " seconds\n";
    file.close();
}

static void freeMatrix(int** matrix, int n) {
    for (int i = 0; i < n; ++i) delete[] matrix[i];
    delete[] matrix;
}

int main() {
    ifstream file("A:/matrices.txt");
    if (!file.is_open()) {
        cerr << "Cannot open matrices.txt" << endl;
        return 1;
    }

    const int sizes[] = { 200, 400, 800, 1200, 1600, 2000 };
    const int numSizes = sizeof(sizes) / sizeof(sizes[0]);


    const int thread_counts[] = { 1, 2, 4, 8 };
    const int num_thread_configs = sizeof(thread_counts) / sizeof(thread_counts[0]);

    const string timeFilename = "A:/timing_parallel.txt";

  
    ofstream clearTime(timeFilename, ios::trunc);
    clearTime.close();

 
    int max_threads = omp_get_max_threads();
    cout << "System maximum threads: " << max_threads << endl;
    cout << "Testing with thread counts: ";
    for (int i = 0; i < num_thread_configs; ++i) {
        cout << thread_counts[i] << " ";
    }
    cout << endl << endl;

    for (int idx = 0; idx < numSizes; ++idx) {
        int n = sizes[idx];
        cout << "\n========================================" << endl;
        cout << "Processing size " << n << "x" << n << "..." << endl;
        cout << "========================================" << endl;

        file.clear();
        file.seekg(0, ios::beg);

        string headerA = "--- Matrix A (" + to_string(n) + "x" + to_string(n) + ") ---";
        string headerB = "--- Matrix B (" + to_string(n) + "x" + to_string(n) + ") ---";

        if (!seekToHeader(file, headerA)) {
            cerr << "Matrix A of size " << n << " not found." << endl;
            continue;
        }
        int** matA = readMatrixFromFile(file, n);
        cout << "Matrix A loaded." << endl;

        if (!seekToHeader(file, headerB)) {
            cerr << "Matrix B of size " << n << " not found." << endl;
            freeMatrix(matA, n);
            continue;
        }
        int** matB = readMatrixFromFile(file, n);
        cout << "Matrix B loaded." << endl;

       
        for (int t = 0; t < num_thread_configs; ++t) {
            int num_threads = thread_counts[t];

         
            if (num_threads > max_threads) {
                cout << "\nSkipping " << num_threads << " threads (only "
                    << max_threads << " available)" << endl;
                continue;
            }

            cout << "\n--- Testing with " << num_threads << " threads ---" << endl;

        
            cout << "Multiplying (parallel)..." << endl;
            auto start = high_resolution_clock::now();
            int** matRes = multiplyMatricesParallel(matA, matB, n, num_threads);
            auto end = high_resolution_clock::now();
            duration<double> elapsed = end - start;
            double seconds_parallel = elapsed.count();

            cout << "Parallel multiplication completed in " << seconds_parallel << " seconds." << endl;

          
            if (num_threads == 1) {
                string resultFilename = "A:/result_matrix_" + to_string(n) + ".txt";
                writeMatrixToFile(matRes, n, resultFilename);
                cout << "Result saved to " << resultFilename << endl;
            }

            
            appendTimeToFile(n, seconds_parallel, num_threads, timeFilename);

         
            if (num_threads == 1) {
              
                cout << "(Baseline time for size " << n << ")" << endl;
            }
            else {
               
                cout << "(Testing with " << num_threads << " threads)" << endl;
            }

            freeMatrix(matRes, n);
        }

        freeMatrix(matA, n);
        freeMatrix(matB, n);
    }

    file.close();

    cout << "\n========================================" << endl;
    cout << "All experiments completed!" << endl;
    cout << "Results saved to " << timeFilename << endl;
    cout << "========================================" << endl;

 
    cout << "\nExperiment summary:" << endl;
    cout << "- Matrix sizes tested: ";
    for (int i = 0; i < numSizes; ++i) {
        cout << sizes[i];
        if (i < numSizes - 1) cout << ", ";
    }
    cout << endl;
    cout << "- Thread counts tested: ";
    for (int i = 0; i < num_thread_configs; ++i) {
        if (thread_counts[i] <= max_threads) {
            cout << thread_counts[i];
            if (i < num_thread_configs - 1 && thread_counts[i + 1] <= max_threads) cout << ", ";
        }
    }
    cout << endl;

    return 0;
}
