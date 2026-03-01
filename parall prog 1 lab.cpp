#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Чтение матрицы из файла (предполагается, что файл уже позиционирован на первую строку данных)
int** readMatrixFromFile(ifstream& file, int n) {
    int** matrix = new int* [n];
    for (int i = 0; i < n; ++i) {
        matrix[i] = new int[n];
        for (int j = 0; j < n; ++j) {
            file >> matrix[i][j];
        }
    }
    return matrix;
}

// Поиск заданного заголовка в файле, начиная с текущей позиции
bool seekToHeader(ifstream& file, const string& header) {
    string line;
    while (getline(file, line)) {
        if (line.find(header) != string::npos) {
            return true;
        }
    }
    return false;
}

// Стандартное умножение матриц O(n^3)
int** multiplyMatrices(int** A, int** B, int n) {
    int** C = new int* [n];
    for (int i = 0; i < n; ++i) {
        C[i] = new int[n]();
    }

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            long long sum = 0;  // используем long long для предотвращения переполнения при суммировании
            for (int k = 0; k < n; ++k) {
                sum += static_cast<long long>(A[i][k]) * B[k][j];
            }
            C[i][j] = static_cast<int>(sum);  // после суммирования результат помещается в int
        }
    }
    return C;
}

// Запись результата в файл
void writeMatrixToFile(int** matrix, int n, const string& filename) {
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

// Добавление времени выполнения в файл
void appendTimeToFile(int n, double seconds, const string& filename) {
    ofstream file(filename, ios::app);
    if (!file.is_open()) {
        cerr << "Cannot open time file: " << filename << endl;
        return;
    }
    file << "Size " << n << "x" << n << ": " << seconds << " seconds\n";
    file.close();
}

// Освобождение памяти матрицы
void freeMatrix(int** matrix, int n) {
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
    const string timeFilename = "A:/timing.txt";

    // Очищаем файл времени
    ofstream clearTime(timeFilename, ios::trunc);
    clearTime.close();

    for (int idx = 0; idx < numSizes; ++idx) {
        int n = sizes[idx];
        cout << "\nProcessing size " << n << "x" << n << "..." << endl;

        // Возвращаемся в начало файла для поиска нужного блока
        file.clear();
        file.seekg(0, ios::beg);

        string headerA = "--- Matrix A (" + to_string(n) + "x" + to_string(n) + ") ---";
        string headerB = "--- Matrix B (" + to_string(n) + "x" + to_string(n) + ") ---";

        // Поиск и чтение матрицы A
        if (!seekToHeader(file, headerA)) {
            cerr << "Matrix A of size " << n << " not found." << endl;
            continue;
        }
        int** matA = readMatrixFromFile(file, n);
        cout << "Matrix A loaded." << endl;

        // Поиск и чтение матрицы B
        if (!seekToHeader(file, headerB)) {
            cerr << "Matrix B of size " << n << " not found." << endl;
            freeMatrix(matA, n);
            continue;
        }
        int** matB = readMatrixFromFile(file, n);
        cout << "Matrix B loaded." << endl;

        // Умножение и замер времени
        cout << "Multiplying..." << endl;
        auto start = high_resolution_clock::now();
        int** matRes = multiplyMatrices(matA, matB, n);
        auto end = high_resolution_clock::now();
        duration<double> elapsed = end - start;
        double seconds = elapsed.count();

        cout << "Multiplication completed in " << seconds << " seconds." << endl;

        // Запись результата и времени
        string resultFilename = "A:/result_matrix_" + to_string(n) + ".txt";
        writeMatrixToFile(matRes, n, resultFilename);
        appendTimeToFile(n, seconds, timeFilename);

        // Освобождение памяти
        freeMatrix(matA, n);
        freeMatrix(matB, n);
        freeMatrix(matRes, n);
    }

    file.close();
    cout << "\nAll done. Times saved to " << timeFilename << endl;
    return 0;
}