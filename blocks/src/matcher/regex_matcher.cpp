#include "regex_matcher.hpp"

#define DEFAULT_FALGS \
(std::regex::ECMAScript|std::regex::optimize|std::regex::nosubs)

regex_matcher::regex_matcher(const char * rx, uint32_t opts) :
	 _str_rx(rx ? rx : ""),
	 _prx(nullptr)
{
	if (rx)
	{
		auto re_flags = DEFAULT_FALGS;

		if (opts & matcher::flags::ICASE)
			re_flags |= std::regex::icase;

		_prx.reset(new std::regex(rx, re_flags));
	}
}
