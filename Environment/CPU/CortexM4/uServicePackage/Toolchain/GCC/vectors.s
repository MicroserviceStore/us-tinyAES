
	.section .init
	.align 4
	.globl __isr_vector
	.global __StackTop
	.global __StackLimit
	.global reset_handler
	.global __got_ro_offset
	.global __got_rw_offset
	.global __got_size

__isr_vector:
	// (Main) Stack Details : WORD 0,1
	.long   __StackTop
	.long   __StackLimit

	// Container Entry Point : WORD 2
	.long   exec_entry_point
	
	// Global Offset Table Details : WORD 3,4,5
	// Linker Script shall have a GOT Section and shall provide these symbols
	.long   __got_ro_offset
	.long   __got_rw_offset
	.long   __got_size

    .size    __isr_vector, . - __isr_vector

    .text

    .end
