#include "ssu_header.h"

int check_already_backuped(char* path);
int check_path_access(char* path);

/*
ssu_remove의 main
char *argv[]에 사용자가 입력한 명령어와 경로가 담겨있다.
먼저 init으로 현재 추적중인 파일 및 디렉토리의 링크드리스트를 만들고, 각종 검사를 통과하면
추적중인 파일 링크드리스트에 제외시킨다.
*/
int main(int argc, char *argv[])
{
    char *path;
    char *skip_ptr;
    int slog_fd;
    int i;

    if(init() == -1) {
      fprintf(stderr, "ERROR: init error.\n");
      exit(1);
    }
    if (argv[1] == NULL)
    {
        fprintf(stderr, "ERROR: <PATH> is not include\nUsage: remove <PATH> : record path to staging area, path will not tracking modification\n");
        exit (1);
    }
    if ((path = cvt_path_2_realpath(argv[1])) == NULL) {
        fprintf(stderr, "ERROR: '%s' is wrong path\n", argv[1]);
        return -1;
    }

    if (!strcmp(pwd_path, path))
        skip_ptr = NULL;
    else
    {
        for (i = 0; pwd_path[i] != '\0'; i++)
        {
            if (pwd_path[i] != path[i])
            {
                skip_ptr = &path[i];
                break;
            }
        }
        if (pwd_path[i] == '\0')
            skip_ptr = &path[i + 1];
    }


    if (access(path, F_OK | R_OK) == -1)
    {
        fprintf(stderr, "'%s' is wrong path\n", skip_ptr);
        exit(1);
    }

    if (check_path_access(path)) {
        exit(1);
    }
    
    if (!check_already_backuped(path)) {
        if (skip_ptr == NULL)
            fprintf(stderr, "\".\" is already remove from staging area\n");
        else
            fprintf(stderr, "\"./%s\" is already remove from staging area\n", skip_ptr);
        exit(1);
    }

    slog_fd = open(slog_path, O_WRONLY | O_APPEND);
    if (slog_fd == -1)
    {
        fprintf(stderr, "'DEBUG: open error %s\n", slog_path);
        exit(1);
    }


    char buffer[BUF_MAX];
    sprintf(buffer, "remove \"%s\"\n", path);
    write(slog_fd, buffer, strlen(buffer));
    close(slog_fd);

    if (skip_ptr == NULL)
        fprintf(stdout, "remove \".\"\n");
    else
        fprintf(stdout, "remove \"./%s\"\n", skip_ptr);

    exit(0);
}

/*
int check_already_backuped(char* path)
입력 : char *path : 검사할 파일 경로
리턴 : 백업이 있으면 1, 없으면 0 리턴
기능 : 입력받은 경로가 추적 파일 링크드리스트에 존재하는 지 링크드리스트를 순회하며 검사
*/
int check_already_backuped(char* path) {
    char* tmp_path = (char*)malloc(sizeof(char)*PATH_MAX);
    char* ptr;
    char* skip_ptr = NULL;
    dirNode* curr_dir;
    fileNode* curr_file;

    strcpy(tmp_path, path);
    curr_dir = backup_dir_list->subdir_head;
    if (curr_dir == NULL)
        return 0;

    // '/' 단위로 잘라가며 진행
    while((ptr = strchr(tmp_path, '/')) != NULL) {
        char* dir_name = substr(tmp_path, 0, strlen(tmp_path) - strlen(ptr));
        char* dir_path = substr(path, 0, strlen(path) - strlen(ptr) + 1);
        while(curr_dir->next_dir != NULL) {
            if (!strcmp(dir_name, curr_dir->next_dir->dir_name)) {
                break;
            }
            curr_dir = curr_dir->next_dir;
        }

        // 생략출력을 위한 포인터 이동
        if (!strcmp(dir_path, pwd_path)) {
            skip_ptr = ptr;
        }

        //같은 디렉토리가 없다면 백업되지 않았으므로 return 0
        if(curr_dir->next_dir == NULL) {
            return 0;
        }

        tmp_path = ptr + 1;
        if(strchr(tmp_path, '/')) {
        curr_dir = curr_dir->next_dir->subdir_head;
        }
    }

    if (curr_dir->next_dir->subdir_head != NULL)
    {
        dirNode *temp_dir = curr_dir->next_dir->subdir_head;
        while (temp_dir->next_dir != NULL)
        {
            if (temp_dir->next_dir->stag == 1 && strcmp(substr(temp_dir->next_dir->dir_path, 0 , strlen(temp_dir->next_dir->dir_path) - 1), path) == 0)
            {
                return 1;
            }
            temp_dir = temp_dir->next_dir;
        }
    }

    curr_file = curr_dir->next_dir->file_head;
    while(curr_file->next_file != NULL) {
        if(curr_file->next_file->stag == 1 && !strcmp(tmp_path, curr_file->next_file->file_name)) {
            return 1;
        }
        curr_file = curr_file->next_file;
    }
    return 0;
}

/*
int check_path_access(char* path)
입력 : char *path : 검사할 경로
리턴 : 검사를 통과하면 0, 통과하지 못하면 1을 리턴
기능 : 입력한 경로가 현재 디렉토리를 벗어나는지, .repo 디렉토리를 포함하는 지 검사.
*/
int check_path_access(char* path) {
    int i;
    int cnt;
    char *origin_path = (char*)malloc(sizeof(char)*(strlen(path)+1));
    char *ptr;
    int depth = 0;
    
    strcpy(origin_path, path);

    if ((ptr = strstr(origin_path, pwd_path)) == NULL)
    {
        fprintf(stderr, "ERROR: %s path must be in %s\n", path, pwd_path);
        return 1;
    }
    else if (ptr != origin_path)
    {
        fprintf(stderr, "ERROR: %s path must be in %s\n", path, pwd_path);
        return 1;
    }
    else if (strstr(origin_path, ".repo"))
    {
        fprintf(stderr, "ERROR: %s path must be not include .repo\n", path);
        return 1;
    }
    else
        return 0;
}