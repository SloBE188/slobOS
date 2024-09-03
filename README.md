# slobOS

## Introduction

**slobOS** was a small, experimental 32-bit x86 operating system project that I developed as a learning exercise and exploration of low-level programming. It was designed to provide a basic yet functional environment with essential features commonly found in operating systems, serving as a foundation for further experimentation and development. However, this project has now been superseded by my new operating system, **[senob](https://github.com/SloBE188/senob)**, which builds upon the concepts and lessons learned from slobOS.

## Features

slobOS included a variety of key features, such as:

- **Bootloader**: A simple bootloader to load the kernel into memory and start execution.
- **GDT (Global Descriptor Table)**: Set up to manage different memory segments.
- **Interrupt Handling**: Basic interrupt management to handle hardware and software interrupts.
- **Heap Management**: A basic heap implementation for dynamic memory allocation.
- **Paging**: Memory paging for managing virtual memory.
- **Disk Driver**: Support for ATA disks using LBA (Logical Block Addressing).
- **FAT16 File System**: Implemented with a virtual file system (VFS) layer.
- **Process and Task Management**: Basic process management including task switching and scheduling.
- **Userland**: A simple user space environment to run user programs.
- **Syscalls**: System calls for interaction between user programs and the kernel.
- **Kernel Commands**: A few built-in kernel commands accessible via syscalls.
- **PS/2 Keyboard Driver**: Support for basic keyboard input.
- **ELF Loader**: Capable of loading and executing ELF (Executable and Linkable Format) binaries.
- **Shell**: A simple command-line interface where users can launch processes with arguments.
- **C Standard Library**: A minimal implementation of the C standard library.
- **Multitasking with PIT**: Simple multitasking using the Programmable Interval Timer (PIT).

## Deprecation Notice

As of now, slobOS is no longer under active development and has been officially replaced by **[senob](https://github.com/SloBE188/senob)**. While slobOS served as an important step in my journey of operating system development, senob represents a more mature, feature-rich, and efficient evolution of the ideas and concepts explored in slobOS.

## Repository

The source code for slobOS will remain available on GitHub for historical reference and educational purposes.

## License

This project is licensed under the MIT License, allowing for both personal and commercial use. See the `LICENSE` file for more details.

## Acknowledgments

slobOS was inspired by numerous resources in the field of operating system development, and I am grateful for the wealth of knowledge available through open-source projects and the community.

For those interested in building their own operating systems, I encourage you to check out **[senob](https://github.com/SloBE188/senob)**, where I continue to apply and expand upon the principles learned during the development of slobOS.

---

## Build slobOS
To build slobOS, follow these steps:

1. Clone the repository:
   ```sh
   git clone [https://github.com/SloBE188/senob.git](https://github.com/SloBE188/slobOS)
   cd slobOS
  
2. Build the operating system Senob
   ```sh
   ./build.sh
  
3. Running Senob
   ```sh
   cd bin
   qemu-system-x86 -chda os.bin
