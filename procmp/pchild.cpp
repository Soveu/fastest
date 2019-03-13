#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

struct Pchild {
  pid_t   pid;
  int     pipeIn;
  int     pipeOut;
};

int spawn_pchild(Pchild* pc, const char* name, char* const* argv, char* const* envp){
  int childIn[2], childOut[2];
  int status = 0;
  
  if(pipe(childIn) + pipe(childOut) < 0)
    return -1;

  pid_t child = fork();

  if(child == -1)
    return -1;

  if(child == 0) {
    /* Child */
    dup2(childIn[0], 0);
    dup2(childOut[1], 1);
    dup2(childOut[1], 2);

    close(childIn[0]);
    close(childIn[1]);
    close(childOut[0]);
    close(childOut[1]);

    _exit(execve(name, argv, envp));
  }

  /* Parent */
  close(childIn[0]);
  close(childOut[1]);

  pc->pid = child;
  pc->pipeIn = childIn[1];
  pc->pipeOut = childOut[0];

  if(waitpid(child, &status, WNOHANG) == 0)
    return 0;

  return status;
}

