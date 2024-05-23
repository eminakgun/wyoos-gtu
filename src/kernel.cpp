
#include <common/types.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <syscalls.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <multitasking.h>


using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

char get_digit(int i) {
    char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    return digits[i];
}

void print_digit(int i) {
    char a = get_digit(i & 0x000003FF);
    if (i > 9)
    {
        printf("given number is not a digit");
    }
    
    char* ptr = &a;
    ++ptr = (char*)'\0';
    printf(&a);
}

void printInt(int i) {
    int res[5];
    int cnt = 0;

    if (i == 0)
    {
        print_digit(0);
    }
    else {
        while (i > 0) {
            int digit = i % 10;
            res[cnt++] = digit;
            i /= 10;
        }
        --cnt;
        while (!(cnt < 0))
        {
            print_digit(res[cnt--]);
        }
    }

}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}
void printfHex16(uint16_t key)
{
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}
void printfHex32(uint32_t key)
{
    printfHex((key >> 24) & 0xFF);
    printfHex((key >> 16) & 0xFF);
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}





class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:
    
    MouseToConsole()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);        
    }
    
    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);

        x += xoffset;
        if(x >= 80) x = 79;
        if(x < 0) x = 0;
        y += yoffset;
        if(y >= 25) y = 24;
        if(y < 0) y = 0;

        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);
    }
    
};

void sysprintf(char* str)
{
    asm volatile("int $0x80" : : "a" (4), "b" (str));
}

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

// Emin's Implementation
uint32_t fork();
void usr_main();
void strategy();
void collatz();
void print_collatz_sequence(int n);
void long_running_program();
int _long_running_program(int n);

extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("Hello kernel!\n");

    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);

    GlobalDescriptorTable gdt;
    TaskManager taskManager;    
    InterruptManager interrupts(0x20, &gdt, &taskManager);
    SyscallHandler syscalls(&interrupts, 0x80, &taskManager);
    
    //printf("\n\n\n\n");

    // Load first program
    // we don't need any syscall since we're in kernal right now
    // after adding a new task, it'll get scheduled after the first timer interrupt
    Task main(&gdt, usr_main);
    taskManager.AddTask(&main);
    interrupts.Activate();
    while (1)
    {
        ;
    }
    
}

// POSIX Interface

// Function to fork a new process
uint32_t fork() {
    uint32_t ebx = 65536;
    // Inline assembly to invoke the fork system call
    // "int $0x80" - Interrupt 0x80 is used to make a system call
    // "=b" (ebx) - Output: store the result in ebx register
    // "a" (FORK_INT) - Input: system call number for fork
    asm volatile("int $0x80" : "=b" (ebx) : "a" (FORK_INT));
    return ebx;
}

// Function to execute a new program
void execve(void func()) {
    uint32_t _func = (uint32_t)func;
    // No outputs
    // "a" (EXECVE_INT) - Input: system call number for execve
    // "b" (_func) - Input: address of the function to execute
    asm volatile("int $0x80" : : "a" (EXECVE_INT), "b" (_func));
}

// Function to wait for a process to change state
void waitpid(int pid, int* status) {
    int _status;
    // "=c" (_status) - Output: store the result in _status variable
    // "a" (WAITPID_INT) - Input: system call number for waitpid
    // "b" (pid) - Input: process ID to wait for
    asm volatile("int $0x80" : "=c" (_status): "a" (WAITPID_INT), "b" (pid));
    *status = _status;
    return;
}

// Function to terminate the calling process
void exit() {
    // No outputs
    // "a" (EXIT_INT) - Input: system call number for exit
    asm volatile("int $0x80" : : "a" (EXIT_INT));
}


// User main function that kernel loads first
void usr_main() {
    //printf("Enter usr_main\n");

    int pid = fork(); // fork a new process to execute given strategy
    if (pid == 0) {
        execve(strategy);
    }
    else {
        //printf("Enter parent process\n");
        // wait strategy process to terminate
        int status = -1;
        waitpid(pid, &status);
        if (status == -1)
        {
            printf("waitpid failed!");
        }
        else
            printf("Strategy is terminated!\n");
        
        printf("End of user main!\n");

        // issue a exit syscall so that this process
        // won't be scheduled anymore.
        exit(); 
    }  
}


void strategy() {
    //printf("Enter strategy\n");
    int collatz_pids[3];
    int lrp_pids[3];

    // fork collatz process 3 times
    for (int i = 0; i < 3; i++) {
        collatz_pids[i] = fork(); 
        if (collatz_pids[i] == 0) {
            collatz();
            //execve(collatz);
        }
    }

    // fork long_running_program 3 times
    for (int i = 0; i < 3; i++) {
        lrp_pids[i] = fork(); 
        if ((lrp_pids[i]) == 0){
            long_running_program();
            //execve(long_running_program);
        }
    }

    // wait all children processes to be terminated
    for (int i = 0; i < 3; i++) {
        int status;
        printf("waitpid: ");
        printInt(i);
        printf(", ");
        waitpid(collatz_pids[i], &status);
        if (status == -1)
        {
            printf("waitpid failed!\n");
        }
        else
            printf("collatz child is terminated\n");
    }
    for (int i = 0; i < 3; i++) {
        int status;
        printf("waitpid: ");
        printInt(i);
        printf(", ");
        waitpid(lrp_pids[i], &status);
        if (status == -1)
        {
            printf("waitpid failed!\n");
        }
        else
            printf("lrp child is terminated\n");
    }
    //printf("all children are terminated!\n");

    // issue a exit syscall so that this process
    // won't be scheduled anymore.
    exit();
}

void collatz() {
    //printf("Enter collatz\n");
    for (int i = 1; i < 100; ++i) {
        print_collatz_sequence(i);
    }

    // issue a exit syscall so that this process
    // won't be scheduled anymore.
    exit();
}

void print_collatz_sequence(int n) {
    /* printInt(n);
    printf(": "); */
    while (n != 1) {
/*         printInt(n);
        printf(", "); */
        if (n % 2 == 0) {
            n /= 2;
        } else {
            n = 3 * n + 1;
        }
    }
    /* printInt(1); */
}

int _long_running_program(int n) {
    int result = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            result += i * j;
        }
    }
    return result;
}

void long_running_program() {
    int result = _long_running_program(1000);
    //printf("LRP result: ");
    //printInt(result);
    //printf("\n");

    // issue a exit syscall so that this process
    // won't be scheduled anymore.
    exit();
}


