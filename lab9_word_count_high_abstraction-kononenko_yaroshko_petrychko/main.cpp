// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include "options_parser.h"
#include "config_parser/config_parser.h"
#include <utility>
#include <vector>
#include <boost/locale.hpp>
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
    INDIR_DOES_NOT_EXIST = 26,
    OUTPUT_FILE_ERROR = 27
};


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

    auto indir = options["indir"].as<std::string>();
    if (!fs::exists(indir)) {
        std::cerr << "Input directory does not exist!" << std::endl;
        return INDIR_DOES_NOT_EXIST;
    } else if (!fs::is_directory(indir)) {
        std::cerr << "Input directory is not a directory!" << std::endl;
        return INDIR_DOES_NOT_EXIST;
    } else if (fs::is_empty(indir)) {
        std::cerr << "Input directory is empty!" << std::endl;
        return INDIR_DOES_NOT_EXIST;
    } else if (fs::status(indir).permissions() == fs::perms::none) {
        std::cerr << "No permissions to read input directory!" << std::endl;
        return INDIR_DOES_NOT_EXIST;
    } else if (fs::status(indir).permissions() == fs::perms::unknown) {
        std::cerr << "Unknown permissions to read input directory!" << std::endl;
        return INDIR_DOES_NOT_EXIST;
    } else if (fs::status(indir).permissions() == fs::perms::mask) {
        std::cerr << "Mask permissions to read input directory!" << std::endl;
        return INDIR_DOES_NOT_EXIST;
    }

    auto indexing_threads_number = options["indexing_threads"].as<int>();

    DiskReader diskReader(options);

    auto start_total = get_current_time_fenced();

    auto collect_filter = tbb::make_filter<void, fs::path>(
            tbb::filter_mode::serial_in_order,
            [&diskReader](tbb::flow_control &fc) -> fs::path {
                return diskReader.collect_disk_entries(fc);
            });

    auto save_filter = tbb::make_filter<fs::path, se_pair>(
            tbb::filter_mode::serial_in_order,
            [&diskReader](fs::path file_path) -> se_pair {
                return diskReader.save_disk_entries(file_path);
            });

    auto process_filter = tbb::make_filter<se_pair, void>(
            tbb::filter_mode::parallel,
            [&diskReader](se_pair entry) {
                diskReader.process_disk_entries(entry);
            });

    tbb::parallel_pipeline(
            indexing_threads_number,
            collect_filter
            &
            save_filter
            &
            process_filter
    );

    auto end = get_current_time_fenced();

    std::cout << "Total=" << to_us(end - start_total) << std::endl;

    start_total = get_current_time_fenced();
    try {
        diskReader.record_results();
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

    std::cout << "Recording results=" << to_us(end - start_total) << std::endl;

    return SUCCESS;
}