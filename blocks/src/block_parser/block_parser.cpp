#include "block_parser.hpp"

#include <cstdio>

void block_parser::init(const char * fname)
{
	m_lexer.reset();
	m_fname = fname;
}

bool block_parser::parse_block()
{
	p_clear_block();
	p_clear_error();
	bool has_block_start = p_find_block_name();

	if (has_block_start && !(p_find_block_open() && p_get_block_body()))
		p_error_report_generate();

	return has_block_start;
}

bool block_parser::p_get_block_body()
{
	bool ret = false;
	size_t stack = 0;
	lexer::tok which = lexer::tok::NONE;

	while (p_find_open_or_close(&which))
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

bool block_parser::p_find_block_name()
{
	bool found_name = false;
	lexer::tok which = lexer::tok::NONE;

	while ((which = m_lexer.block_name()) != lexer::tok::EOI)
	{
		found_name = (lexer::tok::NAME == which);
		if (!found_name)
		{
			m_lexer.next_line();
			continue;
		}
		else
			break;
	}

	return found_name;
}

bool block_parser::p_find_block_open()
{
	bool found_block_open = false;
	lexer::tok which = lexer::tok::NONE;

	while ((which = m_lexer.block_name_open_close()) != lexer::tok::EOI)
	{
		p_save_line_unique(which);
		switch (which)
		{
			default:
			case lexer::tok::NONE:
			{
				m_lexer.next_line();
				continue;
			} break;

			case lexer::tok::NAME:
			{
				p_clear_block();

				if (m_lexer.also_matches_open())
				{
					found_block_open = true;
					goto done;
				}
				else
				{
					m_lexer.advance_past_match();
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

bool block_parser::p_find_open_or_close(lexer::tok * out_which)
{
	bool ret = false;
	lexer::tok which = lexer::tok::NONE;

	while ((which = m_lexer.block_open_close()) != lexer::tok::EOI)
	{
		p_save_line_unique(which);

		if (lexer::tok::NONE == which)
			m_lexer.next_line();
		else if ((lexer::tok::OPEN == which) || (lexer::tok::CLOSE == which))
		{
			m_lexer.advance_past_match();
			ret = true;
			break;
		}
	}

	*out_which = which;
	return ret;
}

void block_parser::p_error_report_generate()
{
	const std::vector<block_line>& block = m_block.get_content();
	size_t first_line_no = block.front().get_line_no();
	const std::string& bad_line = block.back().get_line();

	m_error.create(
		first_line_no,
		bad_line,
		m_fname,
		m_lexer.line_num(),
		m_lexer.line_pos()
	);
}

void block_parser::parsed_block::save_line(
	const char * txt,
	size_t line_num,
	lexer::tok token
)
{
	if (m_last_saved_line_no != line_num)
	{
		m_content.emplace_back(txt, line_num);
		m_last_saved_line_no = line_num;
	}
	m_content.back().mark_token(token);
}

void block_parser::parsed_block::reset()
{
	m_content.clear();
	m_last_saved_line_no = 0;
}

block_parser::error::error() :
	m_did_error_happen(false)
{
	m_text.emplace_back("");
	m_text.emplace_back("");
	m_text.emplace_back("");
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

	m_did_error_happen = true;

	std::string * err = &m_text[0];
	if (fname)
		err->append(fname).append(":");

	err->append(std::to_string(lex_line_num)).append(":");
	err->append(std::to_string(lex_line_pos+1));
	err->append(": improper nesting from line ");
	err->append(std::to_string(first_line_no));

	err = &m_text[1];
	err->append(bad_line_text);

	err = &m_text[2];
	for (size_t i = 0; i < lex_line_pos; ++i)
		err->append(('\t' == bad_line_text[i]) ? "\t" : " ");
	err->append("^");
}

void block_parser::error::reset()
{
	m_did_error_happen = false;
	for (auto& str : m_text)
		str.clear();
}
