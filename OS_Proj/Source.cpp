#include "AdsHiding.h"
#include "UnallocHiding.h"
#include "SlackHiding.h"


int main()
{
	char choice;

	std::cout << "Menu:\n"
		"1: Slack hiding\n"
		"2: Ads hiding\n"
		"3: Unallocated hiding\n";
	std::cin >> choice;

	switch (choice)
	{
	case '1':
		slack_hiding_menu();
		break;
	case '2':
		ads_hiding_menu();
		break;
	case '3':
		unalloc_hiding_menu();
		break;
	}

	return 0;
}