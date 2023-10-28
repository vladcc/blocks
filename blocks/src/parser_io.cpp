#include "parser_io.hpp"
#include <limits>

size_t parser_io::match_leftmost_of(const std::regex* rparr[], size_t len)
{
	size_t which_one = 0;
	
	if(_has_input)
	{
		ptrdiff_t mpos = 0;
		ptrdiff_t pos = std::numeric_limits<ptrdiff_t>::max();
		const char * pstr = _line.c_str();
		const std::regex * prg = nullptr;
		
		for (size_t i = 0; i < len; ++i)
		{
			prg = rparr[i];
			if (prg && std::regex_search((pstr + _match_so_far), _match, *prg)
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
