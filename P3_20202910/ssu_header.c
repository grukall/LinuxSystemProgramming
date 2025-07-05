#include "ssu_header.h"

dirNode* backup_dir_list;
dirNode *version_dir_list;

char *exe_path;
char *pwd_path;
char *home_path;


/*
char *substr(char *str, int beg, int end)
입력 : char *str : 나눌 문자열
      int beg : 리턴할 문자열의 첫 번째 index
      int end : 리턴할 문자열의 마지막 index
리턴 : str를 beg 부터 end까지 잘라서 리턴
기능 : 문자열을 str를 beg 부터 end까지 잘라서 리턴한다.
*/
char *substr(char *str, int beg, int end) {
  char *ret = (char*)malloc(sizeof(char) * (end-beg+1));

  for(int i = beg; i < end && *(str+i) != '\0'; i++) {
    ret[i-beg] = str[i];
  }
  ret[end-beg] = '\0';

  return ret;
}

char *c_str(char *str) {
  return substr(str, 0, strlen(str));
}



int path_list_init(pathNode *curr_path, char *path) {
  pathNode *new_path = curr_path;
  char *ptr;
  char *next_path = "";

  if(!strcmp(path, "")) return 0;

  if(ptr = strchr(path, '/')) {
    next_path = ptr+1;
    ptr[0] = '\0';
  }

  if(!strcmp(path, "..")) {
    new_path = curr_path->prev_path;
    new_path->tail_path = new_path;
    new_path->next_path = NULL;
    new_path->head_path->tail_path = new_path;

    new_path->head_path->depth--;

    if(new_path->head_path->depth == 0) return -1;
  } else if(strcmp(path, ".")) {
    new_path = (pathNode*)malloc(sizeof(pathNode));
    strcpy(new_path->path_name, path);

    new_path->head_path = curr_path->head_path;
    new_path->tail_path = new_path;

    new_path->prev_path = curr_path;
    new_path->next_path = curr_path->next_path;
    
    curr_path->next_path = new_path;
    new_path->head_path->tail_path = new_path;

    new_path->head_path->depth++;
  }

  if(strcmp(next_path, "")) {
    return path_list_init(new_path, next_path);
  }

  return 0;
}

/*
char *cvt_path_2_realpath(char* path)
입력 : char *path : 절대경로로 바꿀 경로
리턴 : path의 절대경로
기능 : path를 절대경로로 바꾼 문자열을 리턴한다.
*/
char *cvt_path_2_realpath(char* path) {

  pathNode *path_head;
  pathNode *curr_path;
  char *ptr;
  char origin_path[PATHMAX];
  char ret_path[PATHMAX] = "";

  if (path == NULL)
    return NULL;

  path_head = (pathNode*)malloc(sizeof(pathNode));
  path_head->depth = 0;
  path_head->tail_path = path_head;
  path_head->head_path = path_head;
  path_head->next_path = NULL;

  if(path[0] != '/') {
    sprintf(origin_path, "%s/%s", pwd_path, path);
  } else {
    strcpy(origin_path, path);
  }

  if(path_list_init(path_head, origin_path) == -1) {
    return NULL;
  }

  curr_path = path_head->next_path;
  while(curr_path != NULL) {
    strcat(ret_path, curr_path->path_name);
    if(curr_path->next_path != NULL) {
      strcat(ret_path, "/");
    }
    curr_path = curr_path->next_path;
  }

  if(strlen(ret_path) == 0) {
    strcpy(ret_path, "/");
  }
  
  return c_str(ret_path);
}

/*
void get_argument(int *argc, char *argv[], char *str)
입력 : int *argc, char *argv[] : 결과를 저장할 인자
       char *str : 사용자가 입력한 문잘열
리턴 : void
기능 : str 문자열을 토큰화 한 후, exe 함수에 전달할 argc와 argv에 값을 할당한다.
*/
void get_argument(int *argc, char *argv[], char *str) {
  int last_idx = 0;
  int i;

  for(i = 0; i <= strlen(str); i++) {
    if(str[i] == ' ' || str[i] == '\t' || str[i] == '\0') {
      if(last_idx == i) {
        last_idx = i+1;
        continue;
      }
      argv[*argc] = substr(str, last_idx, i);
      last_idx = i+1;
      (*argc)++;
    }
  }
  argv[*argc] = (char*)0;
}

/*
char *make_pid_status(char *mode, char *origin_path, char *original_path, char *backupFilePath, char *backupDir)
입력: char *mode - 작업 모드
      char *origin_path - 원본 파일 경로
      char *original_path - 기준 파일 경로
      char *backupFilePath - 백업 파일 경로
      char *backupDir - 백업 디렉토리 경로
리턴: 작업 결과 문자열
기능: 작업 모드, 원본 파일 경로 등을 이용하여 작업 결과 문자열을 생성합니다.
*/
char *make_pid_status(char *mode, char *origin_path, char *original_path, char *backupFilePath, char *backupDir) {
  char result[PATHMAX + 100];
  
  struct stat st;
  if (strcmp(mode, "remove") != 0 && stat(origin_path, &st) == -1) {
    fprintf(stderr, "stat error : %s\n", origin_path);
    exit(1);
  }

  struct tm *pt;
  if (strcmp(mode, "delete") == 0) {
    time_t cur_time = time(NULL);
    pt = localtime(&cur_time);
  }
  else
    pt = localtime(&st.st_mtime);

  sprintf(result, "[%d-%02d-%02d %02d:%02d:%02d][%s][%s]", pt->tm_year + 1900, pt->tm_mon, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec, mode, origin_path);

  //backupFilePath
  if (backupFilePath != NULL) {
    int i = 0;
    char *ptr;
    if (!strcmp(origin_path, original_path))
      ptr = strrchr(origin_path, '/') + 1;
    else {
      while (origin_path[i] == original_path[i]) i++;
      ptr = &origin_path[i]+1;
    }
    char tempPath[PATHMAX];
    strcpy(backupFilePath, backupDir);
    sprintf(tempPath, "/%s_%d%02d%02d%02d%02d%02d", ptr, pt->tm_year + 1900, pt->tm_mon, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec);
    strncat(backupFilePath, tempPath, strlen(tempPath));
  }

  return strdup(result);
}

/*
void backup_file(char *backupFIlePath, char *file_path, struct stat st)
입력: char *backupFIlePath - 백업 파일 경로
      char *file_path - 원본 파일 경로
      struct stat st - 파일의 상태 정보
리턴: 없음
기능: 원본 파일을 백업 파일로 복사하고, 파일의 접근 및 수정 시간을 변경합니다.
*/
void backup_file(char *backupFIlePath, char *file_path, struct stat st) {

  int backup_fd;
  int origin_fd;

  if ((origin_fd = open(file_path, O_RDONLY)) == -1) {
    fprintf(stderr, "fail to open %s : %s\n", file_path, strerror(errno));
    exit(1);
  }

  if ((backup_fd = open(backupFIlePath, O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1) {
    fprintf(stderr, "fail to open %s : %s\n", backupFIlePath, strerror(errno));
    exit(1);
  }

  char buf[BUFMAX];
  int len;
  while ((len = read(origin_fd, buf, BUFMAX)) > 0) {
    write(backup_fd, buf, len);
  }

  struct utimbuf time_buf;
  time_buf.actime = st.st_atime;
  time_buf.modtime = st.st_mtime;

  if (utime(file_path, &time_buf) < 0) {
    fprintf(stderr, "utime error for %s\n", file_path);
    exit(1);
  }

  close(backup_fd);
  close(origin_fd);
}

/*
void make_backupfile(char *backupDir, char *backupFilePath, char *file_path, char *path, char *cur_path, struct stat st)
입력: char *backupDir - 백업 디렉토리 경로
      char *backupFilePath - 백업 파일 경로
      char *file_path - 원본 파일 경로
      char *path - 경로 정보
      char *cur_path - 현재 경로 정보
      struct stat st - 파일의 상태 정보
리턴: 없음
기능: 백업 파일을 생성하거나 디렉토리를 생성하여 파일을 백업합니다.
*/
void make_backupfile(char *backupDir, char *backupFilePath, char *file_path, char *path, char *cur_path, struct stat st) {
  int count;
  char *ptr;

  //printf("backupDir : %s, backupFilePath : %s, file_path : %s, path : %s\n", backupDir, backupFilePath, file_path, path);
   // '/'가 있으면 디렉토리, 없으면 파일로 해석
  if(path != NULL && (ptr = strchr(path, '/')) != NULL) {
    char* cur_dir = substr(cur_path, 0, strlen(cur_path) - strlen(ptr));

    char dir[PATHMAX];
    sprintf(dir, "%s/%s", backupDir, cur_dir);
    if (access(dir, F_OK) == -1) {
      if (mkdir(dir, 0755) == -1) {
        fprintf(stderr, "fail mkdir %s : %s\n", dir, strerror(errno));
        exit(1);
      }
      //printf("mkdir %s\n", dir);
    }
    make_backupfile(backupDir, backupFilePath, file_path, ptr+1, cur_path, st);
  } else {
    //printf("make backupfile : %s\n", backupFilePath);
    backup_file(backupFilePath, file_path, st);
  }

}

/*
void remove_pidlog(char *backup_path)
입력: char *backup_path - 백업 경로
리턴: 없음
기능: 백업된 로그 파일을 삭제합니다.
*/
void remove_pidlog(char *backup_path) {
  struct dirent **list;
  struct stat st;
  int count;

  if ((count = scandir(backup_path, &list, NULL, alphasort)) == -1) {
    fprintf(stderr, "fail to scandir : %s\n", backup_path);
    exit(1);
  }

  for (int i = 0; i < count; ++i) {
    if (list[i]->d_type == DT_DIR && strcmp(list[i]->d_name, ".") != 0 && strcmp(list[i]->d_name, "..") != 0) {
      char logPath[PATHMAX + 2];
      sprintf(logPath, "%s/%s", backup_path, list[i]->d_name);
      //printf("logpath : %s\n", logPath);
      remove_pidlog(logPath);
      if (remove(logPath) == -1) {
        //fprintf(stderr, "fail remove %s : %s\n", logPath, strerror(errno));
      }
      //printf("remove dir : %s\n", logPath);
    }
  }

  for (int i = 0; i < count; ++i) {
    if (list[i]->d_type != DT_DIR) {
      char logPath[PATHMAX + 2];
      sprintf(logPath, "%s/%s", backup_path, list[i]->d_name);
      //printf("logpath : %s\n", logPath);
      if (remove(logPath) == -1) {
       //fprintf(stderr, "fail remove %s : %s\n", logPath, strerror(errno));
      }
      //printf("remove file : %s\n", logPath);
    }
  }

  if (remove(backup_path) == -1) {
    //fprintf(stderr, "fail remove %s : %s\n", backup_path, strerror(errno));
  }
}

/*
int	ft_check_size(int n)
입력: int n - 정수
리턴: 정수 n의 자릿수
기능: 정수 n의 자릿수를 계산하여 반환합니다.
*/
int	ft_check_size(int n)
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
char	*ft_itoa(int n)
입력: int n - 정수
리턴: 정수 n을 문자열로 변환한 결과
기능: 정수 n을 문자열로 변환하여 반환합니다.
*/
char	*ft_itoa(int n)
{
	char	*temp;
	int		n_len;

	n_len = ft_check_size(n);
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
int md5(char *target_path, char *hash_result)
입력: char *target_path - 대상 파일 경로
      char *hash_result - MD5 해시 결과를 저장할 버퍼
리턴: 정수 (0: 성공, 1: 실패)
기능: 대상 파일의 MD5 해시 값을 계산하여 결과를 hash_result에 저장합니다.
*/
int md5(char *target_path, char *hash_result) {
	FILE *fp;
	unsigned char hash[MD5_DIGEST_LENGTH];
	unsigned char buffer[SHRT_MAX];
	int bytes = 0;
	MD5_CTX md5;

	if ((fp = fopen(target_path, "rb")) == NULL){
		printf("ERROR: fopen error for %s\n", target_path);
		return 1;
	}

	MD5_Init(&md5);

	while ((bytes = fread(buffer, 1, SHRT_MAX, fp)) != 0)
		MD5_Update(&md5, buffer, bytes);
	
	MD5_Final(hash, &md5);

	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
		sprintf(hash_result + (i * 2), "%02x", hash[i]);
	hash_result[HASH_MD5-1] = 0;

	fclose(fp);

	return 0;
}

/*
char *cvtHash(char *target_path)
입력: char *target_path - 대상 파일 경로
리턴: 대상 파일의 MD5 해시 값을 문자열로 반환
기능: 대상 파일의 MD5 해시 값을 계산하고 이를 문자열로 반환합니다.
*/
char *cvtHash(char *target_path) {
  char *hash = (char *)malloc(sizeof(char) * HASH_MD5);
  md5(target_path, hash);
  return hash;
}

/*
int cmpHash(char *path1, char *path2)
입력: char *path1 - 비교할 파일 경로 1
char *path2 - 비교할 파일 경로 2
리턴: 정수 (0: 두 파일의 MD5 해시 값이 동일함, 그 외: 해시 값이 다름)
기능: 두 파일의 MD5 해시 값을 비교하여 동일한지 확인합니다.
*/
int cmpHash(char *path1, char *path2) {
return strcmp(cvtHash(path1), cvtHash(path2));
}

/*
char *make_strBackupTime(char *backup_time)
입력: char *backup_time - 백업 시간 문자열 (YYYYMMDDhhmmss)
리턴: 백업 시간을 변환한 문자열
기능: 백업 시간을 받아서 문자열로 변환하여 반환합니다.
*/
char *make_strBackupTime(char *backup_time) {
  char ch;
  struct tm t;
  char result[NAMEMAX];
  sscanf(backup_time, "%d%c%02d%c%02d%c%2d%c%02d%c%02d", &t.tm_year, &ch, &t.tm_mon, &ch, &t.tm_mday, &ch, &t.tm_hour, &ch, &t.tm_min, &ch, &t.tm_sec);
  sprintf(result, "%d%02d%02d%02d%02d%02d", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
  return strdup(result);
}