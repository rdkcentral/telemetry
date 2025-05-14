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

#include "test/mocks/FileioMock.h"
extern FileIOMock *g_fileIOMock;


// Mock Method

extern "C" FILE* popen(const char * command, const char * type)
{
    if (!g_fileIOMock)
    {
        return NULL;
    }
    return g_fileIOMock->popen(command, type);
}

extern "C" int pclose(FILE * stream)
{
    if (!g_fileIOMock)
    {
        return 0;
    }
    return g_fileIOMock->pclose(stream);
}

extern "C" int pipe(int str[2])
{
    if (!g_fileIOMock)
    {
        return 0;
    }
    return g_fileIOMock->pipe(str);
}


extern "C" int fclose(FILE * stream)
{
    if (!g_fileIOMock)
    {
        return 0;
    }
    return g_fileIOMock->fclose(stream);
}

extern "C" size_t fread (void* ptr, size_t size, size_t nmemb,FILE* stream)
{
    if (!g_fileIOMock)
    {
            return 0;
    }
    return g_fileIOMock->fread(ptr, size, nmemb, stream);
}

extern "C" int fseek(FILE *stream, long offset, int whence)
{
    if (!g_fileIOMock)
    {
            return 0;
    }
    return  g_fileIOMock->fseek(stream, offset, whence);
}

extern "C" ssize_t getline(char **str, size_t *n, FILE * stream)
{
    if(!g_fileIOMock)
    {
          return 0;
    }
    return g_fileIOMock->getline(str, n , stream);
}

extern "C" struct dirent * readdir(DIR *rd)
{
     if(!g_fileIOMock)
     {
            return 0;
     }
     return g_fileIOMock->readdir(rd);
}

extern "C" ssize_t read(int fd, void *buf, size_t count)
{
     if(!g_fileIOMock)
     {
             return 0;
     }
     return g_fileIOMock->read(fd, buf, count);
}
 
extern "C" FILE* fopen(const char *fp, const char * str)
{
    if(!g_fileIOMock)
    {
        return (FILE*)NULL;
    }
    return g_fileIOMock->fopen(fp, str);
}

extern "C" int closedir(DIR * cd)
{
    if(!g_fileIOMock)
    {
        return 0;
    }
    return g_fileIOMock->closedir(cd);
}

extern "C" int mkdir (const char * str, mode_t mode)
{
    if(!g_fileIOMock)
    {
        return 0;
    }
    return  g_fileIOMock->mkdir(str, mode);
}


extern "C" int close( int cd)
{
    if(!g_fileIOMock)
    {
        return 0;
    }
    return g_fileIOMock->close(cd);
}

extern "C" DIR *opendir( const char *name)
{
    if(!g_fileIOMock)
    {
        return (DIR *)NULL;
    }
    return  g_fileIOMock->opendir(name);
}

extern "C" int open(const char *pathname, int flags)
{
    if(!g_fileIOMock)
    {
        return 0;
    }
    return g_fileIOMock->open(pathname, flags);
}

extern "C" int fstat(int fd, struct stat *buf)
{
    if(!g_fileIOMock)
    {
            return 0;
    }
    return g_fileIOMock->fstat(fd, buf);
}


extern "C" size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE * stream)
{
    if(!g_fileIOMock)
    {
            return 0;
    }
    return g_fileIOMock->fwrite(ptr, size, nitems, stream);
}

extern "C" CURLcode curl_easy_perform(CURL *easy_handle)
{ 
     if(!g_fileIOMock)
     {

             return (CURLcode)0;
     }
      return g_fileIOMock->curl_easy_perform(easy_handle);
}

extern "C" struct curl_slist *curl_slist_append(struct curl_slist *list, const char * string)
{ 
     if(!g_fileIOMock)
     {
             return (struct curl_slist *)NULL;
     }
     return g_fileIOMock->curl_slist_append(list, string);
}

extern "C" CURL* curl_easy_init()
{
     if(!g_fileIOMock)
     {
             return NULL;
     }
     return g_fileIOMock->curl_easy_init();
}

