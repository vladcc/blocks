#ifndef MATCHER_BASE_HPP
#define MATCHER_BASE_HPP

#include <cstddef>

class matcher
{
public:
	// returned by match()
	enum {
		NO_MATCH = -1
	}; 
	
	enum flags : uint32_t {
		NONE  = 0x00,
		ICASE = 0x01,
	};

public:
	virtual ~matcher() {}
	virtual ptrdiff_t match(const char * text) = 0;
};
#endif
