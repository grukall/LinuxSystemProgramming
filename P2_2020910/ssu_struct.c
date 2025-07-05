#include "ssu_header.h"

dirNode* backup_dir_list;
dirNode *version_dir_list;

char *exe_path;
char *pwd_path;
char *home_path;

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

  file_node->stag = 0;

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
fileNode *fileNode_insert(fileNode *file_head, char *file_name, char *dir_path) {
  fileNode *curr_file = file_head;
  fileNode *next_file, *new_file;
  
  while(curr_file != NULL) {
    next_file = curr_file->next_file;

    if(next_file != NULL && !strcmp(file_name, next_file->file_name)) {
      next_file->stag = 1;
      return curr_file->next_file;
    } else if(next_file == NULL || strcmp(file_name, next_file->file_name) < 0) {
      new_file = (fileNode*)malloc(sizeof(fileNode));
      fileNode_init(new_file);

      new_file->root_dir = file_head->root_dir;
      strcpy(new_file->file_name, file_name);
      strcpy(new_file->file_path, dir_path);
      strcat(new_file->file_path, file_name);
      
      new_file->stag = 1;
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

  free(file_node);
}