#include "block_parser.hpp"
#include "matcher.hpp"
#include "find_files.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <filesystem>

#define BLOCKS_EXIT_HAD_MATCH EXIT_SUCCESS
#define BLOCKS_EXIT_NO_MATCH  1
#define BLOCKS_EXIT_HAD_ERROR 2

static const char * program_name = "blocks";
static const char * program_version = "4.1";

static const char * str_stdin = "-";

enum ematcher {
	M_FIRST = 0,
	B_NAME = M_FIRST,
	B_START,
	B_END,
	B_LINE_COMMENT,
	B_COMMENT_BEGIN,
	B_COMMENT_TERM,
	STRING_RX,
	FILES_INCLUDE_RX,
	FILES_EXCLUDE_RX,
	M_SCALAR_TOTAL,

	// needed for handle_matcher(), not used as indexes
	V_MATCH,
	V_DONT_MATCH,
};

enum elang {
	LANG_NONE = 0,
	LANG_C,
	LANG_AWK,
	LANG_JSON,
	LANG_XML,
	LANG_INFO,
	LANG_END
};

struct mdata {
	const char * pat;
	bool is_regex;
	bool is_icase;
};

struct mM_mdata_vect {
	std::vector<mdata> match;
	std::vector<mdata> dont_match;
};

struct mM_matchers_vect {
	std::vector<std::unique_ptr<matcher>> match;
	std::vector<std::unique_ptr<matcher>> dont_match;
};

struct patterns {
	const matcher * matchers[M_SCALAR_TOTAL];
	mM_matchers_vect mM_vect;
};

struct m_single_type {
	bool is_plus_type_arg;
	bool is_only_next_regex;
};

struct m_single_case {
	bool is_plus_case_arg;
	bool is_only_next_icase;
};

struct match_logic {
	bool do_logic;
	bool and_mM_together;
};

struct prog_options {
	std::vector<const char *> * file_names;
	mdata matchers[M_SCALAR_TOTAL];
	mM_mdata_vect mM_vect;
	const char * mark_start;
	const char * mark_end;
	const char * file_list;
	const char * files_dir;
	const char * lang_name;
	elang which_lang;
	int block_count;
	int skip_count;
	bool line_numbers;
	bool with_filename;
	bool files_with_match;
	bool files_without_match;
	bool are_all_matchers_icase;
	bool ignore_top;
	bool no_defaults;
	bool fatal_error;
	bool verbose_error;
	bool debug;
	bool no_strings;
	bool recursive;
	bool are_all_matchers_regex;
	m_single_type next_type;
	m_single_case next_case;
	match_logic match_how;
};

struct process_result {
	bool was_match;
	bool was_err;
};

struct {
	const char * block_open    = "{";
	const char * block_close   = "}";
	const char * string_rx = "\"([^\\\\\"]|[\\\\].)*\"";
} defaults;

static void print_err(const char * str)
{
	std::cerr << program_name << ": error: " << str << std::endl;
}

static inline void exit_err()
{
	exit(BLOCKS_EXIT_HAD_ERROR);
}

static void errq(const char * str)
{
	print_err(str);
	exit_err();
}

#include "opts_impl.ic"

static void print_line(const char * str)
{
	std::cout << str << std::endl;
}

static void print_str(const char * str)
{
	std::cout << str;
}

static void print_line_stderr(const char * str)
{
	std::cerr << str << std::endl;
}

static void print_str_stderr(const char * str)
{
	std::cerr << str;
}

static const char * line_num_str(size_t num)
{
	const int num_max_len = 32;
	static char num_str[num_max_len];
	snprintf(num_str, num_max_len, "%zu:", num);
	return num_str;
}

static void fatal_error_exit()
{
	std::string err("quitting due to --");
	err.append(fatal_error_opt_long);
	print_line_stderr(err.c_str());
	exit_err();
}

static void print_block_stderr(
	const std::vector<block_parser::block_line>& block
)
{
	for (const auto& b_line : block)
	{
		print_str_stderr(line_num_str(b_line.get_line_no()));
		print_line_stderr(b_line.get_line().c_str());
	}
}

static void print_error_report(const std::vector<std::string>& report)
{
	for (const auto& str : report)
		print_err(str.c_str());
}

static void print_block(
	const prog_options& opts,
	const char * fname,
	const std::vector<block_parser::block_line>& block
)
{
	static std::string fname_on_match("");

	bool with_filename = opts.with_filename;
	bool ignore_top = opts.ignore_top;
	bool line_numbers = opts.line_numbers;
	const char * mark_start = opts.mark_start;
	const char * mark_end = opts.mark_end;

	if (with_filename && fname)
		fname_on_match.assign(fname).append(":");

	if (mark_start)
		print_line(mark_start);

	for (size_t i = 0, end = block.size(); i < end; ++i)
	{
		if (ignore_top && (0 == i))
		{
			size_t j = 0;
			while (j < end)
			{
				if (block[j].has_token(lexer::tok::OPEN))
					break;
				++j;
			}

			i = j;
			continue;
		}

		if (ignore_top && (i == (end-1)))
			break;

		if (with_filename)
			print_str(fname_on_match.c_str());

		if (line_numbers)
			print_str(line_num_str(block[i].get_line_no()));

		print_line(block[i].get_line().c_str());
	}

	if (mark_end)
		print_line(mark_end);
}


static void print_debug_a_matcher(const char * str, const matcher * mtchr)
{
	static std::string buff;

	buff.assign(str).append("'").
		append(mtchr ? mtchr->pattern() : "").
		append("' type: ").
		append(mtchr ? mtchr->type_of() : "none").
		append(" case: ");

		if (mtchr)
		{
			buff.push_back(
				mtchr->is_icase() ?
					case_insensitive_opt_short
					:
					case_sensitive_opt_short
			);
		}
		else
		{
			buff.append("none");
		}

	print_line(buff.c_str());
}

static void print_debug_and_quit(
	const prog_options& opts,
	const patterns& pats
)
{
	static const char * dbg_str[] = {
		"block name: ",
		"block start: ",
		"block end: ",
		"line comment: ",
		"block comment begin: ",
		"block comment terminate: ",
		"string rx: ",
		"files include: ",
		"files exclude: ",
	};

	std::string buff;

	// defaults
	buff.assign("default block name: default block start");
	print_line(buff.c_str());

	buff.assign("default block start: '").append(defaults.block_open).
		append("'");
	print_line(buff.c_str());

	buff.assign("default block end: '").append(defaults.block_close).
		append("'");
	print_line(buff.c_str());

	// lang
	buff.assign("lang: ").append(opts.lang_name);
	print_line(buff.c_str());

	// matchers
	for (int i = M_FIRST; i < M_SCALAR_TOTAL; ++i)
		print_debug_a_matcher(dbg_str[i], pats.matchers[i]);

	buff.assign("files dir: ").append(opts.files_dir ? opts.files_dir : "none");
	print_line(buff.c_str());

	buff.assign("recurse: ").append(opts.recursive ? "yes" : "no");
	print_line(buff.c_str());

	buff.assign("no strings: ");
	buff.append(opts.no_strings ? "on" : "off");
	print_line(buff.c_str());

	if (pats.mM_vect.match.empty())
	{
		print_debug_a_matcher("match: ", nullptr);
	}
	else
	{
		for (const auto& m : pats.mM_vect.match)
			print_debug_a_matcher("match: ", m.get());
	}

	if (pats.mM_vect.dont_match.empty())
	{
		print_debug_a_matcher("don't match: ", nullptr);
	}
	else
	{
		for (const auto& m : pats.mM_vect.dont_match)
			print_debug_a_matcher("don't match: ", m.get());
	}

	buff.assign("match/don't match logic: ");
	if (opts.match_how.do_logic)
		buff.append(opts.match_how.and_mM_together ? "and" : "or");
	else
		buff.append("none");
	print_line(buff.c_str());

	exit(EXIT_SUCCESS);
}

static matcher * make_a_pattern(const mdata * data, matcher_factory& mfact)
{
	static const matcher::type mtypes[2] = {
		matcher::type::STRING,
		matcher::type::REGEX
	};

	static const uint32_t matcher_flags[2] = {
		matcher::flags::NONE,
		(matcher::flags::NONE | matcher::flags::ICASE)
	};

	matcher * ret = nullptr;
	const char * mpat = data->pat;
	bool is_regex = data->is_regex;
	bool is_icase = data->is_icase;

	if (mpat && *mpat)
		 ret = mfact.create(mtypes[is_regex], mpat, matcher_flags[is_icase]);

	return ret;
}

static void make_patterns(const prog_options& opts, patterns& pats)
{
	// so valgrind can track
	static std::unique_ptr<matcher> matchers[M_SCALAR_TOTAL];

	try
	{
		matcher_factory mfact;
		for (int i = M_FIRST; i < M_SCALAR_TOTAL; ++i)
			matchers[i].reset(make_a_pattern(opts.matchers + i, mfact));

		for (const auto& data : opts.mM_vect.match)
			pats.mM_vect.match.emplace_back(make_a_pattern(&data, mfact));

		for (const auto& data : opts.mM_vect.dont_match)
			pats.mM_vect.dont_match.emplace_back(make_a_pattern(&data, mfact));

		for (int i = M_FIRST; i < M_SCALAR_TOTAL; ++i)
			pats.matchers[i] = matchers[i].get();
	}
	catch(const std::runtime_error& e)
	{
		errq(e.what());
	}
}

// <process>
// <match>
static bool match_in_block(
	const matcher * pat,
	const std::vector<block_parser::block_line>& block
)
{
	bool ret = false;
	matcher * pm = const_cast<matcher *>(pat);

	if (pm)
	{
		const std::string * ps = nullptr;
		for (const auto& line : block)
		{
			ps = &(line.get_line());
			if (pm->match(ps->c_str(), ps->length(), 0))
			{
				ret = true;
				break;
			}
		}
	}

	return ret;
}

static bool all_matched(
	const std::vector<std::unique_ptr<matcher>> * vect,
	const std::vector<block_parser::block_line>& block
)
{
	const std::unique_ptr<matcher> * data = vect->data();
	for (size_t i = 0, end = vect->size(); i < end; ++i)
	{
		if (!match_in_block(data[i].get(), block))
			return false;
	}

	return true;
}

static bool all_didnt_match(
	const std::vector<std::unique_ptr<matcher>> * vect,
	const std::vector<block_parser::block_line>& block
)
{
	const std::unique_ptr<matcher> * data = vect->data();
	for (size_t i = 0, end = vect->size(); i < end; ++i)
	{
		if (match_in_block(data[i].get(), block))
			return false;
	}

	return true;
}

static bool match_a_block(
	prog_options& opts,
	const std::vector<std::unique_ptr<matcher>> * match,
	const std::vector<std::unique_ptr<matcher>> * dont_match,
	const std::vector<block_parser::block_line>& block
)
{
	if (match && dont_match)
	{
		if (opts.match_how.and_mM_together)
		{
			return (
				all_matched(match, block)
				&&
				all_didnt_match(dont_match, block)
			);
		}
		else
		{
			return (
				all_matched(match, block)
				||
				all_didnt_match(dont_match, block)
			);
		}
	}
	else if (match)
	{
		return all_matched(match, block);
	}
	else if (dont_match)
	{
		return all_didnt_match(dont_match, block);
	}

	return false;
}
// </match>

static bool process_a_block(
	prog_options& opts,
	const std::vector<std::unique_ptr<matcher>> * match,
	const std::vector<std::unique_ptr<matcher>> * dont_match,
	const std::vector<block_parser::block_line>& block
)
{
	if (match || dont_match)
	{
		if (!match_a_block(opts, match, dont_match, block))
			return false;
	}

	bool should_print = false;

	if (0 == opts.skip_count)
	{
		if (-1 == opts.block_count)
		{
			should_print = true;
		}
		else if (opts.block_count > 0)
		{
			should_print = true;
			--opts.block_count;
		}
	}
	else
	{
		--opts.skip_count;
	}

	return should_print;
}

static process_result process_blocks_from_file(
	block_parser& parser,
	prog_options& opts,
	const char * fname,
	const std::vector<std::unique_ptr<matcher>> * match,
	const std::vector<std::unique_ptr<matcher>> * dont_match
)
{
	process_result res;
	res.was_match = false;
	res.was_err = false;

	parser.init(fname);
	while (parser.parse_block())
	{
		if (0 == opts.block_count)
			return res;

		if (parser.had_error())
		{
			res.was_err = true;
			if (opts.verbose_error)
				print_block_stderr(parser.get_block());

			print_error_report(parser.get_error_report());

			if (opts.fatal_error)
				fatal_error_exit();
		}
		else
		{
			if (process_a_block(
					opts,
					match,
					dont_match,
					parser.get_block()
				))
			{
				res.was_match = true;
				if (opts.files_with_match)
				{
					print_line(fname);
					return res;
				}
				else if (!opts.files_without_match)
				{
					print_block(opts, fname, parser.get_block());
				}
			}
		}
	}

	if (!res.was_match && opts.files_without_match)
		print_line(fname);

	return res;
}

static void process_file(
	process_result& total,
	block_parser& parser,
	prog_options& opts,
	const char * fname,
	const std::vector<std::unique_ptr<matcher>> * match,
	const std::vector<std::unique_ptr<matcher>> * dont_match
)
{
	process_result curr = process_blocks_from_file(
		parser,
		opts,
		fname,
		match,
		dont_match
	);

	if (!total.was_match)
		total.was_match = curr.was_match;

	if (!total.was_err)
		total.was_err = curr.was_err;
}

static int process(
	prog_options& opts,
	const patterns& pats,
	const std::vector<const char *>& file_names
)
{
	if (0 == opts.block_count)
		return BLOCKS_EXIT_HAD_MATCH;

	const std::vector<std::unique_ptr<matcher>> * v_match =
		pats.mM_vect.match.empty() ? nullptr : &(pats.mM_vect.match);

	const std::vector<std::unique_ptr<matcher>> * v_dont_match =
		pats.mM_vect.dont_match.empty() ? nullptr : &(pats.mM_vect.dont_match);

	bool was_file_open_err = false;
	process_result total;
	total.was_match = false;
	total.was_err = false;

	const char * current_file = str_stdin;

	std::ifstream file_in_stream;
	std::istream generic_in_stream(std::cin.rdbuf());

	lexer::matchers lex_matchers(
		pats.matchers[B_NAME],
		pats.matchers[B_START],
		pats.matchers[B_END],
		pats.matchers[B_LINE_COMMENT],
		pats.matchers[B_COMMENT_BEGIN],
		pats.matchers[B_COMMENT_TERM],
		static_cast<const regex_matcher *>(pats.matchers[STRING_RX])
	);

	lexer lex(generic_in_stream, lex_matchers);
	block_parser b_parser(lex);

	if (!file_names.size())
	{
		process_file(
			total,
			b_parser,
			opts,
			current_file,
			v_match,
			v_dont_match
		);
	}
	else
	{
		static std::string err;


		for (auto& fname : file_names)
		{
			current_file = fname;
			if (std::filesystem::is_directory(current_file))
			{
				was_file_open_err = true;
				err.assign(current_file).append(": ").append("Is a directory");
				print_err(err.c_str());
				continue;
			}

			if (0 == strcmp(current_file, str_stdin))
			{
				generic_in_stream.rdbuf(std::cin.rdbuf());
			}
			else
			{
				file_in_stream.open(current_file);
				if (file_in_stream.is_open())
				{
					generic_in_stream.rdbuf(file_in_stream.rdbuf());
				}
				else
				{
					was_file_open_err = true;
					err.assign(current_file).append(": ");
					err.append(std::strerror(errno));
					print_err(err.c_str());
					continue;
				}
			}

			// counts per file
			int block_count = opts.block_count;
			int skip_count = opts.skip_count;
			process_file(
				total,
				b_parser,
				opts,
				current_file,
				v_match,
				v_dont_match
			);
			opts.block_count = block_count;
			opts.skip_count = skip_count;

			if (file_in_stream.is_open())
			{
				file_in_stream.close();
				file_in_stream.clear();
			}
		}
	}

	if (total.was_err || was_file_open_err)
		return BLOCKS_EXIT_HAD_ERROR;

	if (!total.was_match)
		return BLOCKS_EXIT_NO_MATCH;

	return BLOCKS_EXIT_HAD_MATCH;
}
// </process>

// <extra_file_lists>
static void append_dir_search(
	const prog_options& opts,
	const patterns& pats,
	std::vector<const char *>& file_names
)
{
	static std::vector<std::string> dir_files;

	if (opts.files_dir)
	{
		std::string err("foo");

		bool found_files = find_files(
			opts.files_dir,
			opts.recursive,
			static_cast<const regex_matcher *>(pats.matchers[FILES_INCLUDE_RX]),
			static_cast<const regex_matcher *>(pats.matchers[FILES_EXCLUDE_RX]),
			dir_files,
			err
		);

		if (!found_files)
			errq(err.c_str());

		for (auto& fname : dir_files)
			file_names.push_back(fname.c_str());
	}
}

static void append_file_list(
	const prog_options& opts,
	std::vector<const char *>& file_names
)
{
	static std::vector<std::string> flist;

	if (opts.file_list)
	{
		std::ifstream file_list;
		std::istream generic_in_stream(std::cin.rdbuf());


		if (0 != strcmp(opts.file_list, str_stdin))
		{
			file_list.open(opts.file_list);
			if (file_list.is_open())
			{
				generic_in_stream.rdbuf(file_list.rdbuf());
			}
			else
			{
				std::string err;
				err.assign("file list: ").append(opts.file_list);
				err.append(": ").append(std::strerror(errno));
				errq(err.c_str());
			}
		}

		std::string line;
		while (std::getline(generic_in_stream, line))
			flist.push_back(line);

		if (!flist.empty())
		{
			for (size_t i = 0, end = flist.size(); i < end; ++i)
				file_names.push_back(flist[i].c_str());
		}
	}
}

static void append_extra_file_lists(
	const prog_options& opts,
	const patterns& pats,
	std::vector<const char *>& file_names
)
{
	append_dir_search(opts, pats, file_names);
	append_file_list(opts, file_names);
}
// </extra_file_lists>

int main(int argc, char * argv[])
{
	static prog_options opts;
	static patterns pats;
	static std::vector<const char *> file_names;

	handle_options(argc, argv, opts, file_names);
	make_patterns(opts, pats);

	if (opts.debug)
		print_debug_and_quit(opts, pats);

	append_extra_file_lists(opts, pats, file_names);
	return process(opts, pats, file_names);
}
