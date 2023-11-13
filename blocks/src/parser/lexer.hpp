#ifndef PARSER_IO_HPP
#define PARSER_IO_HPP

#include <iostream>
#include <array>

#include "matcher.hpp"

class lexer
{
public:
	enum tok : uint32_t {
		NONE          = 0x00,
		NAME          = 0x01,
		OPEN          = 0x02,
		CLOSE         = 0x04,
		EOI           = 0x40
	};
	
	struct matchers
	{
		matchers(
			const matcher * block_name  = nullptr,
			const matcher * block_open  = nullptr,
			const matcher * block_close = nullptr,
			const matcher * comment       = nullptr,
			const matcher * comment_start = nullptr,
			const matcher * comment_end   = nullptr
		) :
			name(block_name),
			open(block_open),
			close(block_close),
			comment(comment),
			comment_start(comment_start),
			comment_end(comment_end)
		{}
		
		const matcher * name;
		const matcher * open;
		const matcher * close;
		const matcher * comment;
		const matcher * comment_start;
		const matcher * comment_end;
	};
	
public:
	lexer(std::istream& in, const matchers& pats) :
		_in(in),
		_pats(pats),
		_line_pos(0),
		_line_no(0),
		_last_match_len(0),
		_has_input(false),
		_block_comment(false)
	{
		_name[0] = {_pats.name,          _NAME};
		_name[1] = {_pats.comment,       _COMMENT};
		_name[2] = {_pats.comment_start, _COMMENT_START};
		
		_open_close[0] = {_pats.open,          _OPEN};
		_open_close[1] = {_pats.close,         _CLOSE};
		_open_close[2] = {_pats.comment,       _COMMENT};
		_open_close[3] = {_pats.comment_start, _COMMENT_START};
		
		_comment_end[0] = {_pats.comment_end, _COMMENT_END};
	}
		
	inline tok block_name()
	{
		return _leftmost_match(_name.data(), _name.size());
	}

	inline tok block_open_close()
	{
		return _leftmost_match(_open_close.data(), _open_close.size());
	}

	bool next_line();
	
	void reset()
	{
		_in.clear();
		_line.clear();
		_line_no = 0;
		_line_pos = 0;
		_last_match_len = 0;
		_has_input = false;
		next_line();
	}
	
	inline void advance_past_match()
	{
		_line_pos += _last_match_len;
		_last_match_len = 0;
	}
	
	inline const std::string& get_line()
	{return _line;}
	
	inline bool has_input()
	{return _has_input;}
	
	inline size_t line_num()
	{return _line_no;}
	
	inline size_t line_pos()
	{return _line_pos;}

private:
	enum _internal_tok : uint32_t {
		_NAME,
		_OPEN,
		_CLOSE,
		_EOI,
		_COMMENT,
		_COMMENT_START,
		_COMMENT_END,
		_NONE
	};
	
	struct _tok_match
	{
		const matcher * m;
		_internal_tok t;
	};
	
private:
	bool _read_line();
	_internal_tok _match_leftmost_of(const _tok_match * tm, size_t len);
	_internal_tok _leftmost_match_internal(const _tok_match * tm, size_t len);
	
	inline tok _leftmost_match(const _tok_match * tm, size_t len)
	{
		tok ret = tok::NONE;
		switch(_leftmost_match_internal(tm, len))
		{
			case _internal_tok::_NAME:  ret = tok::NAME;  break;
			case _internal_tok::_OPEN:  ret = tok::OPEN;  break;
			case _internal_tok::_CLOSE: ret = tok::CLOSE; break;
			case _internal_tok::_EOI:   ret = tok::EOI;   break;
			default:                    ret = tok::NONE;  break;
		}
		return ret;
	}
	
private:
	std::string _line;
	std::array<_tok_match, 4> _open_close;
	std::array<_tok_match, 3> _name;
	std::array<_tok_match, 1> _comment_end;
	std::istream& _in;
	matchers _pats;
	size_t _line_pos;
	size_t _line_no;
	size_t _last_match_len;
	bool _has_input;
	bool _block_comment;
};
#endif
