
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
    
    // Assign stack pointer and syscall return value
    uint32_t offset = esp - (uint32_t)(taskManager->GetCurrent()->StackStartAddr());
    CPUState* fixed_cpustate = (CPUState*)(new_task->StackStartAddr() + offset); 
    fixed_cpustate->ebx = 0; // syscall return value to indicate its child process
    new_task->SetCPUState(fixed_cpustate);

    return;
}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    

    switch(cpu->eax)
    {
        case FORK_INT:
            fork(esp);
            //taskManager->printTasks();
            printf("1cpu->ebx: ");
            printInt((int)cpu->ebx);
            printf("\n");
            cpu->ebx = taskManager->GetCurrent()->GetPID();
            printf("2cpu->ebx: ");
            printInt((int)cpu->ebx);
            printf("\n");
            break;
        case 4: // print
            printf((char*)cpu->ebx);
            break;
            
        default:
            break;
    }

    
    return esp;
}

