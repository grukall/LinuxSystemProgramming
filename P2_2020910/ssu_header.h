#ifndef SSU_HEADER_H
# define SSU_HEADER_H
#define OPENSSL_API_COMPAT 0x10100000L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <openssl/md5.h>
#include <errno.h>

#include <sys/sysinfo.h>

#define DEBUG printf("DEBUG\n");

#define PATH_MAX 4096
#define BUF_MAX 4096
#define FILE_MAX 256

#define ADD_C     "./ssu_add"
#define REMOVE_C  "./ssu_remove"
#define HELP_C    "./ssu_help"


#define OPT_D         0b000001
#define OPT_R         0b000010
#define OPT_Y         0b000100
#define OPT_A         0b001000
#define OPT_L         0b010000
#define OPT_N         0b100000

#define ADD_SEP        "add \""
#define REMOVE_SEP     "remove \""

#define HASH_MD5  33

typedef struct _backupNode {
  struct _dirNode *root_version_dir;
  struct _fileNode *root_file;

  char origin_path[PATH_MAX];

  struct _backupNode *prev_backup;
  struct _backupNode *next_backup;
} backupNode;

typedef struct _fileNode {
  struct _dirNode *root_dir;

  int stag;

  char file_name[FILE_MAX];
  char file_path[PATH_MAX];
  backupNode *backup_head;

  struct _fileNode *prev_file;
  struct _fileNode *next_file;
} fileNode;

typedef struct _dirNode {
  struct _dirNode *root_dir; //루트 디렉토리 노드

  int stag;       //stag에 등록된 노드

  int file_cnt;   //파일 갯수
  int subdir_cnt; //서브 디렉토리 갯수
  char dir_name[FILE_MAX]; //현재 디렉토리 이름
  char dir_path[PATH_MAX]; //현재 디렉토리 절대경로
  fileNode *file_head;     //파일이면 fileNode 연결
  struct _dirNode *subdir_head; //디렉토리면 dirNode 연결

  struct _dirNode* prev_dir; //이전 노드
  struct _dirNode *next_dir; //다음 노드
} dirNode;

typedef struct _pathNode {
  char path_name[FILE_MAX];
  int depth;

  struct _pathNode *prev_path;
  struct _pathNode *next_path;

  struct _pathNode *head_path;
  struct _pathNode *tail_path;
} pathNode;

dirNode* backup_dir_list;
dirNode *version_dir_list;

char *exe_path; 
char *pwd_path;
char *home_path;

char *repo_path; // .repo 경로
char *clog_path; // .repo/commit.log 경로
char *slog_path; // .repo/staging.log 경로

//ssu_add.c
void cmd_add(char *buffer);

//ssu_help.c
void help();
int help_process(int argc, char* argv[]);

//ssu_header.c
char *substr(char *str, int beg, int end);
char *c_str(char *str);
char *cvt_path_2_realpath(char* path);
int path_list_init(pathNode *curr_path, char *path);


//ssu_struct.c
void dirNode_init(dirNode *dir_node);
dirNode *dirNode_get(dirNode* dir_head, char* dir_name);
dirNode *dirNode_insert(dirNode* dir_head, char* dir_name, char* dir_path);
void dirNode_remove(dirNode *dir_node);

void fileNode_init(fileNode *file_node);
fileNode *fileNode_get(fileNode *file_head, char *file_name);
fileNode *fileNode_insert(fileNode *file_head, char *file_name, char *dir_path);
void fileNode_remove(fileNode *file_node);

//ssu_init.c
int init();
int get_backup_list(int log_fd);
void print_list(dirNode* dirList, int depth, int last_bit);
void print_depth(int depth, int is_last_bit);
int backup_list_insert(dirNode* dirList, char* path, char *origin_path, int isDir);
int backup_list_remove(dirNode* dirList, char* path, char *origin_path, int isDir);
#endif