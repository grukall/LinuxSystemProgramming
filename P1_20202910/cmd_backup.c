#include "ssu_header.h"

void		check_opt(int argc, char *argv[], int *opt_res);
int		check_backuplist(file_inf *inf, file_inf *backup_inf);
void		file_backup(char *backup_path, file_inf *inf, file_inf *backup_inf, int opt_y);
char		*make_backup_path();
file_inf	*make_directory(char *backup_path, file_inf *inf, file_inf *backup_inf, char *file_path, int opt_res[3]);
int		check_backup(char *backup_path);

/*
void cmd_backup(file_inf *backup_inf, int argc, char *argv[])
입력 :	file_inf *backup_inf : 백업 파일들의 정보가 들어있는 링크드리스트
	int argc, char *argv[] : main 함수의 인자
리턴 :	void
기능 :	ssu_backup의 기능 중 backup을 수행한다. 입력한 옵션을 분류하여 저장하고, 입력한 파일경로를 기반으로 링크드리스트 inf를 만든 다음,
	inf와 backup_inf를 비교해가며 백업 디렉토리에 백업을 진행한다.
*/
void cmd_backup(file_inf *backup_inf, int argc, char *argv[])
{
	int opt;
	int opt_res[3] = {0,};
	char *backup_path;
	char *file_path;
	file_inf *inf = NULL;
	
	// get_opt 함수로 달라지는 argv 파일 경로 위치에 따라 if_else문으로 구분지어 실행
	check_opt(argc, argv, opt_res);
	if (opt_res[OPT_Y] || opt_res[OPT_D] || opt_res[OPT_R])
		file_path = check_path(argv[3], "backup");
	else
		file_path = check_path(argv[2], "backup");
	
	//입력한 파일 경로 기반으로 링크드리스트 inf생성
	make_linkedlist(&inf, NULL, file_path, opt_res);
	
	// 현재 시간을 기준으로 백업 파일 경로 생성	
	backup_path = make_backup_path();
	if(mkdir(backup_path, 0777) == -1)
		error("backup> fail to make backup directory");
	
	// -d 나 -r 옵션이 있으면 디렉토리 백업을, 없으면 파일 백업을 진행한다.
	if (opt_res[OPT_D] || opt_res[OPT_R])
		make_directory(backup_path, inf, backup_inf, file_path, opt_res);
	else
		file_backup(strjoin(backup_path, strrchr(inf->file_path, '/')), inf, backup_inf, opt_res[OPT_Y]);
	
	//백업 디렉토리에 아무것도 안들어 있으면 삭제한다.
	check_backup(backup_path);
	exit (0);
}

/*
void check_opt(int argc, char *argv[], int *opt_res)
입력 : 	int argc, char *argv[] : main 함수 인자 그대로 입력
	int *opt_res : 결과를 저장할 int 배열 주소
리턴 :	void
기능 :	getopt 함수를 이용하여 사용자가 입력한 옵션의 종류를 opt_res에 저장한다.
	잘못된 옵션 사용은 에러처리 한다.
*/
void check_opt(int argc, char *argv[], int *opt_res)
{
	int opt;
	while ((opt = getopt(argc, argv, "dry")) != -1) { //-d -r -y옵션을 가지는지 검사
		switch (opt) {
			case 'd':
				if (!opt_res[OPT_D])
					opt_res[OPT_D] = 1;
				else									
				{
					help("backup");
					error("backup> not valid option");
				}
				break ;
			case 'r':
				if (!opt_res[OPT_R])
					opt_res[OPT_R] = 1;
				else
				{
					help("backup");
					error("backup> not valid option");
				}
				break ;
			case 'y':
				if (!opt_res[OPT_Y])
					opt_res[OPT_Y] = 1;
				else
				{
					help("backup");
					error("backup> not valid option");
				}
				break ;
			default:
				help("backup");
				error("backup> not valid option");
		}
	}
}

/*
int	check_backuplist(file_inf *inf, file_inf *backup_inf)
입력 : 	file_inf *inf : 백업 파일들과 비교해볼 file_inf 노드
	file_inf *backup_inf : 백업 파일 링크드리스트
리턴 :	backup_inf 링크드리스트에 이미 백업된 파일이 있으면 return 0, 아니면 리턴 1.	
기능 :	backup_inf 링크드리스트에 inf와 동일한 해쉬값을 가지고 valid가 1인 파일이 있으면 already backuped to 를 출력하고
	0을 리턴한다. 동일한 백업 파일이 없으면 1를 리턴한다. file_backup 함수에서 백업 파일 유무 판단을 위해 사용된다.
*/
int	check_backuplist(file_inf *inf, file_inf *backup_inf)
{
	if (backup_inf == NULL)
		return 1;
	file_inf *temp_inf = backup_inf;
	if (access(inf->file_path, R_OK) == -1)
		error(strjoin("fail to access ", inf->file_path));
	while (temp_inf != NULL)
	{
		if(strcmp(temp_inf->hash, inf->hash) == 0 && temp_inf->valid == 1)
		{
			printf("\"%s\" already backuped to \"%s\"\n", inf->file_path, temp_inf->file_path);
			return 0;
		}
		temp_inf = temp_inf->next;
	}
	return 1;
}

/*
void file_backup(char *backup_path, file_inf *inf, file_inf *backup_inf, int opt_y)
입력 :	char *backup_path : 백업 파일 경로
	file_inf *inf : 백업할 파일의 file_inf 노드
	file_inf *backup_inf : 백업 링크드리스트
	int  opt_y : 옵션 -y의 유무
리턴 :	void
기능 :	백업하고 싶은 파일의 file_inf 노드인 inf를 backup_inf 링크드리스트와 비교하여 중복확인을 한다.
	중복된 파일은 already backuped to를 출력하여 끝내고, 아닌 파일은 backup_path에 파일을 생성하여
	내용을 복사한다. 복사에 성공하면 backup to를 출력하고 로그파일에 기록한다.
*/
void file_backup(char *backup_path, file_inf *inf, file_inf *backup_inf, int opt_y)
{
	int		backup_fd;
	int		file_fd;
	FILE	*log_fd;
	int		file_size;
	int		read_size;
	char	buff[BUFF_SIZE];
	
	if (opt_y || check_backuplist(inf, backup_inf))
	{
		// 백업 파일 생성
		backup_fd = open(backup_path, O_WRONLY | O_CREAT, 0777);
		if (backup_fd == -1)
			error(strjoin("backup>", backup_path));

		file_fd = open(inf->file_path, O_RDONLY);
		if (file_fd == -1)
			error(strjoin("backup>", inf->file_path));
	

		// 원본 파일내용을 백업 파일로 복사
		while ((read_size = read(file_fd, buff, BUFF_SIZE)) > 0)
		{
			if (write(backup_fd, buff, read_size) == -1)
				error(strjoin("backup> fail to write", backup_path));
			if (read_size < BUFF_SIZE)
				break ;
		}
		// 로그 기록 및 터미널 출력
		printf("\"%s\" backuped to \"%s\"\n", inf->file_path, backup_path);
		log_fd = fopen("/home/backup/ssubak.log", "a");
		fseek(log_fd, 0, SEEK_END);
		fprintf(log_fd, "%s:\"%s\" backuped to \"%s\"\n", inf->backup_time, inf->file_path, backup_path);
		close(backup_fd);
		close(file_fd);
		fclose(log_fd);
	}
}


/*char *make_backup_path()
입력 :	void
리턴 :	백업 디렉토리 이름으로 사용할 char 문자열을 반환한다.
기능 :	현재 시간을 기준으로 make_backup_time 함수를 이용하여 백업 디렉토리 이름으로 사용할 문자열을 할당하여 반환한다.
*/
char *make_backup_path()
{
	time_t	timer;
	char	*temp_filename;
	char *temp;

	timer = time(NULL);
	temp_filename = make_backup_time(timer);
	
	temp = strjoin("/home/backup/", temp_filename);
	free(temp_filename);
	return temp;
}

/*
file_inf	*make_directory(char *backup_path, file_inf *inf, file_inf *backup_inf, char *file_path, int opt_res[3])
입력 :	char *backup_path : 백업할 디렉토리 경로
	file_inf *inf : 백업할 파일들의 file_inf 링크드리스트
	file_inf *backup_inf : 백업 파일들의 링크드리스트
	char *file_path : 원본 파일 경로
	int opt_res[3] : 옵션 유무 int 배열
리턴 :	현재 참조중인 inf의 노드를 반환한다. 함수가 재귀적으로 호출될 때 링크드리스트 inf의 진행상황을 동기화하기 위함이기 때문에
	함수 내부적으로만 사용한다.
기능 :	file_path(원본 파일 경로)를 재귀적으로 탐색하면서 inf(백업 하고 싶은 file_inf 링크드리스트)를 backup_path(백업 경로)로
	file_backup 함수를 호출하여 백업한다. 파일을 우선적으로 탐색하여 BFS를 보장한다.
*/
file_inf	*make_directory(char *backup_path, file_inf *inf, file_inf *backup_inf, char *file_path, int opt_res[3])
{
	struct dirent **list;
	struct stat fileStat;
	file_inf *temp_inf = inf;
	int count;
	
	// scandif로 file_path를 열어 정보를 list에 저장한다.
	if ((count = scandir(file_path, &list, NULL, alphasort)) == -1)
		error(strjoin("fail to open directory : ", file_path));
	
	// 파일을 우선 탐색하며 file_backup 함수를 호출한다.
	for (int i = 0; i < count; ++i)
	{
        	if (list[i]->d_type != DT_DIR && strcmp(list[i]->d_name, "ssubak.log") != 0)
		{
            		char fullPath[4096];
            		snprintf(fullPath, sizeof(fullPath), "%s/%s", file_path, list[i]->d_name);

            		if (stat(fullPath, &fileStat) == -1)
				error(strjoin("fail to open stat ", fullPath));

			char *temp_path;
			temp_path = strrchr(temp_inf->file_path, '/');
			file_backup(strjoin(backup_path, temp_path), temp_inf, backup_inf, opt_res[OPT_Y]);
			free(list[i]);
			if (temp_inf->next != NULL)
				temp_inf = temp_inf->next;
			else
				break ;
	        }
    	}
	
	// 현재 디렉토리 경로에 파일 탐색이 끝나면 디렉토리를 탐색한다. 서브 디렉토리가 있으면 해당 파일 경로로 다시 make_directory를 호출한다.
	for (int i = 0; i < count; ++i)
	{
        if (list[i]->d_type == DT_DIR && strcmp(list[i]->d_name, ".") != 0 && strcmp(list[i]->d_name, "..") != 0)
		{
            if (opt_res[OPT_R]) {
				char fullPath[4096];
				snprintf(fullPath, sizeof(fullPath), "%s/%s", file_path, list[i]->d_name);

				char newPath[PATH_MAX];
				snprintf(newPath, sizeof(newPath), "%s/%s", backup_path, list[i]->d_name);
				if (mkdir(newPath, 0777) == -1)
					error(strjoin("fail to make directory", newPath));
				temp_inf = make_directory(newPath, temp_inf, backup_inf, fullPath, opt_res);
            }
			free(list[i]);
		}
	}
	free(list);
	return temp_inf;
}
