#include <linux/linkage.h>
#include <linux/kernel.h>
asmlinkage int sysadd(const char *S)
{  
printk("%s",S);
return 1;
}
