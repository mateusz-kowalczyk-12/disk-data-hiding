#include "SlackHiding.h"


void mapFile(char** pPViewOfFile, DWORD* pFilesize, int* pLastPageEndOffset, HANDLE* pHFile, HANDLE* pHMap);

void unmapFile(char* pViewOfFile, HANDLE* pHFile, HANDLE* pHMap);


void slack_hiding_menu()
{
	char choice;

	std::cout << "Menu\n1: Hide string\n2: Read hidden string\n";
	std::cin >> choice;

	switch (choice)
	{
	case '1':
		hide_string();
		break;
	case '2':
		read_hidden_string();
		break;
	default:
		break;
	}
}


void hide_string()
{
	char* pViewOfFile;
	DWORD fileSize;
	int lastPageEndOffset;
	HANDLE hFile, hMap;

	mapFile(&pViewOfFile, &fileSize, &lastPageEndOffset, &hFile, &hMap);

	std::string stringToHide;
	std::cout << "string to hide: ";
	std::cin.ignore();
	std::getline(std::cin, stringToHide);

	if (pViewOfFile == NULL)
	{
		std::cout << "Error: " << GetLastError() << std::endl;
		return;
	}

	int i;
	for (i = 0; i < stringToHide.length(); i++)
	{
		if (fileSize + i >= lastPageEndOffset)
			break;
		*(pViewOfFile + fileSize + i) = stringToHide[i];
	}
	for (; fileSize + i < lastPageEndOffset; i++)
	{
		*(pViewOfFile + fileSize + i) = '\0';
	}

	FlushViewOfFile(pViewOfFile, fileSize);
	unmapFile(pViewOfFile, &hFile, &hMap);
}

void read_hidden_string()
{
	char* pViewOfFile;
	DWORD fileSize;
	int lastPageEndOffset;
	HANDLE hFile, hMap;

	mapFile(&pViewOfFile, &fileSize, &lastPageEndOffset, &hFile, &hMap);

	if (pViewOfFile == NULL)
	{
		std::cout << "Error: " << GetLastError() << std::endl;
		return;
	}

	for (int i = fileSize; i < lastPageEndOffset; i++)
	{
		std::cout << *(pViewOfFile + i);
	}

	unmapFile(pViewOfFile, &hFile, &hMap);
}


void mapFile(char** pPViewOfFile, DWORD* pFileSize, int* pLastPageEndOffset, HANDLE* pHFile, HANDLE* pHMap)
{
	std::string filename;
	std::cout << "filename: ";
	std::cin >> filename;
	filename = "datafiles/slack_datafiles/" + filename;
	std::wstring wFilename = std::wstring(filename.begin(), filename.end());

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	*pHFile = CreateFile(wFilename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	*pFileSize = GetFileSize(*pHFile, NULL);
	*pLastPageEndOffset = (*pFileSize / systemInfo.dwPageSize) * systemInfo.dwPageSize +
		(*pFileSize % systemInfo.dwPageSize == 0 ? 0 : systemInfo.dwPageSize);

	*pHMap = CreateFileMapping(*pHFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	*pPViewOfFile = (char*)MapViewOfFile(*pHMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
}

void unmapFile(char* pViewOfFile, HANDLE* pHFile, HANDLE* pHMap)
{
	UnmapViewOfFile(pViewOfFile);
	CloseHandle(*pHMap);
	CloseHandle(*pHFile);
}