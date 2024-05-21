
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
    printf("we're in kernel space now\n");
    
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
    ((CPUState*)esp)->ebx = taskManager->GetCurrent()->GetPID();
    return;
}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    

    switch(cpu->eax)
    {
        case FORK_INT:
            fork(esp);
            break;
        case 4: // print
            printf((char*)cpu->ebx);
            break;
            
        default:
            break;
    }

    
    return esp;
}

