#ifndef FIND_FILES_HPP
#define FIND_FILES_HPP

#include "regex_matcher.hpp"

#include <vector>
#include <string>

bool find_files(
	const char * dir_name,
	bool recursive,
	const regex_matcher * files_include,
	const regex_matcher * files_exclude,
	std::vector<std::string>& out_file_list,
	std::string& out_err
);

#endif
