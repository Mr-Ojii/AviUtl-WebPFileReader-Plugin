#include "FileHandle.hpp"

FileHandle::FileHandle(const char* path)
    : hFile(INVALID_HANDLE_VALUE) {

    this->hFile = CreateFile(path,
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

}

FileHandle::~FileHandle() {
    if (this->hFile && this->hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(this->hFile);
        this->hFile = INVALID_HANDLE_VALUE;
    }
}
