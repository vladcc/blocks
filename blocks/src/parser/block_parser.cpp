#include "block_parser.hpp"

#include <cstdio>

void block_parser::init(const char * fname)
{
	_lexer.reset();
	_fname = fname;
}

bool block_parser::parse_block()
{
	_clear_block();
	_clear_error();
	bool has_block_start = _find_block_name();

	if (has_block_start && !(_find_block_open() && _get_block_body()))
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
	bool found_name = false;
	lexer::tok which = lexer::tok::NONE;

	while ((which = _lexer.block_name()) != lexer::tok::EOI)
	{
		found_name = (lexer::tok::NAME == which);
		if (!found_name)
		{
			_lexer.next_line();
			continue;
		}
		else
			break;
	}

	return found_name;
}

bool block_parser::_find_block_open()
{
	bool found_block_open = false;
	lexer::tok which = lexer::tok::NONE;

	while ((which = _lexer.block_name_open_close()) != lexer::tok::EOI)
	{
		switch (which)
		{
			default:
			case lexer::tok::NONE:
			{
				_save_line_unique(which);
				_lexer.next_line();
				continue;
			} break;

			case lexer::tok::NAME:
			{
				_clear_block();
				_save_line_unique(which);

				if (_lexer.also_matches_open())
				{
					found_block_open = true;
					goto done;
				}
				else
				{
					_lexer.advance_past_match();
					continue;
				}
			} break;

			case lexer::tok::OPEN:
			{
				found_block_open = true;
				goto done;
			} break;

			case lexer::tok::CLOSE:
			{
				found_block_open = false;
				goto done;
			}
		}
	}

done:
	return found_block_open;
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

void block_parser::_error_report_generate()
{
	const std::vector<block_line>& block = _block.get_content();
	size_t first_line_no = block.front().get_line_no();
	const std::string& bad_line = block.back().get_line();

	_error.create(
		first_line_no,
		bad_line,
		_fname,
		_lexer.line_num(),
		_lexer.line_pos()
	);
}

void block_parser::parsed_block::save_line(
	const char * txt,
	size_t line_num,
	lexer::tok token
)
{
	if (_last_saved_line_no != line_num)
	{
		_content.emplace_back(txt, line_num);
		_last_saved_line_no = line_num;
	}
	_content.back().mark_token(token);
}

void block_parser::parsed_block::reset()
{
	_content.clear();
	_last_saved_line_no = 0;
}

block_parser::error::error() :
	_did_error_happen(false)
{
	_text.emplace_back("");
	_text.emplace_back("");
	_text.emplace_back("");
}

void block_parser::error::create(
	size_t first_line_no,
	const std::string& bad_line_text,
	const char * fname,
	size_t lex_line_num,
	size_t lex_line_pos
)
{
	reset();

	_did_error_happen = true;

	std::string * err = &_text[0];
	if (fname)
		err->append(fname).append(":");

	err->append(std::to_string(lex_line_num)).append(":");
	err->append(std::to_string(lex_line_pos+1));
	err->append(": improper nesting from line ");
	err->append(std::to_string(first_line_no));

	err = &_text[1];
	err->append(bad_line_text);

	err = &_text[2];
	for (size_t i = 0; i < lex_line_pos; ++i)
		err->append(('\t' == bad_line_text[i]) ? "\t" : " ");
	err->append("^");
}

void block_parser::error::reset()
{
	_did_error_happen = false;
	for (auto& str : _text)
		str.clear();
}
