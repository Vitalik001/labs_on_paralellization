#include "functions.h"
#include <cmath>

double function1(double x, double y) {
    double sum = 0.002;
    for (int i = -2; i <= 2; ++i) {
        for (int j = -2; j <= 2; ++j) {
            double term = 5 * (i + 2) + j + 3 + std::pow((x - 16 * j), 6) + std::pow((y - 16 * i), 6);
            sum += (double) 1 / term;
        }
    }
    sum = 1.0 / (sum);
    return (double) sum;
}

double function2(double x1, double x2) {
    double sum = 20.0;
    sum += -20 * std::exp(-0.2 * std::sqrt((double) 1 / 2 * (x1 * x1 + x2 * x2)));
    sum += -std::exp((double) 1 / 2 * (std::cos(2 * M_PI * x1) + std::cos(2 * M_PI * x2)));
    sum += std::exp(1);
    return sum;
}


double function3(double x, double y) {
    int m = 5;
    double a1[] = {1, 2, 1, 1, 5};
    double a2[] = {4, 5, 1, 2, 4};
    double c[] = {2, 1, 4, 7, 2};
    double sum = 0.0;
    for (int i = 0; i < m; ++i) {
        double term1 = (double) (-1.0/M_PI)*((x - a1[i])*(x - a1[i]) + (y - a2[i])*(y - a2[i]));
        double term2 = (double) M_PI*((x - a1[i])*(x - a1[i]) + (y - a2[i])*(y - a2[i]));
        sum += c[i] * exp(term1) * cos(term2);
    }
    return -1.0 * sum;
}