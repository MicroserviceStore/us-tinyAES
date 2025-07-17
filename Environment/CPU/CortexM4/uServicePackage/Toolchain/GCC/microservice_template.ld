
/* link with the standard c library */
INPUT(-lc)
/* link with the standard GCC library */
INPUT(-lgcc)
/* configure the entry point */
ENTRY(exec_entry_point)

/*
 * DO NOT CHANGE THESE VALUES
 */
__PREDEFINED_MICROSERVICE_FLASH_ORIGIN = 0;             /* Predefined Flash Offset Expected by Microservice Runtime */
__PREDEFINED_MICROSERVICE_RAM_ORIGIN   = 0x80000000;    /* Predefined RAM Offset Expected by Microservice Runtime */


/************************************************
 * CONFIGURATIONS
 ************************************************/

MEMORY
{
    code_section (rxai!w)    : ORIGIN = __PREDEFINED_MICROSERVICE_FLASH_ORIGIN,  LENGTH = MICROSERVICE_CODE_CAPACITY
    ram_section   (!rx)      : ORIGIN = __PREDEFINED_MICROSERVICE_RAM_ORIGIN,    LENGTH = MICROSERVICE_RAM_CAPACITY
}

SECTIONS
{    
    .text.metadata :
    {
        . = . + 256;
    } > code_section
    
    .init     :
    { 
        __init_start__ = .;
        KEEP(*(.init ))
        __init_end__ = .;
    } > code_section

    .text.boot :
    {
        . = ALIGN(4);    
        *(.text.entry)
        . = ALIGN(4);
    } > code_section

    .text :
    {
        . = ALIGN(4);
    
        *(.text)  
        *(.text.*)

        . = ALIGN(4);
    } > code_section

    .rodata :
    {
        . = ALIGN(4);
        
        *(.rodata)
        *(.rodata.*)

        . = ALIGN(4);
    } > code_section

    .stack_dummy (NOLOAD):
    {
        PROVIDE ( _sp_base = . );
        . = . + MICROSERVICE_MAINSTACK_SIZE;
        PROVIDE( _sp_main = . );
    } > ram_section

    .data : 
    {
        . = ALIGN(4);
        __ram_data_start__ = .;
        
        *(.data)
        *(.data.*)

        . = ALIGN(4);
        __ram_data_end__ = .;
    } > ram_section AT > code_section

    /* Microservice Runtime needs a GOT Section */
    .got : 
    {
        . = ALIGN(16);
        *(.got)
        . = ALIGN(4);
    } > ram_section AT > code_section

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        
        *(.bss*)
        *(.noinit_data*)
        *(COMMON)
 
        . = ALIGN(4);
        __bss_end__ = .;
    } > ram_section

    _end = .; PROVIDE (end = .);

    __StackTop = ADDR(.stack_dummy) + SIZEOF(.stack_dummy);
    __StackLimit = ADDR(.stack_dummy);

    PROVIDE( __ram_load_addr = LOADADDR(.data));

    /* Expose GOT Section Details */
    PROVIDE( __got_ro_offset = LOADADDR(.got));
    PROVIDE( __got_rw_offset = ADDR(.got));
    PROVIDE( __got_size = SIZEOF(.got));
 }
