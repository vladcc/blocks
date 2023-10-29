#ifndef STRING_MATCHER_HPP
#define STRING_MATCHER_HPP

#include <cstddef>
#include <string>

#include "matcher_base.hpp"

class str_matcher : public matcher
{
public:
	str_matcher(const char * text, uint32_t opts);
	ptrdiff_t match(const char * text) override;
	
private:
	void _tolower(std::string& text);
	
private:
	std::string _pattern;
	std::string _icase_buff;
	uint32_t _opts;
};
#endif
