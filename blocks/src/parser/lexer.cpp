#include <limits>

#include "lexer.hpp"

lexer::tok lexer::_match_leftmost_of(const tok_match * tms, size_t len)
{
	tok next_tok = EOI;
	
	if (_has_input)
	{
		next_tok = NONE;
		ptrdiff_t mpos = 0;
		ptrdiff_t pos = std::numeric_limits<ptrdiff_t>::max();
		const char * pstr = _line.c_str();
		
		const tok_match * ptm = nullptr;
		matcher * m = nullptr;
		for (size_t i = 0; i < len; ++i)
		{
			ptm = tms+i;
			if ((m = const_cast<matcher *>(ptm->m)))
			{
				mpos = m->match(pstr + _match_so_far);
				if (mpos != matcher::NO_MATCH && mpos < pos)
				{
					pos = mpos;
					next_tok = ptm->t;
				}
			}
		}
		
		if (next_tok != NONE)
			_match_so_far += pos;
	}
	
	return next_tok;
}

lexer::tok lexer::block_name_or_comment()
{
	return _match_leftmost_of(_name_or_comment.data(), _name_or_comment.size());
}

lexer::tok lexer::block_open_close_or_comment()
{
	return _match_leftmost_of(
		_open_close_or_comment.data(),
		_open_close_or_comment.size()
	);
}

bool lexer::next_line()
{
	if ((_has_input = static_cast<bool>(std::getline(_in, _line))))
	{
		++_line_no;
		_match_so_far = 0;
	}
	return _has_input;
}
