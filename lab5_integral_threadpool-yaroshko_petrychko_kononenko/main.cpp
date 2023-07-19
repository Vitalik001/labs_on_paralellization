// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include "options_parser/options_parser.h"
#include "config_parser/config_parser.h"
#include "functions/functions.h"
#include <map>
#include <thread>
#include <future>
#include <cmath>
#include "thread_pool/thread_pool.h"
#include <functional>
#include <chrono>


enum RETURN_CODES {
    SUCCESS = 0,
    INCORRECT_FUNCTION_NUMBER = 2,
    INCORRECT_NUMBER_OF_COMMAND_LINE_ARGUMENTS = 1,
    INCORRECT_CONFIG_FILE = 5,
    INVALID_ARGUMENT_TYPE = 66
};

struct data_chunk {
    double x_0;
    double y_0;
    double x_end;
    double y_end;
    double delta_x;
    double delta_y;
};


using ifunc = double(*)(double, double);

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

void integrate(const ifunc function, const po::variables_map& map, size_t thread_amount, int points_per_thread) {
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

    std::vector<std::future<double>> futures;
    thread_pool threadPool(thread_amount);

    size_t chunks = std::ceil(steps_x / points_per_thread);

    while (
            (std::abs(previous_sum - current_sum) > abs_err or std::abs((previous_sum - current_sum)/previous_sum) > rel_err)
            and
            (iteration <= max_iter)
            ) {

        previous_sum = iteration != 1 ? current_sum: previous_sum;

        auto helper_func = [](data_chunk data, ifunc function){
            double sum = 0;
            double i = data.x_0;
            double j = data.y_0;

            while (i < data.x_end) {
                while (j < data.y_end) {
                    sum += (*function)(i, j + data.delta_y / 2);
                    sum += (*function)(i + data.delta_x / 2,
                                       j + data.delta_y / 2);
                    sum += (*function)(i + data.delta_x / 2, j);

                    j += data.y_end;
                }

                i += data.x_end;
            }

            return sum;
        };

        for (size_t i = 0; i < chunks; ++i) {
            data_chunk data { x_0 + delta_x * i, y_0, x_end, y_end, chunks * delta_x, delta_y };

            auto lmbd = [data, function, &helper_func] (){
                return helper_func(data, function);
            };

            futures.emplace_back(threadPool.submit(lmbd));
        }

        double sum = 0;
        int counter = 0;

        for (auto& fut : futures) {
            counter += 1;
            if (fut.valid()) {
                sum += fut.get();
            }
        }

        current_sum = (sum * delta_x * delta_y + previous_sum) / 4;
        delta_x /= 2;
        delta_y /= 2;
        steps_x *= 2;
        iteration++;
        chunks = std::ceil(steps_x / points_per_thread);
    }

    auto finish_time = get_current_time_fenced();
    auto total_time = finish_time - stage_start_time;

    double abs_error = std::abs(previous_sum - current_sum);
    double rel_error = std::abs((previous_sum - current_sum)/previous_sum);

    std::cout << current_sum << std::endl;
    std::cout << abs_error << std::endl;
    std::cout << rel_error << std::endl;
    std::cout << to_us(total_time) << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Incorrect number of command line arguments" << std::endl;
        return INCORRECT_NUMBER_OF_COMMAND_LINE_ARGUMENTS;
    }

    command_line_options_t command_line_options{argc, argv};
    int function_number;
    size_t thread_amount;
    int points_per_thread;

    try {
        function_number = stoi(command_line_options.get_function());
        thread_amount = stoi(command_line_options.get_threads());
        points_per_thread = stoi(command_line_options.get_points_per_thread());

        if (points_per_thread <= 0 || thread_amount <= 0) {
            throw std::invalid_argument("Invalid argument value!");
        }
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
        integrate(func_map[function_number], config, thread_amount, points_per_thread);
    } catch(std::invalid_argument& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return SUCCESS;
}