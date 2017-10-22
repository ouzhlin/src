/********************************************************************
CREATED: 25/1/2014   11:56
FILE: 	 JOPath.cpp
AUTHOR:  James Ou 
*********************************************************************/

#include "utils/JOPath.h"

#include <algorithm>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <ostream>
#include <stdarg.h>
#include <stdlib.h>
#include <vector>

NS_JOFW_BEGIN

string JOPath::standardisePath(const string &init, bool endSlash/*=true*/)
{
	if (init.empty()){
		return init;
	}
    string path = init;

    replace( path.begin(), path.end(), '\\', '/' );
	if (endSlash && path[path.length() - 1] != '/')
        path += '/';

    return path;
}


string JOPath::getFileBasePath(const string &path)
{
	string temp = JOPath::standardisePath(path, false);
	size_t i = temp.find_last_of('/');

	if (i != string::npos){
		temp = temp.substr(0, i + 1);
	}
	
	return temp;
}

string JOPath::getFileName(const string &path, bool withSuffix/*=true*/)
{
	if (path.empty()){
		return path;
	}
	string temp = JOPath::standardisePath(path, false);
	size_t i = temp.find_last_of('/');
	
	if (i != string::npos){
		temp = temp.substr(i + 1);
	}
	if (withSuffix != true)	{
		string suffix = JOPath::getFileSuffix(temp);
		temp = temp.substr(0, temp.length() - suffix.length() - 1);
	}	
	
	return temp;
}

string JOPath::getFileSuffix(const string &path)
{
	size_t j = path.find_last_of('.');

	if (j != string::npos){
		return path.substr(j + 1);
	}
	return "";
}


string JOPath::getFileWithoutSuffix(const string &path)
{
	size_t j = path.find_last_of('.');

	if (j != string::npos){
		return path.substr(0,j);
	}
	return path;
}

//-----------------------------------------------------------------------
void JOPath::splitFilename(const string& qualifiedName,
    string& outBasename, string& outPath)
{
    string path = qualifiedName;
    // Replace \ with / first
    replace( path.begin(), path.end(), '\\', '/' );
    // split based on final /
    size_t i = path.find_last_of('/');	

    if (i == string::npos){
        outPath.clear();
		outBasename = qualifiedName;
    }
    else{
        outBasename = path.substr(i+1, path.size() - i - 1);
        outPath = path.substr(0, i+1);
    }

}

void JOPath::splitFilename(const string& qualifiedName, string& outBasename, string& outPath, string& ext)
{
	string path = qualifiedName;
	// Replace \ with / first
	replace(path.begin(), path.end(), '\\', '/');
	// split based on final /
	size_t i = path.find_last_of('/');
	size_t j = path.find_last_of('.');

	if (i == string::npos && j != string::npos){
		outPath.clear();
		outBasename = path.substr(0, j);
		ext = path.substr(j + 1, path.size() - j - 1);
	}
	else if (i!=string::npos && j!=string::npos){		
		outPath = path.substr(0, i + 1);
		outBasename = path.substr(i + 1, j -i-1);
		ext = path.substr(j + 1, path.size() - j - 1);
	}
	else{
		outPath = "";
		outBasename = path;
		ext = "";
	}
}




NS_JOFW_END