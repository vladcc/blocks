#include "block_parser.hpp"
#include <cstdio>

#define CALL   "call  "
#define RETURN "return"

#define log(act) if (_streams.log) {dbg_log(act, __func__, -1);} else {}
#define log_call()     log(CALL)
#define log_return()   log(RETURN)

#define logv(act, val)\
if (_streams.log) {dbg_log(act, __func__, (val));} else {}

#define logv_call(val)   logv(CALL, val)
#define logv_return(val) logv(RETURN, val)

const char block_parser::default_prefix[] = "block_parser";

void block_parser::dbg_log(const char * action, const char * fname, int val)
{
	if (_streams.log)
	{
		std::ostream& log_strm = *_streams.log;
		
		log_strm << _streams.log_pref << action << ' ' << fname << "()";
			
		if (val != -1)
			log_strm << " val " << val;
			
		log_strm << " line " << _parse_io.line_num()
			<< " position " << _parse_io.line_pos()
			<< '\n';
	}
}

bool block_parser::parse(const char * fname)
{
	_parse_io.reset();
	_parse_io.read_line();
	_current_block.clear();
	_was_line_saved = false;
	_fname = fname;
	
	bool ret = false;
	while (_parse_io.has_input())
	{
		if (block_name())
			ret = true;
		else
			break;
	}
	
	return ret;
}

bool block_parser::block_name()
{
	log_call();
	
	bool ret = false;
	bool last = false;
	while (until_name())
	{
		last = block_body();
		
		print_block();
		
		if (last)
			ret = true;
		else
			error();
		
		_current_block.clear();
		
		if (!_parse_opts.block_count)
		{
			ret = false;
			break;
		}
	}
	
	logv_return(ret);
	return ret;
}

bool block_parser::block_body()
{	
	log_call(); 
	
	bool ret = false;

	int stack = 0;
	int which = 0;
	
	while (_parse_io.has_input())
	{		
		which = until_open_or_close();
		
		if (OPEN == which)
			++stack;
		else if (CLOSE == which)
		{
			if (stack)
			{
				--stack;
				if (!stack)
					break;
			}
			else
				goto out;
		}
	}
	
	if (!stack)
		ret = true;
		
out:
	logv_return(ret);
	return ret;
}

bool block_parser::until_name()
{
	log_call();
	
	bool ret = false;
	
	const int rarr_size = 2;
	const std::regex * rarr[rarr_size] = {
		_regexps.name,
		_regexps.comment // nullptr is ok
	};
	
	int match = 0;
	while (_parse_io.has_input())
	{
		if ((match = _parse_io.match_first_of(rarr, rarr_size)))
		{	
			if (rarr_size == match) // comment
			{
				next_line();
				continue;
			}
				
			save_line_once(NAME);
			ret = true;
			break;
		}
	
		next_line();
	}
	
	logv_return(ret);
	return ret;
}

int block_parser::until_open_or_close()
{
	log_call();
	
	int which_one = 0;
	
	const int rarr_size = 3;
	const std::regex * rarr[rarr_size] = {
			_regexps.open,
			_regexps.close,
			_regexps.comment // nullptr is ok
	};
	
	while (_parse_io.has_input())
	{
		which_one = _parse_io.match_first_of(rarr, rarr_size);
		
		if (rarr_size == which_one)
			which_one = COMMENT;
		
		save_line_once(which_one);
		
		if (which_one)
		{
			_parse_io.advance_past_match();
			if ((OPEN == which_one) || (CLOSE == which_one))
				break;
		}
		
		next_line();
	}
	
	logv_return(which_one);
	return which_one;
}

bool block_parser::next_line()
{
	log_call();
	_was_line_saved = false;
	
	bool ret = _parse_io.read_line();
	
	logv_return(ret);
	return ret;
}

void block_parser::save_line_once(int token)
{
	log_call();
	if (!_was_line_saved)
	{
		block_line_info nfo(_parse_io.give_line(), token, _parse_io.line_num());
		_current_block.push_back(nfo);
		_was_line_saved = true;
	}
	else if (_current_block.size())
		_current_block.back().what |= token;
		
	log_return();
}

bool block_parser::match_block(const std::regex * rx)
{
	bool ret = false;
	std::cmatch match;
	
	for (auto line : _current_block)
	{
		if (std::regex_search(line.line.c_str(), match, *rx))
		{
			ret = true;
			break;
		}
	}
	
	return ret;
}

void block_parser::print_block()
{
	log_call();
	
	if (!_parse_opts.quiet)
	{
		if (_regexps.regex_match && !match_block(_regexps.regex_match))
			return;
		
		if (_regexps.regex_no_match && match_block(_regexps.regex_no_match))
			return;
		
		if (!_parse_opts.skip_count)
		{ 
			if (-1 == _parse_opts.block_count)
				actual_print_block();
			else if (_parse_opts.block_count > 0)
			{
				actual_print_block();
				--_parse_opts.block_count;
			}
		}
		else
			--_parse_opts.skip_count;
	}
	
	log_return();
}

void block_parser::actual_print_block()
{
	log_call();
	
	if (_parse_opts.print_fname_on_match && _fname)
	{
		std::string fname(_fname);
		fname += ":";
		
		_parse_io.print_line(fname);
		_fname = nullptr;
	}
	
	if (_parse_opts.mark_start)
		_parse_io.print_line(_parse_opts.mark_start);

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
			const int cMax = 16;
			char num[cMax] = {0};
			
			snprintf(num, cMax, "%8d ", _current_block[i].line_no);
			_parse_io.print_str(num);
		}
			
		_parse_io.print_line(_current_block[i].line);
	}

	if (_parse_opts.mark_end)
		_parse_io.print_line(_parse_opts.mark_end);
		
	log_return();
}

void block_parser::error()
{
	log_call();
	
	const block_line_info& first = _current_block[0];
	block_line_info first_line(first.line, first.what, first.line_no);
	
	const block_line_info& last = _current_block.back();
	block_line_info bad_line(last.line, last.what, last.line_no);
	
	std::string err_pref(_streams.err_pref);
	err_pref += "error: ";
	
	std::string err(err_pref);
	
	if (_parse_opts.current_file)
	{
		err += "file ";
		err += *_parse_opts.current_file;
		err += ",";
	}
	
	err += " line ";
	err += std::to_string(_parse_io.line_num());
	err += ", col ";
	err += std::to_string(_parse_io.line_pos());
	err += ": improper nesting from line ";
	err += std::to_string(first_line.line_no);
	_parse_io.print_error(err);
	
	err = err_pref;
	err += bad_line.line;
	_parse_io.print_error(err);
	
	size_t real_pos = _parse_io.line_pos();
	real_pos = (real_pos) ? real_pos-1 : 0;
	
	std::string space(real_pos, ' ');
	space += "^";
	err = err_pref;
	err += space;
	_parse_io.print_error(err);
	
	if (_parse_opts.fatal_error)
		exit(EXIT_FAILURE);
		
	log_return();
}
