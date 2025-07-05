#include "ssu_header.h"


void excute_command(char *buffer)
{
  char *argv[3];
  argv[0] = strtok(buffer, " ");
  argv[1] = strtok(NULL, " ");
  argv[2] = NULL;

  pid_t pid = fork();
  if (!pid)
  {
    // 사용자가 입력한 명령어에 알맞은 파일을 실행시킨다.
    if (strcmp("add", argv[0]) == 0)
      execve(ADD_C, argv, NULL);
    else if (strcmp("remove", argv[0]) == 0)
      execve(REMOVE_C, argv, NULL);
    else
      execve(HELP_C, argv, NULL);
    if (errno)
    {
      fprintf(stderr, "error : %s\n", strerror(errno));
      exit(1);
    }
    exit(0);
  }
  else
  {
    int status;
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status)) {
        int signal_number = WTERMSIG(status);
        const char *signal_name = strsignal(signal_number);
        if (signal_name != NULL) {
            printf("Child process terminated by signal: %s\n", signal_name);
        } else {
            printf("Child process terminated by unknown signal number: %d\n", signal_number);
        }
    }
  }

}

/*
main 함수
./ssu_repo를 실행하면 현재 경로를 파악하고, ./repo 디렉토리와 하위 파일들을 생성한다.
사용자의 명령어 입력을 대기하고, 명령어가 입력되면 유효한 명령어인지 판단한다.
fork와 execve 함수를 시용해 별도의 프로세스에서 명령어를 수행한다.
부모 프로세스는 자식 프로세스의 정상 혹은 비정상 종료를 확인하고 오류를 출력한다.
exit 명령어가 입력되면 프로그램이 종료된다.
*/
int main(void) {
  char *argv;
  char buffer[BUF_MAX];
  char ch;
  int  i = 0;

  int clog_fd;
  int slog_fd;

  pwd_path = (char *)malloc(sizeof(char) * PATH_MAX);
  if (getcwd(pwd_path, PATH_MAX) != NULL) {
      //printf("Current working directory: %s\n", pwd_path);
  } else {
      perror("getcwd() error");
      return EXIT_FAILURE;
  }

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

  while (1)
  {
    i = 0;

    printf("20202910> ");
    while((ch = getchar()) != '\n')
      buffer[i++] = ch;
    buffer[i] = '\0';

    if (strcmp(buffer, "exit") == 0)
      break;
    if (buffer[0] != '\0')
      excute_command(buffer);
  }

  return 0;
}