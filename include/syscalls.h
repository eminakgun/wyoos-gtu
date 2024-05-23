 
#ifndef __MYOS__SYSCALLS_H
#define __MYOS__SYSCALLS_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <multitasking.h>

namespace myos
{

    const uint32_t EXIT_INT = 1;
    const uint32_t FORK_INT = 2;
    const uint32_t WAITPID_INT = 7;
    const uint32_t EXECVE_INT = 11;
    
    class SyscallHandler : public hardwarecommunication::InterruptHandler
    {
    private:
        TaskManager* taskManager;
    public:
        SyscallHandler(hardwarecommunication::InterruptManager* interruptManager, myos::common::uint8_t InterruptNumber, TaskManager* taskManager);
        ~SyscallHandler();
        
        virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);
        void fork(uint32_t esp);
        void execve(CPUState* cpu);
    };
    
    
}


#endif