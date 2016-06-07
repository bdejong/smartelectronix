#pragma once

#if _MSC_VER

#include <string>
#include <windows.h>

class Registry
{
public:
	void			setString(std::string name, std::string data);
	std::string		getString(std::string name);

	void			setLong(std::string name, long data);
	long			getLong(std::string name);


	Registry(std::string base);
	virtual ~Registry();

private:
	HKEY key;
};

#endif
