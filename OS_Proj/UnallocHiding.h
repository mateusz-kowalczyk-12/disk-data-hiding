#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <windows.h>


struct DiskMetadata
{
	DWORD BytesPerSector;
	LONGLONG LLDiskSize;
	LONGLONG unallocOffset;
};


void unalloc_hiding_menu();


void hide_string(HANDLE hPhyDisk, DiskMetadata* pDiskMetadata);

void read_string(HANDLE hPhyDisk, DiskMetadata* pDiskMetadata);

void hide_file_content(HANDLE hPhyDisk, DiskMetadata* pDiskMetadata);

void read_file_content(HANDLE hPhyDisk, DiskMetadata* pDiskMetadata);


void get_disk_metadata(LPCSTR lpFileName, LPHANDLE phPhyDisk, DiskMetadata* pDiskMetadata);

void hide_bytes(const char bytesToWrite[], DWORD bytesToWriteN, HANDLE hPhyDisk, DiskMetadata* pDiskMetadata);

LPBYTE read_hidden_bytes(DWORD bytesToReadN, HANDLE hPhyDisk, DiskMetadata* pDiskMetadata);