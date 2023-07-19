#include "options_parser.h"
#include <mpi.h>
#include "matrix_wrapper.h"
#include <png.h>

void MatrixWrapper::sendRow(int index, int dest, int partition) const {
    MPI_Request request;
    MPI_Isend(array + index * columns * partition, columns * partition, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD, &request);
}

void MatrixWrapper::receiveRow(int index, int source, int partition) const {
    MPI_Recv(array + index * columns * partition, columns * partition, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void MatrixWrapper::send(int index, int dest, int partition) const {
    MPI_Request request;
    MPI_Isend(array + index * columns, columns * partition, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD, &request);
}

void MatrixWrapper::receive(int index, int source, int partition) const {
    MPI_Recv(array + index * columns, columns * partition, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

size_t MatrixWrapper::getRowCount() const {
    return rows;
}

size_t MatrixWrapper::getColumnCount() const {
    return columns;
}

void MatrixWrapper::print() const {
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < columns; ++j) {
            std::cout << array[i * columns + j] << " ";
        }
        std::cout << "\n";
    }
}

void MatrixWrapper::saveTemperatureImage(const std::string& filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    png_infop info_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, columns, rows, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    std::vector<png_byte> row(3 * columns, 0);
    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < columns; x++) {
            size_t index = y * columns + x;
            double temperature = array[index];

            png_byte red = 0;
            png_byte green = 0;
            png_byte blue = 0;

            if (temperature <= 0) {
                blue = 255;
            } else if (temperature <= 25) {
                green = static_cast<png_byte>((temperature / 25) * 255);
                blue = 255 - green;
            } else {
                red = static_cast<png_byte>(((temperature - 25) / 75) * 255);
                green = 255 - red;
            }

            row[x * 3] = red;
            row[x * 3 + 1] = green;
            row[x * 3 + 2] = blue;
        }
        png_write_row(png_ptr, row.data());
    }

    png_write_end(png_ptr, nullptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

void MatrixWrapper::calculate_temp(double thermal_conductivity, double density, double heat_capacity, double delta_t, double delta_x, double delta_y) const {
    double * array_copy = array;
    double alpha = thermal_conductivity / (density * heat_capacity);

    for (size_t i = 1; i < rows - 1; i++) {
        for (size_t j = 1; j < columns - 1; j++) {
            double first_adder = ((array_copy[(i - 1) * columns + j]) - 2 * array_copy[i * columns + j] +
                                  array_copy[(i + 1) * columns + j]) / (pow(delta_x, 2));
            double second_adder = ((array_copy[i * columns + j - 1]) - 2 * array_copy[i * columns + j] +
                                   array_copy[i * columns + j + 1]) / (pow(delta_y, 2));
            array[i * columns + j] = array_copy[i * columns + j] + delta_t * alpha * (first_adder + second_adder);
        }
    }
}



void MatrixWrapper::from_csv(const std::string &path) const {
    std::ifstream file(path);
    std::string line;

    for (size_t i = 0; i < rows; ++i) {
        if (!std::getline(file, line)) {
            std::cerr << "Error: Insufficient rows in the CSV file." << std::endl;
            return;
        }

        std::stringstream ss(line);
        std::string cell;

        for (size_t j = 0; j < columns; ++j) {
            if (!std::getline(ss, cell, ',')) {
                std::cerr << "Error: Insufficient columns in row " << i << " of the CSV file." << std::endl;
                return;
            }

            try {
                array[i * columns + j] = std::stod(cell);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: Invalid data in row " << i << ", column " << j << " of the CSV file." << std::endl;
                return;
            }
        }
    }
}