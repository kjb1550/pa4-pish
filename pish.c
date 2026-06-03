#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
// You may not include additional header files.

#include "pish_history.h"
#define MAX_COMMAND_LENGTH 256

/*
 * Script mode flag. If set to 0, the shell reads from stdin. If set to 1,
 * the shell reads from a file from argv[1].
 */
static int script_mode = 0;

static char prev_dir[4096] = {'\0'};

/*
 * Prints a prompt IF NOT in script mode (see script_mode global flag).
 */
void prompt(void) {
    if (!script_mode) {
        char *working_dir = getcwd(NULL, 0);
        struct passwd *user = getpwuid(getuid());
#ifdef PISH_AUTOGRADER
        printf("%s@pish %s$\n", user->pw_name, working_dir);
#else
        printf("\e[0;35m%s@pish \e[0;34m%s\e[0m$ ", user->pw_name, working_dir);
#endif
        fflush(stdout);
        free(working_dir);
    }
}

void usage_error(void) {
    fprintf(stderr, "pish: Usage error\n");
    fflush(stderr);
}

/*
 * Break down a line of input by whitespace, and put the results into
 * a struct pish_arg to be used by other functions.
 *
 * @param command   A char buffer containing the input command
 * @param arg       Broken down args will be stored here
 */
void parse_command(char *command, struct pish_arg *arg) {
    arg->argc = 0;
    // Zero out argv
    for (int i = 0; i < MAX_ARGC; i++) {
        arg->argv[i] = NULL;
    }
 
    // Strip trailing newline if present
    size_t len = strlen(command);
    if (len > 0 && command[len - 1] == '\n') {
        command[len - 1] = '\0';
    }
 
    // Use strtok to tokenize by spaces and tabs
    char *token = strtok(command, " \t");
    while (token != NULL && arg->argc < MAX_ARGC - 1) {
        arg->argv[arg->argc] = token;
        arg->argc++;
        token = strtok(NULL, " \t");
    }
    // argv must be NULL-terminated
    arg->argv[arg->argc] = NULL;
}

/*
 * Run a command.
 *
 * Built-in commands are handled internally by the pish program.
 * Otherwise, use fork/exec to create child processes to run the program.
 *
 * If the command is empty, do nothing.
 * If NOT in script mode, add the command to history file.
 */
void run(struct pish_arg *arg) {
    // Empty command: do nothing
    if (arg->argc == 0) {
        return;
    }
 
    // Add to history if not in script mode
    if (!script_mode) {
        add_history(arg);
    }
 
    char *cmd = arg->argv[0];
 
    if (strcmp(cmd, "exit") == 0) {
        if (arg->argc != 1) {
            usage_error();
            return;
        }
        exit(EXIT_SUCCESS);
 
    } else if (strcmp(cmd, "cd") == 0) {
        if (arg->argc != 2) {
            usage_error();
            return;
        }
 
        char *path = arg->argv[1];
 
        if (strcmp(path, "-") == 0) {
            // cd -: go to previous directory, or print cwd if no prior cd
            char *cwd = getcwd(NULL, 0);
            if (cwd == NULL) {
                perror("cd");
                return;
            }
            if (prev_dir[0] == '\0') {
                // No previous cd this session: just print cwd
                printf("%s\n", cwd);
                free(cwd);
            } else {
                // Swap prev_dir and cwd, then chdir into old prev_dir
                char target[4096];
                strncpy(target, prev_dir, sizeof(target) - 1);
                target[sizeof(target) - 1] = '\0';
 
                printf("%s\n", target);
                fflush(stdout);
 
                strncpy(prev_dir, cwd, sizeof(prev_dir) - 1);
                prev_dir[sizeof(prev_dir) - 1] = '\0';
                free(cwd);
 
                if (chdir(target) != 0) {
                    perror("cd");
                }
            }
        } else {
            // Normal cd: save cwd into prev_dir, then chdir
            char *cwd = getcwd(NULL, 0);
            if (cwd == NULL) {
                perror("cd");
                return;
            }
            strncpy(prev_dir, cwd, sizeof(prev_dir) - 1);
            prev_dir[sizeof(prev_dir) - 1] = '\0';
            free(cwd);
 
            if (chdir(path) != 0) {
                perror("cd");
            }
        }
 
    } else if (strcmp(cmd, "history") == 0) {
        if (arg->argc == 1) {
            print_history();
        } else if (arg->argc == 2 && strcmp(arg->argv[1], "-c") == 0) {
            clear_history();
        } else {
            usage_error();
        }
 
    } else {
        // Not a built-in: fork and exec
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
        } else if (pid == 0) {
            // Child process
            execvp(cmd, arg->argv);
            // execvp only returns on failure
            perror(cmd);
            exit(EXIT_FAILURE);
        } else {
            // Parent process: wait for child to finish
            wait(NULL);
        }
    }
}

/*
 * The main loop of pish. Repeat until the "exit" command or EOF:
 * 1. Print the prompt
 * 2. Read command from fp (which can be stdin or a script file)
 * 3. Execute the command
 *
 * Assume that each command never exceeds MAX_COMMAND_LENGTH-1 chars.
 */
int pish(FILE *fp) {
    char command[MAX_COMMAND_LENGTH];
    struct pish_arg arg;
 
    while (1) {
        prompt();
 
        if (fgets(command, MAX_COMMAND_LENGTH, fp) == NULL) {
            exit(EXIT_SUCCESS);
        }
 
        parse_command(command, &arg);
        run(&arg);
    }
 
    return 0;
}

/*
 * The entry point of the pish program.
 *
 * - If the program is called with no additional arguments (like "./pish"),
 *   process commands from stdin.
 * - If the program is called with one additional argument
 *   (like "./pish script.sh"), process commands from the file specified by the
 *   additional argument under script mode.
 * - If there are more arguments, call usage_error() and exit with status 1.
 */
int main(int argc, char *argv[]) {
    if (argc == 1) {
        script_mode = 0;
        pish(stdin);
    } else if (argc == 2) {
        script_mode = 1;
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL) {
            perror(argv[1]);
            exit(EXIT_FAILURE);
        }
        pish(fp);
        fclose(fp);
    } else {
        usage_error();
        exit(1);
    }
    
    return EXIT_SUCCESS;
}
