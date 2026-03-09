#ifndef MATCHER_FACTORY_HPP
#define MATCHER_FACTORY_HPP

#include "matcher_base.hpp"

#include <memory>

class matcher_factory
{
public:
	std::unique_ptr<matcher> create(
		matcher::type t,
		const char * pattern,
		uint32_t f = matcher::flags::NONE
	);
};
#endif
