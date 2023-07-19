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
            ("heat_capacity", po::value<double>()->required(), "heat capacity")
            ("thermal_conductivity", po::value<double>()->required(), "thermal conductivity")
            ("density", po::value<double>()->required(), "density")
            ("width", po::value<int>()->required(), "width")
            ("height", po::value<int>()->required(), "height")
            ("initialTemp", po::value<double>()->required(), "initial temp")
            ("boundaryTemp", po::value<double>()->required(), "boundary temp")
            ("delta_x", po::value<double>()->required(), "delta_x")
            ("delta_y", po::value<double>()->required(), "delta_y")
            ("delta_t", po::value<double>()->required(), "delta t")
            ("iterations_save", po::value<int>()->required(), "iterations per every save")
            ("max_iterations", po::value<int>()->required(), "iterations to run")
            ("initial_conditions", po::value<std::string>()->required(), "file to initial conditions");

    std::ifstream file(path);
    po::store(po::parse_config_file(file, desc), options);
    file.close();
    po::notify(options);

    return options;
}