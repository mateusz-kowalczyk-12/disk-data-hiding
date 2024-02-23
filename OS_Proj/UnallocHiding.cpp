#include "UnallocHiding.h"

#define SECTORS_BY_SECONDARY_GPT 34


void unalloc_hiding_menu()
{
	LPCSTR lpFileName = "\\\\.\\PhysicalDrive0";
	HANDLE hPhyDisk = NULL;

	DiskMetadata diskMetadata;
	diskMetadata.BytesPerSector = 0;
	diskMetadata.LLDiskSize = 0;
	diskMetadata.unallocOffset = 0;

	get_disk_metadata(lpFileName, &hPhyDisk, &diskMetadata);
	std::cout << "Unallocated space offset: " <<
		diskMetadata.unallocOffset << " B (LBA " << diskMetadata.unallocOffset / diskMetadata.BytesPerSector << ")" << std::endl;

	std::cout << "\nMenu:\n1: Hide string\n2: Read string\n3: Hide file content\n4: Read file content\n";
	char choice;
	std::cin >> choice;

	switch (choice)
	{
	case '1':
		hide_string(hPhyDisk, &diskMetadata);
		break;
	case '2':
		read_string(hPhyDisk, &diskMetadata);
		break;
	case '3':
		hide_file_content(hPhyDisk, &diskMetadata);
		break;
	case '4':
		read_file_content(hPhyDisk, &diskMetadata);
		break;
	}

	CloseHandle(hPhyDisk);
}


void hide_string(HANDLE hPhyDisk, DiskMetadata* pDiskMetadata)
{
	std::string stringToHide;
	std::cout << "string to hide: ";
	std::cin.ignore();
	std::getline(std::cin, stringToHide);

	hide_bytes(stringToHide.c_str(), stringToHide.length(), hPhyDisk, pDiskMetadata);
}

void read_string(HANDLE hPhyDisk, DiskMetadata* pDiskMetadata)
{
	DWORD bytesToReadN = 0;
	std::cout << "bytes to read number: ";
	std::cin >> bytesToReadN;

	LPBYTE readBytes = read_hidden_bytes(bytesToReadN, hPhyDisk, pDiskMetadata);

	if (readBytes == NULL)
		return;

	for (int i = 0; i < bytesToReadN; i++)
	{
		std::cout << readBytes[i];
	}
	std::cout << std::endl;

	free(readBytes);
}

void hide_file_content(HANDLE hPhyDisk, DiskMetadata* pDiskMetadata)
{
	std::string filename;
	std::cout << "file to hide: ";
	std::cin >> filename;

	std::ifstream srcFile;
	srcFile.open("datafiles/unalloc_datafiles/" + filename, std::ios::binary);
	
	srcFile.seekg(0, std::ios_base::end);
	std::streampos srcFileSize = srcFile.tellg();
	srcFile.seekg(0, std::ios_base::beg);

	char* srcFileContent = (char*)malloc(srcFileSize);
	srcFile.read(srcFileContent, srcFileSize);

	hide_bytes(srcFileContent, srcFileSize, hPhyDisk, pDiskMetadata);

	free(srcFileContent);
}

void read_file_content(HANDLE hPhyDisk, DiskMetadata* pDiskMetadata)
{
	std::string filename;
	std::cout << "destination file: ";
	std::cin >> filename;

	DWORD bytesToReadN = 0;
	std::cout << "bytes to read number: ";
	std::cin >> bytesToReadN;

	std::ofstream dstFile;
	dstFile.open("datafiles/unalloc_datafiles/" + filename, std::ios::binary);

	LPBYTE readBytes = read_hidden_bytes(bytesToReadN, hPhyDisk, pDiskMetadata);
	dstFile.write((const char*)readBytes, bytesToReadN);

	free(readBytes);
}


void get_disk_metadata(LPCSTR lpFileName, LPHANDLE phPhyDisk, DiskMetadata* pDiskMetadata)
{
	DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	LPSECURITY_ATTRIBUTES lpSecurityAttributes = 0;
	DWORD dwCreationDisposition = OPEN_EXISTING;
	DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	HANDLE hTemplateFile = NULL;

	*phPhyDisk = CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
		dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	if (*phPhyDisk == INVALID_HANDLE_VALUE)
	{
		std::cout << "Error: " << GetLastError();
		exit(1);
	}

	const size_t tmpBuffSize = 65536;
	LPBYTE tmpBuffer = (LPBYTE)malloc(tmpBuffSize);
	PDISK_GEOMETRY_EX pDiskGeometryEx;

	DWORD dwIoControlCode = IOCTL_DISK_GET_DRIVE_GEOMETRY_EX;
	LPVOID lpInBuffer = NULL;
	DWORD nInBufferSize = 0;
	LPVOID lpOutBuffer = tmpBuffer;
	DWORD nOutBufferSize = tmpBuffSize;
	DWORD BytesReturned = 0;
	LPOVERLAPPED lpOverlapped = NULL;

	if (!DeviceIoControl(*phPhyDisk, dwIoControlCode, lpInBuffer, nInBufferSize,
		lpOutBuffer, nOutBufferSize, &BytesReturned, lpOverlapped))
	{
		std::cout << "Error: " << GetLastError();
		exit(1);
	}

	pDiskGeometryEx = (PDISK_GEOMETRY_EX)malloc(BytesReturned);
	memcpy(pDiskGeometryEx, tmpBuffer, BytesReturned);

	pDiskMetadata->BytesPerSector = pDiskGeometryEx->Geometry.BytesPerSector;
	pDiskMetadata->LLDiskSize = pDiskGeometryEx->DiskSize.QuadPart;
	std::cout << "Bytes per sector: " << pDiskMetadata->BytesPerSector << std::endl;
	std::cout << "Disk size: " << pDiskMetadata->LLDiskSize / (1024 * 1024) << " MB" << std::endl << std::endl;

	PDRIVE_LAYOUT_INFORMATION_EX pDriveLayoutInformationEx;

	dwIoControlCode = IOCTL_DISK_GET_DRIVE_LAYOUT_EX;
	lpOutBuffer = tmpBuffer;
	nOutBufferSize = tmpBuffSize;

	if (!DeviceIoControl(*phPhyDisk, dwIoControlCode, lpInBuffer, nInBufferSize,
		tmpBuffer, nOutBufferSize, &BytesReturned, lpOverlapped))
	{
		std::cout << "Error: " << GetLastError();
		exit(1);
	}

	pDriveLayoutInformationEx = (PDRIVE_LAYOUT_INFORMATION_EX)malloc(BytesReturned);
	memcpy(pDriveLayoutInformationEx, tmpBuffer, BytesReturned);

	if (pDriveLayoutInformationEx->PartitionStyle == PARTITION_STYLE_GPT)
		pDiskMetadata->LLDiskSize -= SECTORS_BY_SECONDARY_GPT * pDiskMetadata->BytesPerSector;
	int partition_information_count = 1 +
		(BytesReturned - sizeof(DRIVE_LAYOUT_INFORMATION_EX)) / sizeof(PARTITION_INFORMATION_EX);

	for (int i = 0; i < partition_information_count; i++)
	{
		DWORD PartitionNumber = pDriveLayoutInformationEx->PartitionEntry[i].PartitionNumber;
		if (PartitionNumber == 0)
			break;

		LONGLONG LLOffset = pDriveLayoutInformationEx->PartitionEntry[i].StartingOffset.QuadPart;
		LONGLONG LLLength = pDriveLayoutInformationEx->PartitionEntry[i].PartitionLength.QuadPart;

		LONGLONG LLEndOffset = LLOffset + LLLength;
		if (LLEndOffset > pDiskMetadata->unallocOffset)
			pDiskMetadata->unallocOffset = LLEndOffset;

		std::cout << "Partition: " << PartitionNumber << std::endl;
		std::cout << "- Offset: " << LLOffset << " B" << std::endl;
		std::cout << "- Length: " << LLLength / (1024 * 1024) << " MB" << std::endl;
	}

	free(tmpBuffer);
	free(pDiskGeometryEx);
	free(pDriveLayoutInformationEx);
}

void hide_bytes(const char bytesToWrite[], DWORD bytesToWriteN, HANDLE hPhyDisk, DiskMetadata* pDiskMetadata)
{
	int sectorsN = bytesToWriteN / pDiskMetadata->BytesPerSector +
		(bytesToWriteN % pDiskMetadata->BytesPerSector == 0 ? 0 : 1);

	if (pDiskMetadata->unallocOffset + sectorsN * pDiskMetadata->BytesPerSector > pDiskMetadata->LLDiskSize)
	{
		std::cout << "Error: not enough disk space!" << std::endl;
		return;
	}
	LPBYTE sectors = (LPBYTE)malloc(sectorsN * pDiskMetadata->BytesPerSector);
	memset(sectors, 0, sectorsN * pDiskMetadata->BytesPerSector);
	memcpy(sectors, bytesToWrite, bytesToWriteN);

	LARGE_INTEGER offset;
	offset.QuadPart = pDiskMetadata->unallocOffset;

	if (!SetFilePointerEx(hPhyDisk, offset, NULL, FILE_BEGIN))
	{
		std::cout << "Error: " << GetLastError() << std::endl;
		return;
	}
	DWORD bytesWrittenN = 0;
	if (!WriteFile(hPhyDisk, sectors, sectorsN * pDiskMetadata->BytesPerSector, &bytesWrittenN, NULL))
	{
		std::cout << "Error: " << GetLastError() << std::endl;
		return;
	}

	free(sectors);
}

LPBYTE read_hidden_bytes(DWORD bytesToReadN, HANDLE hPhyDisk, DiskMetadata* pDiskMetadata)
{
	int sectorsN = bytesToReadN / pDiskMetadata->BytesPerSector +
		(bytesToReadN % pDiskMetadata->BytesPerSector == 0 ? 0 : 1);

	if (pDiskMetadata->unallocOffset + sectorsN * pDiskMetadata->BytesPerSector > pDiskMetadata->LLDiskSize)
	{
		std::cout << "Error: not enough disk space!" << std::endl;
		return NULL;
	}
	LPBYTE sectors = (LPBYTE)malloc(sectorsN * pDiskMetadata->BytesPerSector);
	memset(sectors, 0, sectorsN * pDiskMetadata->BytesPerSector);

	LARGE_INTEGER offset;
	offset.QuadPart = pDiskMetadata->unallocOffset;

	if (!SetFilePointerEx(hPhyDisk, offset, NULL, FILE_BEGIN))
	{
		std::cout << "Error: " << GetLastError() << std::endl;
		return NULL;
	}
	DWORD bytesReadN = 0;
	if (!ReadFile(hPhyDisk, sectors, sectorsN * pDiskMetadata->BytesPerSector, &bytesReadN, NULL))
	{
		std::cout << "Error: " << GetLastError() << std::endl;
		return NULL;
	}

	LPBYTE readBytes = (LPBYTE)malloc(bytesToReadN);
	memcpy(readBytes, sectors, bytesToReadN);

	free(sectors);
	return readBytes;
}