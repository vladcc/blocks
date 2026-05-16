#include "regex_matcher.hpp"

#define DEFAULT_FALGS \
(std::regex::ECMAScript|std::regex::optimize|std::regex::nosubs)

regex_matcher::regex_matcher(const char * rx, uint32_t opts) :
	 matcher(),
	 m_str_rx(rx ? rx : ""),
	 m_prx(nullptr),
	 m_start(0)
{
	if (rx)
	{
		auto re_flags = DEFAULT_FALGS;

		if (opts & matcher::flags::ICASE)
		{
			re_flags |= std::regex::icase;
			matcher::m_is_icase = true;
		}

		m_prx.reset(new std::regex(rx, re_flags));
	}
}
