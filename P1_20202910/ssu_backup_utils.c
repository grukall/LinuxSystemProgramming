#include "ssu_header.h"

/*
char	*strjoin(char const *s1, char const *s2)
입력 :	char const *s1, char const *s2 : 하나로 합칠 문지열 s1, s2.
리턴 :	s1과 s2의 합친 내용이 들어있는 문자열을 반환
기능 :	문자열 s1과 s2의 내용을 읽어들여 하나의 할당된 문자열에 넣어 리턴해준다.
*/
char	*strjoin(char const *s1, char const *s2)
{
	char	*result;
	int		size_s1;
	int		size_s2;
	int		i;

	i = 0;
	size_s1 = strlen(s1);
	size_s2 = strlen(s2);
	result = (char *)malloc(size_s1 + size_s2 + 1);
	if (result == 0)
		return (NULL);
	while (i < size_s1)
	{
		result[i] = s1[i];
		i++;
	}
	while (i < size_s1 + size_s2)
	{
		result[i] = s2[i - size_s1];
		i++;
	}
	result[i] = '\0';
	return (result);
}

/*
int	check_size(int n)
입력 :	int n : 자릿수를 일고 싶은 수
리턴 :	n의 자릿 수
기능 :	n의 자릿 수를 측정하여 리턴한다. itoa의 문자열 할당에 사용된다.
*/
int	check_size(int n)
{
	int	result;

	result = 0;
	if (n == 0)
		return (1);
	if (n < 0)
		result++;
	while (n)
	{
		n = n / 10;
		result++;
	}
	return (result);
}

/*
char	*itoa(int n)
입력 :	int n : 문자열로 변환하고 싶은 숫자
리턴 :	n의 문자열 버전을 리턴
기능 :	n의 문자열 버전을 리턴한다.
*/
char	*itoa(int n)
{
	char	*temp;
	int		n_len;

	n_len = check_size(n);
	temp = (char *)malloc(n_len + 1);
	if (temp == NULL)
		return (NULL);
	temp[n_len] = '\0';
	if (n < 0)
		temp[0] = '-';
	if (n == 0)
	{
		temp[0] = '0';
		return (temp);
	}
	while (n)
	{
		if (n > 0)
			temp[n_len - 1] = n % 10 + 48;
		else
			temp[n_len - 1] = n % 10 * -1 + 48;
		n = n / 10;
		n_len--;
	}
	return (temp);
}

/*
void error(char *explain_error)
입력 :	char *explain_error : 에러에 대한 설명
리턴 :	void
기능 :	explain_error를 stderr를 통해 출력하고 errno가 설정되어 있으면 해당 errno 내용을 출력한 후 exit(1)로 프로세스를 종료한다.
*/
void error(char *explain_error)
{
	if (errno)
		fprintf(stderr, "%s : %s\n", explain_error, strerror(errno));
	else
		fprintf(stderr, "%s\n", explain_error);
	exit(1);
}

/*
int check_backup(char *backup_path)
입력 :	char *backup_path : 백업 디렉토리 경로
리턴 :	디렉토리가 비어있으면 해당 디렉토리를 삭제하고 0을 리턴한다. 비어있지 않으면 1를 리턴한다.
*/
int check_backup(char *backup_path)
{
	DIR *dir;
	struct dirent *entry;
	struct stat   fileStat;
	
    if ((dir = opendir(backup_path)) == NULL)
		error(strjoin("fail to open directory : ", backup_path));

    while ((entry = readdir(dir)) != NULL) {
        // "."와 ".."는 제외
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
	{
		char fullPath[PATH_MAX];
		snprintf(fullPath, sizeof(fullPath), "%s/%s", backup_path, entry->d_name);

		if (stat(fullPath, &fileStat) == -1)
			error("stat error");

		// 디렉토리인 경우 재귀적으로 탐색
 		if (S_ISDIR(fileStat.st_mode)) {
			if(!check_backup(fullPath))
				return 0;
			}
			else
				return 0;
		}
	}

	if (rmdir(backup_path) < 0)
		error("check code");
	return 1;
}

/*
void check_directory_isEmpty(void)
입력 :	void
리턴 :	void
기능 :	/home/backup 디렉토리에 들어있는 모든 디렉토리를 check_backup으로 검사함으로써 비워진 디렉토리들을 삭제한다.
*/
void check_directory_isEmpty(void)
{
	
	DIR *dir;
	struct dirent *entry;
	struct stat fileStat;
	
    if ((dir = opendir("/home/backup")) == NULL)
		error("fail to open directory : /home/backup");

    while ((entry = readdir(dir)) != NULL) {
        // "."와 ".."는 제외
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
		{
            char fullPath[PATH_MAX + 2];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", "/home/backup", entry->d_name);
			if (strlen(fullPath) == 4097)
				error("too long filepath");

            
			if (stat(fullPath, &fileStat) == -1)
				error("stat error");

            // 디렉토리인 경우 재귀적으로 탐색
            if (S_ISDIR(fileStat.st_mode))
			{
				check_backup(fullPath);
			}
		}
	}
}

/*
int comparePaths(const char *path1, const char *path2)
입력 :	const char *patah1, const char *path2 : 디렉토리 깊이를 비교하고싶은 두 경로
리턴 :	path1과 path2의 깊이 차이만큼 리턴, path1이 더 깊으면 양수, path2가 더 깊으면 음수 리
*/
int comparePaths(const char *path1, const char *path2) {
    int depth1 = 0, depth2 = 0;
    const char *p = path1;
    while (*p) {
        if (*p == '/') {
            depth1++;
        }
        p++;
    }
    p = path2;
    while (*p) {
        if (*p == '/') {
            depth2++;
        }
        p++;
    }
    if (depth1 != depth2) {
        return depth1 - depth2;
    }
    return strcmp(path1, path2);
}

/*
file_inf *check_backuplist_rmdir(char *file_path, file_inf *backup_inf, int *opt_res)
입력 :	char *file_path : 백업 파일을 확인하고 싶은 파일 경로
	file_inf *backup_inf : 백업 파일 링크드리스트
	int opt_res : 옵션 유무가 있는 int배열
리턴 :	file_inf 링크드리스트
기능 :	file_path 경로를 포함한(file_path 하위 파일) origin_path를 가지고 있는 backup_inf의 노드들을 모아서 링크드리스트로
	만들고 리턴한다. -r 옵션이 있으면 서브 디렉토리까지 탐색하고 -d 옵션이면 file_path만 탐색한다.
	추출된 노드들은 comparePath 함수를 거쳐 디렉토리 깊이가 낮은 순서대로 정렬된다.
*/
file_inf *check_backuplist_rmdir(char *file_path, file_inf *backup_inf, int *opt_res)
{
	file_inf *temp_inf = backup_inf;
	file_inf *res_inf = NULL;
	file_inf *temp_inf2 = NULL;
	file_inf *temp_inf3 = NULL;
	file_inf *temp_inf4 = NULL;
	int first = 1;
	
	while (temp_inf != NULL)
	{
		//valid가 1(최신 백업 파일)이고, file_path 하위 경로를 가진 노드일 경우 추출한다. 이때 노드의 origin_path의 처음부터 file_path와 같아야 한다.
		if (temp_inf->valid == 1 && strstr(temp_inf->origin_path, file_path) == temp_inf->origin_path)
		{
			int i = 0;
			//디렉토리 or 파일의 이름을 알아내기 위해 중복 부분을 건너뛴다.
			while (temp_inf->origin_path[i] == file_path[i])
				i++;

			//-r 옵션이거나 -d 이면서 origin_path가 file_path 바로 아래에 있는 경로라면 노드를 링크드리스트에 추가
			if (opt_res[OPT_R] || (opt_res[OPT_D] && strchr(&temp_inf->origin_path[i], '/') == strrchr(&temp_inf->origin_path[i], '/')))
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
					temp_inf3 = res_inf;
					while (temp_inf3->next != NULL)
					{
						temp_inf4 = temp_inf3->next;
						if (comparePaths(temp_inf3->file_path, temp_inf4->file_path) > 0)
						{
							file_inf temp_inf5;
							infcpy(&temp_inf5, temp_inf3);
							infcpy(temp_inf3, temp_inf4);
							infcpy(temp_inf4, &temp_inf5);
						}
						// 만약 중복된 origin_path를 가진 노드가 있다면 할당을 해제하고 포인터를 이전으로 되돌림
						if (strcmp(temp_inf2->origin_path, temp_inf3->origin_path) == 0)
						{
							free(temp_inf2);
							temp_inf2 = res_inf;
							while (temp_inf2->next != NULL)
								temp_inf2 = temp_inf2->next;
						}
						if (temp_inf3->next != NULL)
							temp_inf3 = temp_inf3->next;
					}
				}
			}
		}
		temp_inf = temp_inf->next;
	}
	if (temp_inf2 != NULL)
		temp_inf2->next = NULL;

	return res_inf;
}

/*
int is_realpath(char *path)
입력 :	char *path : 검사할 파일 경로
리턴 :	절대 경로면 1을 리턴, 아니면 0을 리턴
기능 :	파일 경로의 첫 번째 문자를 검사하여 절대경로면 1을 리턴, 아니면 0을 리턴한다.
*/
int is_realpath(char *path)
{
	if (path[0] == '/' || path[0] == '\\')
		return 1;
	return 0;
}

/*
char*	make_path(char *path)
입력 :	char *path : 검사할 파일 경로
출력 :	절대경로로 변환된 파일 경로가 들어있는 char 문자열
기능 :	path가 절대경로인지 상대경로인지 판단하고, 절대경로이면 경로의 권한을 검사해가며 경로 생성
	상대경로이면 절대경로로 변환하며 경로 생성 후, 해당 경로를 반환한다.
*/
char *make_path(char *path)
{
	char *token;
	char buffer[PATH_MAX];

	// 먼저 절대경로인지 상대경로인지 판단
	if (is_realpath(path))
	{
		// 절대경로인 경우 / 부터 안쪽으로 존재하는지 판단
		token = strtok(path, "/");
		snprintf(buffer, sizeof(buffer), "/%s", token);
		while (1)
		{
			if (access(buffer, F_OK) == -1)
			{
				if (mkdir(buffer, 0777) == -1)
					error(strjoin("fail mkdir ", buffer));
			}
			else if (access(buffer, W_OK | X_OK) == -1)
			{
				if (chmod(buffer, 0777) == -1)
					error(strjoin("fail chmod ", buffer));
			}
			token = strtok(NULL, "/");
			if (token == NULL)
				break ;
			strcat(buffer, "/");
			strcat(buffer, token);
		}
	}
	else
	{
		if (getcwd(buffer, PATH_MAX) == NULL) {
			error(strjoin("fail to getcwd in ", path));
		}

		token = strtok(path, "/");
		while (token != NULL)
		{
			if (strcmp(token, "..") == 0) {
				char *ptr = strrchr(buffer, '/');
				if (ptr != NULL)
					*ptr = '\0';
			}
			else if (strcmp(token, ".") != 0) {
				strcat(buffer, "/");
				strcat(buffer, token);
			}
			token = strtok(NULL, "/");
		}
	}

	return strdup(buffer);
	// 절대 경로 반환
}

/*
char *check_path_N(char *path, char *command)
입력 :	char *path : 검사할 파일 경로
	char *command : check_path_N을 호출한 명령어 함수 이름
리턴 :	path의 절대경로를 리턴
기능 :	path의 파일 존재 유무를 판단하고 없으면 절대경로로 변환하고 하위 디렉토리들을 포함해 디렉토리를 생성하고 리턴한다.
*/
char *check_path_N(char *path, char *command)
{
	char *tmp_path;

	if (path == NULL)
	{
		help(command);
		error("please enter the path");
	}
	tmp_path = (char *)malloc(sizeof(char) * PATH_MAX);
	if (!tmp_path)
		error("fail to malloc");
	if (realpath(path, tmp_path) == NULL)
	{
		if (errno && errno != ENOENT)
			error(strjoin("fail to realpath", path));
		errno = 0;
		free(tmp_path);
		return make_path(path);
	}
	if (*tmp_path == '\0')
	{
		help(command);
		error("please enter the path");
	}
	return tmp_path;
}

/*
char *make_backup_time(time_t timer)
입력 :	time_t timer : 문자열로 생성할 시간
리턴 :	timer를 YYMMDDHHMMSS 형식으로 변환한 char 문자열 리턴
기능 :	localtime 함수를 이용해 timer를 YYMMDDHHMMSS 형식의 문자열로 변환 후 리턴한다.
*/
char *make_backup_time(time_t timer)
{
	struct tm *t;
	char *temp_filename = (char *)malloc(sizeof(char) * 13);
	int  num_filename[6];

	t = localtime(&timer);
	num_filename[0] = t->tm_year % 100;
	num_filename[1] = t->tm_mon + 1;
	num_filename[2] = t->tm_mday;
	num_filename[3] = t->tm_hour;
	num_filename[4] = t->tm_min;
	num_filename[5] = t->tm_sec;
	
	char *temp;
	for (int i = 0; i <= 5; i++)
	{
		temp_filename[i * 2] = num_filename[i] / 10 + 48;
		temp_filename[i * 2 + 1] = num_filename[i] % 10 + 48;
	}
	temp_filename[12] = '\0';
	return temp_filename;
}

/*
int length_size(int size)
입력 :	int size : 길이를 알고싶은 숫자
리턴 :	숫자의 길이 리턴
기능 :	get_fsize에서 문자열을 할당할 길이를 리턴한다. 3자리수마다 ,를 붙여야하므로 길이가 1씩 추가된다.
*/
int length_size(int size)
{
	int length = 0;
	int count = 0;
	while (size)
	{
		size = size / 10;
		length++;
		count++;
		if (count % 3 == 0)
		{
			length++;
			count = 0;
		}
	}
	return length;
}

/*
char *get_fsize(char *file_path)
입력 :	char *file_path : 파일 크기를 알고 싶은 파일 경로
리턴 :	지정된 형식의 file_path의 크기가 담긴 char 문자열을 리턴
기능 :	파일의 크기를 측정하고 측정된 값이 3자리가 넘으면, 넘을 때마다 문자열에 ','를 넣는다.
	완성된 문자열은 리턴한다.
*/
char *get_fsize(char *file_path)
{
	int fd;
	int size;
	int length;
	int count = 0;
	char fsize[30];

	if ((fd = open(file_path, O_RDONLY)) == -1)
		error("fail to open");
	else
	{
		if ((size = lseek(fd, 0, SEEK_END)) == -1)
			error("fail to lseek");
		length = length_size(size);
		if (length == 0)
			return strdup("0");
		fsize[length] = '\0';
		while (length)
		{
			fsize[length - 1] = size % 10 + 48;
			size = size / 10;
			length--;
			count++;
			if (count % 3 == 0)
			{
				fsize[length - 1] = ',';
				count = 0;
				length--;
			}
		}
		return strdup(fsize);
	}
	close(fd);
}

/*
void infcpy(file_inf *res_inf, file_inf *src_inf)
입력 :	file_inf *res_inf : 복사한 file_inf 구조체가 저장될 포인터
	file_inf *src_inf : 복사하고 싶은 file_inf 구조체
리턴 :	void
기능 :	src_inf의 내용을 des_inf로 복사한다.
*/
void infcpy(file_inf *res_inf, file_inf *src_inf)
{
	char *debug;

	if (src_inf->origin_path != NULL)
		debug = strcpy(res_inf->origin_path, src_inf->origin_path);
	else
		res_inf->origin_path[0] = '\0';
	if (src_inf->file_path != NULL)
		debug = strcpy(res_inf->file_path, src_inf->file_path);
	else
		res_inf->file_path[0] = '\0';
	if (src_inf->backup_time != NULL)
		debug = strcpy(res_inf->backup_time, src_inf->backup_time);
	else
		res_inf->backup_time[0] = '\0';
	if (src_inf->hash != NULL)
		debug = strcpy(res_inf->hash, src_inf->hash);
	else
		res_inf->hash[0] = '\0';
	if (debug == NULL)
		error("fail to strcpy");
	res_inf->valid = src_inf->valid;
}
