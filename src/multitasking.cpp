
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

bool TaskManager::AddTask(Task* task)
{
    if(numTasks >= 256)
        return false;
   
    if (numTasks == 0){
        printf("set parent pid");
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

    new_task->pcb.state = READY;
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

CPUState* TaskManager::Schedule(CPUState* cpustate)
{
    if(numTasks <= 0)
        return cpustate;
    
    if(currentTask >= 0)
        tasks[currentTask]->cpustate = cpustate;
    
    if(++currentTask >= numTasks)
        currentTask %= numTasks;

    printTasks();
    printf("\nTaskManager::Schedule, currentTask:");
    printInt(currentTask);
    printf("\nTaskManager::Schedule, numTasks:");
    printInt(numTasks);
    printf("\n");

    return tasks[currentTask]->cpustate;
}

    