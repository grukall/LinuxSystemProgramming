#include "ssu_header.h"

void		check_opt_remove(int argc, char *argv[], int *opt_res);
file_inf	*check_backuplist_remove(char *file_path, file_inf *backup_inf);
void		remove_file(char *file_path, file_inf *backup_inf, int opt_a);
void		remove_directory(char *file_path, file_inf *backup_inf, int *opt_res);

/*void cmd_remove(file_inf *backup_inf, int argc, char *argv[])
입력 :	file_inf *backup_inf : 백업 파일 링크드리스트
	int argc, char *argv[] : main 함수 인자 그대로 입력
리턴 :	void
기능 :	ssu_backup의 기능 중 remove를 수행한다. 입력한 옵션을 분류하여 저장하고, 입력한 파일 경로를 확인한다.
	확인된 파일경로를 사용하여 옵션의 종류에 따라 remove_directory나 remove_file을 호출하여 백업 경로의 파일 삭제를 진행한다.
*/
void cmd_remove(file_inf *backup_inf, int argc, char *argv[])
{
	int opt;
	int opt_res[3] = {0,};
	char *backup_path;
	char *file_path;
	file_inf *inf = NULL;
	
	//getopt에 의해 옮겨지는 argv의 파일경로를 읽어들이기 위해 if_else문으로 구분지어 코드를 짯다.
	check_opt_remove(argc, argv, opt_res);
	if (opt_res[OPT_A] || opt_res[OPT_D] || opt_res[OPT_R])
		file_path = check_path_N(argv[3], "remove");
	else
		file_path = check_path_N(argv[2], "remove");
	
	// -d나 -r 옵션이 있으면 remove_directory를, 없으면 remove_file을 호출한다.
	if (opt_res[OPT_D] || opt_res[OPT_R])
		remove_directory(file_path, backup_inf, opt_res);
	else
		remove_file(file_path, backup_inf, opt_res[OPT_A]);
	
	// remove를 실행한 뒤 /home/backup 디렉토리에 빈 백업 디렉토리들이 있으면 삭제한다.
	check_directory_isEmpty();	
	exit (0);
}

/*
void check_opt_remove(int argc, char *argv[], int *opt_res)
입력 :	int argc, char *argv : main 함수 인자 그대로 입력
	int *opt_res : 옵션 유무를 저장할 int 배열
리턴 :	void
기능 :	getopt함수를 사용하여 remove 명령어에서 사용가능한 옵션의 입력 유무를 opt_res에 저장한다.
	잘못된 옵션 사용은 에러 처리한다.
*/
void check_opt_remove(int argc, char *argv[], int *opt_res)
{
	int opt;
	while ((opt = getopt(argc, argv, "dra")) != -1) { //-d -r -y옵션을 가지는지 검사
		switch (opt) {
			case 'd':
				if (!opt_res[OPT_D])
					opt_res[OPT_D] = 1;
				else
				{
					help("remove");
					error("remove> not valid option");
				}
				break ;
			case 'r':
				if (!opt_res[OPT_R])
					opt_res[OPT_R] = 1;
				else
				{
					help("remove");
					error("remove> not valid option");
				}
				break ;
			case 'a':
				if (!opt_res[OPT_A])
					opt_res[OPT_A] = 1;
				else
				{
					help("remove");
					error("remove> not valid option");
				}
				break ;
			default:
				help("remove");
				error("remove> not valid option");
		}
	}
}

/*
file_inf *check_backuplist_remove(char *file_path, file_inf *backup_inf)
입력 :	char *file_path : 백업 파일 유무를 알고싶은 파일의 경로
	file_inf *backup_inf : 백업 파일 링크드리스트
리턴 :	file_path를 origin_path(원본 경로)로 갖고있는 backup_inf의 노드가 있다면
	해당하는 노드들만 모아서 링크드리스트로 만든 후 리턴한다.
	해당하는 노드가 없다면 NULL를 리턴한다.
*/
file_inf *check_backuplist_remove(char *file_path, file_inf *backup_inf)
{
	file_inf *temp_inf = backup_inf;
	file_inf *res_inf = NULL;
	file_inf *temp_inf2 = NULL;
	int first = 1;

	while (temp_inf != NULL)
	{
		if (strcmp(temp_inf->origin_path, file_path) == 0)
		{
			if (first)
			{
				res_inf = (file_inf *)malloc(sizeof(file_inf));
				infcpy(res_inf, temp_inf);
				temp_inf2 = res_inf;
				first = 0;
			}
			else
			{
				temp_inf2->next = (file_inf *)malloc(sizeof(file_inf));
				temp_inf2 = temp_inf2->next;
				infcpy(temp_inf2, temp_inf);
			}
		}
		temp_inf = temp_inf->next;
	}
	if (temp_inf2 != NULL)
		temp_inf2->next = NULL;
	return res_inf;
}

/*
int remove_file(char *file_path, file_inf *backup_inf, int opt_a)
입력 :	char *file_path : 백업 파일을 지우고 싶은 파일의 경로
	file_inf *backup_inf : 백업 파일 링크드리스트
	int opt_a : -a 옵션 유무
리턴 :	void
기능 :	file_path의 백업 파일들을 check_backuplist_remove를 사용해 추출하고
	백업 파일이 있으면 삭제를 진행한다. 백업 파일이 2개 이상이면 백업 파일 목록을 출력하고 숫자를 입력받아 선택된 파일을 삭제한다.
	-a 옵션이 있으면 백업 파일 목록을 출력하지 않고 해당하는 모든 백업 파일을 삭제한다.*/
void remove_file(char *file_path, file_inf *backup_inf, int opt_a)
{
	int		file_fd;
	FILE	*log_fd;
	file_inf *backuped_inf;
	file_inf *temp_inf;
	char	buff[BUFF_SIZE];
	char *backup_time;
	time_t	timer;
	int row;

	timer = time(NULL);
	backup_time = make_backup_time(timer);
	
	//check_backuplist_remove를 호출해 file_path의 백업 파일 링크드리스트를 받는다. 만약 없으면 백업 파일이 없다고 출력 후 함수를 종료한다.
	if ((backuped_inf = check_backuplist_remove(file_path, backup_inf)) != NULL)
	{
		// 백업 파일이 한 개면 바로 삭제한다.
		if (backuped_inf->next == NULL)
		{
			if (remove(backuped_inf->file_path) == -1)
				error(strjoin("fail to remove ", backuped_inf->file_path));
			printf("\"%s\" removed by \"%s\"\n",backuped_inf->file_path, file_path);
			log_fd = fopen("/home/backup/ssubak.log", "a");
			fseek(log_fd, 0, SEEK_END);
			fprintf(log_fd, "%s:\"%s\" removed by \"%s\"\n", backup_time, backuped_inf->file_path, file_path);
			fclose(log_fd);
		}
		else
		{
				temp_inf = backuped_inf;
				int i = 1;
			// -a 옵션이 없으면 백업 파일 목록을 출력한다. 
			if (opt_a == 0)
			{
				printf("backup files of %s\n", file_path);
				printf("0. exit\n");
				while (temp_inf != NULL)
				{
					printf("%d. %s %sbytes\n", i++, temp_inf->backup_time, get_fsize(temp_inf->file_path));
					temp_inf = temp_inf->next;	
				}
				printf(">>");
				if(scanf("%d", &row) == -1)
					error("scanf");
				if (row >= i || row < 0)
					error("not valid row number");
				getchar();
				if (row != 0)
				{
					while (row > 1)
					{
						backuped_inf = backuped_inf->next;
						row--;
					}
					if (remove(backuped_inf->file_path) == -1)
						error(strjoin("fail to remove ", backuped_inf->file_path));

					// removed by를 출력하고 로그 파일에 기록한다.
					printf("\"%s\" removed by \"%s\"\n", backuped_inf->file_path, file_path);
					log_fd = fopen("/home/backup/ssubak.log", "a");
					fseek(log_fd, 0, SEEK_END);
					fprintf(log_fd, "%s:\"%s\" removed by \"%s\"\n", backup_time, backuped_inf->file_path, file_path);
					fclose(log_fd);
				}
			}
			else
			{
				log_fd = fopen("/home/backup/ssubak.log", "a");
				fseek(log_fd, 0, SEEK_END);
				while (backuped_inf != NULL)
				{
					if (remove(backuped_inf->file_path) == -1)
						error(strjoin("fail to remove ", backuped_inf->file_path));
					printf("\"%s\" removed by \"%s\"\n", backuped_inf->file_path, file_path);
					fprintf(log_fd, "%s:\"%s\" removed by \"%s\"\n", backup_time, backuped_inf->file_path, file_path);
					backuped_inf = backuped_inf->next;
				}
				fclose(log_fd);
			}
		}
	}
	else
		printf("%s\n", strjoin("no backup file ", file_path));
}

/*
void remove_directory(char *file_path, file_inf *backup_inf, int *opt_res)
입력 :	char *file_path : 백업 파일을 삭제하고 싶은 디렉토리 경로
	file_inf *backup_inf : 백업 파일 링크드리스트
	int *opt_res : 옵션 유무가 저장된 int 배열
리턴 :	void
기능 :	file_path 하위의 백업 파일 목록을 check_backuplist_rmdir를 사용해 링크드리스트로 받는다.
	받은 링크드리스트의 노드마다 remove_file를 호출하여 삭제를 진행한다.
	check_backuplist_rmdir이 NULL를 리턴하면 해당하는 백업 디렉토리가 없다고 출력한다.
*/
void remove_directory(char *file_path, file_inf *backup_inf, int *opt_res)
{
	file_inf *backuped_inf;

	if ((backuped_inf = check_backuplist_rmdir(file_path, backup_inf, opt_res)) != NULL)
	{
		while (backuped_inf != NULL)
		{
			remove_file(backuped_inf->origin_path, backup_inf, opt_res[OPT_A]);
			backuped_inf = backuped_inf->next;
		}
	}
	else
		printf("%s\n",strjoin("no backup directory ", file_path));
}
