#ifndef PARSER_IO_HPP
#define PARSER_IO_HPP

#include <regex>
#include <iostream>

class parser_io
{
	public:
	inline parser_io(std::istream& ins,
		std::ostream& outs,
		std::ostream& errs
	) :
		_ins(ins),
		_outs(outs),
		_cerr(errs),
		_line(""),
		_match_so_far(0),
		_line_no(0),
		_has_input(false)
	{reset();}
	
	bool match_block_name(const std::regex& b_name);
	int match_first_of(const std::regex& b_open, const std::regex& b_close);
	
	bool read_line();
	
	inline void reset()
	{
		_ins.clear();
		_line.clear();
		_line_no = 0;
		_match_so_far = 0;
		_has_input = false;
	}
	
	inline void print_line(const std::string& str)
	{print_line(str.c_str());}
	
	inline void print_line(const char * str)
	{_outs << str << '\n';}
	
	inline void print_error(const std::string& err)
	{print_error(err.c_str());}
	
	inline void print_error(const char * err)
	{_cerr << err << '\n';}
	
	inline void print_str(const std::string& str)
	{print_str(str.c_str());}
	
	inline void print_str(const char * str)
	{_outs << str;}
	
	inline const std::string& give_line()
	{return _line;}
	
	inline bool has_input()
	{return _has_input;}
	
	inline int line_num()
	{return _line_no;}
	
	inline int line_pos()
	{return _match_so_far;}
	
	private:
	std::istream& _ins;
	std::ostream& _outs;
	std::ostream& _cerr;
	std::cmatch _match;
	std::string _line;
	int _match_so_far;
	int _line_no;
	bool _has_input;
};
#endif
