#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512

int fg_process = -1;
int sigtstp_flag = 0;
int fg_flag = 0;

struct command_line
{
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool is_bg;
};

struct command_line *parse_input()
{
	char input[INPUT_LENGTH];
	struct command_line *curr_command = (struct command_line *) calloc(1, sizeof(struct command_line));

	printf("$ ");
	fflush(stdout);
	fgets(input, INPUT_LENGTH, stdin);

	char *token = strtok(input, " \n");
	while(token){
		if(!strcmp(token,"<")){
			curr_command->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			curr_command->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			curr_command->is_bg = true;
		} else{
			curr_command->argv[curr_command->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}
	return curr_command;
}

void free_cmd(struct command_line *cmd){
    if (!cmd){
        return;
    }

    for (int i = 0; i < cmd->argc; i++){
        free(cmd->argv[i]);
    }

    free(cmd->input_file);
    free(cmd->output_file);

    free(cmd);
}

int main()
{
	struct command_line *curr_command;

	while(true)
	{
		curr_command = parse_input();
        char *token = curr_command->argv[0];

        if (curr_command->is_bg && sigtstp_flag){
            curr_command->is_bg = false;
        }

        if (!token || token[0] == '#'){
            free_cmd(curr_command);
            continue;
        }

	}
	return EXIT_SUCCESS;
}