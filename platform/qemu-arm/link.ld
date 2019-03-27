ENTRY(_start)
SECTIONS
{
	. = 0x8000;
	__kernel_start = .;
	.startup . : { startup.o(.text) }
	.text : {
		*(.text)
	}

	.rodata :
	AT (LOADADDR(.text) + (ADDR(.rodata) - ADDR(.text)))
	{
		_etext = .;
		*(.rodata)

                . = ALIGN(4);
		__circle_init_start = .;
		"*libcircle*.o*"(SORT_BY_INIT_PRIORITY(.init_array*))
		__circle_init_end = .;

		. = ALIGN(4);
		__init_array_start = .;
		*(SORT_BY_INIT_PRIORITY(.init_array.*))
		*(SORT_BY_INIT_PRIORITY(.ctors*))
		*(.init_array)
		__init_array_end = .;

		. = ALIGN(4);
		__fini_array_start = .;
		*(SORT_BY_INIT_PRIORITY(.fini_array.*))
		*(SORT_BY_INIT_PRIORITY(.dtors*))
		*(.fini_array)
		__fini_array_end = .;
	}

	.ARM.exidx : {
		__exidx_start = .;
		*(.ARM.exidx*)
		__exidx_end = .;
	}

	.data : { *(.data) }
	.bss : {
		__bss_start = .;
		*(.bss* COMMON)
		__bss_end = .;
	}
	__iomem_start = .;
	. = . + 0x100000000;
	__kernel_end = .;
}
