#ifndef SSU_HEADER_H
# define SSU_HEADER_H

// 헤더 파일
#include <unistd.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>

// 파일 경로, 파일 이름, 버퍼 최대 크기
#define PATH_MAX 4096
#define NAME_MAX 255
#define BUFF_SIZE 20

// 옵션 별 정수 정의
#define OPT_D 0
#define OPT_R 1
#define OPT_Y 2
#define OPT_A 2
#define OPT_L 2
#define OPT_N 3

// 파일의 정보를 저장하는 file_inf 구조체
typedef struct file_inf {
	char	origin_path[PATH_MAX + 1]; // 파일의 원본 경로
	char	file_path[PATH_MAX + 1];   // 파일의 경로
	char	backup_time[13];           // 파일 백업 시간
	char	hash[33];		   // 파일 해쉬(파일 내용)
	int		valid;		   // 파일이 가장 최근에 백업한 파일이면 1, 아니면 0
	struct file_inf	*next;
}file_inf;

// 로그 파일의 정보를 저장하는 log_inf 구조체
typedef struct log_inf{
	char	file_path[PATH_MAX + 1];	//파일 경로
	char	origin_path[PATH_MAX + 1];	//파일의 원본 경로
	struct log_inf *next;
}log_inf;

// 도움말 출력 함수(cmd_help.c)
void	help(char *command);

// 모든 명령어 파일에서 범용적으로 사용하는 함수(ssu_backup_utils.c)
void 	error(char *explain_error);
char	*check_path(char *path, char *command);
char 	*check_path_N(char *path, char *command);
char	*strjoin(char const *s1, char const *s2);
char	*itoa(int n);
void 	infcpy(file_inf *res_inf, file_inf *src_inf);
char 	*get_fsize(char *file_path);
int 	length_size(int size);
char 	*make_backup_time(time_t timer);
int 	check_backup(char *backup_path);
file_inf	*check_backuplist_rmdir(char *file_path, file_inf *backup_inf, int *opt_res);
void 	check_directory_isEmpty(void);

//ssu_backup.c
void	make_linkedlist(file_inf **inf, log_inf *lg_inf, char *file_path, int *opt_res);

//cmd_backup.c
void	cmd_backup(file_inf *backup_inf, int argc, char *argv[]);

//cmd_remove.c
void	cmd_remove(file_inf *backup_inf, int argc, char *argv[]);

//cmd_recover.c
void 	cmd_recover(file_inf *backup_inf, int argc, char *argv[]);

//cmd_list.c
void	cmd_list(file_inf *backup_inf, log_inf *lg_inf, int argc, char *argv[]);
#endif
