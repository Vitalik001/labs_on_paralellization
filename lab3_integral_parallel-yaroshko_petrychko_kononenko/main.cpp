// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include "options_parser.h"
#include "config_parser/config_parser.h"
#include "functions/functions.h"
#include <map>
#include <thread>
#include <numeric>
#include <chrono>

using ifunc = double(*)(double, double);

enum RETURN_CODES {
    SUCCESS = 0,
    INCORRECT_FUNCTION_NUMBER = 2,
    INCORRECT_NUMBER_OF_COMMAND_LINE_ARGUMENTS = 1,
    INCORRECT_CONFIG_FILE = 5,
    NOT_ACCURATE_ENOUGH_CALCULATIONS = 16,
    INVALID_ARGUMENT_TYPE = 66
};

void f1(double x_0, double y_0, double x_end, double y_end, double delta_x, double delta_y, ifunc function, double& sum) {
    for (double i = x_0; i < x_end; i += delta_x){
        for (double j = y_0; j < y_end; j += delta_y){
            sum += (*function)(i, j + delta_y / 2);
            sum += (*function)(i + delta_x / 2,
                               j + delta_y / 2);
            sum += (*function)(i + delta_x / 2, j);
        }
    }
}

inline std::chrono::high_resolution_clock::time_point get_current_time_fenced()
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us(const D& d)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
}

void integrate(const ifunc function, const po::variables_map& map, size_t thread_amount) {
    // create logic for integration
    int iteration = 1;
    int steps_x = map["init_steps_x"].as<int>();
    int steps_y = map["init_steps_y"].as<int>();
    double x_0 = map["x_start"].as<double>();
    double y_0 = map["y_start"].as<double>();
    double abs_err = map["abs_err"].as<double>();
    double rel_err = map["rel_err"].as<double>();
    double x_end = map["x_end"].as<double>();
    double y_end = map["y_end"].as<double>();
    int max_iter = map["max_iter"].as<int>();
    double delta_x = (x_end - x_0) / steps_x;
    double delta_y = (y_end - y_0) / steps_y;
    double current_sum = 0.0;
    double previous_sum = 0.0;

    auto stage_start_time = get_current_time_fenced();

    for (int i = 0; i <= steps_x; i++) {
        for (int j = 0; j <= steps_y; j++) {
            previous_sum += (*function)(x_0 + i * delta_x, y_0 + j * delta_y);
        }
    }

    previous_sum*=delta_y*delta_x;
    delta_x /= 2;
    delta_y /= 2;

    std::vector<std::thread> threads;
    std::vector<double> sum_vector(thread_amount, 0);

    while (
            (std::abs(previous_sum - current_sum) > abs_err or std::abs((previous_sum - current_sum)/previous_sum) > rel_err)
            and
            (iteration <= max_iter)
            ){

        previous_sum = iteration != 1 ? current_sum: previous_sum;

        for (size_t i = 0; i < thread_amount; ++i) {
            threads.emplace_back(f1, x_0 + delta_x * i, y_0, x_end, y_end, thread_amount * delta_x, delta_y, std::ref(function), std::ref(sum_vector[i]));
        }

        for (auto& i : threads) {
            i.join();
        }

        double sum = std::accumulate(sum_vector.begin(), sum_vector.end(), 0.0);

        current_sum = (sum * delta_x * delta_y + previous_sum) / 4;
        delta_x /= 2;
        delta_y /= 2;
        steps_x *= 2;
        iteration++;

        threads.clear();
        std::fill(sum_vector.begin(), sum_vector.end(), 0);
    }

    auto finish_time = get_current_time_fenced();
    auto total_time = finish_time - stage_start_time;

    double abs_error = std::abs(previous_sum - current_sum);
    double rel_error = std::abs((previous_sum - current_sum)/previous_sum);

    std::cout << current_sum << std::endl;
    std::cout << abs_error << std::endl;
    std::cout << rel_error << std::endl;
    std::cout << to_us(total_time) << std::endl;

    if (iteration > max_iter) {
        throw std::invalid_argument("Calculations were not accurate enough!");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Incorrect number of command line arguments" << std::endl;
        return INCORRECT_NUMBER_OF_COMMAND_LINE_ARGUMENTS;
    }

    command_line_options_t command_line_options{argc, argv};
    int function_number;
    size_t thread_amount;

    try {
        function_number = stoi(command_line_options.get_function());
        thread_amount = stoi(command_line_options.get_threads());
    } catch (const std::invalid_argument& ex) {
        std::cerr << "Invalid type of argument!";
        return INVALID_ARGUMENT_TYPE;
    }

    std::string config_file = command_line_options.get_config();
    po::variables_map config;

    try {
        assert_file_exist(config_file);
        config = parse_config(config_file);
    } catch (const std::invalid_argument& ex) {
        std::cerr << ex.what();
        return INCORRECT_CONFIG_FILE;
    } catch (const boost::wrapexcept<boost::program_options::required_option>& ex) {
        std::cerr << ex.what() << "\n";
        return INCORRECT_CONFIG_FILE;
    } catch (const boost::wrapexcept<boost::program_options::unknown_option>& ex) {
        std::cerr << ex.what() << "\n";
        return INCORRECT_CONFIG_FILE;
    }

    if (function_number > 3 || function_number < 1) {
        std::cerr << "Incorrect function number" << std::endl;
        return INCORRECT_FUNCTION_NUMBER;
    }

    std::map<int, ifunc> func_map {
            {1, function1},
            {2, function2},
            {3, function3}
    };

    try {
        integrate(func_map[function_number], config, thread_amount);
    } catch(std::invalid_argument& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return SUCCESS;
}