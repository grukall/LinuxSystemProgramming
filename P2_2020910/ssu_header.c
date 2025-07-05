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
  char origin_path[PATH_MAX];
  char ret_path[PATH_MAX] = "";

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