#include "lexer.hpp"

#include <limits>

#define left_of(a, b) (a < b)

lexer::_internal_tok lexer::_match_leftmost_of(
	const _tok_match * tms,
	size_t len
)
{
	lexer::_internal_tok match_tok = lexer::_internal_tok::_EOI;

	if (_has_input)
	{
		match_tok = lexer::_internal_tok::_NONE;
		ptrdiff_t match_pos = 0;
		ptrdiff_t last_pos = std::numeric_limits<ptrdiff_t>::max();

		const _tok_match * ptm = nullptr;
		matcher * m = nullptr;
		for (size_t i = 0; i < len; ++i)
		{
			ptm = tms+i;
			if ((m = const_cast<matcher *>(ptm->m)))
			{
				if (m->match(_line.c_str(), _line.length(), _line_pos))
				{
					match_pos = m->position();
					if (left_of(match_pos, last_pos))
					{
						last_pos = match_pos;
						match_tok = ptm->t;
						_last_match_len = m->length();
					}
				}
			}
		}

		if (match_tok != lexer::_internal_tok::_NONE)
			_line_pos += last_pos;
	}

	return match_tok;
}

lexer::_internal_tok lexer::_leftmost_non_comment_intl(
	const _tok_match * tm,
	size_t len
)
{
	lexer::_internal_tok ret = lexer::_internal_tok::_NONE;

	if (!_block_comment)
	{
		ret = _match_leftmost_of(tm, len);
		if (lexer::_internal_tok::_COMMENT_START == ret)
		{
			_block_comment = true;
			advance_past_match();
			ret = _leftmost_non_comment_intl(tm, len);
		}
	}
	else
	{
		ret = _match_leftmost_of(_comment_end.data(), _comment_end.size());
		if (lexer::_internal_tok::_COMMENT_END == ret)
		{
			_block_comment = false;
			advance_past_match();
			ret = _leftmost_non_comment_intl(tm, len);
		}
	}

	return ret;
}

bool lexer::also_matches_open()
{
	matcher * open = const_cast<matcher *>(_pats.open);

	return (open
		&& open->match(_line.c_str(), _line.length(), _line_pos)
		&& (open->position() == 0));
}

bool lexer::next_line()
{
	if ((_has_input = static_cast<bool>(std::getline(_in, _line))))
	{
		++_line_no;
		_line_pos = 0;
		_last_match_len = 0;
	}
	return _has_input;
}
