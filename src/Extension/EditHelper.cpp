#include <WTypes.h>
#include <regex>

extern "C" int isValidRegex(LPCSTR str)
{
	try
	{
		std::regex reg(str);
		return 1;
	}
	catch (...)
	{
		return 0;
	}
};