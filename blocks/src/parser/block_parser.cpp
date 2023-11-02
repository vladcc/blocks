#include <cstdio>

#include "block_parser.hpp"

void block_parser::init(const char * fname)
{
	_lexer.reset();
	_last_line_saved = 0;
	_fname = fname;
}

bool block_parser::parse_block()
{
	_current_block.clear();
	_error_report_clear();
	
	bool has_block_start = _find_block_name();
	
	if (has_block_start && !_get_block_body())
		_error_report_generate();
	
	return has_block_start;
}

bool block_parser::_get_block_body()
{	
	bool ret = false;
	size_t stack = 0;
	lexer::tok which = lexer::tok::NONE;
	
	while (_find_open_or_close(&which))
	{
		if (lexer::tok::OPEN == which)
			++stack;
		else if (lexer::tok::CLOSE == which)
		{
			if (!stack)
				goto out;
			else if (!(--stack))
				break;
		}
	}
	
	if (!stack)
		ret = true;	
out:
	return ret;
}

bool block_parser::_find_block_name()
{
	lexer::tok which = lexer::tok::NONE;
	
	while ((which = _lexer.block_name()) != lexer::tok::EOI)
	{
		if (lexer::tok::NONE == which)
		{
			_lexer.next_line();
			continue;
		}
		else if (lexer::tok::NAME == which)
		{
			_save_line_unique(which);
			break;
		}
	}
	
	return (lexer::tok::NAME == which);
}

bool block_parser::_find_open_or_close(lexer::tok * out_which)
{
	bool ret = false;
	lexer::tok which = lexer::tok::NONE;
	
	while ((which = _lexer.block_open_close()) != lexer::tok::EOI)
	{
		_save_line_unique(which);
		
		if (lexer::tok::NONE == which)
			_lexer.next_line();
		else if ((lexer::tok::OPEN == which) || (lexer::tok::CLOSE == which))
		{
			_lexer.advance_past_match();
			ret = true;
			break;
		}
	}
	
	*out_which = which;
	return ret;
}

void block_parser::_save_line_unique(lexer::tok token)
{
	size_t line_num = _lexer.line_num();
	
	if (_last_line_saved != line_num)
	{
		_current_block.emplace_back(_lexer.get_line(), line_num, token);
		_last_line_saved = line_num;
	}
	else if (_current_block.size())
		_current_block.back().tok_mask |= token;
}

void block_parser::_error_report_clear()
{
	if (!_is_error_report_cleared)
	{
		for (auto& str : _error_report)
			str.clear();
		_is_error_report_cleared = true;
	}
}

void block_parser::_error_report_generate()
{
	_is_error_report_cleared = false;
	
	const block_line& first_line = _current_block.front();
	const block_line& bad_line = _current_block.back();
	
	std::string * err = &_error_report[0];
	if (_fname)
		err->append("file ").append(_fname).append(",");
	
	err->append(" line ").append(std::to_string(_lexer.line_num()));
	err->append(", col ").append(std::to_string(_lexer.line_pos()));
	err->append(": improper nesting from line ");
	err->append(std::to_string(first_line.line_no));
	
	err = &_error_report[1];
	err->append(bad_line.line);
	
	size_t real_pos = _lexer.line_pos();
	real_pos = (real_pos) ? real_pos-1 : 0;
	
	err = &_error_report[2];
	err->append(real_pos, ' ').append("^");
}
