import numpy as np
import os

DATA_PATH = "A:/"

def read_matrix_from_file(filename, size, matrix_name):
    with open(filename, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    in_block = False
    matrix = []
    for line in lines:
        if f"--- Matrix {matrix_name} ({size}x{size}) ---" in line:
            in_block = True
            continue
        if in_block:
            if len(matrix) == size:
                break
            line = line.strip()
            if not line:
                continue
            if line.startswith('---') or '======' in line:
                continue
            parts = line.split()
            try:
                row = [int(x) for x in parts]
            except ValueError:
                continue
            if len(row) != size:
                continue
            matrix.append(row)

    return np.array(matrix, dtype=np.int64)


def read_result_from_file(filename, size):
    with open(filename, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    start_idx = 0
    for i, line in enumerate(lines):
        if f"--- Result Matrix ({size}x{size}) ---" in line:
            start_idx = i + 1
            break

    matrix = []
    for line in lines[start_idx:]:
        if len(matrix) == size:
            break
        line = line.strip()
        if not line:
            continue
        if line.startswith('---') or '======' in line:
            continue
        parts = line.split()
        try:
            row = [int(x) for x in parts]
        except ValueError:
            continue
        if len(row) != size:
            continue
        matrix.append(row)

    return np.array(matrix, dtype=np.int64)


def verify(size, mat_fname, res_fname):
    print(f"Verifying size {size}x{size}...")

    matrix_a = read_matrix_from_file(mat_fname, size, "A")
    matrix_b = read_matrix_from_file(mat_fname, size, "B")

    cpp_result = read_result_from_file(res_fname, size)
    expected_result = np.dot(matrix_a, matrix_b)

    if np.array_equal(cpp_result, expected_result):
        print("  ✓ PASSED")
        return True
    else:
        diff = np.abs(cpp_result - expected_result)
        max_diff = np.max(diff)
        print(f"  ✗ FAILED, max difference = {max_diff}")
        return False


if __name__ == "__main__":
    matrices_file = DATA_PATH + "matrices.txt"
    sizes = [200, 400, 800, 1200, 1600, 2000]

    all_passed = True
    for n in sizes:
        result_file = DATA_PATH + f"result_matrix_{n}.txt"
        if not verify(n, matrices_file, result_file):
            all_passed = False

    if all_passed:
        print("\nAll tests PASSED.")
    else:
        print("\nSome tests FAILED.")