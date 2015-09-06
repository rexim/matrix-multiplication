#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <memory>
#include <exception>
#include <stdexcept>
#include <vector>

#include "./Matrix.h"

std::shared_ptr<Matrix> file_read_matrix(const std::string &filename)
{
    std::ifstream fin(filename);

    if(!fin) {
        throw std::runtime_error("Can't open " + filename);
    }

    size_t n, m;
    fin >> n >> m;

    std::shared_ptr<Matrix> result(new Matrix(n, m));

    for(size_t i = 0; i < n; ++i) {
        for(size_t j = 0; j < m; ++j) {
            fin >> result->get(i, j);
        }
    }

    return result;
}

template <typename T>
T parse_argument(const std::string &argument,
                 const std::string &fail_message)
{
    std::istringstream iss(argument);
    T result = 0;

    if (!(iss >> result)) {
        throw std::runtime_error("Can't parse an argument: " + fail_message);
    }

    return result;
}

std::shared_ptr<Matrix> multiply_matrices(std::shared_ptr<Matrix> matrix1,
                                          std::shared_ptr<Matrix> matrix2)
{
    if(matrix1->columns() != matrix2->rows()) {
        throw std::invalid_argument("Matrices are not compatibles "
                                    "for multiplication.");
    }

    std::shared_ptr<Matrix> result(new Matrix(matrix1->rows(),
                                              matrix2->columns()));

    for(size_t i = 0; i < result->rows(); ++i) {
        for(size_t j = 0; j < result->columns(); ++j) {
            result->get(i, j) = 0;
            for(size_t k = 0; k < matrix1->columns(); ++k) {
                result->get(i, j) += matrix1->get(i, k) * matrix2->get(k, j);
            }
        }
    }

    return result;
}

void multiply_line_worker(size_t *current_row, std::mutex *current_row_mutex,
                          Matrix *matrix1, Matrix *matrix2, Matrix *result)
{
    for(;;) {
        size_t i = 0;
        {
            std::lock_guard<std::mutex> lock_mutex(*current_row_mutex);
            if(*current_row >= result->rows()) {
                return;
            }
            i = (*current_row)++;
        }

        for(size_t j = 0; j < result->columns(); ++j) {
            result->get(i, j) = 0;
            for(size_t k = 0; k < matrix1->columns(); ++k) {
                result->get(i, j) += matrix1->get(i, k) * matrix2->get(k, j);
            }
        }
    }
}

std::shared_ptr<Matrix>
parallel_multiply_matrices(std::shared_ptr<Matrix> matrix1,
                           std::shared_ptr<Matrix> matrix2,
                           const size_t number_of_threads)
{
    if(matrix1->columns() != matrix2->rows()) {
        throw std::invalid_argument("Matrices are not compatibles "
                                    "for multiplication.");
    }

    std::thread threads[number_of_threads];
    std::shared_ptr<Matrix> result(new Matrix(matrix1->rows(),
                                              matrix2->columns()));
    size_t current_row = 0;
    std::mutex current_row_mutex;

    for(size_t i = 0; i < number_of_threads; ++i) {
        threads[i] = std::thread(multiply_line_worker,
                                 &current_row, &current_row_mutex,
                                 matrix1.get(), matrix2.get(),
                                 result.get());
    }

    for(size_t i = 0; i < number_of_threads; ++i) {
        threads[i].join();
    }

    return result;
}

void print_matrix(std::shared_ptr<Matrix> matrix)
{
    std::cout << matrix->rows() << ' ' << matrix->columns() << std::endl;
    for(size_t i = 0; i < matrix->rows(); ++i) {
        for(size_t j = 0; j < matrix->columns(); ++j) {
            std::cout << matrix->get(i, j) << ' ';
        }
        std::cout << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc <= 3) {
        std::cerr << "Usage: ./multiply [filename1] [filename2] [number-of-threads]" << std::endl;
        return 1;
    }

    try {
        auto matrix1 = file_read_matrix(argv[1]);
        auto matrix2 = file_read_matrix(argv[2]);
        auto number_of_threads = parse_argument<size_t>(argv[3], "number-of-threads should be a positive integer.");
        auto result = parallel_multiply_matrices(matrix1, matrix2, number_of_threads);
        print_matrix(result);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
