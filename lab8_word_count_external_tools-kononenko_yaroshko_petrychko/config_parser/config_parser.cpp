#include <iostream>
#include <boost/program_options.hpp>
#include "config_parser.h"
//std::vector<std::string> allowed_extentions;

po::variables_map parse_config(const std::string &path) {

    po::variables_map options;
    po::options_description desc;
    typedef std::vector<std::wstring> type;
    type extentions;
    desc.add_options()
            ("indir", po::value<std::string>()->required(), "path to indexation directory")
            ("out_by_a", po::value<std::string>()->required(), "results sorted by alphabet")
            ("out_by_n", po::value<std::string>()->required(), "results sorted by number of occurrences")
            ("indexing_extensions", po::wvalue<type>(&extentions)->multitoken(), "allowed indexing extentions")
            ("archives_extensions", po::wvalue<type>(&extentions)->multitoken(), "allowed archives extentions")
            ("max_file_size", po::value<int>()->required(), "maximum size of a file")
            ("indexing_threads", po::value<int>()->required(), "number of indexing threads")
            ("merging_threads", po::value<int>()->required(), "number of merging threads")
            ("filenames_queue_size", po::value<int>()->required(), "maximum size of queue of filenames")
            ("raw_files_queue_size", po::value<int>()->required(), "maximum size of queue of raw files")
            ("dictionaries_queue_size", po::value<int>()->required(), "maximum size of queue of dictionaries");

    std::ifstream file(path);
    po::store(po::parse_config_file(file, desc), options);
    file.close();
    po::notify(options);

    return options;
}