
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

void taskA()
{
    while(true)
        sysprintf("A");
}

void taskB()
{
    while(true)
        sysprintf("B");
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
    printf("Hello World! --- http://www.AlgorithMan.de\n");

    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
    
/*     printf("heap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8 ) & 0xFF);
    printfHex((heap      ) & 0xFF); */
    
    void* allocated = memoryManager.malloc(1024);
/*     printf("\nallocated: 0x");
    printfHex(((size_t)allocated >> 24) & 0xFF);
    printfHex(((size_t)allocated >> 16) & 0xFF);
    printfHex(((size_t)allocated >> 8 ) & 0xFF);
    printfHex(((size_t)allocated      ) & 0xFF);
    printf("\n"); */

    GlobalDescriptorTable gdt;
    TaskManager taskManager;    
    InterruptManager interrupts(0x20, &gdt, &taskManager);
    SyscallHandler syscalls(&interrupts, 0x80, &taskManager);
    
    /*
    printf("Initializing Hardware, Stage 1\n");
    
    DriverManager drvManager;
    
        #ifdef GRAPHICSMODE
            KeyboardDriver keyboard(&interrupts, &desktop);
        #else
            PrintfKeyboardEventHandler kbhandler;
            KeyboardDriver keyboard(&interrupts, &kbhandler);
        #endif
        drvManager.AddDriver(&keyboard);
        
    
        #ifdef GRAPHICSMODE
            MouseDriver mouse(&interrupts, &desktop);
        #else
            MouseToConsole mousehandler;
            MouseDriver mouse(&interrupts, &mousehandler);
        #endif
        drvManager.AddDriver(&mouse);
        
        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&drvManager, &interrupts);

        
    printf("Initializing Hardware, Stage 2\n");
    //drvManager.ActivateAll();
    */
    

    //printf("\n\n\n\n");

    // Initialize
    Task main(&gdt, usr_main);
    taskManager.AddTask(&main);
    interrupts.Activate();
    while (1)
    {
        ;//printf("kernel main");
    }
    
}

uint32_t fork() {
    uint32_t ebx = 65536;
    // : input list : output list
    asm volatile("int $0x80" : "=b" (ebx) : "a" (FORK_INT));
    //printf("*ebx after interrupt:");
/*     if (65536 == ebx)
        printf("ebx is unchanged");
    else if (ebx == 0)
        printf("ebx is zero: ");
    else
        printf("ebx is nonzero: "); */
/*     printInt(ebx);
    printf("\n"); */
    return ebx;
}

void execve(void func()) {
    uint32_t _func = (uint32_t)func;
    asm volatile("int $0x80" : : "a" (EXECVE_INT), "b" (_func));
}

void waitpid(int pid, int* status) {
    int _status;
    asm volatile("int $0x80" : "=c" (_status): "a" (WAITPID_INT), "b" (pid));
    *status = _status;
    return;
}

void exit() {
    asm volatile("int $0x80" : : "a" (EXIT_INT));
}

void usr_main() {
    printf("Enter usr_main\n");
    int pid = fork();
    if (pid == 0) {
        printf("Enter child forever loop\n");
        execve(strategy);
        //strategy();
    }
    else {
        printf("Enter parent process\n");
        int status = -1;
        waitpid(pid, &status);
        if (status == -1)
        {
            printf("waitpid failed!");
        }
        else
            printf("Strategy is terminated!\n");
        
        printf("End of user main!\n");
        exit();
    }  
}


void strategy() {
    printf("Enter strategy\n");
    int child_pids[6];
    for (int i = 0; i < 3; i++)
    {
        child_pids[i] = fork(); 
        if (child_pids[i] == 0){
            printf("Start collatz ");
            execve(collatz);
            //collatz();
            printf("exit collatz: ");
            printf("\n");
            exit();
        }
    }
    for (int i = 0; i < 3; i++)
    {
        child_pids[3+i] = fork(); 
        if ((child_pids[3+i]) == 0){
            printf("Start long_running_program ");
            printInt(i);
            printf("\n");
            execve(long_running_program);
            printf("exit long_running_program: ");
            exit();
        }
    }
    for (size_t i = 0; i < 6; i++)
    {
        int status;
        waitpid(child_pids[i], &status);
        if (status == -1)
        {
            printf("waitpid failed!\n");
        }
        else
            printf("child is terminated\n");
    }
    printf("all children are terminated!\n");
    exit();
}

void collatz() {
    //printf("Enter collatz\n");
    for (int i = 1; i < 100; ++i) {
        print_collatz_sequence(i);
    }
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
    printf("LRP result: ");
    printInt(result);
    printf("\n");
    exit();
    return;
}


