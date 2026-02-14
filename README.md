# Sem Kernel: A simple x86 Educational Operating System

**Sem Kernel** (version 0.1) is a x86 operating system designed for educational purposes. It serves as a starting point for learning OS development, inspired on `os-tutorial` by cfenollosa, while introducing other features like a hierarchical filesystem, shell, user mode and a full-screen text editor.

---

## 🏗️ Technical Architecture

### 1. The Boot Process
The kernel starts in **16-bit Real Mode**. The bootloader (`boot/bootsect.asm`) handles the critical transition:
-   **Initialization**: Zeroes segment registers and sets up a temporary stack.
-   **Kernel Loading**: Uses BIOS `int 0x13` to load the kernel from disk to address `0x1000`.
-   **GDT Switch**: Loads the Global Descriptor Table and toggles the protection bit in `CR0` to enter **32-bit Protected Mode**.
-   **High-Level Entry**: Performs a far jump to the 32-bit kernel code, eventually calling `kernel_main`.

### 2. CPU & Interrupt Management
-   **GDT (Global Descriptor Table)**: Defines segments for Kernel Code/Data (Ring 0) and User Code/Data (Ring 3). Includes a **TSS (Task State Segment)** to handle stack switching during user-to-kernel transitions.
-   **IDT & ISRs**: A robust interrupt system.
    -   **Exceptions (0-31)**: Standard CPU faults (division by zero, page faults, etc.).
    -   **IRQs (32-47)**: Hardware interrupts (Timer, Keyboard). The PIC is remapped to avoid collisions with exceptions.
    -   **Syscalls (0x80)**: A gateway for user-mode programs to request kernel services.
-   **Reliability**: Unlike basic tutorials, this kernel passes register pointers to handlers, preventing stack corruption and ensuring ABI compliance.

### 3. Memory Management
-   **Paging**: Implements identity mapping for the first 4MB of RAM. This provides a stable virtual address space where virtual addresses equal physical addresses, forming the foundation for future isolation.
-   **Heap Allocation**: A custom `kmalloc` (bump allocator) provides dynamic memory. It supports page alignment, which is essential for creating new page tables or DMA buffers.

### 4. Hardware Drivers
-   **VGA Video**: A sophisticated driver supporting:
    -   Direct memory mapping at `0xb8000`.
    -   Vertical scrolling and cursor tracking.
    -   Color attributes for different log levels (Info, Error, Shell prompt).
    -   **Backspace Protection**: Prevents users from deleting the prompt or kernel logs.
-   **Keyboard**: A buffered driver with:
    -   Scancode translation to ASCII.
    -   Full support for **Shift** (uppercase and symbols).
    -   Interrupt-driven input that populates a `key_buffer`.

### 5. Hierarchical RAM Filesystem
Since a disk driver is complex for beginners, Sem Kernel implements a **RAM Disk** with features found in real filesystems:
-   **Nested Directories**: Support for `/home/user` style structures.
-   **Full CRUD**: Create, Read, Update, and Delete operations for both files and directories.
-   **Directory Listing**: Tracks parent/child relationships to allow navigation.

---

## 🐚 The Shell & Built-in Tools

The shell provides a Linux-like CLI experience. By default, it starts in `/home/user`.

### Available Commands:
-   `ls`: List contents of the current directory.
-   `cd <dir>`: Change directory (supports `/` for root).
-   `mkdir <name>`: Create a new directory.
-   `touch <name>`: Create a new empty file.
-   `rm <name>`: Delete a file or directory.
-   `cat <file>`: Display the contents of a file.
-   `edit <file>`: Open the **Nano-lite Text Editor**.
-   `user`: Demonstration of switching to **User Mode (Ring 3)**.
-   `clear`: Clear the screen.
-   `help`: Show available commands.
-   `exit`: Shutdown the system (via ACPI).

### Nano-lite Editor
A full-screen interactive editor:
-   Supports multi-line input.
-   Real-time status bar at the bottom.
-   Commands: `Ctrl+X` (Quit).

---

## 🚀 How to Build and Run

### Prerequisites
You need an i386 cross-compiler and the following tools:
-   `nasm`
-   `i386-elf-gcc`
-   `qemu-system-i386`

### Usage
```bash
# Build the OS image and launch in QEMU
make run

# Clean build artifacts
make clean

# Launch with GDB debugging enabled
make debug
```

---

## 📂 Project Organization

-   `boot/`: Assembly bootloader and kernel entry stub.
-   `cpu/`: GDT, IDT, Paging, and Syscall logic.
-   `drivers/`: VGA, Keyboard, and Port I/O.
-   `fs/`: RAM Filesystem implementation.
-   `kernel/`: Shell, Editor, and main initialization.
-   `libc/`: String manipulation and memory utilities.
-   `docs/`: Tutorial documentation ([tutorial.pdf](docs/tutorial.pdf)).

---
*Sem Kernel is built for learning.
