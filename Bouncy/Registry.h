// Registry.h: interface for the RegWriter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTRY_H__FF502DE8_D1C2_42DA_8196_1BAA02A3074C__INCLUDED_)
#define AFX_REGISTRY_H__FF502DE8_D1C2_42DA_8196_1BAA02A3074C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if WIN32

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

#endif // !defined(AFX_REGISTRY_H__FF502DE8_D1C2_42DA_8196_1BAA02A3074C__INCLUDED_)
