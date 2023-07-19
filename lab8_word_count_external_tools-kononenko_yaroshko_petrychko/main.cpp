// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include "options_parser.h"
#include "config_parser/config_parser.h"
#include <vector>
#include <boost/locale.hpp>
#include "threadsafe_queue/threadsafe_queue.h"
#include <thread>
#include "disk_reader/DiskReader.h"

#define fs std::filesystem
#define bl boost::locale

enum RETURN_CODES {
    SUCCESS = 0,
    INVALID_NUMBER_OF_ARGUMENTS = 1,
    ERROR_OPENING_CONFIG = 3,
    ERROR_OPENING_OUTPUT_FILE = 4,
    INCORRECT_CONFIG_FILE = 5,
    ERROR_WRITING_TO_FILE = 6,
    OUTPUT_FILE_ERROR = 27
    };

#define ORDERED
// #define CHECK_BACKENDS

//#ifdef ORDERED
//typedef std::map<std::string, int> word_count_map;
//#else
//typedef std::unordered_map<std::string, int> word_count_map;
//#endif

inline std::chrono::high_resolution_clock::time_point get_current_time_fenced() {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us(const D &d) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
}

int main(int argc, char *argv[]) {
    command_line_options_t command_line_options{argc, argv};
    std::string config_file = command_line_options.get_path_to_file();

    if (argc != 2) {
        std::cerr << "Invalid number of arguments" << std::endl;
        return INVALID_NUMBER_OF_ARGUMENTS;
    }

    bl::localization_backend_manager lbm = bl::localization_backend_manager::global();

    // Selecting ICU backend for boost::locale
    lbm.select("icu");

    // Setting global locale for US
    bl::generator gen;
    std::locale::global(gen("en_US.UTF-8"));

    po::variables_map options;

    try {
        assert_file_exist(config_file);
        options = parse_config(config_file);
    } catch (const std::invalid_argument& ex) {
        std::cerr << ex.what();
        return ERROR_OPENING_CONFIG;
    } catch (const boost::wrapexcept<boost::program_options::required_option>& ex) {
        std::cerr << ex.what() << "\n";
        return INCORRECT_CONFIG_FILE;
    } catch (const boost::wrapexcept<boost::program_options::unknown_option>& ex) {
        std::cerr << ex.what() << "\n";
        return INCORRECT_CONFIG_FILE;
    } catch (const boost::wrapexcept<boost::program_options::invalid_config_file_syntax>& ex) {
        std::cerr << ex.what() << "\n";
        return INCORRECT_CONFIG_FILE;
    }

    auto indexing_threads_number = options["indexing_threads"].as<int>();

    std::vector<std::thread> indexing_threads;
    indexing_threads.reserve(indexing_threads_number);

    auto start_total = get_current_time_fenced();
    auto start_finding = get_current_time_fenced();
    auto start_reading = get_current_time_fenced();

    DiskReader disk_reader(options);

    std::thread reader_thread(&DiskReader::collect_disk_entries, &disk_reader);

    std::thread processor_thread(&DiskReader::save_disk_entries, std::ref(disk_reader));

    for (int i = 0; i < indexing_threads_number; ++i) {
        indexing_threads.emplace_back(&DiskReader::process_disk_entries, &disk_reader);
    }


    reader_thread.join();

    auto end = get_current_time_fenced();
    auto finding = to_us(end - start_finding);


    processor_thread.join();
    end = get_current_time_fenced();
    auto reading = to_us(end - start_reading);


    for (auto &i: indexing_threads) {
        i.join();
    }

    end = get_current_time_fenced();
    auto total = to_us(end - start_total);

    start_total = get_current_time_fenced();

    try {
        disk_reader.record_results();
    } catch (std::ios_base::failure& ex) {
        std::cerr << ex.what() << std::endl;
        return ERROR_OPENING_OUTPUT_FILE;
    } catch (std::runtime_error& ex) {
        std::cerr << ex.what() << std::endl;
        return ERROR_WRITING_TO_FILE;
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return OUTPUT_FILE_ERROR;
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return OUTPUT_FILE_ERROR;
    }


    end = get_current_time_fenced();
    std::cout << "Total=" << total << std::endl;
    std::cout << "Finding=" << finding << std::endl;
    std::cout << "Reading=" << reading << std::endl;
    std::cout << "Writing=" << to_us(end - start_total) << std::endl;

    return SUCCESS;
}