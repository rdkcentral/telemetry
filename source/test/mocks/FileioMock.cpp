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

#include <dlfcn.h>

typedef FILE* (*popen_ptr) (const char * command, const char * type);
typedef int (*pclose_ptr) (FILE * stream);
typedef int (*pipe_ptr) (int str[2]);
typedef int (*fclose_ptr)(FILE * stream);
typedef size_t (*fread_ptr) (void* ptr, size_t size, size_t nmemb,FILE* stream);
typedef int (*fseek_ptr) (FILE *stream, long offset, int whence);
typedef char* (*fgets_ptr) (char *str, int n, FILE *stream);
typedef ssize_t (*getline_ptr) (char **str, size_t *n, FILE * stream);
typedef struct dirent * (*readdir_ptr) (DIR *rd);
typedef ssize_t (*read_ptr)(int fd, void *buf, size_t count);
typedef FILE* (*fopen_ptr)(const char *fp, const char * str);
typedef int (*closedir_ptr) (DIR * cd);
typedef int (*mkdir_ptr) (const char * str, mode_t mode);
typedef int (*close_ptr) ( int cd);
typedef DIR * (*opendir_ptr) ( const char *name);
typedef int (*open_ptr) (const char *pathname, int flags);
typedef int (*fstat_ptr) (int fd, struct stat *buf);
typedef size_t (*fwrite_ptr) (const void *ptr, size_t size, size_t nitems, FILE * stream);
typedef CURLcode (*curl_easy_perform_ptr) (CURL *easy_handle);
typedef struct curl_slist * (*curl_slist_append_ptr) (struct curl_slist *list, const char * string);
typedef CURL* (*curl_easy_init_ptr)();
typedef long (*ftell_ptr) (FILE *stream);
typedef int (*fscanf_ptr) (FILE *, const char *, va_list);
typedef pid_t (*fork_ptr) ();
typedef ssize_t (*write_ptr) (int fd, const void *buf, size_t count);
//typedef int (*stat_ptr) (const char *pathname, struct stat *statbuf);
typedef int (*fprintf_ptr) (FILE* stream, const char* format, va_list args);

popen_ptr popen_func = (popen_ptr) dlsym(RTLD_NEXT, "popen");
pclose_ptr pclose_func = (pclose_ptr) dlsym(RTLD_NEXT, "pclose");
pipe_ptr pipe_func = (pipe_ptr) dlsym(RTLD_NEXT, "pipe");
fclose_ptr fclose_func = (fclose_ptr) dlsym(RTLD_NEXT, "fclose");
fread_ptr fread_func = (fread_ptr) dlsym(RTLD_NEXT, "fread");
fgets_ptr fgets_func = (fgets_ptr) dlsym(RTLD_NEXT, "fgets");
fseek_ptr fseek_func = (fseek_ptr) dlsym(RTLD_NEXT, "fseek");
getline_ptr getline_func = (getline_ptr) dlsym(RTLD_NEXT, "getline");
readdir_ptr readdir_func = (readdir_ptr) dlsym(RTLD_NEXT, "readdir");
read_ptr read_func = (read_ptr) dlsym(RTLD_NEXT, "read");
fopen_ptr fopen_func = (fopen_ptr) dlsym(RTLD_NEXT, "fopen");
closedir_ptr closedir_func = (closedir_ptr) dlsym(RTLD_NEXT, "closedir");
mkdir_ptr mkdir_func = (mkdir_ptr) dlsym(RTLD_NEXT, "mkdir");
close_ptr close_func = (close_ptr) dlsym(RTLD_NEXT, "close");
opendir_ptr opendir_func = (opendir_ptr) dlsym(RTLD_NEXT, "opendir");
open_ptr open_func = (open_ptr) dlsym(RTLD_NEXT, "open");
fstat_ptr fstat_func = (fstat_ptr) dlsym(RTLD_NEXT, "fstat");
fwrite_ptr fwrite_func = (fwrite_ptr) dlsym(RTLD_NEXT, "fwrite");
curl_easy_perform_ptr curl_easy_perform_func = (curl_easy_perform_ptr) dlsym(RTLD_NEXT, "curl_easy_perform");
curl_slist_append_ptr curl_slist_append_func = (curl_slist_append_ptr) dlsym(RTLD_NEXT, "curl_slist_append");
curl_easy_init_ptr curl_easy_init_func = (curl_easy_init_ptr) dlsym(RTLD_NEXT, "curl_easy_init");
ftell_ptr ftell_func = (ftell_ptr) dlsym(RTLD_NEXT, "ftell");
fscanf_ptr fscanf_func = (fscanf_ptr) dlsym(RTLD_NEXT, "fscanf");
fork_ptr fork_func = (fork_ptr) dlsym(RTLD_NEXT, "fork");
write_ptr write_func = (write_ptr) dlsym(RTLD_NEXT, "write");
//stat_ptr stat_func = (stat_ptr) dlsym(RTLD_NEXT, "stat");
fprintf_ptr fprintf_func = (fprintf_ptr) dlsym(RTLD_NEXT, "fprintf");

extern "C" int fscanf(FILE *stream, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    if (g_fileIOMock == nullptr){
       return fscanf_func(stream, format, args);
    }

    int result = g_fileIOMock->fscanf(stream, format, args);
    va_end(args);
    return result;
}

extern "C" char* fgets(char *str, int n, FILE *stream)
{
    if(g_fileIOMock == nullptr){
       return fgets_func(str, n, stream);
    }

    return g_fileIOMock->fgets(str, n, stream);
}

extern "C" long ftell(FILE *stream)
{
    if (g_fileIOMock == nullptr){
        return ftell_func(stream);
    }
    
    return g_fileIOMock->ftell(stream);
}

extern "C" FILE* popen(const char * command, const char * type)
{
    if (g_fileIOMock == nullptr){
	    return popen_func(command, type);
    }
    
    return g_fileIOMock->popen(command, type);
}

extern "C" int pclose(FILE * stream)
{
    if (g_fileIOMock == nullptr){
        return pclose_func(stream);
    }
    
    return g_fileIOMock->pclose(stream);
}

extern "C" int pipe(int str[2])
{
    if (g_fileIOMock == nullptr){
        return pipe_func(str);
    }
    
    return g_fileIOMock->pipe(str);
}   

extern "C" int fclose(FILE * stream)
{
    if (g_fileIOMock == nullptr){
        return fclose_func(stream);
    }
    
    return g_fileIOMock->fclose(stream);
}

extern "C" size_t fread (void* ptr, size_t size, size_t nmemb,FILE* stream)
{
    if (g_fileIOMock == nullptr){
        return fread_func(ptr, size, nmemb, stream);
    }
    
    return g_fileIOMock->fread(ptr, size, nmemb, stream);
}

extern "C" int fseek(FILE *stream, long offset, int whence)
{
    if (g_fileIOMock == nullptr){
        return fseek_func(stream, offset, whence);
    }
    
    return g_fileIOMock->fseek(stream, offset, whence);
}

extern "C" ssize_t getline(char **str, size_t *n, FILE * stream)
{
    if (g_fileIOMock == nullptr){
        return getline_func(str, n, stream);
    }
    
    return g_fileIOMock->getline(str, n, stream);
}

extern "C" struct dirent * readdir(DIR *rd)
{
    if (g_fileIOMock == nullptr){
        return readdir_func(rd);
    }
    
    return g_fileIOMock->readdir(rd);
}

extern "C" ssize_t read(int fd, void *buf, size_t count)
{
    if (g_fileIOMock == nullptr){
        return read_func(fd, buf, count);
    }
    
    return g_fileIOMock->read(fd, buf, count);
}

extern "C" FILE* fopen(const char *fp, const char * str)
{
    if (g_fileIOMock == nullptr){
       return fopen_func(fp, str);
    }

    return g_fileIOMock->fopen(fp, str);

}

extern "C" int closedir(DIR * cd)
{
    if (g_fileIOMock == nullptr){
        return closedir_func(cd);
    }
    
    return g_fileIOMock->closedir(cd);
}

extern "C" int mkdir (const char * str, mode_t mode)
{
    if (g_fileIOMock == nullptr){
       return mkdir_func(str, mode);
    }
    
    return g_fileIOMock->mkdir(str, mode);
}

extern "C" int close( int cd)
{
    if (g_fileIOMock == nullptr){
        return close_func(cd);
    }
    
    return g_fileIOMock->close(cd);
}   

extern "C" DIR *opendir( const char *name)
{
    if (g_fileIOMock == nullptr){
        return opendir_func(name);
    }
    
    return g_fileIOMock->opendir(name);
}

extern "C" int open(const char *pathname, int flags)
{
    if (g_fileIOMock == nullptr){
        return open_func(pathname, flags);
    }
    
    return g_fileIOMock->open(pathname, flags);
}

extern "C" int fstat(int fd, struct stat *buf)
{
    if (g_fileIOMock == nullptr){
        return fstat_func(fd, buf);
    }
    
    return g_fileIOMock->fstat(fd, buf);
}

extern "C" size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE * stream)
{
    if (g_fileIOMock == nullptr){
        return fwrite_func(ptr, size, nitems, stream);
    }
    
    return g_fileIOMock->fwrite(ptr, size, nitems, stream);
}

extern "C" CURLcode curl_easy_perform(CURL *easy_handle)
{
    if (g_fileIOMock == nullptr){
        return curl_easy_perform_func(easy_handle);
    }

    
    return g_fileIOMock->curl_easy_perform(easy_handle);
}

extern "C" struct curl_slist *curl_slist_append(struct curl_slist *list, const char * string)
{
    if (g_fileIOMock == nullptr){
       return curl_slist_append_func(list, string);
    }
    
    return g_fileIOMock->curl_slist_append(list, string);
}

extern "C" CURL* curl_easy_init()
{
    if (g_fileIOMock == nullptr){
        return (CURL*)NULL;
    }
    
    return g_fileIOMock->curl_easy_init();
}

extern "C" pid_t fork(void)
{
   if(g_fileIOMock == nullptr){
	   return fork_func();
   }
   return g_fileIOMock->fork();
}

extern "C" ssize_t write(int fd, const void *buf, size_t count)
{
   if(g_fileIOMock == nullptr){
       return write_func(fd, buf, count);
   }
   return g_fileIOMock->write(fd, buf, count);
}

/*extern "C" int stat(const char *pathname, struct stat *statbuf)
{
   if(g_fileIOMock == nullptr){
        return stat_func(pathname, statbuf);
   }
   return g_fileIOMock->stat(pathname, statbuf);
}*/

extern "C" int fprintf(FILE* stream, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = -1;

    if (g_fileIOMock) {
        result = g_fileIOMock->fprintf(stream, format, args);
    }
    else{
	result = fprintf_func(stream, format, args);
    }
    va_end(args);
    return result;
}
