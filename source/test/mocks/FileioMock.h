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

#ifndef MOCK_FILE_IO_H
#define MOCK_FILE_IO_H
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <curl/curl.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class FileIOInterface
{
    public:
        virtual ~FileIOInterface() {}
        virtual FILE * popen(const char *, const char *) = 0;
        virtual int stat(const char *, struct stat *) = 0;
        virtual int pclose(FILE *) = 0;
        virtual int pipe(int *) = 0;
        virtual int fclose(FILE *) = 0;
	virtual int fscanf(FILE *, const char *) = 0;
        virtual ssize_t getline(char **, size_t *, FILE *) = 0;
        virtual struct dirent * readdir(DIR *) = 0;
        virtual FILE * fopen(const char *, const char *) = 0;
        virtual int closedir(DIR *) = 0;
        virtual int mkdir(const char *, mode_t) = 0;
        virtual DIR* opendir(const char*) = 0;
        virtual int close(int) = 0;
        virtual int open(const char *, mode_t) = 0;
        virtual int fstat(int , struct stat *) = 0;
        virtual ssize_t read(int , void *, size_t ) = 0;
        virtual struct curl_slist *curl_slist_append(struct curl_slist *, const char *) = 0;
        virtual int fseek(FILE *, long, int ) = 0;
        virtual size_t fwrite(const void*, size_t, size_t, FILE* ) = 0;
        virtual size_t fread(void *, size_t, size_t, FILE* ) = 0;
        virtual CURLcode curl_easy_perform(CURL *) = 0;
        virtual CURL* curl_easy_init() = 0;
};

class FileIOMock: public FileIOInterface
{
    public:
        virtual ~FileIOMock() {}
        MOCK_METHOD2(popen, FILE *(const char *, const char *));
        MOCK_METHOD1(pclose, int (FILE *));
        MOCK_METHOD1(pipe, int(int *));
        MOCK_METHOD1(fclose, int (FILE *));
        MOCK_METHOD2(fscanf, int(FILE *, const char *));
        MOCK_METHOD3(getline, ssize_t (char **, size_t *, FILE *));
        MOCK_METHOD1(opendir, DIR*(const char*));
        MOCK_METHOD1(readdir, struct dirent *(DIR *));
        MOCK_METHOD2(fopen, FILE *(const char *, const char *));
        MOCK_METHOD1(closedir, int (DIR *));
        MOCK_METHOD2(mkdir, int (const char *, mode_t));
        MOCK_METHOD3(read, ssize_t(int, void *, size_t));
        MOCK_METHOD2(fstat, int (int , struct stat *));
        MOCK_METHOD1(close, int(int));
	MOCK_METHOD2(stat, int(const char *, struct stat *));
        MOCK_METHOD2(open, int(const char *, mode_t));
        MOCK_METHOD4(fwrite, size_t(const void *, size_t, size_t, FILE *));
        MOCK_METHOD2(curl_slist_append, struct curl_slist *(struct curl_slist *, const char *));
        MOCK_METHOD4(fread, size_t(void* , size_t, size_t, FILE*));
        MOCK_METHOD3(fseek, int(FILE *, long, int));
        MOCK_METHOD1(curl_easy_perform, CURLcode(CURL *));
        MOCK_METHOD0(curl_easy_init, CURL*());
};

#endif

