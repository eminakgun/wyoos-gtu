
#include <multitasking.h>

using namespace myos;
using namespace myos::common;


Task::Task(GlobalDescriptorTable *gdt, void entrypoint())
{
    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
    
    cpustate -> eax = 0;
    cpustate -> ebx = 0;
    cpustate -> ecx = 0;
    cpustate -> edx = 0;

    cpustate -> esi = 0;
    cpustate -> edi = 0;
    cpustate -> ebp = 0;
    
    /*
    cpustate -> gs = 0;
    cpustate -> fs = 0;
    cpustate -> es = 0;
    cpustate -> ds = 0;
    */
    
    // cpustate -> error = 0;    
   
    // cpustate -> esp = ;
    cpustate -> eip = (uint32_t)entrypoint;
    cpustate -> cs = gdt->CodeSegmentSelector();
    // cpustate -> ss = ;
    cpustate -> eflags = 0x202;

    pcb.state = READY;
    
}

Task::~Task()
{
}

        
TaskManager::TaskManager()
{
    numTasks = 0;
    currentTask = -1;
}

TaskManager::~TaskManager()
{
}

Task* TaskManager::GetCurrent() {
/*     printf("\nTaskManager::GetCurrent, currentTask:");
    printInt(currentTask);
    printf("\n"); */
    return tasks[currentTask];
}

void TaskManager::KillCurrent() {
    tasks[currentTask]->pcb.state = TERMINATED;
    //--numTasks;
    /*     
    for (size_t i = currentTask; i < numTasks-1; i++)
    {
        tasks[i] = tasks[i + 1];
    } 
    */
    // TODO relese memory
}

bool TaskManager::AddTask(Task* task)
{
    if(numTasks >= 256)
        return false;
   
    // set pid as zero for the first process ever
    // since we never want to return pid of zero
    if (numTasks == 0){
        task->pcb.pid = 1;
    }

    tasks[numTasks++] = task;
    return true;
}

Task* TaskManager::AddTask() {
    
    // Create a new Task object and initialize it with nullptr for initial values
    Task* new_task = new Task(nullptr, nullptr);
    
    // Add the new task to the task list
    tasks[numTasks] = new_task;
    ++numTasks;

    // Set the PID (Process ID) of the new task to the current number of tasks
    // so that pids are unique since we don't decrement numTasks ever
    new_task->pcb.pid = numTasks;

    // Set the PPID (Parent Process ID) of the new task to the PID of the current task
    new_task->pcb.ppid = tasks[currentTask]->pcb.pid;
    
    // Copy stack of the current process to the new task
    // This is done to create a new task that starts with the same stack state as the current one
    for (size_t i = 0; i < sizeof(tasks[currentTask]->stack); i++){
        new_task->stack[i] = tasks[currentTask]->stack[i];
    }

    return new_task;
}

int TaskManager::WaitTask(uint32_t pid){

    // Check if the current task's PID matches the specified PID (trying to wait on itself)
    if (tasks[currentTask]->pcb.pid == pid)
    {
        return 0;
    }
    
    // Search for the task with the specified PID in the task list
    Task* target = nullptr;
    for (int i = 0; i < numTasks; i++) {
        if (tasks[i]->pcb.pid ==pid) {
            target = tasks[i]; 
        }
    }

    if (target != nullptr)
    {
        /* printf("Found a process to wait\n"); */
        // Update the current task's PCB (Process Control Block) to indicate it is waiting
        tasks[currentTask]->pcb.state = WAITING;
        tasks[currentTask]->pcb.waitpid = pid; // Set the PID of the task it is waiting for
        tasks[currentTask]->pcb.waitstate = target->pcb.state; // Set the wait state to the target task's state
        
        // Return 1 indicating the task is now waiting on the specified PID
        return 1;
    }
    else
        // If no task with the specified PID was found, return 0
        return 0;
}

CPUState* TaskManager::Schedule(CPUState* cpustate){
    //printTasks();

    // If there are no tasks, return the current CPU state
    if(numTasks <= 0) {
        return cpustate;
    }
    
    // If no task is currently scheduled, set the current task to the first task (index 0)
    if (currentTask == -1) {
        // always run main process if there's nothing scheduled
        currentTask = 0;
    }
    else if(currentTask >= 0)
        // Save the CPU state(stack pointer) of the currently scheduled task
        tasks[currentTask]->cpustate = cpustate;   
    
    // Schedule next available process
    Task* next_task = nullptr;
    int next_task_num = 0;

    // Check if any task is in the waiting state
    Task* waiting = nullptr;
    int wait_task_num;
    for (int i = 0; i < numTasks; i++) {
        if (tasks[i]->pcb.state == WAITING) {
            waiting = tasks[i];
            wait_task_num = i;
        }
    }

    // If there is a task in the waiting state, check if the process it's waiting for has terminated
    if (waiting != nullptr)
    {
        //printf("There's a waiting process\n");
        // check waiting pid
        for (int i = 0; i < numTasks; i++) {
            if (tasks[i]->pcb.pid == waiting->pcb.waitpid) {
                if (tasks[i]->pcb.state == TERMINATED){
                    // If the process being waited for has terminated, update next_task and its index
                    //printf("Being waited process has changed its state!\n");
                    waiting->pcb.waitpid = 0;
                    next_task = waiting;
                    next_task_num = wait_task_num;
                    next_task->cpustate->ecx = 1; // set a non-zero number
                    break;
                }
            }
        }
    }

    // If next_task is still nullptr, find the next ready process to schedule
    if (next_task == nullptr)
    {
        //printf("Schedule next READY process");
        for (int i = currentTask; i < numTasks; )
        {
            if(++i >= numTasks)
                i %= numTasks;
            
            if(tasks[i]->pcb.state == READY) {
                // switch to next process
                next_task = tasks[i]; 
                next_task_num = i;
                /* printf("found process: ");
                printInt(i);
                printf("\n"); */
                break;
            }
        }
    }
    
    
/*     printf("\nTaskManager::Schedule, currentTask:");
    printInt(currentTask);
    printf("\nTaskManager::Schedule, numTasks:");
    printInt(numTasks);
    printf("\n"); */

    // move current process to ready queue
    if (tasks[currentTask]->pcb.state != WAITING &&
            tasks[currentTask]->pcb.state != TERMINATED) {
        tasks[currentTask]->pcb.state = READY;
    }

    // Set the state of the next task to RUNNING and update currentTask
    next_task->pcb.state = RUNNING;
    currentTask = next_task_num;

    //printTasks();
    return next_task->cpustate;
}

    