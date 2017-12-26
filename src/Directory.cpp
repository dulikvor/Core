#include "Directory.h"

using namespace core;
using namespace std;

string Directory::GetDir(const string& path)
{
    size_t position = path.find_last_of('/');
    return string(path.data(), position != string::npos ? position : path.length());
};

string Directory::GetProcessName(const std::string &path) {
    size_t position = path.find_last_of('/');
    return string(path.begin() + position, path.end());
}
