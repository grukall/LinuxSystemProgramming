#include "ssu_header.h"

void help(int cmd_bit) {
  if(!cmd_bit) {
    printf("Usage: \n");
  }

  if(!cmd_bit || cmd_bit & CMD_ADD)
    printf("%s add <PATH> [OPTION]... : add new daemon process of <PATH> if <PATH> is file\n  -d : add new daemon process of <PATH> if <PATH> is directory\n  -r : add new daemon process of <PATH> recursive if <PATH> is directory\n  -t <TIME> : set daemon process time to <TIME> sec (default : 1sec)\n", (cmd_bit?"Usage:":"  >"));

  if(!cmd_bit || cmd_bit & CMD_REMOVE)
    printf("%s remove <DAEMON_PID> : delete daemon process with <DAEMON_PID>\n", (cmd_bit?"Usage:":"  >"));

  if(!cmd_bit || cmd_bit & CMD_LIST)
    printf("%s list [DAEMON_PID] : show daemon process list or dir tree\n", (cmd_bit?"Usage:":"  >"));

  if(!cmd_bit || cmd_bit & CMD_HELP)
    printf("%s help [COMMAND] : show commands for program", (cmd_bit?"Usage:":"  >"));

  if(!cmd_bit || cmd_bit & CMD_EXIT)
    printf("%s exit : exit program\n", (cmd_bit?"Usage:":"  >"));

}

/*
int cmd_help(int argc, char *argv[])
입력 : int argc : 인자 개수
	char *argv[] : 인자 리스트
리턴 : 0 고정
기능 : 각 명령어에 대한 설명을 출력한다.
*/
int cmd_help(int argc, char *argv[]) {
  if(argc == 0) {
    help(0);
  } else if(!strcmp(argv[0], "add")) {
    help(CMD_ADD);
  } else if(!strcmp(argv[0], "remove")) {
    help(CMD_REMOVE);
  } else if(!strcmp(argv[0], "list")) {
    help(CMD_LIST);
  } else if(!strcmp(argv[0], "help")) {
    help(CMD_HELP);
  } else if(!strcmp(argv[0], "exit")) {
    help(CMD_EXIT);
  } else {
    fprintf(stderr, "ERROR: invalid command -- '%s'\n", argv[0]);
    fprintf(stderr, "help : show commands for program\n");
  }
  
  printf("\n");

  return 0;
}