#ifndef MATCHER_FACTORY_HPP
#define MATCHER_FACTORY_HPP

#include "matcher_base.hpp"

class matcher_factory
{
public:
	matcher * create(
		matcher::type t,
		const char * pattern,
		uint32_t f = matcher::flags::NONE
	);
};
#endif
