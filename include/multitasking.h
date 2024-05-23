 
#ifndef __MYOS__MULTITASKING_H
#define __MYOS__MULTITASKING_H

#include <common/types.h>
#include <gdt.h>

using namespace myos;
using namespace myos::common;

void printf(char* str);
void printInt(int i);


namespace myos
{

    enum ProcessState {
        READY,
        RUNNING,
        BLOCKED,
        TERMINATED,
        WAITING
    };

    
    struct CPUState
    {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;

        uint32_t esi;
        uint32_t edi;
        uint32_t ebp;

        /*
        common::uint32_t gs;
        common::uint32_t fs;
        common::uint32_t es;
        common::uint32_t ds;
        */
        uint32_t error;

        // already on stack:
        uint32_t eip;
        uint32_t cs;
        uint32_t eflags;
        uint32_t esp;
        uint32_t ss;        
    } __attribute__((packed));
    

    // process control block
    struct PCB
    {
        uint32_t pid;  // Process ID
        uint32_t ppid; // Parent Process ID
        ProcessState state; // State of the process
        uint32_t waitpid; // PID of the process being waited for
        ProcessState waitstate; // State of the process being waited for
    };
    
    class Task
    {
    friend class TaskManager;
    private:
        common::uint8_t stack[4096]; // 4 KiB
        CPUState* cpustate;
        PCB pcb;

    public:
        Task(GlobalDescriptorTable *gdt, void entrypoint());
        ~Task();
        uint8_t* StackStartAddr(){return stack;};
        uint32_t GetPID(){return pcb.pid;};
        void SetCPUState(CPUState* cpustate_) {cpustate = cpustate_;};
    };
    
    
    class TaskManager
    {
    friend class SyscallHandler;
    private:
        Task* tasks[256]; // aka process-table
        int numTasks;
        int currentTask;
    public:
        TaskManager();
        ~TaskManager();
        Task* GetCurrent();
        Task* AddTask();
        bool AddTask(Task* task);
        void KillCurrent();
        int WaitTask(uint32_t pid);
        CPUState* Schedule(CPUState* cpustate);
        void printState(ProcessState state) {
            switch (state)
            {
                case READY: printf("READY, "); break;
                case RUNNING: printf("RUNNING, "); break;
                case BLOCKED: printf("BLOCKED, "); break;
                case TERMINATED: printf("TERMINATED, "); break;
                case WAITING: printf("WAITING, "); break;
                
                default:
                    break;
            }
            return;
        }
        void printTasks(){
            printf("Process Table: currentTask: ");
            printInt(currentTask);
            printf(", numTasks: ");
            printInt(numTasks);

            for (size_t i = 0; i < numTasks; i++)
            {
                printf("\nTask number: ");
                printInt(i);
                printf("\n\t");

                printf("pid: ");
                printInt(tasks[i]->pcb.pid);
                printf(", ");

                printf("ppid: ");
                printInt(tasks[i]->pcb.ppid);
                printf(", ");

                printf("state: ");
                printState(tasks[i]->pcb.state);
                printf(", ");

                printf("waitpid: ");
                printInt(tasks[i]->pcb.waitpid);
                printf(", ");

                printf("waitstate: ");
                printState(tasks[i]->pcb.waitstate);
            }
            printf("\n");
        }
    };
    
    
    
}


#endif