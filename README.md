
# Project Report: Process Management System

## 1. Introduction

The Process Management System is a C++ project designed to implement process management functionalities in an operating system environment. The system includes components such as a Task Manager, Syscall Handler, and Process Control Block (PCB) struct to manage processes efficiently.

## 2. Objectives

The main objectives of the project are:
- To create a robust and efficient system for managing processes.
- To implement process creation, scheduling, waiting, and termination functionalities.
- To ensure proper handling of system calls and process dependencies.

## 3. Requirements

### 3.1 Functional Requirements
- The system should support creating new processes (fork), executing programs (execve), waiting for processes to finish (waitpid), and terminating processes (exit).
- The system should handle system calls from processes and perform the necessary actions based on the syscall type.
- The system should maintain a list of tasks (processes) and manage their states and dependencies.

### 3.2 Non-Functional Requirements
- The system should be efficient and scalable, capable of handling a large number of processes.
- The system should be robust and handle edge cases and errors gracefully.
- The system should have a simple and intuitive interface for ease of use.

## 4. Design

### 4.1 Component Overview
- **Task Manager**: Manages tasks (processes) within the system, including adding tasks, scheduling tasks, waiting for tasks, and terminating tasks.
- **Syscall Handler**: Handles system calls initiated by processes, interprets syscall requests, and interacts with the Task Manager to perform process-related operations.
- **Process Control Block (PCB)**: Stores essential information about a process, including PID, PPID, process state, waitpid, and wait state.

### 4.2 System Architecture and Scheduling

#### System Architecture
- The system follows a modular architecture, with separate components for managing tasks and handling syscalls.
- The Task Manager interacts with the Syscall Handler to perform process-related operations based on syscall requests.

#### Scheduling Algorithm: Round-Robin
- The Task Manager implements a round-robin scheduling algorithm to manage task execution efficiently.
- In round-robin scheduling, tasks are executed in a cyclic manner, with each task receiving a time slice or quantum of CPU time before being preempted to allow the next task to run.
- This scheduling algorithm ensures fair execution of tasks and prevents any single task from monopolizing the CPU.

## 5. Implementation Details

The project is implemented in C++ and organized into header files (`*.h`) and source files (`*.cpp`). The project directory structure includes:

- `include/`: Contains header files for classes and structs used in the project.
- `src/`: Contains source files for implementing the classes and structs.
- `Makefile`: Makefile to build and run the project. 


### Task Manager
- The Task Manager module includes the implementation of the round-robin scheduling algorithm.
- It maintains a queue of tasks and allocates CPU time to each task in a cyclic manner.
- When a task's time slice expires, it is preempted, and the next task in the queue is scheduled to run.

### Syscall Handler
- The Syscall Handler interacts with the Task Manager to schedule and manage tasks based on syscall requests.
- It ensures that processes are scheduled and executed according to the round-robin algorithm's principles.


## 6. Challenges

- Implementing proper synchronization and handling dependencies between processes.
- Ensuring efficient memory management for process data and control structures.
- Handling edge cases and errors in process management operations.

## 7. Future Enhancements

- Implementing more advanced scheduling algorithms for task management.
- Adding support for inter-process communication (IPC) mechanisms.
- Enhancing error handling and recovery mechanisms.

## 8. Conclusion

The Process Management System project provides a foundation for managing processes in an operating system environment. It implements core functionalities for process creation, scheduling, waiting, and termination, with scope for further enhancements and improvements.

---

This detailed project report provides a comprehensive overview of the Process Management System project, including its objectives, requirements, design, implementation, challenges, and future enhancements. It serves as documentation for understanding the project's purpose, design, and potential areas for improvement.