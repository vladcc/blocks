#ifndef PARSER_IO_HPP
#define PARSER_IO_HPP

#include "regex_matcher.hpp"
#include "matcher.hpp"

#include <iostream>
#include <vector>
#include <array>

class lexer
{
public:
	enum tok : uint32_t {
		NONE  = 0x00,
		NAME  = 0x01,
		OPEN  = 0x02,
		CLOSE = 0x04,
		EOI   = 0x40
	};

	struct matchers
	{
		matchers(
			const matcher * block_name  = nullptr,
			const matcher * block_open  = nullptr,
			const matcher * block_close = nullptr,
			const matcher * comment       = nullptr,
			const matcher * comment_start = nullptr,
			const matcher * comment_end   = nullptr,
			const regex_matcher * string_rx = nullptr
		) :
			name(block_name),
			open(block_open),
			close(block_close),
			comment(comment),
			comment_start(comment_start),
			comment_end(comment_end),
			string_rx(string_rx)
		{}

		const matcher * name;
		const matcher * open;
		const matcher * close;
		const matcher * comment;
		const matcher * comment_start;
		const matcher * comment_end;
		const regex_matcher * string_rx;
	};

public:
	lexer(std::istream& in, const matchers& pats) :
		m_pats(pats),
		m_str_find(pats.string_rx),
		m_in(in),
		m_line_pos(0),
		m_line_no(0),
		m_last_match_len(0),
		m_has_input(false),
		m_block_comment(false)
	{
		m_name[0] = {m_pats.comment,       I_COMMENT};
		m_name[1] = {m_pats.comment_start, I_COMMENT_START};
		m_name[2] = {m_pats.name,          I_NAME};

		m_name_open_close[0] = {m_pats.comment,       I_COMMENT};
		m_name_open_close[1] = {m_pats.comment_start, I_COMMENT_START};
		m_name_open_close[2] = {m_pats.name,          I_NAME};
		m_name_open_close[3] = {m_pats.open,          I_OPEN};
		m_name_open_close[4] = {m_pats.close,         I_CLOSE};

		m_open_close[0] = {m_pats.comment,       I_COMMENT};
		m_open_close[1] = {m_pats.comment_start, I_COMMENT_START};
		m_open_close[2] = {m_pats.open,          I_OPEN};
		m_open_close[3] = {m_pats.close,         I_CLOSE};

		m_comment_end[0] = {m_pats.comment_end, I_COMMENT_END};
	}

	inline tok block_name()
	{
		return p_leftmost_non_comment(m_name.data(), m_name.size());
	}

	inline tok block_name_open_close()
	{
		return p_leftmost_non_comment(
			m_name_open_close.data(),
			m_name_open_close.size()
		);
	}

	inline tok block_open_close()
	{
		return p_leftmost_non_comment(m_open_close.data(), m_open_close.size());
	}

	bool next_line();
	bool also_matches_open();

	void reset()
	{
		m_in.clear();
		m_line.clear();
		m_line_no = 0;
		m_line_pos = 0;
		m_last_match_len = 0;
		m_has_input = false;
		next_line();
	}

	inline void advance_past_match()
	{
		m_line_pos += m_last_match_len;
		m_last_match_len = 0;
	}

	inline const std::string& get_line()
	{return m_line;}

	inline bool has_input()
	{return m_has_input;}

	inline size_t line_num()
	{return m_line_no;}

	inline size_t line_pos()
	{return m_line_pos;}

private:
	enum p_internal_tok : uint32_t {
		I_NAME,
		I_OPEN,
		I_CLOSE,
		I_EOI,
		I_COMMENT,
		I_COMMENT_START,
		I_COMMENT_END,
		I_NONE
	};

	struct i_tok_match
	{
		const matcher * m;
		p_internal_tok t;
	};

protected:
	class string_finder
	{
	public:
		string_finder(const regex_matcher * string_rx) :
			m_str_rx(string_rx)
		{
			m_ranges.reserve(8);
		}

		void find_strings(const char * str, size_t len);
		bool is_in_string(size_t pos) const;

	std::array<i_tok_match, 1> m_comment_end;
		inline const auto& o_test_get_ranges() const
		{
			return m_ranges;
		}

	private:
		struct range
		{
			range(size_t s, size_t e) :
				start(s), end(e)
			{}

			size_t start;
			size_t end;
		};
		std::vector<range> m_ranges;
		const regex_matcher * m_str_rx;
	};

private:
	bool p_match(matcher * m, const char * text, size_t len, size_t start);
	p_internal_tok p_match_leftmost_of(const i_tok_match * tm, size_t len);
	p_internal_tok p_leftmost_non_comment_intl(
		const i_tok_match * tm,
		size_t len
	);

	inline tok p_leftmost_non_comment(const i_tok_match * tm, size_t len)
	{
		tok ret = tok::NONE;
		switch(p_leftmost_non_comment_intl(tm, len))
		{
			case p_internal_tok::I_NAME:  ret = tok::NAME;  break;
			case p_internal_tok::I_OPEN:  ret = tok::OPEN;  break;
			case p_internal_tok::I_CLOSE: ret = tok::CLOSE; break;
			case p_internal_tok::I_EOI:   ret = tok::EOI;   break;
			default:                      ret = tok::NONE;  break;
		}
		return ret;
	}

private:
	matchers m_pats;
	string_finder m_str_find;
	std::array<i_tok_match, 5> m_name_open_close;
	std::array<i_tok_match, 4> m_open_close;
	std::array<i_tok_match, 3> m_name;
	std::array<i_tok_match, 1> m_comment_end;
	std::string m_line;
	std::istream& m_in;
	size_t m_line_pos;
	size_t m_line_no;
	size_t m_last_match_len;
	bool m_has_input;
	bool m_block_comment;
};
#endif
