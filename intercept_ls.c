#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

// 定义被拦截的函数指针
typedef DIR* (*real_opendir_t)(const char *name);
typedef int (*real_closedir_t)(DIR *dirp);
typedef struct dirent* (*real_readdir_t)(DIR *dirp);

// 拦截opendir函数
DIR* opendir(const char *name) {
    static real_opendir_t real_opendir = NULL;
    if (real_opendir == NULL) {
        real_opendir = (real_opendir_t)dlsym(RTLD_NEXT, "opendir");
    }

    // 获取当前进程名
    char proc_self[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", proc_self, sizeof(proc_self) - 1);
    if (len != -1) {
        proc_self[len] = '\0';

        // 判断是否是 ls 命令
        if (strstr(proc_self, "/ls") == NULL) {
            // 不是 ls，直接调用原始函数
            return real_opendir(name);
        }
    }

    const char *target_dir = "/home/fleam/intercept-ls/test";
    char resolved_path[PATH_MAX];

    if (name == NULL || strcmp(name, ".") == 0) {
        if (getcwd(resolved_path, sizeof(resolved_path)) == NULL) {
            return NULL;
        }
    } else {
        if (realpath(name, resolved_path) == NULL) {
            strncpy(resolved_path, name, sizeof(resolved_path));
        }
    }

    if (strcmp(resolved_path, target_dir) == 0) {
        printf("对指定目录的 opendir 操作被拦截，不允许打开此目录。\n");
        return NULL;
    }

    return real_opendir(name);
}

// 拦截closedir函数
int closedir(DIR *dirp) {
    static real_closedir_t real_closedir = NULL;
    if (real_closedir == NULL) {
        real_closedir = (real_closedir_t)dlsym(RTLD_NEXT, "closedir");
    }
    return real_closedir(dirp);
}

// 拦截readdir函数
struct dirent* readdir(DIR *dirp) {
    static real_readdir_t real_readdir = NULL;
    if (real_readdir == NULL) {
        real_readdir = (real_readdir_t)dlsym(RTLD_NEXT, "readdir");
    }

    // 要拦截的指定目录
    const char *target_dir = "/home/fleam/intercept-ls/test";
    struct stat st;
    char full_path[1024];
    if (dirp != NULL) {
        while (1) {
            struct dirent *entry = real_readdir(dirp);
            if (entry == NULL) {
                return NULL;
            }
            snprintf(full_path, sizeof(full_path), "%s/%s", target_dir, entry->d_name);
            if (stat(full_path, &st) == 0) {
                // 这里可以进一步处理，比如不返回该目录项
                continue;
            }
            return entry;
        }
    }
    return NULL;
}