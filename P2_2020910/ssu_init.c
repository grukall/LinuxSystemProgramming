#include "ssu_header.h"

/*
void checkAllStag(dirNode *dirList)
입력 : dirNode *dirList : 추가할 디렉토리 위치
리턴 : void
기능 : 추적 파일 링크드리스트의 현재 노드인 dirList에 실제 경로에 있는 모든 파일과 디렉토리를
      링크드리스트에 추가한다.
*/
void checkAllStag(dirNode *dirList)
{
  dirList->stag = 1;

  struct dirent **list;
  int count;

 // 디렉토리를 scandir에 실패 시 에러처리 한다. scandir에 성공하면 알파벳 순서로 list에 저장된다.
	if ((count = scandir(dirList->dir_path, &list, NULL, alphasort)) == -1)
	{
    fprintf(stderr, "fail to scandir %s : %s\n", dirList->dir_path, strerror(errno));
    exit(1);
  }
	
	//파일 먼저 탐색하여 BFS를 보장한다.
	for (int i = 0; i < count; ++i)
  {
      if (list[i]->d_type != DT_DIR) {
          fileNode* curr_file = fileNode_insert(dirList->file_head, list[i]->d_name, dirList->dir_path);
    	}
  }
	//파일 탐색이 끝나면 서브 디렉토리를 탐색한다.
  for (int i = 0; i < count; ++i){
    if (list[i]->d_type == DT_DIR && strcmp(list[i]->d_name, ".") != 0 && strcmp(list[i]->d_name, "..") != 0 && strcmp(list[i]->d_name, ".repo") != 0)
    {
      dirNode* curr_dir = dirNode_insert(dirList->subdir_head, list[i]->d_name, dirList->dir_path);
      checkAllStag(curr_dir);
      free(list[i]);
    }
	}
	free(list);
}

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
int backup_list_remove(dirNode* dirList, char* path, char *origin_path, int isDir) {
  char* ptr;
  
  if(ptr = strchr(path, '/')) {
    char* dir_name = substr(path, 0, strlen(path) - strlen(ptr));
    dirNode* curr_dir = dirNode_get(dirList->subdir_head, dir_name);
    backup_list_remove(curr_dir, ptr+1, origin_path, isDir);

    if (curr_dir->file_cnt == 0 && curr_dir->subdir_cnt == 0)
      dirNode_remove(curr_dir);
    curr_dir = NULL;
  } else {
    dirNode *temp_dir = dirList->subdir_head->next_dir;
    while (temp_dir != NULL)
    {
      //slog의 경로와 일치하는 dirNode가 있다면, 삭제
      if (strcmp(substr(temp_dir->dir_path, 0, strlen(temp_dir->dir_path) - 1), strtok(origin_path, "\"")) == 0)
      {
        allDelete_dirNode(temp_dir);
        dirNode_remove(temp_dir);
        return 0;
      }
      temp_dir = temp_dir->next_dir;
    }
    if (isDir == 1)
    {
        fprintf(stderr, "DEBUG : there is no dirNode %s\n", origin_path);
        exit(1);
    }
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
int backup_list_insert(dirNode* dirList, char* path, char *origin_path, int isDir) {
  char* ptr;
  
  // '/'가 있으면 디렉토리, 없으면 파일로 해석
  if(ptr = strchr(path, '/')) {
    char* dir_name = substr(path, 0, strlen(path) - strlen(ptr));

    //노드에 서브 디렉토리로 dir_name, dir_path 등록 dirList에 넣기, 이미 존재하면, 그 노드의 다음 노드 반환
    dirNode* curr_dir = dirNode_insert(dirList->subdir_head, dir_name, dirList->dir_path);
    backup_list_insert(curr_dir, ptr+1, origin_path, isDir);
  } else {
    dirNode *temp_dir = dirList->subdir_head->next_dir;
    while (temp_dir != NULL)
    {
      //slog의 경로와 일치하는 dirNode가 있다면, stag 표시
      if (strcmp(temp_dir->dir_path, strtok(origin_path, "\"")) == 0)
      {
        checkAllStag(temp_dir);
        return 0;
      }
      temp_dir = temp_dir->next_dir;
    }
    if (isDir == 1)
    {
      char* dir_name = substr(path, 0, strlen(path));
      dirNode* curr_dir = dirNode_insert(dirList->subdir_head, dir_name, dirList->dir_path);
      checkAllStag(curr_dir);
      return 0;
    }
    //'/'가 없으면 파일로 가정
    char* file_name = strtok(path, "\"");
    //노드에 파일 이름으로 file_name, 현재 디렉토리로 dir_path 넣기. 이미 존재하면, 그 노드의 다음 노드  
    fileNode* curr_file = fileNode_insert(dirList->file_head, file_name, dirList->dir_path);
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
      printf("%s/ %d %d %c\n", curr_dir->dir_name, curr_dir->file_cnt,
        curr_dir->subdir_cnt, curr_dir->stag ? '*' : ' ');
      print_list(curr_dir, depth+1, last_bit);
      curr_dir = curr_dir->next_dir;
    } else {
      print_depth(depth, last_bit);
      printf("%s %c\n", curr_file->file_name, curr_file->stag ? '*' : ' ');
      
      backupNode* curr_backup = curr_file->backup_head->next_backup;
      while(curr_backup != NULL) {
        print_depth(depth+1, last_bit);
        curr_backup = curr_backup->next_backup;
      }
      curr_file = curr_file->next_file;
    }
  }
  
  while(curr_dir != NULL) {
    last_bit |= (curr_dir->next_dir == NULL)?(1 << depth):0;
    
    print_depth(depth, last_bit);
    printf("%s/ %d %d %c\n", curr_dir->dir_name, curr_dir->file_cnt,
      curr_dir->subdir_cnt, curr_dir->stag ? '*' : ' ');
    print_list(curr_dir, depth+1, last_bit);
    curr_dir = curr_dir->next_dir;
  }
  
  while(curr_file != NULL) {
    last_bit |= (curr_file->next_file == NULL)?(1 << depth):0;

    print_depth(depth, last_bit);
    printf("%s %c\n", curr_file->file_name, curr_file->stag ? '*' : ' ');
    curr_file = curr_file->next_file;
  }
}
/*
int init_backup_list(int slog_fd)
입력 : int slog_fd : stagging.log의 파일 디스크립터
리턴 : 정상 종료시 0 리턴
기능 : stagging.log를 한 줄씩 읽어들여 링크드리스트 dirList를 생성한다.
*/
int init_backup_list(int slog_fd) {
  int len;
  char buf[BUF_MAX];

  char *origin_path;

  backup_dir_list = (dirNode*)malloc(sizeof(dirNode));
  dirNode_init(backup_dir_list);

  while(len = read(slog_fd, buf, BUF_MAX)) {
    // slog_fd에서 개행문자를 찾아 그자리에 널문자 넣기, slog_fd는 BUF_MAX만큼 이동
    char *ptr = strchr(buf, '\n');
    ptr[0] = '\0';

    //printf("sentence : %s\n", buf);

    //파일 디스크립터가 ptr로 토큰화한 문장의 다음으로 이동
    lseek(slog_fd, -(len-strlen(buf))+1, SEEK_CUR);

    //buf에 add " 가 들어있다면, backuplist에 insert
    if((ptr = strstr(buf, ADD_SEP)) != NULL) {
      origin_path = substr(buf, 5, strlen(buf) - 1);
      struct stat statbuf;
      if (stat(origin_path, &statbuf) == -1)
      {
        fprintf(stderr, "stat error %s : %s\n", origin_path, strerror(errno));
        exit(1);
      }
      if (S_ISREG(statbuf.st_mode))
        backup_list_insert(backup_dir_list, origin_path, origin_path, 0);
      else
        backup_list_insert(backup_dir_list, origin_path, origin_path, 1);
    }

    //buf에 remove " 가 들어있다면, backuplist의 해당 파일을 remove.
    if((ptr = strstr(buf, REMOVE_SEP)) != NULL) {
      origin_path = substr(buf, 8, strlen(buf) - 1);
      struct stat statbuf;
      if (stat(origin_path, &statbuf) == -1)
      {
        fprintf(stderr, "stat error %s : %s\n", origin_path, strerror(errno));
        exit(1);
      }
      if (S_ISREG(statbuf.st_mode))
        backup_list_remove(backup_dir_list, origin_path, origin_path, 0);
      else
        backup_list_remove(backup_dir_list, origin_path, origin_path, 1);
    }
  }

  return 0;
}

/*
int init()
입력 : void
리턴 : void
기능 : 각 명령어 시작 전, init()을 호출해 링크느리스트 dirList, stagging.log, commit.log
      를 생성한다.
*/
int init() {
  int clog_fd;
  int slog_fd;

  pwd_path = (char *)malloc(sizeof(char) * PATH_MAX);
  if (getcwd(pwd_path, PATH_MAX) != NULL) {
      //printf("Current working directory: %s\n", pwd_path);
  } else {
      perror("getcwd() error");
      return EXIT_FAILURE;
  }

  //.repo 경로 및 clog, slog 경로 생성
  repo_path = (char *)malloc(sizeof(char) * PATH_MAX);
  sprintf(repo_path, "%s/.repo", pwd_path);
  clog_path = (char *)malloc(sizeof(char) * PATH_MAX);
  sprintf(clog_path, "%s/.commit.log", repo_path);
  slog_path = (char *)malloc(sizeof(char) * PATH_MAX);
  sprintf(slog_path, "%s/.staging.log", repo_path);

  if(access(repo_path, F_OK)) {
    if(mkdir(repo_path, 0755) == -1) {
      fprintf(stderr, "mkdir error %s\n", repo_path);
      return -1;
    };
  }

  if((slog_fd = open(slog_path, O_RDWR|O_CREAT, 0777)) == -1) {
    fprintf(stderr, "ERROR: open error for '%s'\n", slog_path);
    return -1;
  }

  if((clog_fd = open(clog_path, O_RDWR|O_CREAT, 0777)) == -1) {
    fprintf(stderr, "ERROR: open error for '%s'\n", clog_path);
    return -1;
  }

  //staging.log 내용으로 backuplist 생성
  init_backup_list(slog_fd);

  //printf("\n****backup_list******\n");
  // print_list(backup_dir_list, 0, 0);

  close(slog_fd);
  close(clog_fd);

  return 0;
}
