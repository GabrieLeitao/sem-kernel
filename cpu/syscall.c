#include "syscall.h"
#include "../drivers/screen.h"
#include "../kernel/shell.h"

static void syscall_handler(registers_t *regs) {
    // Syscall number in EAX
    // Arguments in EBX, ECX, EDX, etc.
    uint32_t syscall_num = regs->eax;

    if (syscall_num == 0) { // print
        kprint((char*)regs->ebx);
    } else if (syscall_num == 1) { // exit
        // To return to kernel mode from here, we need to restore the kernel state.
        // For this simple example, we can use a longjmp-like approach or just
        // rely on the fact that we're in a syscall handler (Ring 0).
        // A proper 'exit' would terminate the process and return to the scheduler.
        // Since we don't have a scheduler yet, we'll just print a message.
        kprint("\nUser Mode process requested exit.\n");
        shell_print_prompt();
    }
}

void init_syscalls() {
    register_interrupt_handler(0x80, syscall_handler);
}
