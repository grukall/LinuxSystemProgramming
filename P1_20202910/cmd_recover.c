#include "ssu_header.h"

void check_opt_recover(int argc, char *argv[], int *opt_res);
file_inf *check_backuplist_recover(char *file_path, file_inf *backup_inf);
void file_recover(file_inf *backup_inf, file_inf *inf, char *file_path, char *new_path);
void recover_file(file_inf *inf, file_inf *backup_inf, int *opt_res, char *file_path, char *new_path);
int recover_directory_origin(char *file_path, char *new_path, file_inf *backuped_inf, file_inf *backup_inf, file_inf *inf, int *opt_res);
void recover_directory(char *file_path, char *new_path, file_inf *backup_inf, file_inf *inf, int *opt_res);

/*
void cmd_recover(file_inf *backup_inf, int argc, char *argv[])
입력 :	file_inf *backup_inf : 백업 파일 링크드리스트
	int argc, char *argv : main 함수 인자 그대로 입력
리턴 :	void
기능 :	ssu_backup의 기능 중 recover를 수행한다. 입력된 옵션을 분류하여 저장하고 입력한 파일의 경로를 확인한다.
	확인된 파일경로를 사용하여 옵션의 종류에 따라 recover_directory나 recover_file을 호출하여 파일 복구를 진행한다.
	-n 옵션이 있을 경우, 입력한 경로에 디렉토리를 생성한 다음 복구를 진행한다.
*/
void cmd_recover(file_inf *backup_inf, int argc, char *argv[])
{
	int opt;
	int opt_res[4] = {0,};
	char *backup_path;
	char *file_path;
	char *new_path = NULL;
	file_inf *inf = NULL;
	
	//getopt에 의해 옮겨지는 argv의 파일경로를 읽어들이기 위해 if_else문으로 구분지어 코드를 짯다.
	check_opt_recover(argc, argv, opt_res);
	if (opt_res[OPT_R] || opt_res[OPT_D] || opt_res[OPT_L] || opt_res[OPT_N])
	{
		file_path = check_path_N(argv[3], "recover");
		if (opt_res[OPT_N])
		{
			new_path = check_path_N(argv[4], "recover");
			if (mkdir(new_path, 0777) == -1)
				error(strjoin("fail to mkdir ", new_path));
		}
	}
	else
		file_path = check_path_N(argv[2], "recover");
	
	//입력한 경로에 파일이 존재하염 파일 내용을 백업 파일들과 비교하기 위해 make_linkedlist를 호출하여 file_inf 링크드리스트로 만든다.
	if (access(file_path, F_OK) != -1)
		make_linkedlist(&inf, NULL, file_path, opt_res);

	// -d나 -r 옵션이 있으면 recover_directory를, 없으면 recover_file을 호출한다.
	if (opt_res[OPT_D] || opt_res[OPT_R])
		recover_directory(file_path, new_path, backup_inf, inf, opt_res);
	else
	{
		// 원본 경로에 파일이 있는 경우 파일 내용 비교를 위해 inf를 입력한다.
		if (inf != NULL)
			recover_file(inf, backup_inf, opt_res, inf->file_path, new_path);
		else
			recover_file(inf, backup_inf, opt_res, file_path, new_path); 
	}
 	check_directory_isEmpty();
	exit(0);
}

/*
void check_opt_recover(int argc, char *argv[], int *opt_res)
입력 :	int argc, char *argv : main 함수 인자 그대로 입력
	int *opt_res : 옵션 유무를 저장할 int 배열
리턴 :	void
기능 :	getopt함수를 사용하여 recover 명령어에서 사용가능한 옵션의 입력 유무를 opt_res에 저장한다.
	잘못된 옵션 사용은 에러 처리한다.
*/
void check_opt_recover(int argc, char *argv[], int *opt_res)
{
	int opt;
	while ((opt = getopt(argc, argv, "drln")) != -1) { //-drln옵션을 가지는지 검사
		switch (opt) {
			case 'd':
				if (!opt_res[OPT_D])
					opt_res[OPT_D] = 1;
				else
				{
					help("recover");
					error("backup> not valid option");
				}
				break ;
			case 'r':
				if (!opt_res[OPT_R])
					opt_res[OPT_R] = 1;
				else
				{
					help("recover");
					error("backup> not valid option");
				}
				break ;
			case 'l':
				if (!opt_res[OPT_L])
					opt_res[OPT_L] = 1;
				else
				{
					help("recover");
					error("backup> not valid option");
				}
				break ;
			case 'n':
				if (!opt_res[OPT_N])
					opt_res[OPT_N] = 1;
				else
				{
					help("recover");
					error("backup> not valid option");
				}
				break ;
			default:
				help("recover");
				error("backup> not valid option");
		}
	}
}

/*
file_inf *check_backuplist_remcover(char *file_path, file_inf *backup_inf)
입력 :	char *file_path : 백업 파일 유무를 알고싶은 파일의 경로
	file_inf *backup_inf : 백업 파일 링크드리스트
리턴 :	file_path를 origin_path(원본 경로)로 갖고있거나 file_path가 파일 경로인 backup_inf의 노드가 있다면
	해당하는 노드들만 모아서 링크드리스트로 만든 후 리턴한다.
	해당하는 노드가 없다면 NULL를 리턴한다.
*/
file_inf *check_backuplist_recover(char *file_path, file_inf *backup_inf)
{
	file_inf *temp_inf = backup_inf;
	file_inf *res_inf = NULL;
	file_inf *temp_inf2 = NULL;
	int first = 1;

	while (temp_inf != NULL)
	{
		if (strcmp(temp_inf->origin_path, file_path) == 0 || strcmp(temp_inf->file_path, file_path) == 0)
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
int file_recover(file_inf *backup_inf, file_inf *inf, char *file_path, char *new_path)
입력 :	file_inf *backup_inf : 백업 파일 링크드리스트
	file_inf *inf : 복구하고 싶은 파일의 file_inf 노드, 파일의 원본 경로에 파일이 존재하지 않으면 NULL이 들어온다.
	char *file_path : 복구하고 싶은 파일의 원본 경로
	char *new_path : -n 옵션이 있을 때, 새로운 복구 파일로 지정될 파일 경로, -n옵션이 없다면 NULL로 들어온다.
리턴 :	void
기능 :	recover_file에서 호출될 함수. file_path나 new_path중 하나를 복구 경로로 지정하고 해당 파일을 open한다. 옵션은 O_CREAT와 O_TRUNC를 사용하여
	파일이 없으면 생성하고, 파일이 있으면 내용을 덮어쓴다.
	만약 원본파일이 존재하고, 원본파일과 백업할 파일의 내용이 동일하면, not changed with를 출력하고 함수를 종료한다.
	복구 성공 시, 사용한 백업 파일은 삭제한다.
*/
void file_recover(file_inf *backup_inf, file_inf *inf, char *file_path, char *new_path)
{
	int		backuped_fd;
	int		des_fd;
	FILE	*log_fd;
	int		file_size;
	int		read_size;
	char	buff[BUFF_SIZE];
	char	*des_path;

	time_t timer;
	timer = time(NULL);
	char *backup_time = make_backup_time(timer);
	
	// new_path가 있으면 복구 경로를 new_path로 지정하고, 없으면 원본 파일 경로를 복구 경로로 지정한다.
	if (new_path != NULL)
		des_path = new_path;
	else
		des_path = file_path;

	backuped_fd = open(backup_inf->file_path, O_RDONLY);
	if (backuped_fd == -1)
		error(strjoin("recover> fail to open ", backup_inf->file_path));
	
	// new_path(새로운 경로)가 있거나, 원본 파일이 있을 때 백업파일과 파일 내용이 다른 경우, 복구를 진행한다.
	if (new_path != NULL || inf == NULL || new_path == NULL && inf != NULL && (strcmp(backup_inf->hash, inf->hash) != 0))
	{
		des_fd = open(des_path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
		if (des_fd == -1)
			error(strjoin("recover> fail to open ", des_path));
	
		while ((read_size = read(backuped_fd, buff, BUFF_SIZE)) > 0)
		{
			if (write(des_fd, buff, read_size) == -1)
				error(strjoin("recover> fail to write", des_path));
			if (read_size < BUFF_SIZE)
				break ;
		}
		printf("\"%s\" recovered to \"%s\"\n", backup_inf->file_path, des_path);
		log_fd = fopen("/home/backup/ssubak.log", "a");
		fseek(log_fd, 0, SEEK_END);
		fprintf(log_fd, "%s:\"%s\" recovered to \"%s\"\n", backup_time, des_path, backup_inf->file_path);
		fclose(log_fd);

		//복구 성공 시 사용한 백업 파일은 삭제한다.
		if (remove(backup_inf->file_path) == -1)
			error(strjoin("fail to remove ", backup_inf->file_path));
		close(des_fd);
	}
	else
		printf("\"%s\" not changed with \"%s\"\n", des_path, backup_inf->file_path);
	close(backuped_fd);
}

/*
void recover_file(file_inf *inf, file_inf *backup_inf, int *opt_res, char *file_path, char *new_path)
입력 :	file_inf *inf : 복구하고 싶은 파일의 file_inf 노드, 원본 경로에 파일이 없으면 NULL로 들어온다.
	file_inf *backup_inf : 백업 파일 링크드리스트
	int *opt_res : 옵션 유무를 저장한 int 배열
	char *file_path : 복구 파일이 저장될 파일 경로
	char *new_path : -n 옵션이 있을 때, 새로 지정될 복구 파일 경로
리턴 :	void
기능 :	입력받은 file_path(복구할 파일 경로)의 백업 파일이 있는지 check_backuplist_recover로 확인하고 백업파일들의 링크드리스트를 받아(backuped_inf)
	입력받은 옵션에 따라 복구를 진행한다.
	-l 옵션이 있을 시 backuped_inf 중 가장 최신 버전 백업 파일로 복구한다.
*/
void recover_file(file_inf *inf, file_inf *backup_inf, int *opt_res, char *file_path, char *new_path)
{
	int		file_fd;
	FILE	*log_fd;
	file_inf *backuped_inf;
	file_inf *temp_inf;
	char	buff[BUFF_SIZE];
	int row;
	
	// file_path의 백업 파일을 링크드리스트로 backuped_inf에 저장
	if ((backuped_inf = check_backuplist_recover(file_path, backup_inf)) != NULL)
	{
		temp_inf = backuped_inf;

		// 만약 백업 파일이 하나라면 file_recover를 호출
		if (backuped_inf->next == NULL)
		{
			if (opt_res[OPT_N])
				file_recover(backuped_inf, inf, file_path, new_path);
			else
				file_recover(backuped_inf, inf, file_path, NULL);
		}
		else
		{
			int i = 1;
			// -l옵션이 없으면 백업 파일 목록을 출력 후 숫자를 입력받아 해당하는 파일로 복구를 진행
			if (opt_res[OPT_L] == 0)
			{
				printf("backup files of %s\n", file_path);
				printf("0. exit\n");
				while (temp_inf != NULL)
				{
					printf("%d. %s %sbytes\n", i++, temp_inf->backup_time, get_fsize(temp_inf->file_path));
					temp_inf = temp_inf->next;	
				}
				write(1, ">>", 2);
				scanf("%d", &row);
				printf("row : %d\n", row);
				getchar();
				if (row >= i || row < 0)
					error("not valid row number");
				if (row != 0)
				{
					while (row > 1)
					{
						backuped_inf = backuped_inf->next;
						row--;
					}
					if (opt_res[OPT_N])
						file_recover(backuped_inf, inf, file_path, new_path);
					else
						file_recover(backuped_inf, inf, file_path, NULL);
				}
			}
			else
			{
				//backuped_inf의 file끼리 비교해서 가장 최근 파일을 recover
				file_inf *biggest_inf;
				while (temp_inf != NULL)
				{
					if (biggest_inf == NULL || strcmp(biggest_inf->backup_time, temp_inf->backup_time) < 0)
						biggest_inf = temp_inf;
					temp_inf = temp_inf->next;
				}
				if (biggest_inf != NULL)
				{
					if (opt_res[OPT_N])
						file_recover(backuped_inf, inf, file_path, new_path);
					else
						file_recover(backuped_inf, inf, file_path, NULL);
				}
			}
		}
	}
	else
		printf("%s\n", strjoin("no backup file ", file_path));
}

/*
int recover_directory_origin(char *file_path, char *new_path, file_inf *backuped_inf, file_inf *backup_inf, file_inf *inf, int *opt_res)
입력 :	char *file_path : 복구 파일이 저장될 파일 경로
	char *new_path : -n 옵션이 있을 때, 새로 지정될 복구 파일 경로
	file_inf *backuped_inf : check_backuplist_rmdir로 생성된 file_path를 포함하는 백업 파일 링크드리스트
	file_inf *backup_inf : 백업 파일 링크드리스트
	file_inf *inf : 복구하고 싶은 파일의 file_inf 링크드리스트, 원본 경로에 파일이 없으면 NULL로 들어온다.
	inf *opt_res : 옵션 유무가 저장된 int 배열
리턴 :	디렉토리 탐색 중, backuped_inf의 원본 파일 경로가 현재 탐색중인 디렉토리에 속하지 않으면 0을 리턴하고, 아니면 1를 리턴한다.
	함수 내부적으로만 사용된다.
기능 :	디렉토리 단위의 복구를 진행한다. backuped_inf(복구하고자 하는 파일의 경로를 포함하는 백업파일들로 구성된 링크드리스트)의 원본 파일 경로 안에
	재귀적으로 디렉토리를 생성하며 복구를 진행한다. 만약 -n 옵션이 있다면, new_path 안에 복구가 진행된다.
	원본 파일 경로에 파일이 있다면 해당 파일의 정보가 담긴 inf를 갖고 file_recover를 호출한다. 없으면 NULL로 호출한다.
*/
int recover_directory_origin(char *file_path, char *new_path, file_inf *backuped_inf, file_inf *backup_inf, file_inf *inf, int *opt_res)
{
	file_inf *temp_inf = backuped_inf;
	char *ptr;
	char *ptr2;
	
    	while (temp_inf != NULL) {
		ptr = temp_inf->origin_path;
		int i = 0;
		if (strstr(temp_inf->origin_path, file_path) == NULL)  //이 디렉토리가 아니면 이전으로 돌아간다.
			return 0;
		while (*ptr == file_path[i])
		{
			ptr++;
			i++;
		}
		ptr2 = strrchr(ptr, '/');
		//origin_path와 file_path가 같은 지점까지 이동 후, origin_path의 strchr과 strrchr이 다르면 새로운 디렉토리
		if (strchr(ptr, '/') != ptr2)
		{
			//경로의 마지막 '/'부분의 뒷부분을 추출
			i = 0;
			char d_name[PATH_MAX];
			while (++ptr != ptr2)
				d_name[i++] = *ptr;
			d_name[i] = '\0';

           		char fullPath[PATH_MAX];
			char newPath[PATH_MAX];
			// -n 옵션이 있으면 new_path를 새로운 디렉토리로, 없으면 file_path를 새로운 디렉토리로 지정
			if (opt_res[OPT_N])
			{
            			snprintf(newPath, sizeof(newPath), "%s/%s", new_path, d_name);
				if ((access(newPath, F_OK) == -1) && (mkdir(newPath, 0777) == -1))
					error(strjoin("fail to mkdir ", newPath));
            			snprintf(fullPath, sizeof(fullPath), "%s/%s", file_path, d_name);
				if(recover_directory_origin(fullPath, newPath, temp_inf, backup_inf, inf, opt_res))
					break;
			}
			else
			{
         		   	snprintf(fullPath, sizeof(fullPath), "%s/%s", file_path, d_name);
				if ((access(fullPath, F_OK) == -1) && (mkdir(fullPath, 0777) == -1))
					error(strjoin("fail to mkdir ", fullPath));
				if(recover_directory_origin(fullPath, newPath, temp_inf, backup_inf, inf, opt_res))
					break;
			}
		}
		else
		{
			//경로의 마지막 '/'부분의 뒷부분을 추출
			i = 0;
			char d_name[PATH_MAX];
			while (*(++ptr) != '\0')
				d_name[i++] = *ptr;
			d_name[i] = '\0';

			// 파일이면 backuped_inf의 origin_path 중 inf에 해당하는 내용이 있으면 해당 inf로 recover진행
			file_inf *temp_inf2 = inf;
            		char newPath[PATH_MAX];
			if (opt_res[OPT_N])
            			snprintf(newPath, sizeof(newPath), "%s/%s", new_path, d_name);
			while (temp_inf2 != NULL)
			{
				if (strcmp(temp_inf2->file_path, temp_inf->origin_path) == 0)
				{
					recover_file(temp_inf2, backup_inf, opt_res, temp_inf2->file_path, newPath);
					break ;
				}
				temp_inf2 = temp_inf2->next;
			}
			if (temp_inf2 == NULL)
				recover_file(NULL, backup_inf, opt_res, temp_inf->origin_path, newPath);
			temp_inf = temp_inf->next;
		}
	}
	
	return 1;
}

/*
void recover_directory(char *file_path, char *new_path, file_inf *backup_inf, file_inf *inf, int *opt_res)
입력 :	char *file_path : 복구 파일이 저장될 파일 경로
	char *new_path : -n 옵션이 있을 때, 새로 지정될 복구 파일 경로
	file_inf *backup_inf : 백업 파일 링크드리스트
	file_inf *inf : 복구하고 싶은 파일의 file_inf 링크드리스트, 원본 경로에 파일이 없으면 NULL로 들어온다.
	inf *opt_res : 옵션 유무가 저장된 int 배열
리턴 :	void
기능 :	file_path 하위의 백업 파일 목록을 check_backuplist_rmdir를 사용해 링크드리스트로 받는다.
	file_path에 디렉토리가 존재하지 않으면 디렉토리를 생성하고 recover_directory_origin을 호출한다.
	check_backuplist_rmdir이 NULL를 리턴하면 해당하는 백업 디렉토리가 없다고 출력한다.
*/
void recover_directory(char *file_path, char *new_path, file_inf *backup_inf, file_inf *inf, int *opt_res)
{
	file_inf *backuped_inf;

		if ((backuped_inf = check_backuplist_rmdir(file_path, backup_inf, opt_res)) != NULL)
		{
			if (new_path == NULL && access(file_path, F_OK) == -1 && mkdir(file_path, 0777) == -1)
				error(strjoin("fail to mkdir ", file_path));
			recover_directory_origin(file_path, new_path, backuped_inf, backup_inf, inf, opt_res);
		}
		else
			printf("%s\n",strjoin("no backup directory ", file_path));
}
