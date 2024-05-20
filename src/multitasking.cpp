
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
    return tasks[currentTask];
}

bool TaskManager::AddTask(Task* task)
{
    if(numTasks >= 256)
        return false;
   
    tasks[numTasks++] = task;
    
    if (currentTask == -1)
    {
        // main process
        currentTask = 0;
        tasks[0]->pcb.pid = 999;
    }
    return true;
}

Task* TaskManager::AddTask() {
    //Task* new_task = new Task(nullptr, nullptr);
    
    //tasks[numTasks++] = new_task; 
    Task* new_task = tasks[numTasks++];
    
    //next_task = new_task;
    new_task->pcb.state = READY;
    new_task->pcb.pid = numTasks;
    new_task->pcb.ppid = tasks[currentTask]->pcb.pid;

    // copy stack of the current process to new task
    for (size_t i = 0; i < sizeof(tasks[currentTask]->stack); i++){
        new_task->stack[i] = tasks[currentTask]->stack[i];
    }

    //tasks[numTasks]->cpustate = 
    //tasks[numTasks]->cpustate->ebx = (uint32_t)0;
    
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

    printf("\nTaskManager::Schedule, currentTask:");
    printInt(currentTask);
    printf("\n");

    printf("\nTaskManager::Schedule, numTasks:");
    printInt(numTasks);
    printf("\n");

    return tasks[currentTask]->cpustate;
}

    