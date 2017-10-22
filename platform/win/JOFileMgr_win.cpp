/********************************************************************
CREATED: 18/1/2014   14:54
FILE: 	 JOFileMgr.cpp
AUTHOR:  James Ou 
*********************************************************************/

#if _WIN32

#include "manager/JOFileMgr.h"
#include "utils/JOTime.h"
#include "utils/JOLog.h"
#include "utils/JOPath.h"

#include <algorithm>
#include <ios>
#include <fstream>
//////////////////////
#include <Windows.h>
#include <ShlObj.h>
//#include <regex>
#include <io.h>
#include <direct.h>
using namespace std;

NS_JOFW_BEGIN

std::string JOFileMgr::getFileExplorerDir(std::string rootPath/*=""*/)
{

	LPITEMIDLIST   pList = NULL;
	BROWSEINFOA bs;
	char buff[MAX_PATH] = "\0";
	memset(&bs, 0, sizeof(BROWSEINFOA));
	bs.pszDisplayName = buff;
	bs.pidlRoot = pList;
	bs.lpszTitle = "select dir";
	bs.ulFlags = BIF_RETURNONLYFSDIRS;
	bs.lParam = NULL;
	bs.lpfn = NULL;
	bs.hwndOwner = NULL;
	if (rootPath.length() > 0)
	{
		LPITEMIDLIST  pIdl = NULL;
		IShellFolder* pDesktopFolder;
		char          szPath[MAX_PATH];
		OLECHAR       olePath[MAX_PATH];
		ULONG         chEaten;
		ULONG         dwAttributes;

		strcpy(szPath, rootPath.c_str());
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
		{
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szPath, -1, olePath, MAX_PATH);
			pDesktopFolder->ParseDisplayName(NULL, NULL, olePath, &chEaten, &pIdl, &dwAttributes);
			pDesktopFolder->Release();
		}
		bs.pidlRoot = pIdl;
	}
	pList = SHBrowseForFolderA(&bs);
	if (pList){
		if (SHGetPathFromIDListA(pList, buff))
		{
			return JOPath::standardisePath(buff, false);
		}
	}

	return "";
}

NS_JOFW_END

#endif

