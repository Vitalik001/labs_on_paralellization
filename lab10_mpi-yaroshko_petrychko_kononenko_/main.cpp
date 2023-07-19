// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <iostream>
#include <fstream>
#include "options_parser.h"
#include "config_parser/config_parser.h"
#include <mpi.h>
#include "matrix_wrapper/matrix_wrapper.h"
#include <string>

void createTemperatureCSV(const std::string& filename, int height, int width, double initialTemp, double boundaryTemp) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::vector<std::vector<double>> temperatureMatrix(height, std::vector<double>(width));

    // Set initial temperature
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            temperatureMatrix[i][j] = initialTemp;
        }
    }

    // Set boundary temperature for chosen columns and specified lengths
    int column1, column2, length1, length2, row1, row2, length3, length4;
    std::cout << "Enter the index (zero-based) of the first column to set the boundary temperature: ";
    std::cin >> column1;
    std::cout << "Enter the index (zero-based) of the second column to set the boundary temperature: ";
    std::cin >> column2;
    std::cout << "Enter the length of the boundary region in the first column: ";
    std::cin >> length1;
    std::cout << "Enter the length of the boundary region in the second column: ";
    std::cin >> length2;
    std::cout << "Enter the index (zero-based) of the first row to set the boundary temperature: ";
    std::cin >> row1;
    std::cout << "Enter the index (zero-based) of the second row to set the boundary temperature: ";
    std::cin >> row2;
    std::cout << "Enter the length of the boundary region in the first row: ";
    std::cin >> length3;
    std::cout << "Enter the length of the boundary region in the second row: ";
    std::cin >> length4;

    if (column1 >= 0 && column1 < width && column2 >= 0 && column2 < width && length1 > 0 && length2 > 0 && row1 >= 0 && row1 < height && row2 >= 0 && row2 < height && length3 > 0 && length4 > 0) {
        // Set boundary temperature for the first column
        for (int j = 0; j < length1; ++j) {
                if (j >= 0 && j < width) {
                    temperatureMatrix[j][column1] = boundaryTemp;
                }
        }

        // Set boundary temperature for the second column
        for (int j = 0; j < length2; ++j) {
            if (j >= 0 && j < width) {
                temperatureMatrix[j][column2] = boundaryTemp;
            }
        }

        for (int j = 0; j < length3; ++j) {
            if (j >= 0 && j < height) {
                temperatureMatrix[row1][j] = boundaryTemp;
            }
        }

        for (int j = 0; j < length4; ++j) {
            if (j >= 0 && j < height) {
                temperatureMatrix[row2][j] = boundaryTemp;
            }
        }

    } else {
        std::cout << "Invalid column index or boundary region length. No boundary temperature applied.\n";
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            file << temperatureMatrix[i][j];
            if (j != width - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
    std::cout << "CSV file created: " << filename << std::endl;
}

//void createTemperatureCSVFiles(int height, int width, double initialTemp, double boundaryTemp, const std::string& path) {
//    std::ofstream boundaryFile(path);
//    if (!boundaryFile) {
//        std::cerr << "Failed to create boundary_temperature.csv" << std::endl;
//        return;
//    }
//
//    for (int i = 0; i < height; i++) {
//        for (int j = 0; j < width; j++) {
//            if (j != 0) {
//                boundaryFile << ",";
//            }
//
//            if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
//                boundaryFile << boundaryTemp;
//            } else {
//                boundaryFile << initialTemp;
//            }
//        }
//        boundaryFile << "\n";
//    }
//
//    boundaryFile.close();
//
//    std::cout << "CSV files created successfully." << std::endl;
//}

int main(int argc, char* argv[]) {
    auto options = parse_config("./configs/config1.cfg");
    auto heat_capacity = options["heat_capacity"].as<double>();
    auto thermal_conductivity = options["thermal_conductivity"].as<double>();
    auto density = options["density"].as<double>();
    auto width = options["width"].as<int>();
    auto height = options["height"].as<int>();
    auto delta_x = options["delta_x"].as<double>();
    auto delta_y = options["delta_y"].as<double>();
    auto delta_t = options["delta_t"].as<double>();
    auto initialTemp = options["initialTemp"].as<double>();
    auto boundaryTemp = options["boundaryTemp"].as<double>();
    auto iterations_save = options["iterations_save"].as<int>();
    auto max_iterations = options["max_iterations"].as<int>();
    auto initial_file = options["initial_conditions"].as<std::string>();

    double alpha = thermal_conductivity / (density * heat_capacity);

    if (delta_t > std::pow(std::max(delta_y, delta_x), 2) / (4 * alpha)) {
        throw std::runtime_error("The Von Neumann Criterion is not satisfied!");
    }

    int commsize, rank, len;
    char procname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(procname, &len);

    int partition = height / (commsize - 1);

    if (rank == 0) {
        MatrixWrapper matrix(height, width);

        matrix.from_csv(initial_file);

        for (int i = 0; i < commsize - 1; ++i) {
            matrix.sendRow(i, i + 1, partition);
        }

        for (int j = 0; j < max_iterations / iterations_save; ++j) {
            for (int i = 0; i < commsize - 1; ++i) {
                matrix.receiveRow(i, i + 1, partition);
            }

            matrix.saveTemperatureImage("./data/image" + std::to_string(j + 1) + ".png");
        }
    } else if (rank == 1) {
        MatrixWrapper matrix(partition + 1, width);

        matrix.receive(0, 0, partition);

        for (int i = 0; i < max_iterations; ++i) {
            matrix.sendRow(partition - 1, rank + 1, 1);
            matrix.receiveRow(partition, rank + 1, 1);

            matrix.calculate_temp(thermal_conductivity, density, heat_capacity, delta_t, delta_x, delta_y);

            if (i % iterations_save == 0) {
                matrix.send(0, 0, partition);
            }
        }
    } else if (rank == commsize - 1) {
        MatrixWrapper matrix(partition + 1, width);

        matrix.receive(1, 0, partition);

        for (int i = 0; i < max_iterations; ++i) {
            matrix.sendRow(1, rank - 1, 1);
            matrix.receiveRow(0, rank - 1, 1);

            matrix.calculate_temp(thermal_conductivity, density, heat_capacity, delta_t, delta_x, delta_y);

            if (i % iterations_save == 0) {
                matrix.send(1, 0, partition);
            }
        }
    } else {
        MatrixWrapper matrix(partition + 2, width);

        matrix.receive(1, 0, partition);

        for (int i = 0; i < max_iterations; ++i) {
            matrix.sendRow(1, rank - 1, 1);
            matrix.sendRow(partition, rank + 1, 1);

            matrix.receiveRow(0, rank - 1, 1);
            matrix.receiveRow(partition + 1, rank + 1, 1);

            matrix.calculate_temp(thermal_conductivity, density, heat_capacity, delta_t, delta_x, delta_y);

            if (i % iterations_save == 0) {
                matrix.send(1, 0, partition);
            }
        }
    }

    std::cout << "Process: " << rank << " of Node: " << procname << std::endl;

    MPI_Finalize();

    return 0;
}