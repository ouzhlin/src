/********************************************************************
 CREATED: 18/1/2014   14:54
 FILE: 	 JOFileMgr.cpp
 AUTHOR:  James Ou
 *********************************************************************/

#include "manager/JOFileMgr.h"
#include "utils/JOTime.h"
#include "utils/JOLog.h"
#include "utils/JOPath.h"

#include <algorithm>
#include <ios>
#include <fstream>

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

using namespace std;

NS_JOFW_BEGIN

std::string JOFileMgr::getFileExplorerDir(std::string rootPath/*=""*/)
{
    return "";
}

NS_JOFW_END
