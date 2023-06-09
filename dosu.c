#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <shadow.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_command(char* args[]) {
  pid_t pid = fork();

  // if err
  if (pid == -1) {
    perror(0);
    return -1;
  }

  // child
  if (pid == 0) {
    if (setuid(0) < 0) {
      perror("SETUID");
      exit(EXIT_FAILURE);
    }
    execvp(args[0], args);
    perror(0);
    _exit(127);
  }

  // parent wait
  siginfo_t info;
  if (0 > waitid(P_PID, pid, &info, WEXITED)) {
    perror(0);
    return -1;
  }

  return info.si_status;
}

void putenv_var(char* key, char* value) {
  int len_k = strlen(key);
  int len_v = strlen(value);

  int size = (len_k + len_v + 1 + 1);
  char* new = malloc(sizeof(char) * size);
  if (new == NULL) {
    fprintf(stderr, "MALLOC_FAILED");
    exit(-1);
  }
  memcpy(new, key, len_k);
  new[len_k] = '=';
  memcpy(new + len_k + 1, value, len_v);
  new[size - 1] = '\0';
  putenv(new);
  free(new);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "NO_CMD\n");
    exit(1);
  }

  if (geteuid() != 0) {
    fprintf(stderr, "NO_SUID\n");
    exit(1);
  }

  const char* prompt = getenv("DOSU_PROMPT");
  if (prompt == NULL) {
    // No prompt
    prompt = "";
  }
  char* password = getpass(prompt);

  struct spwd* spw = getspnam(getlogin());
  if (spw == NULL) {
    fprintf(stderr, "GETSPNAM_FAILED");
    exit(-1);
  }
  if (spw == NULL) {
    fprintf(stderr, "NO_PASSWORD\n");
    exit(EXIT_FAILURE);
  }

  if (strcmp(spw->sp_pwdp, crypt(password, spw->sp_pwdp)) != 0) {
    fprintf(stderr, "WRONG_PASSWORD\n");
    exit(EXIT_FAILURE);
  }

  int num_groups = getgroups(0, NULL);
  if (num_groups == -1) {
    fprintf(stderr, "GETGROUPS_FAILED");
    exit(-1);
  }

  gid_t* groups = malloc(num_groups * sizeof(gid_t));
  int status = getgroups(num_groups, groups);

  if (status == -1) {
    perror("GETGROUPS_FAILED");
    exit(EXIT_FAILURE);
  }

  struct group* grp = getgrnam("dosu");
  if (grp == NULL) {
    fprintf(stderr, "SYS_NO_DOSU_GROUP\n");
    exit(EXIT_FAILURE);
  }

  char access = 0;
  for (int i = 0; i < num_groups; i++) {
    if (groups[i] == grp->gr_gid) {
      access = 1;
      break;
    }
  }

  if (access == 0) {
    fprintf(stderr, "USR_NO_DOSU_GROUP\n");
    exit(EXIT_FAILURE);
  }

  // create a list of args
  char** exec_args = malloc(sizeof(char*) * argc);

  for (int i = 0; i < argc - 1; i++) {
    exec_args[i] = argv[i + 1];
  }

  // NULL at the end
  exec_args[argc - 1] = NULL;

  // env vars
  struct passwd* pwd = getpwnam("root");
  if (pwd == NULL) {
    fprintf(stderr, "GETPWNAM_FAILED");
    exit(-1);
  }
  putenv_var("USER", "root");
  putenv_var("LOGNAME", "root");
  putenv_var("HOME", pwd->pw_dir);

  return execute_command(exec_args);
}
