#include "AdsHiding.h"


void ads_hiding_menu()
{
    std::cout << "Menu:\n1: Create custom ADS\n2: Change Zone.Identifier ADS\n";
    char choice;
    std::cin >> choice;

    switch (choice)
    {
    case '1':
        custom_ads();
        break;
    case '2':
        change_Zone_Identifier();
        break;
    }
}


void custom_ads()
{
    std::cout << "Menu:\n1: Fill ADS by hand\n2: Copy existing file to ADS\n";
    char choice;
    std::cin >> choice;

    switch (choice)
    {
    case '1':
        fill_ads_by_hand();
        break;
    case '2':
        copy_existing_file_to_ads();
        break;
    }
}

void change_Zone_Identifier()
{
    std::string filename;
    std::cout << "filename: ";
    std::cin >> filename;

    char zoneId;
    std::cout << "ZoneId: ";
    std::cin >> zoneId;

    std::fstream file;
    file.open("datafiles/ads_datafiles/" + filename + ":Zone.Identifier", std::ios::out);
    file << "[ZoneTransfer]\nZoneId=" << zoneId;
    file.close();
}


void fill_ads_by_hand()
{
    std::string filename;
    std::cout << "filename:";
    std::cin >> filename;

    std::string adsName;
    std::cout << "ADS name: ";
    std::cin >> adsName;

    std::string content;
    std::cout << "content: ";
    std::cin >> content;

    std::ofstream file;
    file.open("datafiles/ads_datafiles/" + filename + ":" + adsName, std::ios::out);
    file << content;
    file.close();
}

void copy_existing_file_to_ads()
{
    std::string srcFilename, dstFilename, adsName;
    std::cout << "source filename: ";
    std::cin >> srcFilename;
    std::cout << "destination filename: ";
    std::cin >> dstFilename;
    std::cout << "ADS name: ";
    std::cin >> adsName;

	std::ifstream srcFile(srcFilename, std::ios::binary);
	std::ofstream dstFile ("datafiles/ads_datafiles/" + dstFilename + ":" + adsName, std::ios::binary);

	std::copy(
		std::istreambuf_iterator<char>(srcFile),
		std::istreambuf_iterator<char>(),
		std::ostream_iterator<char>(dstFile)
	);

	srcFile.close();
	dstFile.close();

    /*LPCTSTR program_path = L"datafiles/ads_datafiles/file0.txt:calc.exe";

    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo;

    if (CreateProcess(program_path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
    {
        WaitForSingleObject(processInfo.hProcess, INFINITE);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    }
    else
    {
        MessageBox(NULL, L"B³¹d uruchamiania aplikacji!", L"B³¹d", MB_OK | MB_ICONERROR);
    }*/
}