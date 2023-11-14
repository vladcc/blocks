#include <cstring>

#include "string_matcher.hpp"

str_matcher::str_matcher(const char * pattern, uint32_t opts) :
	_pattern(pattern ? pattern : ""),
	_icase_buff(""),
	_pos(0),
	_opts(opts)
{
	if ((_opts & matcher::flags::ICASE) && !_pattern.empty())
		_tolower(_pattern);
}

bool str_matcher::match(const char * text, size_t len, size_t start)
{
	bool ret = false;
	
	if (!_pattern.empty())
	{
		const char * pstart = text + start;
		
		if (_opts & matcher::flags::ICASE)
		{
			_icase_buff.assign(pstart);
			_tolower(_icase_buff);
			pstart = _icase_buff.c_str();
		}
		
		const char * found = strstr(pstart, _pattern.c_str());
		if (found)
		{
			ret = true;
			_pos = found - pstart;
		}
	}
	
	return ret;
}

void str_matcher::_tolower(std::string& text)
{
	char * str = const_cast<char *>(text.c_str());
	for (size_t i = 0, end = text.length(); i < end; ++i)
		str[i] = tolower(str[i]);
}
