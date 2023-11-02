#include <limits>

#include "lexer.hpp"

lexer::_internal_tok lexer::_match_leftmost_of(
	const _tok_match * tms,
	size_t len
)
{
	lexer::_internal_tok next_tok = lexer::_internal_tok::_EOI;
	
	if (_has_input)
	{
		next_tok = lexer::_internal_tok::_NONE;
		ptrdiff_t mpos = 0;
		ptrdiff_t pos = std::numeric_limits<ptrdiff_t>::max();
		
		const _tok_match * ptm = nullptr;
		matcher * m = nullptr;
		for (size_t i = 0; i < len; ++i)
		{
			ptm = tms+i;
			if ((m = const_cast<matcher *>(ptm->m)))
			{
				mpos = m->match(_line.c_str(), _line.length(), _line_pos);
				if (mpos != matcher::NO_MATCH && mpos < pos)
				{
					pos = mpos;
					next_tok = ptm->t;
					_last_match_len = m->length();
				}
			}
		}
		
		if (next_tok != lexer::_internal_tok::_NONE)
			_line_pos += pos;
	}
	
	return next_tok;
}

lexer::_internal_tok lexer::_internal_any_or_comment(
	const _tok_match * tm,
	size_t len
)
{
	lexer::_internal_tok ret = lexer::_internal_tok::_NONE;
	
	if (_block_comment)
	{
		ret = _match_leftmost_of(_comment_end.data(), _comment_end.size());
		if (lexer::_internal_tok::_COMMENT_END == ret)
		{
			_block_comment = false;
			advance_past_match();
			ret = _internal_any_or_comment(tm, len);
		}
	}
	else
	{
		ret = _match_leftmost_of(tm, len);	
		if (lexer::_internal_tok::_COMMENT_START == ret)
		{
			_block_comment = true;
			ret = lexer::_internal_tok::_NONE;
		}
	}
	
	return ret;
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
