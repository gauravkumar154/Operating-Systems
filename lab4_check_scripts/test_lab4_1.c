// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "sh", 0 };

int
main(void)
{
  int pid, wpid,pid2;

  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

//   for(;;){
    printf(1, "init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){
        pid2 = fork();
    if(pid2>0)
    {
        wait();
        exec("sh", argv);
        printf(1, "init: exec sh failed\n");
        exit();
    }
    else if(pid==0)
    {
        exec("memtest2", argv);
    }
    else
    {
        exit();
    }
      
    }
    while((wpid=wait()) >= 0 && wpid != pid)
      printf(1, "zombie!\n");
//   }
}
