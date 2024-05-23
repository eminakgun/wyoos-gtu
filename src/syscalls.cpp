
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
    
    // Create a new task using the TaskManager
    Task* new_task = taskManager->AddTask();
    
    // Assign stack and base pointer addresses for the new stack space,
    // esp and ebp are crucial in terms of program execution
    // all of the stask space is copied but esp and ebp must be updated
    // according to the new stack space, since start address of the process's stack space
    // is completely different from the one being forked
    uint32_t offset = esp - (uint32_t)(taskManager->GetCurrent()->StackStartAddr());
    CPUState* fixed_cpustate = (CPUState*)(new_task->StackStartAddr() + offset);

    // do the same for ebp
    uint32_t offset2 = ((CPUState*)esp)->ebp - (uint32_t)(taskManager->GetCurrent()->StackStartAddr());
    fixed_cpustate->ebp = (uint32_t)(new_task->StackStartAddr()) + offset2;

    // Set ebx in the new CPU state to indicate its child process (return value)
    fixed_cpustate->ebx = 0; // syscall return value to indicate its child process
    new_task->SetCPUState(fixed_cpustate);

    // Set ebx in the current CPU state to indicate the PID of the new child process 
    // (return value for parent process)
    ((CPUState*)esp)->ebx = new_task->GetPID();
    return;
}

void SyscallHandler::execve(CPUState* cpu) {
    // Reset the base pointer (ebp) to the stack start address of the current task
    cpu->ebp = (uint32_t)taskManager->GetCurrent()->StackStartAddr();

    // Reset the stack pointer (esp) to the base pointer, effectively resetting the stack
    cpu->esp = cpu->ebp;

    // Set instruction pointer as the entry point of the requested function
    cpu->eip = cpu->ebx; 
}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;

    // Handle different system calls based on the value of eax register
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
            taskManager->KillCurrent(); // terminate current task
            // Schedule the next task and update the stack pointer (esp)
            esp = (uint32_t)taskManager->Schedule(cpu);
            break;
        case WAITPID_INT:
            if (!(taskManager->WaitTask(cpu->ebx))) {
                // If the task to wait for is not found, set ecx register to -1
                cpu->ecx = -1;
                //printf("Process to wait is not found\n");
            }
            else
                // switch process since parent is now in waiting state 
                esp = (uint32_t)taskManager->Schedule(cpu);
            break;
        case 4: // print
            printf((char*)cpu->ebx);
            break;
            
        default:
            break;
    }

    //taskManager->printTasks();    
    return esp; // Return the updated stack pointer
}

