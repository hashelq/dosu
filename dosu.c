#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>
#include <grp.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

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

int main(int argc, char *argv[]) {
  if (argc < 2) {
	fprintf(stderr, "NO_CMD\n");
	exit(1);
  }

  if (geteuid() != 0) {
    fprintf(stderr, "NO_SUID\n");
    exit(1);
  }

  char *password = getpass("");

  struct spwd *spw = getspnam(getlogin());
  if (spw == NULL) {
    fprintf(stderr, "NO_PASSWORD\n");
    exit(EXIT_FAILURE);
  }

  if (strcmp(spw->sp_pwdp, crypt(password, spw->sp_pwdp)) != 0) {
    fprintf(stderr, "WRONG_PASSWORD\n");
    exit(EXIT_FAILURE);
  }

  int num_groups = getgroups(0, NULL);
  
  gid_t *groups = malloc(num_groups * sizeof(gid_t));
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

  int ex_status = execute_command(exec_args);

  if (ex_status != 0) {
	perror("CANNOT_EXECUTE");
	exit(EXIT_FAILURE);
  }

  return 0;
}
