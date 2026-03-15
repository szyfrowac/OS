# Assignment 1 - Operating Systems

## Group Details
1. Bala Phanikar Challa
2. Hemant Sunkara
3. Shambhavi Rani
4. Prajeet Singh 

## Environment
- Course: CS F372 - Operating Systems
- Semester: Second Semester 2025-26
- Target platform: Ubuntu 24.x VM
- Language: C
- Kernel work target for Q5: Linux 6.14

## Repository Contents
- `q1.c` - Parent/child program using `fork()` and an unnamed pipe.
- `q2.c` - Memory-monitor style process utility using `fork()`, `system("ps ...")`, and `kill()`.
- `q3.c` - `mygrep` pipeline using `grep`, `cut`, `sort`, and `uniq` through pipes and `execvp()`.
- `station_controller.c` - Simple shell for the station controller problem in Q4.
- `q5.c` - User-space wrapper and test program for the custom `myfork` system call.
- `myfork-q5.c` - Kernel-side system call implementation used for Q5.
- `belgium.txt`, `france.txt` - Sample text files for Q3.
- `Q5.docx` - Supporting notes/documentation for Q5.

## Build
Run the following from this directory:

```bash
gcc -Wall -Wextra -o q1 q1.c
gcc -Wall -Wextra -o q2 q2.c
gcc -Wall -Wextra -o mygrep q3.c
gcc -Wall -Wextra -o station_controller station_controller.c
gcc -Wall -Wextra -o q5 q5.c
```

Note: `station_controller.c` currently builds with warnings, but compilation succeeds.

## Run
Examples:

```bash
./q1
./q2
./mygrep Hello belgium.txt france.txt
./station_controller
./q5
```

## Question-wise Summary

### Q1 - Random number producer and factor finder
- The parent reads `n`, randomly selects values from the fixed array `{3, 15, 4, 6, 7, 17, 9, 2}`, and sends each value to the child through an unnamed pipe.
- The child prints all factors of the received number and sleeps for `time(NULL) % n` seconds.
- The parent sleeps for `x % n` seconds after sending each value.
- Execution stops after all array elements have been visited at least once or when `Ctrl+C` is detected.

### Q2 - Memory monitor
- The program accepts `n`, `k`, and `r` as inputs.
- It periodically displays the top `k` processes by memory usage using `ps` output sorted by `%mem`.
- After every `r` display iterations, a PID is requested and may be killed using `SIGKILL`.
- Special values supported in the current program:
  - `-2` terminates the program.
  - `-1` skips killing and resumes monitoring.

### Q3 - `mygrep`
- `mygrep` is implemented as a command pipeline built with `fork()`, `pipe()`, `dup2()`, and `execvp()`.
- Pipeline used by the program:

```text
grep -nH PATTERN files... | cut -d: -f2 | sort -n | uniq
```

- This matches the assignment requirement of using standard UNIX tools instead of implementing search/sort/deduplication logic manually in C.

### Q4 - Station controller shell
- Internal commands:
  - `set_freq <MHz>`
  - `get_freq`
  - `quit`
- External commands:
  - `top`
  - `ping <host>` (implemented as exactly 4 pings)
- The shell initializes frequency to `800 MHz`.
- `SIGINT` does not terminate the shell; instead, it resets the carrier frequency back to the safe default and prints the required warning.

## Q5 - Kernel Modification (`myfork`) 

This is the most important systems-level component in the assignment because it requires modifying and rebuilding the Linux kernel rather than only writing a user-space C program.

### Objective
Add a new system call named `myfork` to Linux 6.14. Its behaviour should be identical to `fork()`, with one additional side effect:
- it logs the PID of the calling process into the kernel buffer.

### Files for Q5
- `myfork-q5.c` - kernel-space implementation of the new syscall.
- `q5.c` - user-space test program and syscall wrapper.
- `myfork.o` - object file artifact for the kernel-side implementation.

### Kernel-side implementation
The syscall implementation in `myfork-q5.c` is:

```c
SYSCALL_DEFINE0(myfork)
{
    long ret;
    pr_info("myfork: Process %d is cloning itself.\n", current->pid);

    ret = kernel_clone(&(struct kernel_clone_args){
        .flags = SIGCHLD,
        .exit_signal = SIGCHLD,
    });

    return ret;
}
```

### What this code does
- `SYSCALL_DEFINE0(myfork)` declares a new zero-argument system call.
- `pr_info(...)` writes a message to the kernel log buffer, which can later be checked using `dmesg`.
- `current->pid` fetches the PID of the process that invoked the syscall.
- `kernel_clone(...)` performs the actual process creation, making the syscall behave like a basic `fork()` implementation.
- `SIGCHLD` ensures standard parent/child termination semantics consistent with normal process cloning.

### User-space wrapper and test program
The wrapper in `q5.c` calls syscall number `467`:

```c
long myfork(void)
{
    return syscall(467);
}
```

The test program then invokes `myfork()` and prints the returned PID value:

```c
pid = myfork();
printf("Hello world! PID=%ld\n", pid);
```

### Kernel integration steps (Linux 6.14)
The file `myfork-q5.c` contains only the syscall body. To make it callable from user space, it must be integrated into the Linux source tree and the kernel must be rebuilt.

1. Copy the implementation file into the kernel source tree, typically under a directory such as `kernel/`.
2. Update the relevant `Makefile` so the object is compiled, for example by adding:

```make
obj-y += myfork-q5.o
```

3. Add the syscall declaration if needed in the syscall headers, typically in `include/linux/syscalls.h`:

```c
asmlinkage long sys_myfork(void);
```

4. Register the syscall in the x86-64 syscall table (for Linux 6.14 on x86_64), typically in:

```text
arch/x86/entry/syscalls/syscall_64.tbl
```

Add an entry using the chosen syscall number, for example:

```text
467    common    myfork    sys_myfork
```

5. Rebuild the kernel and its modules.
6. Install the rebuilt kernel.
7. Reboot into the new kernel.
8. Recompile `q5.c` in user space.

### Important note about syscall numbering
- The wrapper uses syscall number `467`.
- If `467` is already in use in your Linux 6.14 source tree, choose any free syscall number and update both:
  - the syscall table entry, and
  - the wrapper in `q5.c`
- Both sides must match exactly.

### Testing Q5
After booting into the modified kernel:

```bash
gcc -Wall -Wextra -o q5 q5.c
./q5
dmesg | grep myfork
```

### Expected result
- The user-space program should print `Hello world!` along with the return value from the fork-like syscall.
- The kernel log should contain a message similar to:

```text
myfork: Process <pid> is cloning itself.
```

### Why Q5 is important
Q5 goes beyond library usage and process control in user space. It demonstrates:
- how a user program transitions from user mode to kernel mode,
- how a new system call is wired into the Linux syscall table,
- how kernel logging works through `pr_info`, and
- how existing kernel functionality (`kernel_clone`) can be reused to implement new OS features.

## Submission Notes
- Include all source files, executables, and this README in the final tarball.
- Replace the placeholder group details before submission.
- For Q5 demo/viva, keep both the user-space test (`q5.c`) and kernel-side implementation (`myfork-q5.c`) ready, along with `dmesg` output from the modified kernel.
