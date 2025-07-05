#include "ssu_header.h"

void help_backup(void){
printf("> backup <PATH> [OPTION]... : backup file if <PATH> is file\n-d : backup files in directory if <PATH> is directory\n-r : backup files in directory recursive if<PATH> is directory\n-y : backup file although already backuped\n");
}

void help_remove(void){
printf("> remove <PATH> [OPTION]... : remove backuped file if <PATH> is file\n-d : remove backuped files in directory if <PATH> is directory\n-r : remove backuped files in directory recursive if <PATH> is directory\n-a : remove all backuped files\n");
}

void help_recover(void){
printf("> recover <PATH> [OPTION]... : recover backuped file if <PATH> is file\n-d : recover backuped files in directory if <PATH> is directory\n-r : recover backuped files in directory recursive if <PATH> is directory\n-l : recover latest backuped file\n-n <NEW_PATH> : recover backuped file with new path\n");
}

void help_list(void){
printf("> list [PATH] : show backup list by directory structure\n>> rm <INDEX> [OPTION]... : remove backuped files of [INDEX] with [OPTION]\n>> rc <INDEX> [OPTION]... : recover backuped files of [INDEX] with [OPTION]\n>> vi(m) <INDEX> : edit original file of [INDEX]\n>> exit : exit program\n");
}

void help_help(void){
printf("> help [COMMAND] : show commands for program\n");
}

void help_all(void){
	help_backup();
	help_remove();
	help_recover();
	help_list();
	help_help();
}

/*
void help(char *command)
입력 :	char *command : 도움말을 출력할 command
리턴 :	void
기능 :	ssu_backup의 기능 중 하나인 help이다. 입력된 명령어에 알맞은 도움말을 출력한다.
*/
void help(char *command)
{
	if (command == NULL || command[0] == '\0')
	{
		printf("Usage:\n");
		help_all();
	}
	else if (!strcmp(command, "backup"))
	{
		printf("Usage:");
		help_backup();
	}
	else if (!strcmp(command, "remove"))
	{
		printf("Usage:");
		help_remove();
	}
	else if (!strcmp(command, "recover"))
	{
		printf("Usage:");
		help_recover();
	}
	else if (!strcmp(command, "list"))
	{
		printf("Usage:");
		help_list();
	}
	else if (!strcmp(command, "help"))
	{
		printf("Usage:");
		help_help();
	}
	else
		error("help> invalid command. Check the command list using \n\n  ssu_backup help\n");
}
