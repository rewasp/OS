#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //printf ("system call!\n");
  int argc=0;
  char **argv = NULL;
  
  argv = (char*)malloc(sizeof(char*) * (5));

  switch (*(int *) f->esp)
  {
    case SYS_HALT:
      //printf("HALT!\n");
      halt ();
      break;
    case SYS_EXEC:
      //printf("SYS_EXEC!\n");
      get_arguments(f->esp, &argc, argv);
      //hex_dump(f->esp, f->esp, 32, true);
      //hex_dump(0xbfffffb8, 0xbfffffb8, 32, true);
      f->eax = exec((char*)argv[0]);
      break;
    case SYS_EXIT:
      //printf("SYS_EXIT!\n");
      get_arguments (f->esp, &argc, argv);
      //hex_dump(f->esp, f->esp, 32, true);
      exit ((int)argv[0]);
      break;
    case SYS_WAIT:
      //printf("SYS_WAIT!\n");
      get_arguments (f->esp, &argc, argv);
      //hex_dump(f->esp, f->esp, 32, true);
      //printf("Wait Process ID : %d\n", (int)argv[0]);
      f->eax = process_wait((int)argv[0]);
      break;
    case SYS_READ:
      //printf("SYS_READ!\n");
      get_arguments(f->esp, &argc, argv);
      read((char*)argv[0], (char*)argv[1], (unsigned)argv[2]);
      break;
    case SYS_WRITE:
      get_arguments(f->esp, &argc, argv);
      //printf("SYS_WRITE!\n");
      write(0, (const char*)(argv[1]), (unsigned)(argv[2]));
      break;
    case SYS_PIBONACCI:
      get_arguments(f->esp, &argc, argv);
      f->eax = pibonaccis((int)argv[0]);
      break;
    case SYS_SUM:
      get_arguments(f->esp, &argc, argv);
      f->eax = (int)argv[0];
      break;
    default:
      //printf("Wrong system call!\n");
      exit(-1);
  }
  
  //thread_exit ();
}

void
get_arguments(void *esp, int *argc, char **argv){
  /*printf("\n\n#dump\n");
  hex_dump(esp, esp, 32, true);
  printf("\n");*/

  static int cnt=0;

  is_safe_address(esp);
  is_safe_address((int*)esp + 3);
  /*if(cnt > 3)
    thread_exit();
  cnt++;*/

  *argc = *((int*)(esp));
  argv[0] = (char*)(*((int*)(esp) + 1));

  //printf("argc : %d\n", *argc);

  //printf("\nParsed address : %x\n", *((int*)(esp) + (*argc) + 1));
  //hex_dump(*((int*)(esp) + (*argc) + 1), *((int*)(esp) + (*argc) + 1), 8, true);
  argv[1] = (char*)(*((int*)(esp) + 2));
  argv[2] = (int)(*((int*)(esp) + 3));
}

pid_t
exec (const char *file){
  pid_t tid;
  struct thread *child;

  tid = process_execute(file);
  if (tid == -1)
    return -1;

    //printf("exe\n\n");
  struct list_elem *elem_ptr;
  elem_ptr = list_begin(&thread_current()->list_child);
  while(1){
    if(elem_ptr == list_end(&thread_current()->list_child))
      break;

    child = list_entry(elem_ptr, struct thread, elem_child);

    if(child->tid == tid){
      //printf("Find tid %d : %s.\n", tid, child->name);
      sema_init(&child->sema_for_load_child, 0);
      sema_init(&child->sema_for_load_parent, 0);

      sema_down(&child->sema_for_load_parent);
      if(child->is_successful == false){
        sema_up(&child->sema_for_load_child);
        return -1;
      }
      sema_up(&child->sema_for_load_child);
      break;
    }

    elem_ptr = list_next(elem_ptr);
  }

  //printf("thread executed : %d\n", tid);

  return tid;
}

void read(char *src, char *dst, unsigned size){
  unsigned i=0;

  is_safe_address(src);
  is_safe_address(src+size);
  is_safe_address(dst);
  is_safe_address(dst+size);

  for(i=0; i<size; i++){
    *(dst+i) = *(src+i);
  }
}

int
write (int fd, const char *buffer, unsigned size){
  unsigned int temp=0;
  //printf("write entered\n");

  for(temp=0; temp<size; temp++){
    printf("%c", *((char*)buffer + temp));
  }
  //printf("  size : %d\n", size);
  //printf("%s", (char*)buffer);
  return 0;
}

void
is_safe_address(void *addr){
  if(!(is_user_vaddr(addr) && (addr > 0x08048000))){
    //printf("Not user address.\n");
    exit(-1);
  }
}

static void
halt (void){
  shutdown_power_off ();
}

int pibonaccis(int n){
  int result=0;
  int i=0, t0=1, t1=1;

  if(n <= 0)
    return 0;
  else if(n == 1 || n == 2)
    return 1;

  for(i=2; i<n; i++){
    result = t0 + t1;
    t0 = t1;
    t1 = result;
  }

  return result;
}

void
exit (int status)
{
  //printf("Exit : status is %d\n", status);
  thread_current()->exit_value = status;
  thread_current()->isdying = true;
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit ();
}
