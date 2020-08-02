#include "parser_io.hpp"
#include <limits>

int parser_io::match_first_of(const std::regex* rparr[], int len)
{
	int which_one = 0;
	
	if(_has_input)
	{
		int mpos = 0;
		int pos = std::numeric_limits<int>::max();
		const char * pstr = _line.c_str();
		
		for (int i = 0; i < len; ++i)
		{
			if (rparr[i] &&
				std::regex_search((pstr + _match_so_far), _match, *(rparr[i]))
			)
			{
				mpos = _match.position();
				if (mpos < pos)
				{
					pos = mpos;
					which_one = i+1;
				}
			}
		}
		
		if (which_one)
			_match_so_far += pos;
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
