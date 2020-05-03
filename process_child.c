#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdint.h>

struct ProcessChild {
  int   stdin;
  int   stdout;
  int   status;
  /* WARNING: status field does NOT contain current child status */
  pid_t pid;
  bool  running; 
};

/* TODO: description */
struct ProcessChild spawn_process_child(
  const char* name, char* const* argv, int stdin_fd, int stdout_fd)
{
  struct ProcessChild child;
  int childIn[2], childOut[2];
  
  if(stdout_fd == -1 && pipe(childOut) != 0) {
    perror("pipe(childOut)");
    child.status = -1;
    return child;
  }
  if(stdin_fd == -1 && pipe(childIn) != 0) {
    perror("pipe(childIn)");
    child.status = -1;
    return child;
  }

  child.pid = fork();

  if(child.pid == -1) {
    perror("fork");
    child.status = -1;
    return child;
  }

  if(child.pid == 0) {
    /* Child */
    int childin = (stdin_fd == -1 ? childIn[0] : stdin_fd);
    if(dup2(childin, 0) == -1) {
      perror("dup2");
      _exit(1);
    }

    int childout = (stdout_fd == -1 ? childOut[1] : stdout_fd);
    if(dup2(childout, 1) == -1) {
      perror("dup2");
      _exit(1);
    }
    //dup2(childOut[1], 2);

    close(childin);
    close(childout);

    if(stdin_fd == -1) close(childIn[1]);
    if(stdout_fd == -1) close(childOut[0]);

    execvp(name, argv);
    perror("execvp");
    _exit(1);
    /* No return */
  }

  /* Parent */
  if(stdin_fd == -1) close(childIn[0]);
  if(stdout_fd == -1) close(childOut[1]);

  child.stdin = (stdin_fd == -1 ? childIn[1] : stdin_fd);
  child.stdout = (stdout_fd == -1 ? childOut[0] : stdout_fd);

  child.status = 0;
  usleep(1000); // Sometimes speed is a curse...
  child.running = (waitpid(child.pid, &child.status, WNOHANG) == 0);

  return child;
}

bool is_child_ok(struct ProcessChild* pc) {
  int pid = waitpid(pc->pid, &pc->status, WNOHANG);
  
  /* Nothing happened */
  if(pid == 0) return true; 
  
  /* This should NOT happen */
  if(pid == -1) {
    perror("waitpid @ is_child_ok");
    return false;
  }
  
  pc->running = false;
  return pc->status == 0;
}

bool are_children_ok(struct ProcessChild* children, size_t n) {
  for(int i=0; i<n; ++i) {
    if(!is_child_ok(&children[i])) return false;
  }

  return true;
}

