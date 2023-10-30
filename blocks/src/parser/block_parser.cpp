#include "block_parser.hpp"
#include <cstdio>

#define CALL   "call  "
#define RETURN "return"

#define log(act) if (_streams.log) {_dbg_log(act, __func__, -1);} else {}
#define log_call()     log(CALL)
#define log_return()   log(RETURN)

#define logv(act, val)\
if (_streams.log) {_dbg_log(act, __func__, (val));} else {}

#define logv_call(val)   logv(CALL, val)
#define logv_return(val) logv(RETURN, val)

const char block_parser::default_prefix[] = "block_parser";

void block_parser::_dbg_log(const char * action, const char * fname, int val)
{
	if (_streams.log)
	{
		std::ostream& log_strm = *_streams.log;
		
		log_strm << _streams.log_pref << action << ' ' << fname << "()";
			
		if (val != -1)
			log_strm << " val " << val;
			
		log_strm << " line " << _lexer.line_num()
			<< " position " << _lexer.line_pos()
			<< '\n';
	}
}

void block_parser::_init_state(const char * fname)
{
	_lexer.reset();
	_lexer.read_line();
	_current_block.clear();
	_was_line_saved = false;
	_fname = fname;
	_fname_on_match.assign(_fname).append(":");
}

bool block_parser::parse(const char * fname)
{
	if (0 == _parse_opts.block_count)
		return false;
	
	_init_state(fname);
	
	bool ret = false;
	bool block_correct = false;
	while (find_block_name())
	{
		block_correct = _get_block_body();
		
		_post_process_block();
		
		if (block_correct)
			ret = true;
		else
			_report_error();
		
		_current_block.clear();
		
		if (0 == _parse_opts.block_count)
			break;
	}
	
	return ret;
}

bool block_parser::_get_block_body()
{	
	log_call(); 
	
	bool ret = false;
	
	tok which = NONE;
	size_t stack = 0;
	while (find_open_or_close(&which))
	{
		if (OPEN == which)
			++stack;
		else if (CLOSE == which)
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
	logv_return(ret);
	return ret;
}

bool block_parser::find_block_name()
{
	log_call();
	
	bool ret = false;
	
	const size_t marr_size = 2;
	const matcher * marr[marr_size] = {
		_matchers.name,
		_matchers.comment // nullptr is ok
	};
	
	size_t match = 0;
	while (_lexer.has_input())
	{
		if ((match = _lexer.match_leftmost_of(marr, marr_size)))
		{	
			if (marr_size == match) // comment is found
			{
				_read_next_line();
				continue;
			}
				
			_save_line_once(NAME);
			ret = true;
			break;
		}
	
		_read_next_line();
	}
	
	logv_return(ret);
	return ret;
}

bool block_parser::find_open_or_close(tok * out_which)
{
	log_call();
	
	bool ret = false;
	size_t which_match = 0;
	
	const size_t marr_size = 3;
	const matcher * marr[marr_size] = {
			_matchers.open,
			_matchers.close,
			_matchers.comment // nullptr is ok
	};
	
	while (_lexer.has_input())
	{
		which_match = _lexer.match_leftmost_of(marr, marr_size);
		
		if (marr_size == which_match)
			which_match = COMMENT;
		
		_save_line_once(static_cast<tok>(which_match));
		
		if (which_match)
		{
			_lexer.advance_past_match();
			if ((OPEN == which_match) || (CLOSE == which_match))
			{
				*out_which = static_cast<tok>(which_match);
				ret = true;
				break;
			}
		}
		
		_read_next_line();
	}
	
	logv_return(which_match);
	return ret;
}

bool block_parser::_read_next_line()
{
	log_call();
	_was_line_saved = false;
	
	bool ret = _lexer.read_line();
	
	logv_return(ret);
	return ret;
}

void block_parser::_save_line_once(tok token)
{
	log_call();
	if (!_was_line_saved)
	{
		_current_block.emplace_back(_lexer.get_line(),
			token, _lexer.line_num());
		_was_line_saved = true;
	}
	else if (_current_block.size())
		_current_block.back().what |= token;
		
	log_return();
}

bool block_parser::_match_in_block(const matcher * m)
{	
	bool ret = false;
	matcher * ccm = const_cast<matcher *>(m);
	
	if (ccm)
	{
		for (auto line : _current_block)
		{
			if (ccm->match(line.line.c_str()) != matcher::NO_MATCH)
			{
				ret = true;
				break;
			}
		}
	}
	
	return ret;
}

void block_parser::_post_process_block()
{
	log_call();
	
	if (!_parse_opts.quiet)
	{
		if (_matchers.pat_match && !_match_in_block(_matchers.pat_match))
			return;
		
		if (_matchers.pat_no_match && _match_in_block(_matchers.pat_no_match))
			return;
		
		if (0 == _parse_opts.skip_count)
		{ 
			if (-1 == _parse_opts.block_count)
				_print_block();
			else if (_parse_opts.block_count > 0)
			{
				_print_block();
				--_parse_opts.block_count;
			}
		}
		else
			--_parse_opts.skip_count;
	}
	
	log_return();
}

void block_parser::_print_block()
{
	log_call();
	
	if (_parse_opts.print_fname_on_match && _fname)
	{
		_lexer.print_line(_fname_on_match);
		_fname = nullptr;
	}
	
	if (_parse_opts.mark_start)
		_lexer.print_line(_parse_opts.mark_start);

	for (size_t i = 0, end = _current_block.size(); i < end; ++i)
	{
		if (_parse_opts.ignore_top && (0 == i))
		{
			size_t j = 0;
			while (j < end)
			{
				if (OPEN & _current_block[j].what)
					break;
				++j;
			}
			
			i = j;
			continue;
		}
		
		if (_parse_opts.ignore_top && (i == (end-1)))
			break;
		
		
		if (_parse_opts.line_numbers)
		{
			const int num_max_len = 16;
			char num[num_max_len] = {0};
			
			snprintf(num, num_max_len, "%8d ", _current_block[i].line_no);
			_lexer.print_str(num);
		}
			
		_lexer.print_line(_current_block[i].line);
	}

	if (_parse_opts.mark_end)
		_lexer.print_line(_parse_opts.mark_end);
		
	log_return();
}

void block_parser::_report_error()
{
	log_call();
	
	_err_pref.clear();
	_err_text.clear();
	_spaces.clear();
	
	const block_line_info& first_line = _current_block.front();
	const block_line_info& bad_line = _current_block.back();
	
	_err_pref = _streams.err_pref;
	_err_pref += "error: ";
	
	_err_text = _err_pref;
	if (_parse_opts.current_file)
	{
		_err_text += "file ";
		_err_text += *_parse_opts.current_file;
		_err_text += ",";
	}
	
	_err_text += " line ";
	_err_text += std::to_string(_lexer.line_num());
	_err_text += ", col ";
	_err_text += std::to_string(_lexer.line_pos());
	_err_text += ": improper nesting from line ";
	_err_text += std::to_string(first_line.line_no);
	_lexer.print_error(_err_text);
	
	_err_text = _err_pref;
	_err_text += bad_line.line;
	_lexer.print_error(_err_text);
	
	size_t real_pos = _lexer.line_pos();
	real_pos = (real_pos) ? real_pos-1 : 0;
	
	_spaces.insert(0, real_pos, ' ');
	_spaces += "^";
	_err_text = _err_pref;
	_err_text += _spaces;
	_lexer.print_error(_err_text);
	
	if (_parse_opts.fatal_error)
		exit(EXIT_FAILURE);
		
	log_return();
}
