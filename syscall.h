#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/vaddr.h"
#include "threads/thread.h"

/* Process identifier. */
typedef int pid_t;
#define PID_ERROR ((pid_t) -1)

void syscall_init (void);

static void halt (void);
void exit (int status);
int write (int fd, const char *buffer, unsigned length);
void read(char *src, char *dst, unsigned size);
pid_t exec (const char *file);
void get_arguments(void *esp, int *argc, char **argv);
void is_safe_address(void *addr);


#endif /* userprog/syscall.h */
