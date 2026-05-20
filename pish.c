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
    // TODO
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
    // TODO
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
    // TODO
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
    // TODO
    return EXIT_SUCCESS;
}
