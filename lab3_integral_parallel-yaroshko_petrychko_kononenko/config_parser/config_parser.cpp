#include <iostream>
#include <boost/program_options.hpp>
#include "config_parser.h"


po::variables_map parse_config(const std::string& path) {

    po::variables_map options;
    po::options_description desc;

    desc.add_options()
            ("abs_err", po::value<double>()->required(), "Absolute error")
            ("rel_err", po::value<double>()->required(), "Relative err")
            ("x_start", po::value<double>()->required(),"x start")
            ("x_end", po::value<double>()->required(), "x end")
            ("y_start", po::value<double>()->required(), "y start")
            ("y_end", po::value<double>()->required(), "y end")
            ("init_steps_x", po::value<int>()->required(), "init_steps_x")
            ("init_steps_y", po::value<int>()->required(), "init_steps_y")
            ("max_iter", po::value<int>()->required(), "max_iter");

    std::ifstream file(path);
    po::store(po::parse_config_file(file, desc), options);
    file.close();
    po::notify(options);
    return options;
}