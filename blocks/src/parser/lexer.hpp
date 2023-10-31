#ifndef PARSER_IO_HPP
#define PARSER_IO_HPP

#include <iostream>
#include <array>

#include "matcher.hpp"

class lexer
{
public:
	enum tok : uint32_t {
		NONE    = 0x00,
		NAME    = 0x01,
		OPEN    = 0x02,
		CLOSE   = 0x04,
		COMMENT = 0x08,
		// COMMENT_OPEN  = 0x10,
		// COMMENT_CLOSE = 0x20,
		EOI     = 0x40
	};
	
	struct matchers
	{
		matchers(const matcher * block_name,
			const matcher * block_open,
			const matcher * block_close,
			const matcher * comment,
			const matcher * pat_match,
			const matcher * pat_no_match
		) :
			name(block_name),
			open(block_open),
			close(block_close),
			comment(comment),
			pat_match(pat_match),
			pat_no_match(pat_no_match)
		{}
		
		const matcher * name;
		const matcher * open;
		const matcher * close;
		const matcher * comment;
		const matcher * pat_match;
		const matcher * pat_no_match;
	};
	
public:
	inline lexer(std::istream& in, const matchers& pats) :
		_in(in),
		_pats(pats),
		_match_so_far(0),
		_line_no(0),
		_has_input(false)
	{
		_name_or_comment[0] = {_pats.name, NAME};
		_name_or_comment[1] = {_pats.comment, COMMENT};
		_open_close_or_comment[0] = {_pats.open, OPEN};
		_open_close_or_comment[1] = {_pats.close, CLOSE};
		_open_close_or_comment[2] = {_pats.comment, COMMENT};
	}
	
	size_t match_leftmost_of(const matcher * m[], size_t len);
	
	tok block_name_or_comment();
	tok block_open_close_or_comment();
	
	bool next_line();
	
	inline void reset()
	{
		_in.clear();
		_line.clear();
		_line_no = 0;
		_match_so_far = 0;
		_has_input = false;
		next_line();
	}
	
	inline void advance_past_match()
	{++_match_so_far;}
	
	inline const std::string& get_line()
	{return _line;}
	
	inline bool has_input()
	{return _has_input;}
	
	inline size_t line_num()
	{return _line_no;}
	
	inline size_t line_pos()
	{return _match_so_far;}
	
private:
	struct tok_match
	{
		const matcher * m;
		tok t;
	};

private:
	bool _read_line();
	tok _match_leftmost_of(const tok_match * tm, size_t len);

private:
	std::string _line;
	std::array<tok_match, 3> _open_close_or_comment;
	std::array<tok_match, 2> _name_or_comment;
	std::istream& _in;
	matchers _pats;
	size_t _match_so_far;
	size_t _line_no;
	bool _has_input;
};
#endif
