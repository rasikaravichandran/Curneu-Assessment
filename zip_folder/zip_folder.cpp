#include "stdafx.h"
#include <string>
#include <iostream>
#include <vector>
#include <Shlwapi.h> 
#include "zip.h"
#include "unzip.h"
#include "zlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
using namespace std; 
#pragma comment(lib, "Shlwapi.lib")
bool nyAddfiletoZip(zipFile zfile, const std::string& fileNameinZip, const std::string& srcfile);
bool nyCollectfileInDirtoZip(zipFile zfile, const std::string& filepath, const std::string& parentdirName);
bool nyCreateZipfromDir(const std::string& dirpathName, const std::string& zipfileName, const std::string& parentdirName);
int _tmain(int argc, _TCHAR* argv[])
{
	std::string dirpath = "C:\\curneu_assignment\\wei";			// The source file (or) folder 
	std::string zipfileName = "C:\\curneu_assignment\\lango.rar";	// Destination compression package 
	nyCreateZipfromDir(dirpath, zipfileName, "wei");
	system("pause");
	return 0;
}
bool nyAddfiletoZip(zipFile zfile, const std::string& fileNameinZip, const std::string& srcfile)
{
	if (NULL == zfile || fileNameinZip.empty()
	{
		return 0;
	}
	int nErr = 0;
	zip_fileinfo zinfo = {0};
	tm_zip tmz = { 0 };
	zinfo.tmz_date = tmz;
	zinfo.dosDate = 0;
	zinfo.internal_fa = 0;
	zinfo.external_fa = 0;
	char sznewfileName[MAX_PATH] = { 0 };
	memset(sznewfileName, 0x00, sizeof(sznewfileName));
	strcat_s(sznewfileName, fileNameinZip.c_str());
	if (srcfile.empty())
	{
		strcat_s(sznewfileName, "\\");
	}
	nErr = zipOpenNewFileInZip(zfile, sznewfileName, &zinfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
	if (nErr != ZIP_OK)
	{
		return false;
	}
	if (!srcfile.empty())
	{
		FILE* srcfp = _fsopen(srcfile.c_str(), "rb", _SH_DENYNO);
		if (NULL == srcfp)
		{
			std::cout << "Open source file failed." << std::endl;
			return false;
		}
		int numBytes = 0;
		char* pBuf = new char[1024 * 100];
		if (NULL == pBuf)
		{
			std::cout << "new buffer failed." << std::endl;
			return 0;
		}
		while (!feof(srcfp))
		{
			memset(pBuf, 0x00, sizeof(pBuf));
			numBytes = fread(pBuf, 1, sizeof(pBuf), srcfp);
			nErr = zipWriteInFileInZip(zfile, pBuf, numBytes);
			if (ferror(srcfp))
			{
				break;
			}
		}
		delete[] pBuf;
		fclose(srcfp);
	}
	zipCloseFileInZip(zfile);
	return true;
}
bool nyCollectfileInDirtoZip(zipFile zfile, const std::string& filepath, const std::string& parentdirName)
{
	if (NULL == zfile || filepath.empty())
	{
		return false;
	}
	bool bFile = false;
	std::string relativepath = "";
	WIN32_FIND_DATAA findFileData;
	char szpath[MAX_PATH] = { 0 };
	if (::PathIsDirectoryA(filepath.c_str()))
	{
		strcpy_s(szpath, sizeof(szpath) / sizeof(szpath[0]), filepath.c_str());
		int len = strlen(szpath) + strlen("\\*.*") + 1;
		strcat_s(szpath, len, "\\*.*");
	}
	else
	{
		bFile = true;
		strcpy_s(szpath, sizeof(szpath) / sizeof(szpath[0]), filepath.c_str());
	}
	HANDLE hFile = ::FindFirstFileA(szpath, &findFileData);
	if (NULL == hFile)
	{
		return false;
	}
	do 
	{
		if (parentdirName.empty())
			relativepath = findFileData.cFileName;
		else
			relativepath = parentdirName + "\\" + findFileData.cFileName; 
		if (findFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0)
			{
				nyAddfiletoZip(zfile, relativepath, "");
				char szTemp[MAX_PATH] = { 0 };
				strcpy_s(szTemp, filepath.c_str());
				strcat_s(szTemp, "\\");
				strcat_s(szTemp, findFileData.cFileName);
				nyCollectfileInDirtoZip(zfile, szTemp, relativepath);
			}
			continue;
		}
		char szTemp[MAX_PATH] = { 0 };
		if (bFile)
		{
			strcpy_s(szTemp, filepath.c_str());
		}
		else
		{ 
			strcpy_s(szTemp, filepath.c_str());
			strcat_s(szTemp, "\\");
			strcat_s(szTemp, findFileData.cFileName);
		}
		nyAddfiletoZip(zfile, relativepath, szTemp);
	} while (::FindNextFileA(hFile, &findFileData));
	FindClose(hFile);
	return true;
}
bool nyCreateZipfromDir(const std::string& dirpathName, const std::string& zipfileName, const std::string& parentdirName)
{
	bool bRet = false;
	zipFile zFile = NULL;
	if (!::PathFileExistsA(zipfileName.c_str()))
	{
		zFile = zipOpen(zipfileName.c_str(), APPEND_STATUS_CREATE);
	}
	else
	{
		zFile = zipOpen(zipfileName.c_str(), APPEND_STATUS_ADDINZIP);
	}
	if (NULL == zFile)
	{
		std::cout << "create zip file failed." << std::endl;
		return bRet;
	}
	if (nyCollectfileInDirtoZip(zFile, dirpathName, parentdirName))
	{
		bRet = true;
	}
	zipClose(zFile, NULL);
	return bRet;
}
