#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

long myfork(void)
{
    return syscall(467);
}

int main()
{
    long pid;

    pid = myfork();

    printf("Hello world! PID=%ld\n", pid);

    return 0;
}
