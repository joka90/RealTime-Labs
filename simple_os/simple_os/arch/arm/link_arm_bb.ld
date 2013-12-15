ENTRY(start)
SECTIONS
{
 . = 0x80000000; 
 .startup . : { obj/startup_arm_bb.o(.text) }
 .text : { *(.text) }
 .data : { *(.data) }
 .bss : { *(.bss) }
 . = . + 0x5000; /* 4kB of stack memory */
 stack_bottom = .;
}
