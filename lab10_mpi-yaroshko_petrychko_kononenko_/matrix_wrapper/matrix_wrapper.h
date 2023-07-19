//
// Created by nazar on 26.05.23.
//

#include <iostream>
#include <fstream>

#ifndef TEMPLATE_MATRIX_WRAPPER_H
#define TEMPLATE_MATRIX_WRAPPER_H

class MatrixWrapper {
private:
    size_t rows;
    size_t columns;

public:
    double * array;

    MatrixWrapper(size_t rows_, size_t columns_) : columns(columns_), rows(rows_) {
        array = new double[columns * rows];
    }

    void sendRow(int index, int dest, int partition) const;
    void receiveRow(int index, int source, int partition) const;
    void from_csv(const std::string& path) const;
    void send(int index, int dest, int partition) const;
    void receive(int index, int source, int partition) const;

    [[nodiscard]] size_t getRowCount() const;
    [[nodiscard]] size_t getColumnCount() const;

    ~MatrixWrapper() {
        delete[] array;
    }

    double& operator[](size_t index) {
        return array[index];
    }

    const double& operator[](size_t index) const {
        return array[index];
    }

    void print() const;

    void saveTemperatureImage(const std::string &filename) const;

    void
    calculate_temp(double thermal_conductivity, double density, double heat_capacity, double delta_t, double delta_x,
                   double delta_y) const;
};

#endif //TEMPLATE_MATRIX_WRAPPER_H
