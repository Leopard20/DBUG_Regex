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
#include <iterator>
#include <algorithm>
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

#define ERROR_NOT_ENOUGH_ARGS -1
#define ERROR_INVALID_ARGS -2
#define ERROR_INVALID_REGEX -3

#define COPY_STREAM(x) strcpy_s(output, x.str().length() + 1, x.str().c_str())

extern "C"
{
	DLLEXPORT void STDCALL RVExtension(char* output, int outputSize, const char* function);
	DLLEXPORT int STDCALL RVExtensionArgs(char* output, int outputSize, const char* function, const char** argv, int argc);
	DLLEXPORT void STDCALL RVExtensionVersion(char* output, int outputSize);
}

class stream : public std::stringstream
{
public:
	stream& set(const char* chars) noexcept {
		this->clear();
		this->str(chars);
		return *this;
	}
};

void STDCALL RVExtension(char* output, int outputSize, const char* function)
{
	stream sstream;
	sstream.set("Error: Not enough input arguments");
	COPY_STREAM(sstream);
}

int STDCALL RVExtensionArgs(char* output, int outputSize, const char* function, const char** argv, int argc)
{
	stream sstream;

	std::string func{ function };

	if (func[0] == '"')
	{
		func = func.substr(1, func.length() - 2);
	}

	if (argc < (func == "r" ? 3 : 2)) {
		sstream.set("Error: Not enough input arguments");
		COPY_STREAM(sstream);
		return ERROR_NOT_ENOUGH_ARGS;
	};

	std::string text{ argv[0] };

	if (text == "") {
		sstream.set("Error: Invalid arguments");
		COPY_STREAM(sstream);
		return ERROR_INVALID_ARGS;
	}

	std::string find_what{ argv[1] };
	std::string replace_with = (func == "r") ? argv[2] : "";

	int icase{ 1 };
	
	if (argc > 3)
	{
		sstream.set(argv[3]) >> icase;
		if (sstream.fail()) {
			icase = 1;
		}
	}

	int iter{ 0 };

	if (argc > 4)
	{
		sstream.set(argv[4]) >> iter;
		if (sstream.fail())
		{
			iter = 0;
		}
	}

	if (find_what[0] == '"')
	{
		find_what = find_what.substr(1, find_what.length() - 2);
	}
	if (text[0] == '"')
	{
		text = text.substr(1, text.length() - 2);
	}
	if (replace_with[0] == '"')
	{
		replace_with = replace_with.substr(1, replace_with.length() - 2);
	}

	try {
		std::regex rx_find(find_what);

		if (icase) {
			rx_find.assign(find_what, std::regex_constants::icase);
		}

		[&]{
			if (func == "f") {
				std::smatch rx_match;
				if (std::regex_search(text, rx_match, rx_find))
				{
					size_t startFrom = rx_match.position() - std::count(text.cbegin(), text.cbegin() + rx_match.position(), '"') / 2;
					size_t length = rx_match.length() - std::count(text.cbegin() + rx_match.position(), text.cbegin() + rx_match.position() + rx_match.length(), '"') / 2;
					sstream.set("") << '[' << startFrom << ',' << length << ']';
				}
				else
				{
					sstream.set("[-1, 0]");
				}
				COPY_STREAM(sstream);

				return;
			}
			
			auto words_begin = std::sregex_iterator(text.begin(), text.end(), rx_find);
			auto words_end = std::sregex_iterator();

			size_t cntMatch = std::distance(words_begin, words_end);

			if (func == "c") {
				sstream.set("") << cntMatch;
				COPY_STREAM(sstream);
				return;
			}

			std::string result = std::regex_replace(text, rx_find, replace_with);	

			sstream.set("") << "[\"\",]" << cntMatch;
			size_t allowedSize = outputSize - sstream.str().size() - 1;

			if (result.size() - iter * allowedSize > allowedSize) {
				size_t charSize(sizeof(result[0]));
				charSize = charSize > 1 ? charSize : 1;
				size_t start(std::min(iter * allowedSize / charSize, result.length() - 1));
				size_t length(std::min(allowedSize / charSize, result.length() - start));
				result = result.substr(start, length);
				++iter;
			}
			else 
			{
				iter = 0;
			}

			sstream.set("") << "[\"" << result << "\"," << cntMatch << ']';
			COPY_STREAM(sstream);
			return;
		}();
	}
	catch (std::regex_error)
	{
		sstream.set("Error: Invalid regex expression");
		COPY_STREAM(sstream);
		return ERROR_INVALID_REGEX;
	}

	return iter;
}

void STDCALL RVExtensionVersion(char* output, int outputSize)
{
	stream sstream;
	sstream.set("v1.0");
	COPY_STREAM(sstream);
}