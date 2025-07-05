#include "ssu_header.h"

void help_add(void){
printf("> add <PATH> : record path to staging area, path will tracking modification\n");
}

void help_remove(void){
printf("> remove <PATH> : record path to staging area, path will not tracking modification\n");
}

void help_status(void){
printf("> status : show staging area status\n");
}

void help_commit(void){
printf("> commit <NAME> : backup staging area with commit name\n");
}

void help_revert(void){
printf("> revert <NAME> : recover commit version with commit name\n");
}

void help_log(void){
printf("> log : show commit log\n");
}

void help_help(void){
printf("> help : show commands for program\n");
}

void help_exit(void){
printf("> exit : exit program\n");
}

void help_all(void){
	help_add();
	help_remove();
	help_status();
	help_commit();
	help_revert();
	help_log();
	help_help();
	help_exit();
}

int main(int argc, char *argv[])
{
	help(argv[1]);
	exit(0);
}

/*
void help(char *command)
입력 :	char *command : 도움말을 출력할 command
리턴 :	void
기능 :	ssu_repo의 기능 중 하나인 help이다. 입력된 명령어에 알맞은 도움말을 출력한다.
*/
void help(char *command)
{
	if (command == NULL || command[0] == '\0')
	{
		printf("Usage:\n");
		help_all();
	}
	else if (!strcmp(command, "add"))
	{
		printf("Usage:");
		help_add();
	}
	else if (!strcmp(command, "remove"))
	{
		printf("Usage:");
		help_remove();
	}
	else if (!strcmp(command, "status"))
	{
		printf("Usage:");
		help_status();
	}
	else if (!strcmp(command, "commit"))
	{
		printf("Usage:");
		help_commit();
	}
	else if (!strcmp(command, "revert"))
	{
		printf("Usage:");
		help_revert();
	}
	else if (!strcmp(command, "help"))
	{
		printf("Usage:");
		help_help();
	}
	else if (!strcmp(command, "exit"))
	{
		printf("Usage:");
		help_exit();
	}
	else
		fprintf(stderr, "help> invalid command. Check the command list using \n\n  ssu_repo help\n");
	printf("\n");
}
