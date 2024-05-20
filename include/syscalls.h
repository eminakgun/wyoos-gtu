 
#ifndef __MYOS__SYSCALLS_H
#define __MYOS__SYSCALLS_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <multitasking.h>

namespace myos
{

    const uint32_t FORK_INT = 2;
    
    class SyscallHandler : public hardwarecommunication::InterruptHandler
    {
    private:
        TaskManager* taskManager;
    public:
        SyscallHandler(hardwarecommunication::InterruptManager* interruptManager, myos::common::uint8_t InterruptNumber, TaskManager* taskManager);
        ~SyscallHandler();
        
        virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);
        void fork(uint32_t esp);
    };
    
    
}


#endif