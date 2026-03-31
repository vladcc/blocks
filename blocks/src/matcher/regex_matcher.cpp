#include "regex_matcher.hpp"

#define DEFAULT_FALGS \
(std::regex_constants::ECMAScript | std::regex_constants::optimize)

regex_matcher::regex_matcher(const char * rx, uint32_t opts) :
	 _str_rx(rx ? rx : ""),
	 _prx(nullptr)
{
	if (rx)
	{
		auto re_flags = DEFAULT_FALGS;

		if (opts & matcher::flags::ICASE)
			re_flags |= std::regex_constants::icase;

		_prx.reset(new std::regex(rx, re_flags));
	}
}
