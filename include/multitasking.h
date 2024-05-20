 
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
        TERMINATED
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
    

    struct PCB
    {
        uint32_t pid;  // Process ID
        uint32_t ppid; // Parent Process ID
        ProcessState state;
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
        CPUState* Schedule(CPUState* cpustate);
        void printTasks(){
            for (size_t i = 0; i < numTasks; i++)
            {
                printf("\nTask number: ");
                printInt(i);
                printf("\n\t");

                printf("pid: ");
                printInt(tasks[i]->pcb.pid);
                printf("\n\t");

                printf("ppid: ");
                printInt(tasks[i]->pcb.ppid);
                printf("\n\t");

                printf("state: ");
                printInt(tasks[i]->pcb.state);
                printf("\n");
            }
            
        }
    };
    
    
    
}


#endif