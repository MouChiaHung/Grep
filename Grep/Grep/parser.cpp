 /******************************************************************************
 **                      INCLUDE
 *******************************************************************************/
#include "parser.h"
using namespace std;

/******************************************************************************
**                      DEFINE
*******************************************************************************/
#define WHEREARG  __func__
#define LOG(...) debug_print_impl(WHEREARG, ##__VA_ARGS__)

/******************************************************************************
**                      GLOBAL VARIABLE
*******************************************************************************/

/******************************************************************************
**                      DECLARATION
*******************************************************************************/

/******************************************************************************
**                      IMPLEMENTATION
*******************************************************************************/
void debug_print_impl(const char* func, _Printf_format_string_ char const* const _Format, ...) {
#ifdef NO_LOG
    return;
#endif 
    va_list args;
    va_start(args, _Format);
    char db_buf[128] = "";
    //sprintf_s(db_buf, "[%s]", func);
    int db_len = 0;
    db_len = strlen(db_buf);
    vsprintf_s(db_buf + db_len, 128 - db_len, _Format, args);
    printf(db_buf);
    va_end(args);
}

Parser::Parser() {
    mCwd = "";
}

Parser::~Parser() {
    keywords.clear();
}

bool Parser::parse(string val) {
    if (keywords.empty()) {
        return false;
    }
    for (std::vector<string>::iterator it = keywords.begin(); it != keywords.end(); it++) {
        if (val.find(*it) != string::npos) {
            return true;
        }
    }
    return false;
}

wchar_t*& Parser::getWC(const char* c) {
    const size_t lsrc = strlen(c) + 1;
    size_t ldest = 0;
    wchar_t* wc = new wchar_t[lsrc];
    mbstowcs_s(&ldest, wc, lsrc, c, lsrc - 1);
    return wc;
}

string Parser::wc2str(const wchar_t* wc) {
    string str = "";
    while (*wc != 0) {
        str += *wc;
        wc++;
    }
    return str;
}

bool Parser::queryDIR(string dir_name) {
    bool ret = false;
    if (mCwd.empty()) {
        return false;
    }
    if (keywords.empty()) {
        return false;
    }
    string dir = "";
    int fis_len;

    //employee wide char
    dir = dir_name  + "\\*.*";;
    //employee WIN File API
    vector<string> targets;
    WIN32_FIND_DATA  fd;
    WIN32_FIND_DATA  fd_dir;
    HANDLE hFind = ::FindFirstFile(getWC(dir.c_str()), &fd);
    HANDLE hFind_dir = ::FindFirstFile(getWC(dir.c_str()), &fd_dir);
    string str_subdir;
    string str_tmp;

    //recursive call for diving into sub-directories
    do {
        if ((fd_dir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
            //ignore trival file node
            while(true) {
                FindNextFile(hFind_dir, &fd_dir);
                str_tmp = wc2str(fd_dir.cFileName);
                if (str_tmp.compare(".") && str_tmp.compare("..")){
                    break;
                }
            }
            if ((fd_dir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
                str_subdir = wc2str(fd_dir.cFileName);
                ret = queryDIR(dir_name + "\\" + str_subdir);
            }
        }
    } while(::FindNextFile(hFind_dir, &fd_dir));
    
    //iterate same layer files
    do { 
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            str_tmp = wc2str(fd.cFileName);
            string fname = dir_name + "\\" + str_tmp;
            targets.push_back(fname);
        }
    } while(::FindNextFile(hFind, &fd));    

    for (std::vector<string>::iterator it=targets.begin(); it!=targets.end(); it++) {
        std::cout << "Parsing target file:" << *it << "..." << std::endl;
        if (query(*it) == false) {
            LOG("FAILED for %s\n", (*it).c_str());
            return false;
        }
    }
    return true;   
}

bool Parser::query(string file_name) {
    if (mCwd.empty()) {
        return false;
    }
    if (keywords.empty()) {
        return false;
    }
    string dir = "";
    string path_in = "";
    string path_out = "";
    ifstream fis;
    ofstream fos;
    int fis_len;

    //get absolute path
    dir = mCwd;

    //time stamp
    char tstamp[128];
    sprintf_s(tstamp, "%d", (int)time(NULL));
    string stamp(tstamp);

    //input file init
    path_in = file_name;
    fis.open(path_in.c_str(), ios::in | ios::binary);
    if (fis.fail()) {
        return false;
    }

    //output file init
    //path_out = dir + "\\" + "result_" + stamp + ".txt";
    path_out = dir + "\\" + "out" + ".txt";

    fos.open(path_out, fstream::in | fstream::out | fstream::app | fstream::binary);
    if (fos.fail()) {
        return false;
    }
    else fos.clear();

    //length of fis
    fis.seekg(0, ios::end);
    fis_len = fis.tellg();
    LOG("[%s] has %d bytes\n", file_name.c_str(), fis_len);

    //read from src
    fis.seekg(0, ios::beg);
    string line;
    //extract
    int i = 0;
    std::vector<string> v;
    while (++i < RETRY_LIMIT) {
        if (getline(fis, line)) {
            if (line.back() == '\r') {
                line.pop_back();
            }
            if (parse(line)) {
                v.push_back(line);
            }
        }
        else {
            break;
        }
    }
    fis.close();

    //write to dest
    string str;
    int l;
    time_t t;
    struct tm tm;
    time(&t);
    char buf[128];
    localtime_s(&tm, &t);
    asctime_s(buf, &tm);
    char buf2[128];
    sprintf_s(buf2, "%s", buf);
    string str_time(buf2);
    if (str_time.back() == '\n') {
        str_time.pop_back();
    }
    string title = "\r\n[From " + path_in + " at " + str_time + " ]\n";
    fos.write(title.c_str(), title.length());
    title = "[--------------------------------------------------------------------------------------------------------------------------------]\n";
    fos.write(title.c_str(), title.length());
    for (std::vector<string>::iterator it = v.begin(); it != v.end(); it++) {
        fos.write(it->c_str(), it->length());
        fos.put('\n');
    }
    title = "[--------------------------------------------------------------------------------------------------------------------------------]\n";
    fos.write(title.c_str(), title.length());
    fos.close();

    LOG("Time:%s\n", str_time.c_str());
    return true;
}

bool Parser::load_keywords(string file_name) {
    if (file_name.compare("keywords.txt")) {
        LOG("Keywords only live in keywords.txt !\n");
        return false;
    }
    string dir = "";
    string path_in = "";
    string path_out = "";
    char cwd[1024];
    ifstream fis;
    ofstream fos;
    int fis_len;

    //get absolute path
    if (!_getcwd(cwd, 1024)) {
        return false;
    }
    dir = cwd;
    mCwd = cwd;

    //time stamp
    char tstamp[128];
    sprintf_s(tstamp, "%d", (int)time(NULL));
    string stamp(tstamp);

    //input file init
    path_in = dir + "\\" + file_name;
    fis.open(path_in.c_str(), ios::in | ios::binary);
    if (fis.fail()) {
        return false;
    }

#if 0
    //output file init
    path_out = dir + "\\" + path_in.substr(path_in.find_last_of("/\\") + 1) + "_bkp_" + stamp + ".txt";
    fos.open(path_out, fstream::in | fstream::out | fstream::trunc | fstream::binary);
    if (fos.fail()) {
        return false;
    }
    else fos.clear();
#endif

    //remove the previous result.txt
    string prev_file = mCwd + "\\" + "out" + ".txt";
    remove(prev_file.c_str());

    //length of fis
    fis.seekg(0, ios::end);
    fis_len = fis.tellg();
    LOG("[%s] has %d bytes\n", path_in.c_str(), fis_len);

    //read from src
    fis.seekg(0, ios::beg);
    string line;
    //extract
    int i = 0;
    while (++i < RETRY_LIMIT) {
        if (getline(fis, line)) {
            if (line.back() == '\r') {
                line.pop_back();
            }
            keywords.push_back(line);
        }
        else {
            break;
        }
    }
    fis.close();

#if 0
    //write to dest
    string str;
    int l;
    for (std::vector<string>::iterator it = keywords.begin(); it != keywords.end(); it++) {
        fos.write(it->c_str(), it->length());
        fos.put('\n');
    }
#endif

    fos.close();

    return true;
}
