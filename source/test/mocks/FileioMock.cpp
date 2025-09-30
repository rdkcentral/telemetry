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
#include <stdarg.h>
#include <fcntl.h>



//typedef int (*fcntl_ptr) (int fd, int cmd, long arg);
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
//typedef int (*open_ptr) (const char *pathname, int flags);
typedef int (*open_ptr)(const char *pathname, int flags, ...);
typedef int (*fstat_ptr) (int fd, struct stat *buf);
typedef size_t (*fwrite_ptr) (const void *ptr, size_t size, size_t nitems, FILE * stream);
typedef CURLcode (*curl_easy_perform_ptr) (CURL *easy_handle);
typedef struct curl_slist* (*curl_slist_append_ptr) (struct curl_slist *list, const char * string);
typedef CURL* (*curl_easy_init_ptr)();
typedef CURLcode (*curl_easy_setopt_mock_ptr) (CURL *curl, CURLoption option, void* parameter);
typedef CURLcode (*curl_easy_getinfo_mock_ptr) (CURL *curl, CURLINFO info, void* arg);
typedef long (*ftell_ptr) (FILE *stream);
typedef int (*fscanf_ptr) (FILE *, const char *, va_list);
typedef pid_t (*fork_ptr) ();
typedef ssize_t (*write_ptr) (int fd, const void *buf, size_t count);
//typedef int (*stat_ptr) (const char *pathname, struct stat *statbuf);
typedef int (*fprintf_ptr) (FILE* stream, const char* format, va_list args);
//pedef CURLcode (*curl_easy_setopt_ptr) (CURL *curl, CURLoption option, parameter);
typedef void (*curl_easy_cleanup_ptr) (CURL *handle);
typedef void (*curl_slist_free_all_ptr) (struct curl_slist *list);
typedef int (*munmap_ptr) (void *addr, size_t len);
typedef void* (*mmap_ptr) (void *addr, size_t length, int prot, int flags, int fd, off_t offset);
typedef int (*mkstemp_ptr) (char *tmpl);
typedef ssize_t (*sendfile_ptr) (int out_fd, int in_fd, off_t *offset, size_t count);

typedef void (*rdkcertselector_free_ptr) (rdkcertselector_h *thiscertsel);
typedef rdkcertselector_h (*rdkcertselector_new_ptr) (const char *certsel_path, const char *hrotprop_path, const char *cert_group);
//protocol
typedef pid_t (*getpid_ptr) (void);
typedef void (*exit_ptr) (int status);

//fcntl_ptr fcntl_func = (fcntl_ptr) dlsym(RTLD_NEXT, "fcntl");
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
//open_ptr open_func = (open_ptr) dlsym(RTLD_NEXT, "open");
open_ptr open_func = (open_ptr)dlsym(RTLD_NEXT, "open");
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
curl_easy_setopt_mock_ptr curl_easy_setopt_mock_func = (curl_easy_setopt_mock_ptr) dlsym(RTLD_NEXT, "curl_easy_setopt_mock");
curl_easy_getinfo_mock_ptr curl_easy_getinfo_mock_func = (curl_easy_getinfo_mock_ptr) dlsym(RTLD_NEXT, "curl_easy_getinfo_mock");
curl_easy_cleanup_ptr curl_easy_cleanup_func = (curl_easy_cleanup_ptr) dlsym(RTLD_NEXT, "curl_easy_cleanup");
curl_slist_free_all_ptr curl_slist_free_all_func = (curl_slist_free_all_ptr) dlsym(RTLD_NEXT, "curl_slist_free_all");
//curl_easy_setopt_ptr curl_easy_setopt_func = (curl_easy_setopt_ptr) dlsym(RTLD_NEXT, "curl_easy_setopt");
munmap_ptr munmap_func = (munmap_ptr) dlsym(RTLD_NEXT, "munmap");
mmap_ptr mmap_func = (mmap_ptr) dlsym(RTLD_NEXT, "mmap");
mkstemp_ptr mkstemp_func = (mkstemp_ptr) dlsym(RTLD_NEXT, "mkstemp");
sendfile_ptr sendfile_func = (sendfile_ptr) dlsym(RTLD_NEXT, "sendfile");

rdkcertselector_new_ptr rdkcertselector_new_func = (rdkcertselector_new_ptr) dlsym(RTLD_NEXT, "rdkcertselector_new");
rdkcertselector_free_ptr rdkcertselector_free_func = (rdkcertselector_free_ptr) dlsym(RTLD_NEXT, "rdkcertselector_free");

getpid_ptr getpid_func = (getpid_ptr) dlsym(RTLD_NEXT, "getpid");
exit_ptr exit_func = (exit_ptr) dlsym(RTLD_NEXT, "exit");
/*
extern "C" int fcntl(int fd, int cmd, long arg)
{
    if (g_fileIOMock) {
        return g_fileIOMock->fcntl(fd, cmd, arg);
    }
    // fallback to real function if needed
}
*/

extern "C" void exit(int status)
{
    if (g_fileIOMock == nullptr){
        exit_func(status);
    }
    // In test mode, do nothing or handle as needed
    throw std::runtime_error("Exit called"); 
}

extern "C" int fscanf(FILE *stream, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    static int call_count = 0;
    if (g_fileIOMock == nullptr){
       return fscanf_func(stream, format, args);
    }
    if (strcmp(format, "%d") == 0) {
        int *out_ptr = va_arg(args, int *);
        if (call_count == 0) {
            *out_ptr = 1234;
            call_count++;
            va_end(args);
            return 1;
       }	    
       else {
            va_end(args);
            return 0; // No more PIDs
        }
    }
    va_end(args);
    int result = g_fileIOMock->fscanf(stream, format, args);
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
/*
extern "C" int open(const char *pathname, int flags)
{
    if (g_fileIOMock == nullptr){
        return open_func(pathname, flags);
    }
    
    return g_fileIOMock->open(pathname, flags);
}*/


extern "C" int open(const char *pathname, int flags, ...)
{
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }

    if (g_fileIOMock == nullptr) {
        if (flags & O_CREAT) {
            return open_func(pathname, flags, mode);
        } else {
            return open_func(pathname, flags);
        }
    }

    if (flags & O_CREAT) {
        return -1;
    } else {
        return g_fileIOMock->open(pathname, flags);
    }
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

extern "C" CURLcode curl_easy_setopt_mock(CURL *handle, CURLoption option, void* parameter)
{
  //va_list args;
   //a_start(args, option);
   //har *param = va_arg(args, char*);
   //rintf("option %d param %s\n", option, param);
   //a_end(args);
    if (g_fileIOMock == nullptr){
        return CURLE_OK;
    }
    return g_fileIOMock->curl_easy_setopt_mock(handle, option, parameter);
}

extern "C" CURLcode curl_easy_getinfo_mock(CURL *curl, CURLINFO info, void* arg)
{
    if (g_fileIOMock == nullptr){
        return CURLE_OK;
    }
    return g_fileIOMock->curl_easy_getinfo_mock(curl, info, arg);
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
        return curl_easy_init_func();
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
/*
extern "C" CURLcode curl_easy_setopt(CURL *curl, CURLoption option, ...)
{
    va_list args;
    va_start(args, option);
    CURLcode result = CURLE_OK;

    if (g_fileIOMock) {
        result = g_fileIOMock->curl_easy_setopt(curl, option, args);
    }
    else {
        result = curl_easy_setopt_func(curl, option, args);
    }
    va_end(args);
    return result;
}
*/
extern "C" void curl_easy_cleanup(CURL *handle)
{
    if (g_fileIOMock == nullptr){
        return curl_easy_cleanup_func(handle);
    }

    return g_fileIOMock->curl_easy_cleanup(handle);
}

extern "C" void curl_slist_free_all(struct curl_slist *list)
{
    if (g_fileIOMock == nullptr){
        return curl_slist_free_all_func(list);
    }

    return g_fileIOMock->curl_slist_free_all(list);
}

extern "C" int munmap(void *addr, size_t len)
{
    if (g_fileIOMock == nullptr){
        return munmap_func(addr, len);
    }

    return g_fileIOMock->munmap(addr, len);
}

extern "C" void* mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    if (g_fileIOMock == nullptr){
        return mmap_func(addr, length, prot, flags, fd, offset);
    }

    return g_fileIOMock->mmap(addr, length, prot, flags, fd, offset);
}

extern "C" int mkstemp(char *tmpl)
{
    if (g_fileIOMock == nullptr){
        return mkstemp_func(tmpl);
    }

    return g_fileIOMock->mkstemp(tmpl);
}

extern "C" ssize_t sendfile(int out_fd, int in_fd, off_t* offset, size_t count)
{
    if (g_fileIOMock == nullptr){
        return sendfile_func(out_fd, in_fd, offset, count);
    }

    return g_fileIOMock->sendfile(out_fd, in_fd, offset, count);
}

extern "C" rdkcertselector_h rdkcertselector_new(const char *certsel_path, const char *hrotprop_path, const char *cert_group)
{
    if (g_fileIOMock == nullptr){
        return rdkcertselector_new_func(certsel_path, hrotprop_path, cert_group);
    }

    return g_fileIOMock->rdkcertselector_new(certsel_path, hrotprop_path, cert_group);
}   

extern "C" void rdkcertselector_free(rdkcertselector_h *thiscertsel)
{
    if (g_fileIOMock == nullptr){
        return rdkcertselector_free_func(thiscertsel);
    }

    return g_fileIOMock->rdkcertselector_free(thiscertsel);
}

extern "C" pid_t getpid(void)
{
   if(g_fileIOMock == nullptr){
       return getpid_func();
   }
   return g_fileIOMock->getpid();
}
