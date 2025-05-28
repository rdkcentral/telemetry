/*
* Copyright 2020 Comcast Cable Communications Management, LLC
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* SPDX-License-Identifier: Apache-2.0
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <stdarg.h>
#ifndef FOPEN_MOCK_H
#define FOPEN_MOCK_H

#include <cstdio>
#include <gmock/gmock.h>

class FileMock
{
public:
    MOCK_METHOD(FILE*, fopen, (const char* filename, const char* mode), ());
    MOCK_METHOD(int, fclose, (FILE* stream), ());
    // MOCK_METHOD(int, fprintf, (FILE* stream, const char* format, const char* message), ());
    MOCK_METHOD(int, fprintf, (FILE* stream, const char* format, va_list), ());
    MOCK_METHOD(char*, fgets, (char* buffer, int size, FILE* stream), ());
    MOCK_METHOD(int, fscanf, (FILE* stream, const char* format, va_list args), ());
    MOCK_METHOD(int, fseek, (FILE* stream, long offset, int whence), ());
    MOCK_METHOD(long, ftell, (FILE* stream), ());
    MOCK_METHOD(size_t, fread, (void* ptr, size_t size, size_t count, FILE* stream), ());
    MOCK_METHOD(size_t, fwrite, (const void* ptr, size_t size, size_t count, FILE* stream), ());
    MOCK_METHOD(FILE*, popen, (const char* command, const char* type), ());
    MOCK_METHOD(int, pclose, (FILE* stream), ());
    MOCK_METHOD(int, pipe, (int* fd), ());
    MOCK_METHOD(int, open, (const char* pathname, int flags), ());
    MOCK_METHOD(int, close, (int fd), ());
    MOCK_METHOD(DIR*, opendir, (const char* name), ());
    MOCK_METHOD(struct dirent*, readdir, (DIR* dirp), ());
    MOCK_METHOD(int, closedir, (DIR* dirp), ());
    MOCK_METHOD(int, mkdir, (const char* pathname, mode_t mode), ());
    MOCK_METHOD(int, fstat, (int fd, struct stat* statbuf), ());
    MOCK_METHOD(ssize_t, read, (int fd, void* buf, size_t count), ());
    MOCK_METHOD(struct curl_slist*, curl_slist_append, (struct curl_slist* list, const char* str), ());
    MOCK_METHOD(CURLcode, curl_easy_perform, (CURL* handle), ());
    MOCK_METHOD(CURL*, curl_easy_init, (), ());
    MOCK_METHOD(ssize_t, getline, (char** lineptr, size_t* n, FILE* stream), ());
    MOCK_METHOD(pid_t, fork, (), ());
    MOCK_METHOD(ssize_t, write, (int fd, const void* buf, size_t count), ());
   //MOCK_METHOD(CURLcode, curl_easy_setopt, (CURL *handle, CURLoption option, parameter), ());
    MOCK_METHOD(void, curl_easy_cleanup, (CURL *handle), ());
    MOCK_METHOD(void, curl_slist_free_all, (struct curl_slist *list), ());
    // MOCK_METHOD(int, stat, (const char *pathname, struct stat *statbuf), ());
};

extern FileMock* g_fileIOMock;

#endif // FOPEN_MOCK_H
