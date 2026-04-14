#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
// CPU intensive work
void cpu_bound_task() {
volatile long long x = 0;
for(long long i = 0; i < 1e9; i++) {
x += i;
}
}
int main() {
struct timeval start, end;
struct rusage usage;
gettimeofday(&start, NULL);
// Create 8 child processes
for(int i = 0; i < 8; i++) {
if(fork() == 0) {
cpu_bound_task();
exit(0);
}
}
// Parent waits
for(int i = 0; i < 8; i++) {
wait(NULL);
}
gettimeofday(&end, NULL);
getrusage(RUSAGE_CHILDREN, &usage);
double time = (end.tv_sec - start.tv_sec) +
(end.tv_usec - start.tv_usec) / 1e6;
printf("CPU-BOUND RESULTS\n");
printf("Involuntary Context Switches: %ld\n", usage.ru_nivcsw);
printf("Voluntary Context Switches: %ld\n", usage.ru_nvcsw);
printf("Total Elapsed Time: %.2f s\n", time);
return 0;
}
