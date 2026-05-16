#include "lexer.hpp"

#include <limits>

#define left_of(a, b) (a < b)

bool lexer::p_match(matcher * m, const char * text, size_t len, size_t start)
{
	ptrdiff_t pos = 0;
	while (m->match(text, len, start))
	{
		if (!m_pats.string_rx)
			return true;

		pos = m->position();
		if (!m_str_find.is_in_string(pos))
			return true;

		start = pos + m->length();
	}
	return false;
}

lexer::p_internal_tok lexer::p_match_leftmost_of(
	const i_tok_match * tms,
	size_t len
)
{
	lexer::p_internal_tok match_tok = lexer::p_internal_tok::I_EOI;

	if (m_has_input)
	{
		match_tok = lexer::p_internal_tok::I_NONE;
		ptrdiff_t match_pos = 0;
		ptrdiff_t last_pos = std::numeric_limits<ptrdiff_t>::max();

		const i_tok_match * ptm = nullptr;
		matcher * m = nullptr;
		const char * pline = m_line.c_str();
		size_t llen = m_line.length();
		for (size_t i = 0; i < len; ++i)
		{
			ptm = tms+i;
			if ((m = const_cast<matcher *>(ptm->m)))
			{
				if (p_match(m, pline, llen, m_line_pos))
				{
					match_pos = m->position();
					if (left_of(match_pos, last_pos))
					{
						last_pos = match_pos;
						match_tok = ptm->t;
						m_last_match_len = m->length();
					}
				}
			}
		}

		if (match_tok != lexer::p_internal_tok::I_NONE)
			m_line_pos = last_pos;
	}

	return match_tok;
}

lexer::p_internal_tok lexer::p_leftmost_non_comment_intl(
	const i_tok_match * tm,
	size_t len
)
{
	lexer::p_internal_tok ret = lexer::p_internal_tok::I_NONE;

	if (!m_block_comment)
	{
		ret = p_match_leftmost_of(tm, len);
		if (lexer::p_internal_tok::I_COMMENT_START == ret)
		{
			m_block_comment = true;
			advance_past_match();
			ret = p_leftmost_non_comment_intl(tm, len);
		}
	}
	else
	{
		ret = p_match_leftmost_of(m_comment_end.data(), m_comment_end.size());
		if (lexer::p_internal_tok::I_COMMENT_END == ret)
		{
			m_block_comment = false;
			advance_past_match();
			ret = p_leftmost_non_comment_intl(tm, len);
		}
	}

	return ret;
}

bool lexer::also_matches_open()
{
	matcher * open = const_cast<matcher *>(m_pats.open);

	return (open
		&& p_match(open, m_line.c_str(), m_line.length(), m_line_pos)
		&& (open->position() == static_cast<ptrdiff_t>(m_line_pos)));
}

bool lexer::next_line()
{
	if ((m_has_input = static_cast<bool>(std::getline(m_in, m_line))))
	{
		++m_line_no;
		m_line_pos = 0;
		m_last_match_len = 0;

		if (m_pats.string_rx)
			m_str_find.find_strings(m_line.c_str(), m_line.length());
	}
	return m_has_input;
}

void lexer::string_finder::find_strings(const char * str, size_t len)
{
	size_t start = 0;
	size_t end = 0;
	regex_matcher * m = const_cast<regex_matcher *>(m_str_rx);

	m_ranges.clear();
	while (m->match(str, len, start))
	{
		start = m->position();
		end = start + m->length();
		m_ranges.emplace_back(start, end);
		start = end;
	}
}

bool lexer::string_finder::is_in_string(size_t pos) const
{
	for (const auto& r : m_ranges)
	{
		if (pos >= r.start && pos < r.end)
			return true;
	}
	return false;
}
