#ifndef INIT_COMMANDS_H
#define INIT_COMMANDS_H

#define ALLOC_SIZE 512

#define STDOUT 0
#define STDERR 1
#define EXCODE 2

#define STDOUT_FILE "/tmp/stdout.log"
#define STDERR_FILE "/tmp/stderr.log"

int run_command(void *v);

char* exec_command(char *cmd);

char *build_response(int return_code, int stream);

#endif // INIT_COMMANDS_H
