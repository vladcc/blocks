#include "string_matcher.hpp"

str_matcher::str_matcher(const char * pattern, uint32_t opts) :
	_pattern(pattern ? pattern : ""),
	_last_line(nullptr),
	_ppat(nullptr),
	_pos(0),
	_plen(0),
	_opts(opts),
	_cfirst('\0'),
	_icase(false)
{
	if (!_pattern.empty())
	{
		if (_opts & matcher::flags::ICASE)
		{
			_icase = true;
			_tolower(_pattern);
		}
		_ppat = _pattern.c_str();
		_plen = _pattern.length();
		_cfirst = _ppat[0];
	}
}

bool str_matcher::match(const char * text, size_t len, size_t start)
{
	if (start >= len)
		return false;

	if (_ppat)
	{
		size_t ppos = 0;
		for (size_t i = start; i < len; ++i)
		{
			if (_ch_case(text[i]) == _cfirst)
			{
				ppos = 1;
				for (size_t j = i+1; j < len && ppos < _plen; ++j, ++ppos)
				{
					if (_ch_case(text[j]) != _ppat[ppos])
						break;
				}

				if (_plen == ppos)
				{
					_pos = i;
					return true;
				}
			}
		}
	}

	return false;
}

