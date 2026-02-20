#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512

int fg_process = -1;
int fg_only_flag = 0;
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
    int childStatus;

	while(true)
	{
		curr_command = parse_input();
        char *token = curr_command->argv[0];

        if (curr_command->is_bg && fg_only_flag){
            curr_command->is_bg = false;
        }

        if (!token || token[0] == '#'){
            free_cmd(curr_command);
            continue;
        }

        if (!strcmp(token, "exit")){
            free_cmd(curr_command);
            kill(0, SIGTERM);
            break;
        }

        if (!strcmp(token, "cd")){
            if (!curr_command->argv[1]){
                chdir(getenv("HOME"));
            } else if (chdir(curr_command->argv[1]) != 0){
                perror("Directory Change Failed");
            } else {
                chdir(curr_command->argv[1]);
            }
            free_cmd(curr_command);
            continue;
        }

        if (!strcmp(token, "status")){
            if (WIFSIGNALED(fg_flag)){
                printf("Terminates by signal %d\n", WTERMSIG(fg_flag));
            } else {
                printf("Exit value: %d\n", WEXITSTATUS(fg_flag));
            }
            fflush(stdout);
            free_cmd(curr_command);
            continue;
        }

        pid_t childpid = fork();

        if (childpid == -1){
            perror("Fork failed!");
            exit(1);
        } else if(childpid == 0) {
            //Child Process

            //Redirect output if file is given
            if (curr_command->output_file){
                int fd_out = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0640);
                if (fd_out == -1){
                    perror("Error opening file");
                    exit(1);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }
            //Redirect input if file is given
            if (curr_command->input_file){
                int fd_in = open(curr_command->input_file, O_RDONLY, 0640);
                if (fd_in == -1){
                    perror("Error opening file");
                    exit(1);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }
            //Redirect ouput of background process if no file specified
            if (curr_command->is_bg && !curr_command->output_file && !fg_only_flag){
                int fd_out = open("/dev/null", O_WRONLY);
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }
            //Redirect input of background process if not file specified
            if (curr_command->is_bg && !curr_command->input_file && !fg_only_flag){
				int fd_in = open("/dev/null", O_RDONLY);
				dup2(fd_in, STDIN_FILENO);
				close(fd_in);
			}

            curr_command->argv[curr_command->argc] = NULL;

            execvp(token, curr_command->argv);
			perror("Error with execvp");
			exit(2);

        } else {
            //Parent case
            if (curr_command->is_bg && !fg_only_flag){
                printf("Background pid is %d\n", childpid);
                fflush(stdout);
            } else {
                fg_process = childpid;
                waitpid(childpid, &childStatus, 0);
                fg_process = -1;
                fg_flag = childStatus;
                if (WIFSIGNALED(childStatus)){
                    printf("Terminated by signal %d\n", WTERMSIG(childStatus));
                    fflush(stdout);
                }
            }
            free_cmd(curr_command);
        }
	}
	return EXIT_SUCCESS;
}