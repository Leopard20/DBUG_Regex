#if defined(_MSC_BUILD)
#define _CRT_SECURE_NO_WARNINGS
#define DLLEXPORT __declspec (dllexport)
#define STDCALL __stdcall
#else
#define DLLEXPORT __attribute__((dllexport))
#define STDCALL 
#endif

#include <string>
#include <cstring>
#include <regex>
#include <sstream>
#include "sqf-value/sqf-value/methodhost.hpp"
#include "sqf-value/sqf-value/value.hpp"

/*
#include <cctype>

/// Try to find in the Haystack the Needle - ignore case
bool find_icase(const std::string& strHaystack, const std::string& strNeedle)
{
	auto it = std::search(
		strHaystack.begin(), strHaystack.end(),
		strNeedle.begin(), strNeedle.end(),
		[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
	);
	return (it != strHaystack.end());
}
*/

extern "C"
{
	DLLEXPORT void STDCALL RVExtension(char* output, int outputSize, const char* function);
	DLLEXPORT int STDCALL RVExtensionArgs(char* output, int outputSize, const char* function, const char** argv, int argc);
	DLLEXPORT void STDCALL RVExtensionVersion(char* output, int outputSize);
}

void STDCALL RVExtension(char* output, int outputSize, const char* function)
{
	auto temp_str = "1";
	strcpy_s(output, sizeof(temp_str), temp_str);
}

using ret_d = sqf::method::ret<sqf::value, float>;
int STDCALL RVExtensionArgs(char* output, int outputSize, const char* function, const char** argv, int argc)
{
	auto res = sqf::methodhost::instance().execute(output, outputSize, function, argv, argc);
	return res;
}

void STDCALL RVExtensionVersion(char* output, int outputSize)
{
	auto temp_str = "v1.0";
	strcpy_s(output, sizeof(temp_str), temp_str);
}

ret_d verify(std::string text)
{
	try {
		std::regex rx_find(text);
		
		return ret_d::ok(1);
	}
	catch (...)
	{
		return ret_d::err(0);
	}
}

sqf::methodhost& sqf::methodhost::instance()
{
	using namespace std::string_literals;
	static sqf::methodhost h({
		{ "verify", {sqf::method::create(verify) } },
		});
	return h;
}