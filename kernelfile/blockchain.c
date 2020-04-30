#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
asmlinkage int sys_my_time(struct timespec *t) {
  getnstimeofday(t);
  
  return 0;
}



