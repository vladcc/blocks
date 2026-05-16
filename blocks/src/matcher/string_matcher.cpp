#include "string_matcher.hpp"

str_matcher::str_matcher(const char * pattern, uint32_t opts) :
	matcher(),
	m_pattern(pattern ? pattern : ""),
	m_ppat(nullptr),
	m_pos(0),
	m_plen(0),
	m_opts(opts),
	m_cfirst('\0'),
	m_icase(false)
{
	if (!m_pattern.empty())
	{
		if (m_opts & matcher::flags::ICASE)
		{
			m_icase = true;
			matcher::m_is_icase = true;
			p_tolower(m_pattern);
		}
		m_ppat = m_pattern.c_str();
		m_plen = m_pattern.length();
		m_cfirst = m_ppat[0];
	}
}

bool str_matcher::match(const char * text, size_t len, size_t start)
{
	if (start >= len)
		return false;

	if (m_ppat)
	{
		size_t ppos = 0;
		for (size_t i = start; i < len; ++i)
		{
			if (p_ch_case(text[i]) == m_cfirst)
			{
				ppos = 1;
				for (size_t j = i+1; j < len && ppos < m_plen; ++j, ++ppos)
				{
					if (p_ch_case(text[j]) != m_ppat[ppos])
						break;
				}

				if (m_plen == ppos)
				{
					m_pos = i;
					return true;
				}
			}
		}
	}

	return false;
}

