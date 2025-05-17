#pragma once
#include <windows.h>

class FileHandle {
private:
    HANDLE hFile;
public:
    FileHandle(const char* path);
    ~FileHandle();
    HANDLE getHandle() { return hFile; };
};
