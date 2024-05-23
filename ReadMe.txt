
# Process Management System

## Introduction

The Process Management System is a C++ project designed to implement process management functionalities in an operating system environment. The system includes components such as a Task Manager, Syscall Handler, and Process Control Block (PCB) struct to manage processes efficiently.

## Requirements

### Dependencies
- g++ (GNU Compiler Collection)
- binutils
- libc6-dev-i386
- VirtualBox (for running the operating system as a virtual machine)
- grub-legacy
- xorriso

### Installation (for Ubuntu)
```bash
sudo apt-get install g++ binutils libc6-dev-i386
sudo apt-get install VirtualBox grub-legacy xorriso
```

## Usage

### Building the Kernel
To build the kernel, simply run:
```bash
make
```

### Running the Kernel
To run the kernel in a VirtualBox virtual machine, use the following command:
```bash
make run
```

### Installing the Kernel (Optional)
To install the kernel on the host system, use the following command:
```bash
sudo make install
```

## Makefile

The provided Makefile includes the following targets:

- `run`: Cleans the project, builds the kernel, and starts the operating system in a VirtualBox virtual machine.
- `install`: Installs the kernel on the host system (optional).
- `clean`: Cleans the project by removing object files, the kernel binary, and the ISO image.

## Directory Structure

- `src/`: Contains the source files (*.cpp, *.s) for implementing the project.
- `include/`: Contains the header files (*.h) for defining classes and structs.
- `obj/`: Contains the object files (*.o) generated during the build process.
- `mykernel.iso`: The bootable ISO image of the operating system.
- `linker.ld`: The linker script used for linking object files and generating the kernel binary.

For more details on the Makefile targets and directory structure, refer to the comments in the Makefile itself.

## Author

Muhammet Emin Akgün
