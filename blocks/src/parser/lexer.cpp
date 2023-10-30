#include <limits>

#include "lexer.hpp"

size_t lexer::match_leftmost_of(const matcher * m[], size_t len)
{
	size_t which_one = 0;
	
	if(_has_input)
	{
		ptrdiff_t mpos = 0;
		ptrdiff_t pos = std::numeric_limits<ptrdiff_t>::max();
		const char * pstr = _line.c_str();
		
		matcher * pm = nullptr;
		for (size_t i = 0; i < len; ++i)
		{
			if ((pm = const_cast<matcher *>(m[i])))
			{
				mpos = pm->match(pstr + _match_so_far);
				if (mpos != matcher::NO_MATCH && mpos < pos)
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

bool lexer::read_line()
{
	if ((_has_input = bool(std::getline(_ins, _line))))
	{
		++_line_no;
		_match_so_far = 0;
	}
	return _has_input;
}
