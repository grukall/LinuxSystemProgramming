#include "ssu_header.h"

/*
void allDelete_dirNode(dirNode *dirList)
입력 : dirNode *dirList : 지우고자 하는 dirNode
리턴 : void
기능 : 추적 파일 링크드리스트의 현재 노드인 dirList에서 하위 디렉토리와 파일 노드를
      모두 삭제한다.
*/
void allDelete_dirNode(dirNode *dirList)
{
  dirList->stag = 1;

  dirNode *temp_dir = dirList->subdir_head->next_dir;
  dirNode *curr_dir;
  fileNode *temp_file = dirList->file_head->next_file;
  fileNode *curr_file;

  //파일 삭제 고민
  while (temp_file != NULL)
  {
    curr_file = temp_file->next_file;
    fileNode_remove(temp_file);
    temp_file = curr_file;
    curr_file = NULL;
  }
  while (temp_dir != NULL)
  {
    allDelete_dirNode(temp_dir);
    curr_dir = temp_dir->next_dir;
    dirNode_remove(temp_dir);
    temp_dir = curr_dir;
    curr_dir = NULL;
  }
}

/*
int backup_list_remove(dirNode* dirList, char* path, char *origin_path, int isDir)
입력 : dirNode *dirList : 추적 링크드리스트
      char *path : 현재 디렉토리 or 파일 이름
      char *origin_path : 초기 경로
      int isDir : 디렉토리이면 1, 아니면 0
리턴 : 정상적으로 종료 시 리턴 0
기능 : 추적 링크드리스트에서 path에 해당하는 노드를 찾고, 노드포함 하위 트리를 삭제한다.
*/
int backup_list_remove(dirNode* dirList, char* path, char *origin_path) {
  char* ptr;
  
  if(ptr = strchr(path, '/')) {
    char* dir_name = substr(path, 0, strlen(path) - strlen(ptr));
    dirNode* curr_dir = dirNode_get(dirList->subdir_head, dir_name);
    backup_list_remove(curr_dir, ptr+1, origin_path);

    if (curr_dir->file_cnt == 0 && curr_dir->subdir_cnt == 0)
      dirNode_remove(curr_dir);
    curr_dir = NULL;
  } else {
    char* file_name = path;
    fileNode* curr_file = fileNode_get(dirList->file_head, file_name);
    fileNode_remove(curr_file);
    curr_file = NULL;
  }
  return 0;
}

/*
int backup_list_insert(dirNode* dirList, char* path, char *origin_path, int isDir)
입력 : dirNode *dirList : 추적 링크드리스트
      char *path : 현재 디렉토리 or 파일 이름
      char *origin_path : 초기 경로
      int isDir : 디렉토리이면 1, 아니면 0
리턴 : 정상적으로 종료 시 리턴 0
기능 : 추적 링크드리스트에서 path에 해당하는 노드를 찾고, 노드포함 하위 트리를 생성한다.
*/
int backup_list_insert(dirNode* dirList, char* path, char *origin_path, fileNode *filenode) {
  char* ptr;
  
  // '/'가 있으면 디렉토리, 없으면 파일로 해석
  if(ptr = strchr(path, '/')) {
    char* dir_name = substr(path, 0, strlen(path) - strlen(ptr));

    //노드에 서브 디렉토리로 dir_name, dir_path 등록 dirList에 넣기, 이미 존재하면, 그 노드의 다음 노드 반환
    dirNode* curr_dir = dirNode_insert(dirList->subdir_head, dir_name, dirList->dir_path);
    backup_list_insert(curr_dir, ptr+1, origin_path, filenode);
  } else {
    char* file_name = strtok(path, "\"");
    //노드에 파일 이름으로 file_name, 현재 디렉토리로 dir_path 넣기. 이미 존재하면, 그 노드의 다음 노드  
    fileNode* curr_file = fileNode_insert(dirList->file_head, file_name, dirList->dir_path, filenode);
  }

  return 0;
}

// 디버그용 함수
void print_depth(int depth, int is_last_bit) {
  for(int i = 1; i <= depth; i++) {
    if(i == depth) {
      if((1 << depth) & is_last_bit) {
          printf("┗ ");
      } else {
          printf("┣ ");
      }
      break;
    }
    if((1 << i) & is_last_bit) {
      printf("  ");
    } else {
      printf("┃ ");
    }
  }
}

//디버그용 함수
void print_list(dirNode* dirList, int depth, int last_bit) {
  dirNode* curr_dir = dirList->subdir_head->next_dir;
  fileNode* curr_file = dirList->file_head->next_file;
  
  while(curr_dir != NULL && curr_file != NULL) {
    if(strcmp(curr_dir->dir_name, curr_file->file_name) < 0) {
      print_depth(depth, last_bit);
      printf("%s\n", curr_dir->dir_name);
      print_list(curr_dir, depth+1, last_bit);
      curr_dir = curr_dir->next_dir;
    } else {
      print_depth(depth, last_bit);
      printf("%s\n", curr_file->file_name);
      
      backupNode* curr_backup = curr_file->backup_head->next_backup;
      while(curr_backup != NULL) {
        print_depth(depth+1, last_bit);
        printf("[%s] [%s]\n", curr_backup->mode, curr_backup->backup_time);
        curr_backup = curr_backup->next_backup;
      }
      curr_file = curr_file->next_file;
    }
  }
  
  while(curr_dir != NULL) {
    last_bit |= (curr_dir->next_dir == NULL)?(1 << depth):0;
    
    print_depth(depth, last_bit);
    printf("%s/\n", curr_dir->dir_name);
    print_list(curr_dir, depth+1, last_bit);
    curr_dir = curr_dir->next_dir;
  }
  
  while(curr_file != NULL) {
    last_bit |= (curr_file->next_file == NULL)?(1 << depth):0;

    print_depth(depth, last_bit);
    printf("%s\n", curr_file->file_name);

   backupNode* curr_backup = curr_file->backup_head->next_backup;
    while(curr_backup != NULL) {
      print_depth(depth+1, (
        last_bit | ((curr_backup->next_backup == NULL)?(1 << depth+1):0)
        ));
      printf("[%s] [%s]\n", curr_backup->mode, curr_backup->backup_time);
      curr_backup = curr_backup->next_backup;
    }
    curr_file = curr_file->next_file;
  }
}

/*
int init_backup_list(int slog_fd)
입력 : int slog_fd : pid.log의 파일 디스크립터
리턴 : 정상 종료시 0 리턴
기능 : pid.log를 한 줄씩 읽어들여 링크드리스트 dirList를 생성한다.
*/
int init_backup_list(int slog_fd) {
  int len;
  char buf[BUFMAX];
  char *origin_path;

  while(len = read(slog_fd, buf, BUFMAX)) {
    // slog_fd에서 개행문자를 찾아 그자리에 널문자 넣기, slog_fd는 BUF_MAX만큼 이동
    char *ptr = strchr(buf, '\n');
    ptr[0] = '\0';


    //파일 디스크립터가 ptr로 토큰화한 문장의 다음으로 이동
    lseek(slog_fd, -(len-strlen(buf))+1, SEEK_CUR);

    //log 바꾸고 다시
    int index = 0;
    int count = 0;
    char *mode;
    char *backup_time;
    int index_mode;
    while (1) {
      if (buf[index] == ']') {
        count++;
        if (count == 1) {
          index_mode = index + 2;
        }
        if (count == 2)
          break ;
      }
      index++;
    }

    index += 2;
    backup_time = substr(buf, 1, index_mode - 2);
    mode = substr(buf, index_mode, index - 2);
    origin_path = substr(buf, index, strlen(buf) - 1);
    //printf("mode : %s, origin_path : %s\n", mode, origin_path);

    if (!strcmp(mode, "delete")) {
      fileNode *node = find_fileNode(backup_dir_list, origin_path);
      backupNode_insert(node->backup_head, mode, backup_time, origin_path);
    }
    else if (!strcmp(mode, "create")) {
      backup_list_insert(backup_dir_list, origin_path, origin_path, NULL);
      char *str_backuptime = make_strBackupTime(backup_time);
      fileNode *node = find_fileNode(backup_dir_list, origin_path);
      backupNode_insert(node->backup_head, mode, backup_time, origin_path);
    }
    else if (!strcmp(mode, "modify")) {
      char *str_backuptime = make_strBackupTime(backup_time);
      fileNode *node = find_fileNode(backup_dir_list, origin_path);
      backupNode_insert(node->backup_head, mode, backup_time, origin_path);
    }
    free(mode);
    free(origin_path);
    mode = NULL;
    origin_path = NULL;
  }

  return 0;
}

/*
int init()
입력 : char *path : 실행 경로
리턴 : void
기능 : 각 명령어 시작 전, init()을 호출해 헤더에 선언된 변수들에 대한 초기화를 진행하고,
       monitor_list.log가 있는지 확인하고 없으면 생성한다.
*/
int init(char *path) {

  exe_path = path;
  pwd_path = (char *)malloc(sizeof(char) * PATHMAX);
  if (getcwd(pwd_path, PATHMAX) != NULL) {
      //printf("Current working directory: %s\n", pwd_path);
  } else {
      perror("getcwd() error");
      return EXIT_FAILURE;
  }

  char *home = getenv("HOME");
  char buf[PATHMAX];
  sprintf(buf, "%s/backup", home);
  backup_path = strdup(buf);
  memset(buf, 0, PATHMAX);
  sprintf(buf, "%s/monitor_list.log", backup_path);
  backup_log_path = strdup(buf);

  struct stat temp;
  if (stat(backup_path, &temp) == -1) {
    if (mkdir(backup_path, 0755) == -1){
      fprintf(stderr, "mkdir failed : %s\n", strerror(errno));
      exit(1);
    }
  }
  if((monitor_log_fd = open(backup_log_path, O_RDWR|O_CREAT, 0777)) == -1) {
    fprintf(stderr, "ERROR: open error for '%s'\n", backup_log_path);
    return -1;
  }

  close(monitor_log_fd);

  return 0;
}
