#ifndef SSU_HEADER_H
# define SSU_HEADER_H
#define OPENSSL_API_COMPAT 0x10100000L
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <utime.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <openssl/md5.h>
#include <errno.h>
#include <signal.h>

#include <sys/sysinfo.h>

#define HASH_MD5  33
#define MAX(a, b) (a>b?a:b)
#define STUID "20202910"
#define DEBUG printf("DEBUG\n");

#define PATHMAX 4096
#define NAMEMAX 255
#define BUFMAX 1024
#define ARGMAX 16
#define FILEMAX 256

#define OPT_D         0b000001
#define OPT_R         0b000010
#define OPT_T         0b000100

#define CMD_ADD       0b00000001
#define CMD_REMOVE    0b00000010
#define CMD_LIST      0b00000100
#define CMD_HELP      0b00001000
#define CMD_EXIT      0b00010000

#define S_ADD 0b001
#define S_REM 0b010

typedef struct _backupNode {
  struct _dirNode *root_version_dir;
  struct _fileNode *root_file;

  char origin_path[PATHMAX];
  char backup_time[NAMEMAX];
  char mode[10];

  struct _backupNode *prev_backup;
  struct _backupNode *next_backup;
} backupNode;

typedef struct _fileNode {
  struct _dirNode *root_dir;

  int backup_count;

  //daemon에서 사용
  time_t mtime;
  char *file_md5;

  char file_name[FILEMAX];
  char file_path[PATHMAX];
  backupNode *backup_head;

  struct _fileNode *prev_file;
  struct _fileNode *next_file;
} fileNode;

typedef struct _dirNode {
  struct _dirNode *root_dir; //루트 디렉토리 노드

  int stag;       //stag에 등록된 노드

  int file_cnt;   //파일 갯수
  int subdir_cnt; //서브 디렉토리 갯수
  char dir_name[FILEMAX]; //현재 디렉토리 이름
  char dir_path[PATHMAX]; //현재 디렉토리 절대경로
  fileNode *file_head;     //파일이면 fileNode 연결
  struct _dirNode *subdir_head; //디렉토리면 dirNode 연결

  struct _dirNode* prev_dir; //이전 노드
  struct _dirNode *next_dir; //다음 노드
} dirNode;

typedef struct _pathNode {
  char path_name[FILEMAX];
  int depth;

  struct _pathNode *prev_path;
  struct _pathNode *next_path;

  struct _pathNode *head_path;
  struct _pathNode *tail_path;
} pathNode;

char *exe_path;
char *pwd_path;
char *home_path;
char *backup_path;
char *backup_log_path;

int monitor_log_fd;

dirNode* backup_dir_list;
dirNode *version_dir_list;
//pathNode *staging_area;

//ssu_header.c
char  *substr(char *str, int beg, int end);
char  *c_str(char *str);
char  *cvt_path_2_realpath(char* path);
int   path_list_init(pathNode *curr_path, char *path);
void  get_argument(int *argc, char *argv[], char *str);
char  *make_pid_status(char *mode, char *origin_path, char *original_path, char *backupFilePath, char *backupDir);
char	*ft_itoa(int n);
void  make_backupfile(char *backupDir, char *backupFilePath, char *file_path, char *path, char *cur_path, struct stat st);
void  remove_pidlog(char *backup_path);
char  *make_strBackupTime(char *backup_time);

int md5(char *target_path, char *hash_result);
char *cvtHash(char *target_path);

//ssu_init.c
int   init(char *path);
int   get_backup_list(int log_fd);
void  print_list(dirNode* dirList, int depth, int last_bit);
void  print_depth(int depth, int is_last_bit);
int   backup_list_insert(dirNode* dirList, char* path, char *origin_path, fileNode *filenode);
int   backup_list_remove(dirNode* dirList, char* path, char *origin_path);
int   init_backup_list(int slog_fd);

//ssu_help.c
void  help(int cmd_bit);
int   cmd_help(int argc, char *argv[]);

//ssu_struct.c
void    dirNode_init(dirNode *dir_node);
dirNode *dirNode_get(dirNode* dir_head, char* dir_name);
dirNode *dirNode_insert(dirNode* dir_head, char* dir_name, char* dir_path);
void    dirNode_remove(dirNode *dir_node);
dirNode *find_dirNode(dirNode *list, char *dir_path);

void      fileNode_init(fileNode *file_node);
fileNode  *fileNode_get(fileNode *file_head, char *file_name);
fileNode  *fileNode_insert(fileNode *file_head, char *file_name, char *dir_path, fileNode *filenode);
void      fileNode_remove(fileNode *file_node);
fileNode  *find_fileNode(dirNode *list, char *file_path);

void backupNode_init(backupNode *backup_node);
backupNode *backupNode_get(backupNode *backup_head, char *backup_time);
backupNode *backupNode_insert(backupNode *backup_head, char *mode, char *backup_time, char *file_path);
void backupNode_remove(backupNode *backup_node);

#endif