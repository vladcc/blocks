#ifndef STRING_MATCHER_HPP
#define STRING_MATCHER_HPP

#include <cstddef>
#include <string>

#include "matcher_base.hpp"

class str_matcher : public matcher
{
public:
	str_matcher(const char * text, uint32_t opts);
	bool match(const char * text, size_t len, size_t start) override;
	ptrdiff_t position() override
	{
		return _pos;
	}
	size_t length() override
	{
		return _pattern.length();
	}
	
private:
	void _tolower(std::string& text);
	
private:
	std::string _pattern;
	std::string _icase_buff;
	ptrdiff_t _pos;
	uint32_t _opts;
};
#endif
