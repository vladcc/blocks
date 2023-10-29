#include <cstring>

#include "string_matcher.hpp"

str_matcher::str_matcher(const char * pattern, uint32_t opts) :
	_pattern(pattern ? pattern : ""), _icase_buff(""), _opts(opts)
{
	if (!_pattern.empty() && (_opts & matcher::flags::ICASE))
		_tolower(_pattern);
}

ptrdiff_t str_matcher::match(const char * text)
{
	ptrdiff_t ret = matcher::NO_MATCH;
	
	if (!_pattern.empty())
	{
		if (_opts & matcher::flags::ICASE)
		{
			_icase_buff.assign(text);
			_tolower(_icase_buff);
			text = _icase_buff.c_str();
		}
		
		const char * found = strstr(text, _pattern.c_str());
		if (found)
			ret = found - text;
	}
	
	return ret;
}

void str_matcher::_tolower(std::string& text)
{
	char * str = const_cast<char *>(text.c_str());
	for (size_t i = 0, end = text.length(); i < end; ++i)
		str[i] = tolower(str[i]);
}
