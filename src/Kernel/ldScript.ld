SECTIONS
{
	
	.text : {
		*(.text*)
	}
	.rodata : { *(.rodata*) }
	.data : { *(.data*) }
	.bss : { *(.bss) }
	_edata = .;
	_end = .;
	/DISCARD/ : 
	{
		*(.comment)
	}
}
