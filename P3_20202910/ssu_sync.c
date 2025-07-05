#include "ssu_header.h"

int ssu_daemon_init(void);
void sigusr_handler(int signo);

char origin_path[PATHMAX];
char logpath[PATHMAX];
char backupDir[PATHMAX];
int monitor = 1;
int logfd;

/*
int file_monitoring(fileNode *file, char *file_path)
입력 : fileNode *file : 업데이트 할 파일 노드
      char *file_path : 검사할 파일 경로
리턴 : 파일이 존재하지 않을 시 1을 리턴한다.
기능 : 추적하고 있는 파일의 변경상태를 확인한다. 상태가 기존과 다르면 변경된 상태를 로그파일에 쓰고,
       파일 노드의 정보를 업데이트 한다.
*/
int file_monitoring(fileNode *file, char *file_path) {
    struct stat st;
    int logfd = open(logpath, O_RDWR | O_CREAT | O_APPEND, 0777);
    if (logfd == -1) {
        perror("Failed to open log file");
        exit(1);
    }

    if (access(file_path, F_OK) == -1) {
        //printf("%s is deleted\n", file_path);
        char *filelog = make_pid_status("remove", file_path, NULL, NULL, NULL);
        write(logfd, filelog, strlen(filelog));
        write(logfd, "\n", 1);
        close(logfd);
        return 1;
    }

    if (stat(file_path, &st) == -1) {
        perror("stat error");
        close(logfd);
        exit(1);
    }

    char *new_md5 = cvtHash(file_path);
    if (file->mtime != st.st_mtime && strcmp(file->file_md5, new_md5) != 0) {
        // printf("%s modified, before mtime : %ld, new mtime : %ld\n", file_path, file->mtime, st.st_mtime);
        // printf("%s modified, before md5 : %s, new md5 : %s\n", file_path, file->file_md5, new_md5);

        char backupFilePath[PATHMAX];
        char *filelog = make_pid_status("modify", file_path,origin_path, backupFilePath, backupDir);
        int i = 0;
        if (!strcmp(file_path, origin_path))
          make_backupfile(backupDir, backupFilePath, file_path, NULL, NULL, st);
        else {
          while (file_path[i] == origin_path[i]) i++;
          make_backupfile(backupDir, backupFilePath, file_path, &file_path[i]+1,  &file_path[i]+1, st);
        }
        
        write(logfd, filelog, strlen(filelog));
        write(logfd, "\n", 1);
        file->mtime = st.st_mtime;
        free(file->file_md5);
        file->file_md5 = new_md5;
        close(logfd);
        return 0;
    }
    free(new_md5);
    close(logfd);
    return 0;
}

/*
int iterate_directory_monitoring(dirNode *list)
입력 : dirNode *list : 검사할 디렉토리 노드 리스트
리턴 : 모든 파일과 서브디렉토리가 삭제되었을 시 1을 리턴한다.
기능 : 디렉토리 내부의 모든 파일 및 서브 디렉토리를 순회하며 모니터링 한다. 삭제된 파일이나 서브 디렉토리를
       제거하고, 상태를 갱신한다.
*/
int iterate_directory_monitoring(dirNode *list) {
  if (list->file_cnt) {
    fileNode *cur_file = list->file_head->next_file;
    while (cur_file) {
      if (file_monitoring(cur_file, cur_file->file_path) == 1) {
        fileNode *temp = cur_file->next_file;
        fileNode_remove(cur_file);
        list->file_cnt -= 1;
        cur_file = temp;
      }
      else
        cur_file = cur_file->next_file;
    }
  }

  if (list->subdir_cnt) {
    dirNode *cur_dir = list->subdir_head->next_dir;
    while (cur_dir) {
      if (iterate_directory_monitoring(cur_dir) == 1) {
        dirNode *temp = cur_dir->next_dir;
        dirNode_remove(cur_dir);
        list->subdir_cnt -= 1;
        cur_dir = temp;
      }
      else
        cur_dir = cur_dir->next_dir;
    }
  }
  if (list->file_cnt == 0 && list->subdir_cnt == 0)
    return 1;
  return 0;
}

/*
int file_create(fileNode *file, char *file_path)
입력 : fileNode *file : 생성할 파일 노드
      char *file_path : 생성할 파일 경로
리턴 : 항상 1을 리턴한다.
기능 : 새로운 파일을 생성하고, 해당 파일의 초기 상태를 로그파일에 기록한다. 파일의 메타데이터와 MD5 해시 값을
       파일 노드에 저장한다.
*/
int file_create(fileNode *file, char *file_path) {
  char backupFilePath[PATHMAX];

  logfd = open(logpath, O_RDWR | O_APPEND);
  if (!logfd)
    exit(1);
  char *filelog = make_pid_status("create", file_path, origin_path, backupFilePath, backupDir);
  write(logfd, filelog, strlen(filelog));
  write(logfd, "\n", 1);
  close(logfd);

  struct stat st;
  if (stat(file_path, &st) == -1) {
    fprintf(stderr,"stat error %s\n", file_path);
    exit(1);
  }
  
  int i = 0;
  if (!strcmp(file_path, origin_path))
    make_backupfile(backupDir, backupFilePath, file_path, NULL, NULL, st);
  else {
    while (file_path[i] == origin_path[i]) i++;
    make_backupfile(backupDir, backupFilePath, file_path, &file_path[i]+1, &file_path[i]+1, st);
  }
  // 파일 초기 mtime과 md5 가져오기
  //printf("st.st_mtime : %ld\n", st.st_mtime);
  file->mtime = st.st_mtime;
  file->file_md5 = cvtHash(file_path);
  return 1;
}

/*
void iterate_directory_create(char *dirpath, int opt)
입력 : char *dirpath : 생성할 디렉토리 경로
      int opt : 옵션 플래그
기능 : 지정된 디렉토리와 그 하위의 모든 파일 및 서브 디렉토리를 탐색하여 파일 노드를 생성하고 초기 상태를 로그파일에
       기록한다. 디렉토리의 모든 내용을 백업한다.
*/
void iterate_directory_create(char *dirpath, int opt) {
  struct dirent **list;
  struct stat fileStat;
  int count;
  
  // 파일경로와 옵션에 맞는 
  //pid.log에 create log 넣기

  // 디렉토리를 scandir에 실패 시 에러처리 한다. scandir에 성공하면 알파벳 순서로 list에 저장된다.
	if ((count = scandir(dirpath, &list, NULL, alphasort)) == -1) {
    fprintf(stderr, "fail to scandir : %s\n", dirpath);
    exit(1);
  }
  
  //파일 우선 탐색
	for (int i = 0; i < count; ++i){
    char newPath[PATHMAX + 2];
    snprintf(newPath, sizeof(newPath), "%s/%s", dirpath, list[i]->d_name);

    if (list[i]->d_type != DT_DIR && find_fileNode(backup_dir_list, newPath) == NULL) {
      fileNode *filenode = (fileNode *)malloc(sizeof(fileNode));
      fileNode_init(filenode);
      file_create(filenode, newPath);
      backup_list_insert(backup_dir_list, newPath, newPath, filenode);
    }
  }

  //파일 탐색이 끝나면 서브 디렉토리를 탐색한다.
  for (int i = 0; i < count; ++i) {
    if (list[i]->d_type == DT_DIR && strcmp(list[i]->d_name, ".") != 0 && strcmp(list[i]->d_name, "..") != 0) {
      char fullPath[PATHMAX];
      snprintf(fullPath, sizeof(fullPath), "%s/%s", dirpath, list[i]->d_name);
      // 서브 디렉토리 탐색을 위해 iterate_directory_create를 재귀적으로 호출한다.
      if (opt & OPT_R)
        iterate_directory_create(fullPath, opt);
      free(list[i]);
    }
	}
}

/*
int daemon_process(int argc, char *argv[])
입력 : int argc : 인자 개수
      char *argv[] : 인자 리스트
리턴 : 항상 0을 리턴한다.
기능 : 데몬 프로세스를 초기화하고, 주어진 경로와 옵션에 따라 파일 및 디렉토리를 모니터링한다. 모니터링 중 변경된 상태를
      로그파일에 기록하며 백업을 수행한다.
*/
int daemon_process(int argc, char *argv[]) {
  pid_t pid;
  int opt;
  int period = 1;

  strcpy(origin_path, argv[0]);
  int i = 1;
  while (argv[i]) {
    if (!strcmp(argv[i], "-r"))
      opt |= OPT_R;
    else if (!strcmp(argv[i], "-d"))
      opt |= OPT_D;
    else if (!strcmp(argv[i], "-t")) {
      opt |= OPT_T;
      period = atoi(argv[++i]);
      i++;
    }
    i += 1;
  }

  pid = getpid();

  if (ssu_daemon_init() < 0) {
      fprintf(stderr, "ssu_daemon_init failed\n");
      exit(1);
  }

  //여기서부터 구현
  sprintf(logpath, "%s/%d.log", backup_path, pid);
  sprintf(backupDir, "%s/%d", backup_path, pid);

  //pid.log 생성
  logfd = open(logpath, O_RDWR | O_CREAT | O_APPEND, 0777);
  if (!logfd)
    exit(1);
  close(logfd);

  //백업용 디렉토리 생성
  if (mkdir(backupDir, 0755) == -1) {
    fprintf(stderr, "fail mkdir %s\n", backupDir);
    exit(1);
  } 
  
  struct stat st;
  if (stat(origin_path, &st) == -1) {
    exit(1);
  }

  //디렉토리용 backup_list
  backup_dir_list = (dirNode*)malloc(sizeof(dirNode));
  dirNode_init(backup_dir_list);
  //파일용 fileNode
  fileNode *file_node = NULL;

  close(logfd);

  struct sigaction sig;
  sigset_t set;
  sigemptyset(&set);
  sig.sa_handler = sigusr_handler;
  sig.sa_mask = set;
  sig.sa_flags = 0;
  sigaction(SIGUSR1, &sig, NULL);
  while (monitor) {
    if (S_ISREG(st.st_mode)) {
      if (!access(origin_path, F_OK) && file_node == NULL) {
        file_node = (fileNode *)malloc(sizeof(fileNode));
        file_create(file_node, origin_path);
      }
      else if (file_node != NULL) {
        if (file_monitoring(file_node, origin_path)) {
          if (file_node->file_md5 != NULL)
            free(file_node->file_md5);
          file_node = NULL;
        }
      }
    }
    else {
      //dir_create 계속 호출해서 추가된 디렉토리나 파일 추적해야함
      iterate_directory_create(origin_path, opt);
      iterate_directory_monitoring(backup_dir_list);
    }
    sleep(period);
  }
  exit(0);
}

/*
void sigusr_handler(int signo)
입력 : int signo : 시그널 번호
기능 : SIGUSR1 신호를 받으면 반복문의 조건을 false로 바꾼다.
*/
void sigusr_handler(int signo) {
  monitor = 0;
}

/*
int ssu_daemon_init(void)
리턴 : 데몬 프로세스 초기화가 성공하면 0, 실패하면 -1을 리턴한다.
기능 : 데몬 프로세스를 초기화한다. 시그널 핸들러를 설정하고, 프로세스 그룹을 생성한다.
*/
int ssu_daemon_init(void) {
    pid_t pid;
    int fd, maxfd;

    //1.부모 프로세스인 add는 종료되었으므로 충족

    //2. 프로세스 그룹에서 탈퇴, 새로운 프로세스 그룹을 생성
    pid = getpid();
    //printf("process %d running as daemon\n", pid);
    setsid();

    //3. 불필요한 시그널 무시
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    
    //4. 마스크 초기화
    umask(0);

    //5. 현재 디렉토리를 루트 디렉토리로 설정
    chdir("/");

    //6. 열려있는 fd 모두 닫기
    maxfd = getdtablesize();
    for (fd = 0; fd < maxfd; fd++)
        close(fd);

    // 표준 입출력, 표준 에러를 /dev/null로 바꾸기
    fd = open("/dev/null", O_RDWR);
    dup(0);
    dup(0);

    return 0;
}

/*
  int check_option(int argc, char *argv[], char *path, int cmd_bit, int *period)
  
  입력 : int argc : 인자 개수
        char *argv[] : 인자 리스트
        char *path : 경로
        int cmd_bit : 명령 비트
        int *period : 주기
  리턴 : 옵션 비트
  기능 : 주어진 인자 리스트와 경로를 검사하여 옵션을 설정하고 옵션 비트를 반환한다.
 */
int check_option(int argc, char *argv[], char *path, int cmd_bit, int *period) {
  int i;
  int opt_bit = 0;
  struct stat statbuf;

  int is_backuped_file = 0;
  int is_backuped_dir = 0;

  for(i = 0; i < argc; i++) {
    if(!strcmp(argv[i], "-d") && (cmd_bit & CMD_ADD)) opt_bit |= OPT_D;
    else if(!strcmp(argv[i], "-r") && (cmd_bit & CMD_ADD)) opt_bit |= OPT_R;
    else if(!strcmp(argv[i], "-t") && (cmd_bit & CMD_ADD)) {
      if(i+1 == argc) {
        fprintf(stderr, "ERROR: missing operand -- '%s'\n", argv[i]);
        fprintf(stderr, "ERROR: '%s' option needs <PERIOD>\n", argv[i]);
        return -1;
      }

      *period = atoi(argv[i+1]);
      opt_bit |= OPT_T;
      i++;
    }
    else {
      fprintf(stderr, "ERROR: invalid option -- '%s'\n", argv[i]);
      help(cmd_bit);
      return -1;
    }
  }

  if(cmd_bit & CMD_ADD && stat(path, &statbuf) == -1) {
    fprintf(stderr, "ERROR: stat error for '%s'\n", path);
    return -1;
  }

  if(opt_bit & (OPT_D | OPT_R)) {
    if(cmd_bit & CMD_ADD && S_ISREG(statbuf.st_mode)) {
      fprintf(stderr, "ERROR: '%s' is file.\n\n", path);
      return -1;
    }
  } else {
    if(cmd_bit & CMD_ADD && S_ISDIR(statbuf.st_mode)) {
      fprintf(stderr, "ERROR: '%s' is directory.\n\n", path);
      return -1;
    }
  }

  return opt_bit;
}

/*
  int check_path_access(char* path, int opt_bit)
  
  입력 : char *path : 경로
        int opt_bit : 옵션 비트
  리턴 : 성공 여부 (0 : 성공, -1 : 실패)
  기능 : 주어진 경로와 옵션을 검사하여 접근 가능 여부를 반환한다.
 */
int   check_path_access(char* path, int opt_bit) {
  int i;
  int cnt;
  char *origin_path = (char*)malloc(sizeof(char)*(strlen(path)+1));
  char* ptr;
  int depth = 0;
  
  strcpy(origin_path, path);
  if (((opt_bit & OPT_R) && !strcmp(path, getenv("HOME"))) || strstr(path, backup_path)) {
    fprintf(stderr, "ERROR: '%s' include backup directory\n", path);
    return -1;
  }

  while(ptr = strchr(origin_path, '/')) {
    char *tmp_path = substr(origin_path, 0, strlen(origin_path) - strlen(ptr));

    depth++;
    origin_path = ptr+1;

    //printf("%s\n", substr(path, 0, strlen(path) - strlen(origin_path)));
    if(depth == 2 && strcmp(substr(path, 0, strlen(path) - strlen(origin_path)), "/home/")) {
      fprintf(stderr, "ERROR: path must be in user directory\n");
      fprintf(stderr, " - '%s' is not in user directory\n", path);
      return -1;
    }
  }

  if(depth <= 2 && strcmp(getenv("HOME"), path)){
    fprintf(stderr, "ERROR: path must be in user directory\n");
    fprintf(stderr, " - '%s' is not in user directory\n", path);
    return -1;
  }

  return 0;
}

/*
  int check_already_backuped(char* path)
  
  입력 : char *path : 경로
  리턴 : 백업 여부 (0 : 백업되지 않음, 1 : 백업됨)
  기능 : 주어진 경로가 이미 백업되었는지 확인하고 백업 여부를 반환한다.
 */
int check_already_backuped(char* path) {

    int monitor_list_fd;

    if ((monitor_list_fd = open(backup_log_path, O_RDONLY)) == -1) {
      fprintf(stderr, "open error : %s\n", backup_log_path);
      exit(1);
    }

    int len;
    char buf[BUFMAX];
    char *origin_path;
    while (len = read(monitor_log_fd, buf, BUFMAX)) {
      // monitoring_fd에서 개행문자를 찾아 그자리에 널문자 넣기, monitor_log_fd는 BUF_MAX만큼 이동
      char *ptr = strchr(buf, '\n');
      ptr[0] = '\0';

      //파일 디스크립터가 ptr로 토큰화한 문장의 다음으로 이동
      lseek(monitor_log_fd, -(len-strlen(buf))+1, SEEK_CUR);

      int index = 0;
      while (buf[index] != ':')
        index++;
      index += 2;
      origin_path = substr(buf, index, strlen(buf));
      if (!strcmp(origin_path, path))
        return 1;
      free(origin_path);
      origin_path = NULL;
    }

  close(monitor_list_fd);
  return 0;
}

/*
  int cmd_add(int argc, char *argv[])
  
  입력 : int argc : 인자 개수
        char *argv[] : 인자 리스트
  리턴 : 성공 여부 (0 : 성공, -1 : 실패)
  기능 : 주어진 경로를 백업 대상으로 추가하고, 해당 경로를 모니터링하기 위한 데몬 프로세스를 생성한다.
 */
int cmd_add(int argc, char *argv[]) {
  int opt_bit;
  int pid = 0;
  int period = 1; // 주기는 디폴트 1초
  char *skip_ptr;
  char *origin_path;
  char buf[BUFMAX];

  if(argc < 1) {
    fprintf(stderr, "ERROR: <PATH> is not include.\n");
    help(CMD_ADD);
    return -1;
  }

  if((origin_path = cvt_path_2_realpath(argv[0])) == NULL) {
    fprintf(stderr, "ERROR: '%s' is wrong path\n", argv[0]);
    return -1;
  }

  int i;
  if (!strcmp(pwd_path, origin_path))
      skip_ptr = NULL;
  else {
    for (i = 0; pwd_path[i] != '\0'; i++) {
      if (pwd_path[i] != origin_path[i]) {
          skip_ptr = &origin_path[i];
          break;
      }
    }
    if (pwd_path[i] == '\0')
      skip_ptr = &origin_path[i + 1];
  }

  if(access(origin_path, F_OK)) {
    fprintf(stderr, "ERROR: <PATH> is not exist.\n");
    return -1;
  }

  if((opt_bit = check_option(argc-1, argv+1, origin_path, CMD_ADD, &period)) == -1) {
    return -1;
  }

   if(check_path_access(origin_path, opt_bit)) {
    return -1;
  }
  
  /// 이미 추적중인 파일이면 이미 추적중인 파일이라고 출력
  if (check_already_backuped(origin_path)) {
      if (skip_ptr == NULL)
          fprintf(stderr, "\".\" already exist in monitor_list.log\n");
      else
          fprintf(stderr, "\"./%s\" already exist in monitor_list.log\n", skip_ptr); 
      exit(1);
  }

  if((monitor_log_fd = open(backup_log_path, O_RDWR|O_CREAT|O_APPEND, 0777)) == -1) {
    fprintf(stderr, "ERROR: open error for '%s'\n", backup_log_path);
    return -1;
  }

  //데몬 프로세스 생성
  //origin_path 경로와 옵션을 넘겨준다.
  int argc_d;
  char *argv_d[ARGMAX];

  int index = 2;
  argv_d[0] = strdup("daemon");
  argv_d[1] = strdup(origin_path);
  if (opt_bit & OPT_D)
    argv_d[index++] = strdup("-d");
  if (opt_bit & OPT_R)
    argv_d[index++] = strdup("-r");
  if (opt_bit & OPT_T) {
    argv_d[index++] = strdup("-t");
    argv_d[index++] = ft_itoa(period);
  }
  argv_d[index] = '\0';

  if ((pid = fork()) < 0) {
    fprintf(stderr, "ERROR: fork error\n");
    exit(1);
  }
  else if (pid == 0) {
      execv("./ssu_sync", argv_d);
      exit(0);
  }

  sprintf(buf, "%d : %s\n", pid, origin_path);
  write(monitor_log_fd, buf, strlen(buf));
  printf("monitoring started (%s) : %d\n\n", origin_path, pid);

  close(monitor_log_fd);
  exit(0);
}

/*
  int cmd_remove(int argc, char *argv[])
  입력 :  int argc : 인자 개수
         char *argv[] : 인자 리스트
  리턴 : 없음
  기능 : 주어진 PID에 해당하는 모니터링을 종료하고, 관련된 로그를 삭제한다.
 */
int cmd_remove(int argc, char *argv[]) {
  if (argc == 0) {
    fprintf(stderr, "ERROR: <PID> is not include.\n");
    help(CMD_REMOVE);
    return -1;
  }
  int monitor_list_fd;
  int temp_fd;

  if ((monitor_list_fd = open(backup_log_path, O_RDWR)) == -1) {
    fprintf(stderr, "open error : %s\n", backup_log_path);
    exit(1);
  }

  if ((temp_fd = open(backup_log_path, O_RDWR)) == -1) {
    fprintf(stderr, "open error : %s\n", backup_log_path);
    exit(1);
  }

  int len;
  char buf[BUFMAX];
  char *pid;
  int next_offset;
  int cur_offset = 0;
  while (len = read(monitor_list_fd, buf, BUFMAX)) {
    // monitoring_fd에서 개행문자를 찾아 그자리에 널문자 넣기, monitor_log_fd는 BUF_MAX만큼 이동
    char *ptr = strchr(buf, '\n');
    ptr[0] = '\0';

    //파일 디스크립터가 ptr로 토큰화한 문장의 다음으로 이동
    next_offset = lseek(monitor_list_fd, -(len-strlen(buf))+1, SEEK_CUR);

    int find = 0;
    char *remove_path;
    while (buf[find] != ' ')
      find++;
    pid = substr(buf, 0, find);
    remove_path = substr(buf, find+3, strlen(buf));
    if (!strcmp(pid, argv[0])) {

      if (kill(atoi(pid), SIGUSR1) == -1) {
        perror("kill");
        exit(1);
      }

      //monitor_list.log에 해당 줄 삭제한다.
      while (len = read(monitor_list_fd, buf, BUFMAX)) {
        write(temp_fd, buf, len);
        cur_offset = lseek(temp_fd, 0, SEEK_CUR);
      }
      ftruncate(monitor_log_fd, cur_offset);

      //pid.log 삭제 하고
      char pidlog[PATHMAX];
      snprintf(pidlog, PATHMAX, "%s/%s.log", backup_path, pid);
      //printf("pidlog : %s\n", pidlog);
      if(remove(pidlog) == -1) {
        printf("remove error : %s\n", strerror(errno));
        exit(1);
      }
    
      // 백업 파일 목록을 삭제해야 함
      char piddir[PATHMAX];
      snprintf(piddir, PATHMAX, "%s/%s", backup_path, pid);
      //printf("piddir : %s\n", piddir);
      remove_pidlog(piddir);

      close(monitor_list_fd);
      close(temp_fd);

      printf("monitoring ended (%s) : %s\n\n", remove_path, pid);
      exit(0);
    }

    free(pid);
    pid = NULL;
    cur_offset = lseek(temp_fd, next_offset, SEEK_SET);
  }

  fprintf(stderr, "'%s' is not existing in monitor_list.log\n\n", argv[0]);
  close(monitor_list_fd);
  close(temp_fd);
  exit(0);
}

/*
void print_monitorlist(void)
입력 : 없음
리턴 : 없음
기능 : 모니터링 목록을 출력한다.
*/
void print_monitorlist(void) {
  int monitor_list_fd;
  int isempty = 1;

  if ((monitor_list_fd = open(backup_log_path, O_RDONLY)) == -1) {
    fprintf(stderr, "open error : %s\n", backup_log_path);
    exit(1);
  }

  int len;
  char buf[BUFMAX];
  char *origin_path;
  while (len = read(monitor_log_fd, buf, BUFMAX)) {
    // monitoring_fd에서 개행문자를 찾아 그자리에 널문자 넣기, monitor_log_fd는 BUF_MAX만큼 이동
    char *ptr = strchr(buf, '\n');
    ptr[0] = '\0';
    isempty = 0;

    //파일 디스크립터가 ptr로 토큰화한 문장의 다음으로 이동
    lseek(monitor_log_fd, -(len-strlen(buf))+1, SEEK_CUR);
    printf("%s\n", buf);
  }
  if (isempty) {
    fprintf(stderr, "there is no monitoring proccess\n");
  }
  close(monitor_list_fd);
}

/*
char check_monitor_listLog(char pid)
입력 : char* pid : PID 값
리턴 : 모니터링 대상의 경로
기능 : 주어진 PID에 해당하는 모니터링 대상의 경로를 반환한다.
*/
char *check_monitor_listLog(char* pid) {

    int monitor_list_fd;

    if ((monitor_list_fd = open(backup_log_path, O_RDONLY)) == -1) {
      fprintf(stderr, "open error : %s\n", backup_log_path);
      exit(1);
    }

    int len;
    char buf[BUFMAX];
    char *log_pid;
    char *origin;
    while (len = read(monitor_log_fd, buf, BUFMAX)) {
      // monitoring_fd에서 개행문자를 찾아 그자리에 널문자 넣기, monitor_log_fd는 BUF_MAX만큼 이동
      char *ptr = strchr(buf, '\n');
      ptr[0] = '\0';

      //파일 디스크립터가 ptr로 토큰화한 문장의 다음으로 이동
      lseek(monitor_log_fd, -(len-strlen(buf))+1, SEEK_CUR);

      int index = 0;
      while (buf[index] != ':')
        index++;
      log_pid = substr(buf, 0, index - 1);
      origin = substr(buf, index + 2, strlen(buf));
      //printf("origin_path : %s, pid : %s\n", origin_path, pid);
      if (!strcmp(log_pid, pid))
        return origin;
      free(log_pid);
      log_pid = NULL;
      free(origin);
      origin = NULL;
    }
  close(monitor_list_fd);
  return 0;
}

/*
int cmd_list(int argc, char *argv[])
입력 :  int argc : 인자 개수
        char *argv[] : 인자 리스트
리턴 : 없음
기능 : 주어진 PID에 해당하는 모니터링 대상의 백업 리스트를 출력한다.
*/
int cmd_list(int argc, char *argv[]) {
  int pid_fd;
  char pid[7];
  char pid_log[PATHMAX];
  char *origin_path;
  
  if (argc == 0) {
    print_monitorlist();
    exit(0);
  }

  strcpy(pid, argv[0]);

  if ((origin_path = check_monitor_listLog(pid)) == NULL) {
    fprintf(stderr, "<DAEMON_PID> is not exist\n");
    exit(0);
  }

  backup_dir_list = (dirNode *)malloc(sizeof(dirNode));
  dirNode_init(backup_dir_list);

  sprintf(pid_log, "%s/%s.log",backup_path, pid);
  if ((pid_fd = open(pid_log, O_RDONLY)) == -1) {
    fprintf(stderr, "open error %s : %s\n", pid_log, strerror(errno));
    exit(1);
  }
  
  init_backup_list(pid_fd);
  dirNode *dir = find_dirNode(backup_dir_list, origin_path);
  if (dir == NULL) {
    fileNode *file = find_fileNode(backup_dir_list, origin_path);
    if (file == NULL) {
      fprintf(stderr, "DEBUG : something wrong.. there is no %s in stagging area", origin_path);
      exit(1);
    }
    dir = file->root_dir;
    //printf("%s\n", origin_path);
    print_list(dir, 0, 0);
  }
  else {
    printf("%s\n", dir->dir_path);
    print_list(dir, 1, 0);
  }
}

/*
int cmd_exit()
입력 : 없음
리턴 : 없음
기능 : 프로그램을 종료한다.
*/
int cmd_exit() {
  printf("\n");

  exit(0);
}

int exec_proc(int argc, char *argv[]) {
  pid_t pid;
  int status;

  if((pid = fork()) < 0) {
    fprintf(stderr, "ERROR: fork error\n");
    exit(1);
  } else if(pid == 0) {
    execv(exe_path, argv);
    exit(0);
  } else {
    pid = wait(&status);
    if (WIFSIGNALED(status)) {
        printf("abnormal extermination : signal number = %d\n", WTERMSIG(status));
    }
  }

  return 0;
}

int prompt() {
  int idx = 0;
  int depth = 0;
  char input[BUFMAX] = {0, };
  int argc = 0;
  char *argv[ARGMAX];
  
  printf("%s> ", STUID);

  scanf("%[^\n]s ", input);
  while (getchar() != '\n');

  get_argument(&argc, argv, input);

  if(argc == 0) return 0;
  else if(!strcmp(argv[0], "add")
    || !strcmp(argv[0], "remove")
    || !strcmp(argv[0], "list")
    || !strcmp(argv[0], "help")) return exec_proc(argc, argv);
  else if(!strcmp(argv[0], "exit")) return cmd_exit();
  else {
    argc = 0;
    get_argument(&argc, argv, "help");
    return exec_proc(argc, argv);
  }

  return 0;
}

/*
main 함수와 prompt, exec_proc
./ssu_sync를 실행하면 사용자 홈 디렉토리 밑에 backup 디렉토리를 생성한다.
사용자의 명령어 입력을 대기하고, 명령어가 입력되면 유효한 명령어인지 판단한다.
fork와 execve 함수를 시용해 별도의 프로세스에서 명령어를 수행한다.
부모 프로세스는 자식 프로세스의 정상 혹은 비정상 종료를 확인하고 오류를 출력한다.
exit 명령어가 입력되면 프로그램이 종료된다.
*/
int main(int argc, char* argv[]) {

  if(init(argv[0]) == -1) {
    fprintf(stderr, "ERROR: init error.\n");
    return -1;
  }
  
  if(!strcmp(argv[0], "add")) return cmd_add(argc-1, argv+1);
  else if(!strcmp(argv[0], "remove")) return cmd_remove(argc-1, argv+1);
  else if(!strcmp(argv[0], "list")) return cmd_list(argc-1, argv+1);
  else if(!strcmp(argv[0], "help")) return cmd_help(argc-1, argv+1);
  else if(!strcmp(argv[0], "daemon")) return daemon_process(argc-1, argv+1);

  while(prompt() != -1);

  return 0;
}