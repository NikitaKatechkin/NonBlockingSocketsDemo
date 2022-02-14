#pragma once

#include <string>
#include <locale>
#include <algorithm>
#include <cctype>

namespace PNet
{
	namespace Helpers
	{
		inline void ltrim(std::string& s)
		{
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch)
			{
				return !(std::isspace(ch) || ch == '\0');
			}));
		}

		inline void rtrim(std::string& s)
		{
			s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch)
			{
				return !(std::isspace(ch) || ch == '\0');
			}).base(), s.end());
		}

		inline void trim(std::string& s)
		{
			ltrim(s);
			rtrim(s);
		}

		inline std::string ltrim_copy(std::string s)
		{
			ltrim(s);
			return s;
		}

		inline std::string rtrim_copy(std::string s)
		{
			rtrim(s);
			return s;
		}

		inline std::string trim_copy(std::string s)
		{
			trim(s);
			return s;
		}
	}
}