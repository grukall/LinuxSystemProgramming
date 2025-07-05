#include "ssu_header.h"

dirNode* backup_dir_list;
dirNode *version_dir_list;

/*
fileNode *find_fileNode(dirNode *list, char *file_path)
입력 : dirNode *list : backup_list
      char *file_path : 찾을 파일노드 경로
리턴 : 찾은 파일 노드의 주소, 없으면 NULL
기능 : file_path와 일치하는 list의 fileNode를 반환한다. 없으면 NULL을 리턴한다.
*/
fileNode *find_fileNode(dirNode *list, char *file_path) {
  fileNode *file_list = list->file_head->next_file;

  while (file_list) {
    if (!strcmp(file_list->file_path, file_path))
      return file_list;
    file_list = file_list->next_file;
  }

  dirNode *subdir = list->subdir_head->next_dir;
  while (subdir) {
    if ((file_list = find_fileNode(subdir, file_path)) != NULL)
      return file_list;
    subdir = subdir->next_dir;
  }

  return NULL;
}

/*
dirNode *find_dirNode(dirNode *list, char *dir_path)
입력 : dirNode *list : backup_list
      char *dir_path : 찾을 디렉토리 경로
리턴 : 찾은 디렉토리 노드의 주소, 없으면 NULL
기능 : dir_path와 일치하는 list의 dirNode를 반환한다. 없으면 NULL을 리턴한다.
*/
dirNode *find_dirNode(dirNode *list, char *dir_path) {

  if (!strcmp(substr(list->dir_path, 0 , strlen(list->dir_path) - 1), dir_path)) {
    return list;
  }
  
  //리스트의 서브 디렉토리를 순회하며 검사
  dirNode *subdir = list->subdir_head->next_dir;
  char *ptr;
  while (subdir) {
    //printf("dir_path : %s, subdir->dir_path : %s\n", dir_path, substr(subdir->dir_path, 0, strlen(subdir->dir_path) - 1));
    if ((ptr = strstr(dir_path, substr(subdir->dir_path, 0, strlen(subdir->dir_path) - 1))) != NULL) {
      return find_dirNode(subdir, dir_path);
    }
    subdir = subdir->next_dir;
  }
  return NULL;
}
/*
void dirNode_init(dirNode *dir_node)
입력 : dirNode *dir_node : 초기화할 dirNode
리턴 : void
기능 : dirNode 링크드리스트의 초기화
*/
void dirNode_init(dirNode *dir_node) {
  dir_node->root_dir = NULL;

  dir_node->stag = 0;
  dir_node->file_cnt = 0;
  dir_node->subdir_cnt = 0;

  dir_node->file_head = (fileNode*)malloc(sizeof(fileNode));
  dir_node->file_head->prev_file = NULL;
  dir_node->file_head->next_file = NULL;
  dir_node->file_head->root_dir = dir_node;

  dir_node->subdir_head = (dirNode*)malloc(sizeof(dirNode));
  dir_node->subdir_head->prev_dir = NULL;
  dir_node->subdir_head->next_dir = NULL;
  dir_node->subdir_head->root_dir = dir_node;

  dir_node->prev_dir = NULL;
  dir_node->next_dir = NULL;
}

/*
dirNode *dirNode_get(dirNode* dir_head, char* dir_name)
입력 : dirNode *dir_head : 순회할 dirNode의 head
       char *dir_name : 찾고자 하는 디렉토리의 이름
리턴 : dir_name과 일치하는 dirNode
기능 : dirNode의 dir_name 이 dir_name과 일치하는 노드를 찾아 반환한다.
*/
dirNode *dirNode_get(dirNode* dir_head, char* dir_name) {
  dirNode *curr_dir = dir_head->next_dir;
  
  while(curr_dir != NULL) {
    if(curr_dir != NULL && !strcmp(dir_name, curr_dir->dir_name)) {
      return curr_dir;
    }
    curr_dir = curr_dir->next_dir;
  }
  return NULL;
}

/*
dirNode *dirNode_insert(dirNode* dir_head, char* dir_name, char* dir_path)
입력 : dirNode *dir_head : 순회할 dirNode 리스트의 head
       char *dir_name : 생성할 dirNode의 dir_name
       char *dir_path : 생성할 dirNOde의 dir_path
리턴 : 새로 생성한 dirNode의 주소
기능 : dir_name과 dir_path를 가진 새로운 dirNode를 dir_head가 가리키는 리스트의
       알맞은 순서에 넣는다. 새로운 노드가 들어가는 위치는 노드가 뒤의 노드들보다 상위 디렉토리 이거나,
       알파벳 순서로 앞이면 앞쪽에 배치하고, 아니면 리스트의 가장 끝에 추가한다.
*/
dirNode *dirNode_insert(dirNode* dir_head, char* dir_name, char* dir_path) {
  dirNode *curr_dir = dir_head;
  dirNode *next_dir, *new_dir;
  
  //링크드 리스트의 끝까지 이동
  while(curr_dir != NULL) {
    // 현재 노드의 다음 노드
    next_dir = curr_dir->next_dir;

    //dir_name과 다음 노드의 dir_name이 일치하면 다음 노드를 반환
    if(next_dir != NULL && !strcmp(dir_name, next_dir->dir_name)) {
      return curr_dir->next_dir;
    } else if(next_dir == NULL || strcmp(dir_name, next_dir->dir_name) < 0) {
      // 다음 노드가 없거나 dir_name이 다음 노드의 dir_name보다 상위 디렉토리면 or
      // 알파벳 순서로 앞의 디렉토리면 노드 할당
      new_dir = (dirNode*)malloc(sizeof(dirNode));
      dirNode_init(new_dir);

      //새로운 노드의 루트 디렉토리를 기존 헤드의 루트 디렉토리로 설정
      new_dir->root_dir = dir_head->root_dir;
      
      //새로운 노드의 dir_name을 dir_name으로 설정
      sprintf(new_dir->dir_name, "%s", dir_name);

      //새로운 노드의 dir_path를 기존 dir_path에 dir_name을 붙여 생성
      sprintf(new_dir->dir_path, "%s%s/", dir_path, dir_name);

      //새로운 노드를 중간에 끼워 넣음
      new_dir->prev_dir = curr_dir;
      new_dir->next_dir = next_dir;
      
      //다음 노드가 있다면 다음 노드의 이전노드를 새로운 노드로 설정
      if(next_dir != NULL) {
        next_dir->prev_dir = new_dir;
      }
      curr_dir->next_dir = new_dir;

      //서브 디렉토리 노드가 늘어났으므로 subdir_cnt 증가
      dir_head->root_dir->subdir_cnt++;

      //다음 노드를 반환
      return curr_dir->next_dir;
    }
    curr_dir = next_dir;
  }
  return NULL;
}

/*
void dirNode_remove(dirNode *dir_node)
입력 : dirNode *dir_node : 삭제하고자 하는 dirNode의 주소
리턴 : void
기능 : dir_node를 리스트로부터 삭제한다.
*/
void dirNode_remove(dirNode *dir_node) {
  dirNode *next_dir = dir_node->next_dir;
  dirNode *prev_dir = dir_node->prev_dir;

  if(next_dir != NULL) {
    next_dir->prev_dir = dir_node->prev_dir;
    prev_dir->next_dir = dir_node->next_dir;
  } else {
    prev_dir->next_dir = NULL;
  }

  if (dir_node->subdir_head != NULL)
    free(dir_node->subdir_head);
  if (dir_node->file_head != NULL)
    free(dir_node->file_head);
  dir_node->subdir_head = NULL;
  dir_node->file_head = NULL;

  free(dir_node);
}

/*
void fileNode_init(fileNode *file_node)
입력 : fileNode *file_node : 초기화할 filnode
리턴 : void
기능 : fileNode 링크드리스트의 초기화
*/
void fileNode_init(fileNode *file_node) {
  file_node->root_dir = NULL;

  file_node->backup_count = 0;
  file_node->mtime = 0;
  file_node->file_md5 = NULL;

  file_node->backup_head = (backupNode*)malloc(sizeof(backupNode));
  file_node->backup_head->prev_backup = NULL;
  file_node->backup_head->next_backup = NULL;
  file_node->backup_head->root_file = file_node;

  file_node->prev_file = NULL;
  file_node->next_file = NULL;
}

/*
fileNode *fileNode_get(fileNode *file_head, char *file_name)
입력 : fileNode *file_head : 순회할 fileNode의 head
       char *file_name : 찾고자 하는 파일의 이름
리턴 : file_name과 일치하는 fileNode
기능 : fileNode의 file_name이 file_name과 일치하는 노드를 찾아 반환한다.
*/
fileNode *fileNode_get(fileNode *file_head, char *file_name) {
  fileNode *curr_file = file_head->next_file;
  
  while(curr_file != NULL) {
    if(curr_file != NULL && !strcmp(file_name, curr_file->file_name)) {
      return curr_file;
    }
    curr_file = curr_file->next_file;
  }
  return NULL;
}

/*
fileNode *fileNode_insert(fileNode *file_head, char *file_name, char *dir_path)
입력 : fileNode *file_head : 순회할 fileNode 리스트의 head
       char *file_name : 생성할 fileNode의 file_name
       char *dir_path : 생성할 fileNode의 dir_path
리턴 : 새로 생성한 fileNode의 주소
기능 : file_name과 dir_path를 가진 새로운 fileNode를 file_head가 가리키는 리스트의
       알맞은 순서에 넣는다. 새로운 노드가 들어가는 위치는 알파벳 순서로 앞이면 앞쪽에 배치하고, 아니면 리스트의 가장 끝에 추가한다.
*/
fileNode *fileNode_insert(fileNode *file_head, char *file_name, char *dir_path, fileNode *filenode) {
  fileNode *curr_file = file_head;
  fileNode *next_file, *new_file;
  if (filenode != NULL)
    new_file = filenode;
  
  while(curr_file != NULL) {
    next_file = curr_file->next_file;

    if(next_file != NULL && !strcmp(file_name, next_file->file_name)) {
      return curr_file->next_file;
    } else if(next_file == NULL || strcmp(file_name, next_file->file_name) < 0) {
      if (filenode != NULL)
        new_file = filenode;
      else {
        new_file = (fileNode*)malloc(sizeof(fileNode));
        fileNode_init(new_file);
      }

      new_file->root_dir = file_head->root_dir;
      strcpy(new_file->file_name, file_name);
      strcpy(new_file->file_path, dir_path);
      strcat(new_file->file_path, file_name);
      
      new_file->prev_file = curr_file;
      new_file->next_file = next_file;
      
      if(next_file != NULL) {
        next_file->prev_file = new_file;
      }
      curr_file->next_file = new_file;

      file_head->root_dir->file_cnt++;

      return curr_file->next_file;
    }
    curr_file = next_file;
  }
  return NULL;
}

/*
void fileNode_remove(fileNode *file_node)
입력 : fileNode *file_node : 삭제하고자 하는 fileNode의 주소
리턴 : void
기능 : file_node를 리스트로부터 삭제한다.
*/
void fileNode_remove(fileNode *file_node) {
  fileNode *next_file = file_node->next_file;
  fileNode *prev_file = file_node->prev_file;

  if(next_file != NULL) {
    next_file->prev_file = file_node->prev_file;
    prev_file->next_file = file_node->next_file;
  } else {
    prev_file->next_file = NULL;
  }

  if (file_node->backup_head != NULL)
    free(file_node->backup_head);
  file_node->backup_head = NULL;

  if (file_node->file_md5 != NULL)
    free(file_node->file_md5);

  free(file_node);
}

/*
void backupNode_init(backupNode *backup_node)
입력 : backupNode *backup_node : 초기화할 backupnode
리턴 : void
기능 : backup 노드의 초기화
*/
void backupNode_init(backupNode *backup_node) {
  backup_node->root_version_dir = NULL;
  backup_node->root_file = NULL;

  backup_node->prev_backup = NULL;
  backup_node->next_backup = NULL;
}

/*
backupNode *backupNode_insert(backupNode *backup_head, char *mode, char *backup_time, char *file_path)
입력 : backupNode *backup_head : 순회할 backupNode 리스트의 head
       char *mode : backupNode에 저장할 파일의 상태
       char *backup_time : backupNode에 저장할 상태 변경 시간
       char *file_path : 원본 파일 경로
리턴 : 새로 생성한 backupNode의 주소
기능 : mode와 backup_time을 가진 새로운 backupNode를 backup_head가 가리키는 리스트에서
       알맞은 순서에 넣는다. 새로운 노드가 들어가는 위치는 알파벳 순서로 앞이면 앞쪽에 배치하고, 아니면 리스트의 가장 끝에 추가한다.
*/
backupNode *backupNode_insert(backupNode *backup_head, char *mode, char *backup_time, char *file_path) {
  backupNode *curr_backup = backup_head;
  backupNode *next_backup, *new_backup;

  while(curr_backup != NULL) {
    next_backup = curr_backup->next_backup;
    if(next_backup != NULL && !strcmp(backup_time, next_backup->backup_time)) {
      return curr_backup->next_backup;
    } else if(next_backup == NULL || strcmp(backup_time, next_backup->backup_time) < 0) {
      new_backup = (backupNode*)malloc(sizeof(backupNode));
      backupNode_init(new_backup);

      new_backup->root_file = backup_head->root_file;
      new_backup->root_file->backup_count += 1;

      strcpy(new_backup->backup_time, backup_time);
      strcpy(new_backup->origin_path, file_path);
      strcpy(new_backup->mode, mode);

      new_backup->prev_backup = curr_backup;
      new_backup->next_backup = next_backup;
      
      if(next_backup != NULL) {
        next_backup->prev_backup = new_backup;
      }
      curr_backup->next_backup = new_backup;

      return curr_backup->next_backup;
    }
    curr_backup = curr_backup->next_backup;
  }
  return NULL;
}