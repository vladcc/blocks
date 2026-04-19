#ifndef STRING_MATCHER_HPP
#define STRING_MATCHER_HPP

#include "matcher_base.hpp"

#include <cstddef>
#include <string>
#include <cstring>

class str_matcher : public matcher
{
public:
	str_matcher(const char * text, uint32_t opts);
	bool match(const char * text, size_t len, size_t start) override;
	ptrdiff_t position() const override
	{
		return _pos;
	}
	size_t length() const override
	{
		return _pattern.length();
	}
	const char * type_of() const override
	{
		return "string";
	}
	const char * pattern() const override
	{
		return _pattern.c_str();
	}

private:
	inline void _tolower(std::string& text)
	{
		char * str = const_cast<char *>(text.c_str());
		for (size_t i = 0, end = text.length(); i < end; ++i)
			str[i] = tolower(str[i]);
	}
	inline char _ch_case(char ch) const
	{
		return _icase ? tolower(ch) : ch;
	}

private:
	std::string _pattern;
	const char * _last_line;
	const char * _ppat;
	ptrdiff_t _pos;
	size_t _plen;
	uint32_t _opts;
	char _cfirst;
	bool _icase;
};
#endif
