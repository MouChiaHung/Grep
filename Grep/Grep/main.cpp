#include <iostream>
#include "parser.h"
int main()
{
	Parser p;
	p.load_keywords("keywords.txt");
	p.queryDIR("files");

	std::cout << "------------ grep done ------------\n";
	system("PAUSE");
	return 0;
}
