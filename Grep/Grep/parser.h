/**
 * mou @ 2021
 */
#ifndef __PARSE__
#define __PARSE__
#include <fstream>
#include <string>
#include <memory>
#include <cstdarg>
#include <iostream>
#include <filesystem>
#include "stdlib.h"
#include <direct.h>
#include <time.h>
#include <vector>
#include <windows.h>

#ifndef DEBUG
#define DEBUG
#endif

#define KEYWORDS_LEN_LIMIT 1024
#define RETRY_LIMIT 1024*1024
 //#define PRINT_DFT_IDFT
using namespace std;

class Parser {
private:
	wchar_t*& getWC(const char* c);
	string wc2str(const wchar_t* wc);
protected:

	std::vector<string> keywords;
	bool parse(string val);
public:
	Parser();
	~Parser();
	string mCwd;
	bool query(string file_name);
	bool queryDIR(string dir_name);
	bool load_keywords(string file_name);
};

#endif