#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
// I/O simulation (sleep-based)
void io_bound_task() {
for(int i = 0; i < 10000; i++) {
usleep(100); // simulate I/O wait
}
}
int main() {
struct timeval start, end;
struct rusage usage;
gettimeofday(&start, NULL);
// Create 8 child processes
for(int i = 0; i < 8; i++) {
if(fork() == 0) {
io_bound_task();
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
printf("IO-BOUND RESULTS\n");
printf("Involuntary Context Switches: %ld\n", usage.ru_nivcsw);
printf("Voluntary Context Switches: %ld\n", usage.ru_nvcsw);
printf("Total Elapsed Time: %.2f s\n", time);
return 0;
}
