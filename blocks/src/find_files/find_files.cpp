#include "find_files.hpp"
#include <filesystem>
#include <cstring>

namespace
{
void filter_file(
	const char * fname,
	const regex_matcher * files_include,
	const regex_matcher * files_exclude,
	std::vector<std::string>& out_file_list
)
{
	bool take = false;
	regex_matcher * mincl = const_cast<regex_matcher *>(files_include);
	regex_matcher * mexcl = const_cast<regex_matcher *>(files_exclude);
	std::size_t len = strlen(fname);

	if (mincl && mexcl)
		take = mincl->match(fname, len, 0) && !mexcl->match(fname, len, 0);
	else if (mincl)
		take = mincl->match(fname, len, 0);
	else if (mexcl)
		take = !mexcl->match(fname, len, 0);
	else
		take = true;

	if (take)
		out_file_list.push_back(fname);
}
}

bool find_files(
	const char * dir_name,
	bool recursive,
	const regex_matcher * files_include,
	const regex_matcher * files_exclude,
	std::vector<std::string>& out_file_list,
	std::string& out_err
)
{
	try
	{
		if (recursive)
		{
			for (const auto& dir_entry
				: std::filesystem::recursive_directory_iterator{
					dir_name,
					std::filesystem::directory_options::follow_directory_symlink
				}
			)
			{
				const char * fname = dir_entry.path().c_str();
				filter_file(fname, files_include, files_exclude, out_file_list);
			}
		}
		else
		{
			for (const auto& dir_entry
				: std::filesystem::directory_iterator{dir_name}
			)
			{
				const char * fname = dir_entry.path().c_str();
				filter_file(fname, files_include, files_exclude, out_file_list);
			}
		}

		return true;
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		out_err.assign(e.what());
		return false;
	}
}
