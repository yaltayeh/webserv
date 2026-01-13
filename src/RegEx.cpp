/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegEx.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yaltayeh <yaltayeh@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/05 21:33:13 by mkurkar           #+#    #+#             */
/*   Updated: 2026/01/14 00:23:16 by yaltayeh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RegEx.hpp"

/*
 * This function is like playing a matching game!
 * It checks if a word matches a special pattern.
 * For example:
 * Pattern: "cat*" will match: "cat", "cats", "catfood"
 * Pattern: "?at" will match: "cat", "rat", "hat"
 * The * is like a magic star that matches anything!
 * The ? is like a surprise box that matches any letter!
 * example:
 * Pattern:	h e * o ?
 * String: 	h e l l o !
 * 		   	↓ ↓ ↓ ↓ ↓ ↓
 * Step 1: 	h = h ✓ (exact match)
 * Step 2: 	e = e ✓ (exact match)
 * Step 3: 	* matches 'l l' ✓ (asterisk can match multiple chars)
 * Step 4: 	o = o ✓ (exact match)
 * Step 5: 	? matches '!' ✓ (question mark matches any single char)
 * Result: MATCH ✓
 *
 * This function is like playing a matching game!
 * It checks if a word matches a special pattern.
 * For example:
 * Pattern: "cat*" will match: "cat", "cats", "catfood"
 * Pattern: "?at" will match: "cat", "rat", "hat"
 * The * is like a magic star that matches anything!
 * The ? is like a surprise box that matches any letter!
 */
static int match_pattern(const char *pattern,
						 const char *pattern_end,
						 const char *str,
						 const char *str_end);

RegEx::RegEx(const std::string &pattern)
	: _pattern(pattern)
{
}

RegEx::~RegEx()
{
}

int RegEx::match(const std::string &str) const
{
	const char *pattern_ptr = _pattern.c_str();
	const char *str_ptr = str.c_str();
	return match_pattern(pattern_ptr,
						 pattern_ptr + _pattern.size(),
						 str_ptr,
						 str_ptr + str.size());
}

int RegEx::matchPattern(const std::string &pattern, const std::string &str)
{
	return RegEx(pattern).match(str);
}

static int match_pattern_util(const char *pattern,
							  const char *pattern_end,
							  const char *str,
							  const char *str_end,
							  char qout)
{
	while (pattern != pattern_end && str != str_end)
	{
		if ((*pattern == '\"' || *pattern == '\'') && (*pattern == qout || qout == '\0'))
		{
			if (qout)
				qout = '\0';
			else
				qout = *pattern;
			pattern++;
			continue;
		}
		else if (*pattern == '*' && qout == '\0')
			return (match_pattern(pattern, pattern_end, str, str));
		else if (*pattern == '?' || *pattern == *str)
		{
			pattern++;
			str++;
			continue;
		}
		return (0);
	}
	while (*pattern == '*')
		pattern++;
	return (pattern == pattern_end && str == str_end);
}

static int match_pattern(const char *pattern,
						 const char *pattern_end,
						 const char *str,
						 const char *str_end)
{
	while (*pattern == '*')
		pattern++;
	if (pattern >= pattern_end)
		return (1);
	while (*str)
	{
		if (match_pattern_util(pattern, pattern_end, str, str_end, '\0'))
			return (1);
		str++;
	}
	return (match_pattern_util(pattern, pattern_end, str, str_end, '\0'));
}
