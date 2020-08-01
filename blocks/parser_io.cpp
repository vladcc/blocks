#include "parser_io.hpp"
#include <limits>

bool parser_io::match_block_name(const std::regex& b_name)
{
	bool ret = false;
	
	if (has_input())
	{
		const char * pstr = _line.c_str();
		if (std::regex_search((pstr + _match_so_far), _match, b_name))
		{
			_match_so_far += _match.position();
			ret = true;
		}
	}	
	return ret;
}

int parser_io::match_first_of(const std::regex& b_open,
	const std::regex& b_close
)
{
	int which_one = 0;
	
	if (has_input())
	{
		int pos = std::numeric_limits<int>::max();
		
		const char * pstr = _line.c_str();
		if (std::regex_search((pstr + _match_so_far), _match, b_open))
		{
			pos = _match.position();
			which_one = 1;
		}
		
		if (std::regex_search((pstr + _match_so_far), _match, b_close))
		{
			int mpos = _match.position();
			if (mpos < pos)
			{
				pos = mpos;
				which_one = 2;
			}
		}
		
		if (which_one)
			_match_so_far += (pos+1);
	}
	return which_one;
}	

bool parser_io::read_line()
{
	if ((_has_input = bool(std::getline(_ins, _line))))
	{
		++_line_no;
		_match_so_far = 0;
	}
	return _has_input;
}
