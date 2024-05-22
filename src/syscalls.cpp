
#include <syscalls.h>

using namespace myos::hardwarecommunication;

void printf(char* str);
void printInt(int i);

SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber, TaskManager* taskManager)
:    InterruptHandler(interruptManager, InterruptNumber  + interruptManager->HardwareInterruptOffset()),
    taskManager(taskManager)
{
}

SyscallHandler::~SyscallHandler()
{
}


void printf(char*);

void SyscallHandler::fork(uint32_t esp) {
    /* printf("we're in kernel space now\n"); */
    
    // create new task
    Task* new_task = taskManager->AddTask();
    
    // Assign stack and base pointer addresses for the new stack space
    uint32_t offset = esp - (uint32_t)(taskManager->GetCurrent()->StackStartAddr());
    CPUState* fixed_cpustate = (CPUState*)(new_task->StackStartAddr() + offset);

    uint32_t offset2 = ((CPUState*)esp)->ebp - (uint32_t)(taskManager->GetCurrent()->StackStartAddr());
    fixed_cpustate->ebp = (uint32_t)(new_task->StackStartAddr()) + offset2;

    fixed_cpustate->ebx = 0; // syscall return value to indicate its child process
    new_task->SetCPUState(fixed_cpustate);

    // assign ebx as non-zero for parent process syscall return value
    ((CPUState*)esp)->ebx = new_task->GetPID();
    return;
}

void SyscallHandler::execve(CPUState* cpu) {
    cpu->esp = cpu->ebp; // reset stack pointer
    cpu->eip = cpu->ebx; // set instruction pointer for requested function
}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    

    switch(cpu->eax)
    {
        case FORK_INT:
            fork(esp);
            break;
        case EXECVE_INT:
            execve(cpu);
            break;
        case EXIT_INT:
            //taskManager->printTasks(); 
            taskManager->KillCurrent();
            esp = (uint32_t)taskManager->Schedule(cpu);
            break;
        case WAITPID_INT:
            if (!(taskManager->WaitTask(cpu->ebx))) {
                cpu->ecx = -1;
                //printf("Process to wait is not found\n");
            }
            else // switch process since parent is now in waiting state 
                esp = (uint32_t)taskManager->Schedule(cpu);
            break;
        case 4: // print
            printf((char*)cpu->ebx);
            break;
            
        default:
            break;
    }

    //taskManager->printTasks();    
    return esp;
}

