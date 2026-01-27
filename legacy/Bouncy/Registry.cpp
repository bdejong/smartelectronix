#include "Registry.h"

#if _MSC_VER

#include "winbase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Registry::Registry(std::string base)
{
	std::string tmp = "Software\\" + base;
	LONG error = RegCreateKey(HKEY_CURRENT_USER,tmp.c_str(),&key);

	if(error != ERROR_SUCCESS)
	{
		key = 0;

		LPVOID lpMsgBuf;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,error,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0,NULL);

		MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

		LocalFree(lpMsgBuf);
	}
}

Registry::~Registry()
{
	if(key != 0)
		RegCloseKey(key);
}


void Registry::setString(std::string name, std::string data)
{
	if(key != 0)
	{
		RegSetValueEx(key,name.c_str(),0,REG_SZ,(const unsigned char *)data.c_str(),data.length());
	}
}

std::string	Registry::getString(std::string name)
{
	if(key != 0)
	{
		DWORD type = 0;
		BYTE data[256];
		DWORD size = 256;

		long error = RegQueryValueEx(key,name.c_str(),NULL,&type,data,&size);

		if(error == ERROR_SUCCESS && type == REG_SZ && size != 0)
		{
			return std::string((char *)data);
		}
		else
			return "";
	}
	else
		return "";
}

void Registry::setLong(std::string name, long data)
{
	if(key != 0)
	{
		RegSetValueEx(key,name.c_str(),0,REG_DWORD,(const unsigned char *)&data,4);
	}
}

long Registry::getLong(std::string name)
{
	if(key != 0)
	{
		DWORD type = 0;
		DWORD data;
		DWORD size = sizeof(data);

		long error = RegQueryValueEx(key,name.c_str(),NULL,&type,(unsigned char *)&data,&size);

		if(error == ERROR_SUCCESS && type == REG_DWORD && size == 4)
		{
			return data;
		}
		else
			return -1;
	}
	else
		return -1;
}

#endif
