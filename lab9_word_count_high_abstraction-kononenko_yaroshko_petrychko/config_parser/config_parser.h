#ifndef TEMPLATE_CONFIG_PARSER_H
#define TEMPLATE_CONFIG_PARSER_H

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

po::variables_map parse_config(const std::string &path);

#endif //TEMPLATE_CONFIG_PARSER_H
