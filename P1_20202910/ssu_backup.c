#include "ssu_header.h"

void		check_file(void);
char		*check_path(char *path, char *command);
int		check_command(char *command);
file_inf	*make_struct(char *path, time_t current_time, log_inf *lg_inf);
void		check_duplicate(file_inf *inf, file_inf *last_inf);
void		check_duplicate_log(log_inf *inf, log_inf *last_inf);
void		makelist(file_inf **inf, log_inf *lg_inf, char *dirPath, int *opt_res);
void		make_linkedlist(file_inf **inf, log_inf *lg_inf, char *file_path, int *opt_res);
void		make_loglist(log_inf **inf);

/*
메인 함수
입력 :	터미널에서 입력받은 명령어와 파일경로 및 옵션들
기능 :	/home/backup의 파일들과 ssubak.log 파일을 읽어 백업파일 링크드리스트인 backup_inf를 만들고, 입력받은 명령어를 구분해 적합한 명령어를 실행시킨다.
*/
int main(int argc, char *argv[])
{
	int command_num;
	file_inf *backup_inf = NULL;
	log_inf *lg_inf = NULL;
	umask(000);

	//백업 파일 경로와 로그 파일 체크 및 생성
	check_file(); 

	//ssubak.log 파일의 정보를 정리한 링크드리스트 lg_inf 생성
	make_loglist(&lg_inf);

	//lg_inf와 /home/backup에 있는 파일들의 정보를 종합하여 백업 파일들의 링크드리스트 backup_inf 생성
	make_linkedlist(&backup_inf, lg_inf, "/home/backup", NULL);

	//사용한 lg_inf(로그파일 링크드리스트)는 할당 해제.
	log_inf *temp_inf = lg_inf;
	while (lg_inf != NULL)
	{
		temp_inf = lg_inf->next;
		free(lg_inf);
		lg_inf = temp_inf;
	}
	
	// 입력 인자가 잘못되었거나 유효하지 않은 명령어이면 에러 처리한다.
	if (argc <= 1)
	{
		fprintf(stderr, "ERROR: wrong input.\n%s help : show commands for program\n", argv[0]);
		exit (1);
	}
	if ((command_num = check_command(argv[1])) == -1)
	{
		fprintf(stderr, "ERROR: invalid command -- '%s'\n%s help : show commands for program\n", argv[1], argv[0]);
		exit (1);
	}

	//command 종류에 따라 내장 명령어 함수 실행
	switch (command_num) {
		case 0:
			help(argv[2]);
			break ;
		case 1:
			cmd_backup(backup_inf, argc, argv);
			break ;
		case 2:
			cmd_remove(backup_inf, argc, argv);
			break ;
		case 3:
			cmd_recover(backup_inf, argc, argv);
			break ;
		case 4:
			printf("list\n");
			break ;
		default:
			error("system error");
			break ;
	}
	exit(0);
}

/*
void check_file(void)
입력 : void
리턴 값: void
기능 : /home/backup 디렉토리와 /home/backup/ssubak.log 파일이 있는지 확인하고 없으면 생성한다.
*/
void check_file(void)
{
	if (access("/home/backup", F_OK) == -1)
	{
		if (mkdir("/home/backup", 0777) == -1)
			error("fail to make directory /home/backup");
	}
	if (access("/home/backup/ssubak.log", F_OK) == -1)
	{
		int	fd;
		if ((fd = open("/home/backup/ssubak.log",O_RDWR | O_CREAT, 0777)) == -1)
			error("fail to make logfile /home/backup/ssubak.log");
		close(fd);
	}
}

/*
char *check_path(char *path, char *command)
입력 :	char *path : 검사할 파일 경로
	char *command : check_path를 실행한 명령어 함수 이름
리턴 : char 문자열 포인터, 절대경로로 변환된 파일경로를 리턴한다.
기능 : 입력받은 path를 절대경로로 변환한다. 이때, 변경된 절대경로가 '/'면 주어진 경로를 벗어난 파일경로로 간주하고 에러처리 한다.
*/
char *check_path(char *path, char *command)
{
	char *tmp_path;
	
	tmp_path = (char *)malloc(sizeof(char) * PATH_MAX);
	if (!tmp_path)
		error("fail to malloc");
	if (realpath(path, tmp_path) == NULL)
	{
		help(command);
		error("fail to realpath");
	}
	if (*tmp_path == '\0')
	{
		help("backup");
		error("please enter the path");
	}
	if (strstr(tmp_path, "/home") != tmp_path)
		error("To go beyond the home directory");
	return tmp_path;
}

/*
int check_command(char *command)
입력 :	char *command : 사용자가 입력한 명령어
리턴 :	입력한 명령어에 맞는 int값
기능 :	사용자가 입력한 명령어에 맞추어 알맞은 int값을 리턴힌다. main함수에서 switch문을 통한 명령어 실행에 사용된다.
*/
int check_command(char *command)
{
	if (!strcmp(command,"help"))
		return 0;
	else if (!strcmp(command, "backup"))
		return 1;
	else if (!strcmp(command,"remove"))
		return 2;
	else if (!strcmp(command,"recover"))
		return 3;
	else if (!strcmp(command,"list"))
		return 4;
	return -1;
}

/*
file_inf *make_struct(char *path, time_t current_time, log_inf *lg_inf)
입력 :	char	*path : file_inf로 생성할 파일 경로
	time_t	current_time : 생성할 구조체에 입력할 시간, 0 입력 파일의 최종 수정시간으로 설정
	log_inf	*lg_inf : 생성할 구조체에 사용할 로그파일 링크드리스트, NULL 입력 시 file_inf의 origin_path입력이 생략된다.

리턴 :	path에 있는 파일과 current_time을 갖고 file_inf 구조체를 생성한다. lg_inf가 NULL이 아니라면 lg_inf에서 path에 있는 파일의 원본경로를 찾아 origin_path에 넣는다.
기능 :	입력한 파일경로에 있는 파일의 file_inf 구조체를 생성하는 함수이다. make_list 함수에서 링크드리스트를 만들 때 사용된다.
*/
file_inf *make_struct(char *path, time_t current_time, log_inf *lg_inf)
{
	struct stat temp_stat;
	int			fd;
	int			count;
	char 		buffer[BUFF_SIZE] = {0,};

	file_inf *newnode = (file_inf *)malloc(sizeof(file_inf));
	if (newnode == NULL)
		error(strjoin("fail to malloc : ", path));

	strcpy(newnode -> file_path, path);
	if (newnode -> file_path == NULL)
		error(strjoin("fail to strcpy :", path));

	if (stat(path,  &temp_stat) == -1)
		error(strjoin("fail to open stat", path));
	
	//current_time이 0이면, 파일의 최종 수정 시간을 file_inf의 backup_time에 입력 한다.
	if (current_time == 0)
		strcpy(newnode->backup_time, make_backup_time(temp_stat.st_mtime));
	else
		strcpy(newnode->backup_time, make_backup_time(current_time));
	if (newnode->backup_time == NULL)
		error("fail to strcpy backuptime");

	fd = open(path, O_RDONLY);
	if (fd == -1)
		error(strjoin("fail to open file : ", path));

	//MD5 라이브러리를 이용해 파일 내용을 hash로 만들어 file_inf의 hash에 저장한다.
	MD5_CTX md5_ctx;
	MD5_Init(&md5_ctx);
	for (int i = 0; i < 33; i++)
		newnode->hash[i] = 0;
	while ((count = read(fd, buffer, BUFF_SIZE)) > 0)
		MD5_Update(&md5_ctx, buffer, count);
	MD5_Final(newnode->hash, &md5_ctx);

	// 만약 lg_inf(로그 파일 링크드리스트)가 NULL이 아니라면, path가 lg_inf의 노드 중 하나와 일치한다면, file_inf의 origin_path를 lg_inf의 origin_path(원본 경로)로 설정한다.
	if (lg_inf != NULL)
	{
		log_inf *temp_inf = lg_inf;
		while (temp_inf != NULL)
		{
			if (strcmp(temp_inf->file_path, path) == 0)
			{
				strcpy(newnode->origin_path, temp_inf->origin_path);
				break ;
			}
			temp_inf = temp_inf->next;
		}
		if (temp_inf == NULL)
			error(strjoin("DEBUG : backup file doesn't have log", path));
	}
	
	// 기본 valid는 1로 설정
	newnode->valid = 1;
	newnode->next = NULL;
	close(fd);
	return newnode;
}

/*
void check_duplicate(file_inf *inf, file_inf *last_inf)
입력 :	file_inf *inf : 검사 할 파일 정보 링크드리스트
	file_inf *last_inf : inf의 마지막 노드
리턴 :	void
기능 :	파일 정보 링크드리스트의 valid값을 갱신한다. 기존 링크드리스트에 last_inf와 정보가 일치하는 노드가 있으면 valid를 0으로 설정(가장 최근에 백업한 파일이 아니라는 뜻)한다. make_list 함수에서 사용된다.
*/
void check_duplicate(file_inf *inf, file_inf *last_inf)
{
	file_inf *temp_inf = inf;
	while (temp_inf != last_inf)
	{
		if (strcmp(temp_inf->origin_path, last_inf->origin_path) == 0 && strcmp(temp_inf->hash, last_inf->hash) == 0)
			temp_inf->valid = 0;
		temp_inf = temp_inf->next;
	}
}

/*
void check_duplicate_log(log_inf *inf, log_inf *last_inf)
입력 :	log_inf *inf : 로그 파일 링크드리스트
	lof_inf *last_inf : inf의 마지막 노드
리턴 :	void
기능 :	기존 링크드리스트에 last_inf와 정보가 일치하는 노드가 있으면 last_inf를 free하고 이전 상태로 되놀려놓는다. log_inf를 만드는 함수인 make_loglist에서 사용된다.
*/
void check_duplicate_log(log_inf *inf, log_inf *last_inf)
{
	log_inf *temp_inf = inf;
	while (temp_inf != last_inf)
	{
		if (strcmp(temp_inf->origin_path, last_inf->origin_path) == 0 && strcmp(temp_inf->file_path, last_inf->file_path) == 0)
		{
			while (temp_inf->next != last_inf)
				temp_inf = temp_inf->next;
			free(last_inf);
			temp_inf->next = NULL;
			last_inf = temp_inf;
		}
		temp_inf = temp_inf->next;
	}
}

/*
void	makelist(file_inf **inf, log_inf *lg_inf, char *dirPath, int *opt_res)
입력 :	file_inf **inf : 생성된 링크드리스트가 저장될 주소
	log_inf *lg_inf: 로그 파일 정보가 들어있는 링크드리스트, NULL시 링크드리스트의 노드들에 origin_path는 비워진 채로 생성된다.
	char *dirPath : 리스트로 만들 디렉토리 경로
	int *opt_res : 명령어 함수에서 추출된 옵션 배열, NULL 시 서브 디렉토리까지 탐색
출력 :	void
기능 :	dirPath 안에 있는 파일들과 서브 디렉토리를 재귀적으로 탐색(BFS)하여 각 파일들의 정보가 담긴 file_inf 링크드리스트를 생성한다.
	opt_res가 NULL이 아니라면 OPT_R(-r 옵션이 존재)이 있다면 서브 디렉토리까지 탐색하고, 없다면 입력한 디렉토리만 탐색한다.
	생성된 링크드리스트는 inf 주소에 저장된다. make_linkedlist함수 에서 사용된다.
*/
void	makelist(file_inf **inf, log_inf *lg_inf, char *dirPath, int *opt_res)
{
	struct dirent **list;
    	struct stat fileStat;
	file_inf *temp_inf = *inf;
	int count;
	
	// 디렉토리를 scandir에 실패 시 에러처리 한다. scandir에 성공하면 알파벳 순서로 list에 저장된다.
	if ((count = scandir(dirPath, &list, NULL, alphasort)) == -1)
		error(strjoin("fail to open directory : ", dirPath));
	while (temp_inf != NULL && temp_inf->next != NULL)
		temp_inf = temp_inf->next;
	
	//파일 먼저 탐색하여 BFS를 보장한다.
	for (int i = 0; i < count; ++i){
        	if (list[i]->d_type != DT_DIR && strcmp(list[i]->d_name, "ssubak.log") != 0) {
            	char fullPath[PATH_MAX + 2];
            	snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, list[i]->d_name);
			if (strlen(fullPath) == 4097)
				error("too long filepath");

			if (*inf == NULL)
			{
				*inf = make_struct(fullPath, 0, lg_inf);
				temp_inf = *inf;
			}
			else
			{
				temp_inf->next = make_struct(fullPath, 0, lg_inf);
				temp_inf = temp_inf->next;
				if (lg_inf != NULL)
					check_duplicate(*inf, temp_inf);
			}
			free(list[i]);
        	}
    	}
	//파일 탐색이 끝나면 서브 디렉토리를 탐색한다.
    	for (int i = 0; i < count; ++i){
		while (temp_inf != NULL && temp_inf->next != NULL)
			temp_inf = temp_inf->next;
		if (list[i]->d_type == DT_DIR && strcmp(list[i]->d_name, ".") != 0 && strcmp(list[i]->d_name, "..") != 0)
		{
			char fullPath[PATH_MAX];
			snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, list[i]->d_name);

			// 서브 디렉토리 탐색을 위해 make_list를 재귀적으로 호출한다.
			if (opt_res == NULL || (opt_res != NULL && opt_res[OPT_R]))
				makelist(inf, lg_inf, fullPath, opt_res);
			free(list[i]);
        	}
	}
	free(list);
}

/*
void make_linkedlist(file_inf **inf, log_inf *lg_inf, char *file_path, int *opt_res)
입력 :	fine_inf **inf : 생성될 링크드리스트 or file_inf 노드가 저장될 주소
	log_inf *lg_inf : 로그 파일 정보가 들어있는 링크드리스트, NULL시 링크드리스트의 노드들에 origin_path는 비워진 채로 생성된다.
	char *file_path : 리스트로 만들 파일 or 디렉토리 경로
	int *opt_res : 명령어 함수에서 추출된 옵션 배열, NULL 시 서브 디렉토리까지 탐색
리턴 :	void
기능 :	file_path가 파일인지 디렉토리인지 검사하고, 옵션과 file_path가 올바른지 검사한다. 문제 없으면 각 상황에 맞게 make_list나 make_struct를 호출하여
	생성된 링크드 리스트나 노드를 inf 주소에 연결한다.
*/
void make_linkedlist(file_inf **inf, log_inf *lg_inf, char *file_path, int *opt_res)
{
	struct stat filestat;
	
	if (stat(file_path, &filestat) == -1)
		error(strjoin("fail to open stat ", file_path));
	if (opt_res != NULL)
	{
		if (S_ISREG(filestat.st_mode) && (opt_res[OPT_D] || opt_res[OPT_R]))
				error("please input directory path when you use option -d or -r");
		if (S_ISDIR(filestat.st_mode) && (!opt_res[OPT_D] && !opt_res[OPT_R]))
				error("please input option -d or -r when you use directory path");
	}
	if (S_ISREG(filestat.st_mode))
		*inf = make_struct(file_path, time(NULL), lg_inf);
	else if (S_ISDIR(filestat.st_mode))
		makelist(inf, lg_inf, file_path, opt_res);
	else
		error("is neither a regular file nor a directory.\n");
}

/*
void make_loglist(log_inf **inf)
입력 :	log_inf **inf : 생성될 로그파일 링크드리스트를 저장할 주소
출력 :	void
기능 :	/home/backup/ssubak.log 파일의 정보를 취합하여 링크드리스트를 생성한다. 로그 파일에서 backuped to 가 적힌 문장만 분석해서 노드를 생성하고 나머지는 무시된다.
	중복된 파일의 노드는 제거하여 후에 backup 링크드리스트를 생성할 때 사용될 수 있게끔 가공한다. 생성돤 링크드리스트는 inf 주소에 연결된다.
*/
void make_loglist(log_inf **inf)
{
	int	fd;
	char	ch;
	int	i = 0;
	int	first = 1;
	char	middle[15];
	log_inf *temp_inf = *inf;
	log_inf *before_inf = temp_inf;

	if ((fd = open("/home/backup/ssubak.log", O_RDONLY)) == -1)
			error("fail to open /home/backup/ssubak.log");
	
	//로그 파일을 모두 읽을 때까지 반복해서 읽어들인다.
	while (read(fd, &ch, 1) > 0)
	{
		if (ch == '\"') {
			i = 0;
			if (first)
			{
				*inf = (log_inf *)malloc(sizeof(log_inf));
				if (*inf == NULL)
					error("fail to malloc");
				temp_inf = *inf;
				before_inf = temp_inf;
				first = 0;
			}
			else if (temp_inf != NULL)
			{
				temp_inf->next = (log_inf *)malloc(sizeof(log_inf));
				if (temp_inf->next == NULL)
					error("fail to malloc");
				temp_inf = temp_inf->next;
				if (before_inf->next != temp_inf)
					before_inf = before_inf->next;
			}
			
			// 첫 번째 토큰인 파일 경로를 노드의 origin_path에 저장한다.
			if(read(fd, &ch, 1) == -1)
				error("fail to read");
			(temp_inf->origin_path)[i++] = ch;
			while (1)
			{
				if(read(fd, &ch, 1) == -1)
					error("fail to read");
				if (ch == '\"')
					break ;
				(temp_inf->origin_path)[i++] = ch;
			}
			(temp_inf->origin_path)[i] = '\0';

			// 중앙 토큰이 backuped to면 읽어들여 노드의 file_path에 저장하고 아니면 무시한다.
			i = 0;
			read(fd, middle, 14);
			middle[14] = 0;
			if (strcmp(middle, " backuped to \"") == 0)
			{
				if(read(fd, &ch, 1) == -1)
					error("fail to read");
				(temp_inf->file_path)[i++] = ch;
				while (1)
				{
					if(read(fd, &ch, 1) == -1)
						error("fail to read");
					if (ch == '\"')
						break ;
					(temp_inf->file_path)[i++] = ch;
				}
				(temp_inf->file_path)[i] = '\0';
				//중복된 파일의 노드는 제거한다.
				check_duplicate_log(*inf, temp_inf);
			}
			else
			{
				//할당된 노드는 해제하고 현재 포인터를 이전으로 되돌려 다음 노드 할당을 준비한다.
				if (temp_inf != *inf)
				{
					free(temp_inf);
					temp_inf = before_inf;
					before_inf = *inf;
					while (before_inf->next != temp_inf)
						before_inf = before_inf->next;
					temp_inf->next = NULL;
				}
				else
				{
					free(temp_inf);
					first = 1;
				}
				while (1)
				{
					if(read(fd, &ch, 1) == -1)
						error("fail to read");
					if (ch == '\n')
						break ;
				}
			}
		}
	}
	if (temp_inf != NULL)
		temp_inf->next = NULL;
}
