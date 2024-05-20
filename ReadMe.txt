

On fork
The child process and the parent process run in separate memory spaces.  At the time of fork() both memory spaces have the same content.
Memory writes, file mappings (mmap(2)), and unmappings (munmap(2)) performed by one of the processes do not affect the other.
The child has its own unique process ID,.
The child's parent process ID is the same as the parent's process ID.
- RETURN VALUE
    On success, the PID of the child process is returned in the parent, and 0 is returned in the child.  On failure, -1 is returned in the parent, no child process is created, and errno is set appropriately.