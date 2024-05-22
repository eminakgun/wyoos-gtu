
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
   
    if (numTasks == 0){
        printf("set parent pid\n");
        task->pcb.pid = 1;
    }

    tasks[numTasks++] = task;
    
/*     printf("\nTaskManager::AddTask::1, currentTask:");
    printInt(currentTask);
    printf("\nTaskManager::AddTask::1, numTasks:");
    printInt(numTasks);
    printf("\n"); */
    return true;
}

Task* TaskManager::AddTask() {
    
    Task* new_task = new Task(nullptr, nullptr);
    
    tasks[numTasks] = new_task;
    ++numTasks;

    new_task->pcb.pid = numTasks;
    new_task->pcb.ppid = tasks[currentTask]->pcb.pid;
    
    // copy stack of the current process to new task
    for (size_t i = 0; i < sizeof(tasks[currentTask]->stack); i++){
        new_task->stack[i] = tasks[currentTask]->stack[i];
    }

/*     printf("\nTaskManager::AddTask::2, currentTask:");
    printInt(currentTask);
    printf("\nTaskManager::AddTask::2, numTasks:");
    printInt(numTasks); */

    return new_task;
}

int TaskManager::WaitTask(uint32_t pid){

    if (tasks[currentTask]->pcb.pid == pid) // myself
    {
        return 0;
    }
    
    // find target task
    Task* target = nullptr;
    for (int i = 0; i < numTasks; i++) {
        if (tasks[i]->pcb.pid ==pid) {
            target = tasks[i]; 
        }
    }

    if (target != nullptr)
    {
        /* printf("Found a process to wait\n"); */
        tasks[currentTask]->pcb.state = WAITING;
        tasks[currentTask]->pcb.waitpid = pid;
        tasks[currentTask]->pcb.waitstate = target->pcb.state;
        return 1;
    }
    else
        return 0;
}

CPUState* TaskManager::Schedule(CPUState* cpustate){
    //printTasks();

    if(numTasks <= 0) {
        return cpustate;
    }
    
    if (currentTask == -1) {
        // always run main process if there's nothing scheduled
        currentTask = 0;
    }
    else if(currentTask >= 0)
        // save stack pointer
        tasks[currentTask]->cpustate = cpustate;   
    
    // Schedule next available process
    Task* next_task = nullptr;
    int next_task_num = 0;

    // Check if any of them is in waiting state
    Task* waiting = nullptr;
    int wait_task_num;
    for (int i = 0; i < numTasks; i++) {
        if (tasks[i]->pcb.state == WAITING) {
            waiting = tasks[i];
            wait_task_num = i;
        }
    }
    if (waiting != nullptr)
    {
        //printf("There's a waiting process\n");
        // check waiting pid
        for (int i = 0; i < numTasks; i++) {
            if (tasks[i]->pcb.pid == waiting->pcb.waitpid) {
                if (tasks[i]->pcb.state == TERMINATED){
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

    if (next_task == nullptr)
    {
        //printf("Schedule next READY process");
        for (int i = currentTask; i < numTasks; )
        {
/*             printf("process");
            printInt(i);
            printf("\n"); */
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

    next_task->pcb.state = RUNNING;
    currentTask = next_task_num;

    //printTasks();
    return next_task->cpustate;
}

    