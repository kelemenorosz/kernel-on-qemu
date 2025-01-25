ENTRY (_start)

PHDRS {
	
	text PT_LOAD;
	data PT_LOAD; 
	virtual PT_LOAD;

}

SECTIONS
{

	. = 128K;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	} : text

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	} : text

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	} : data

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	} : data

	. = 192K;

	.virtual8086 BLOCK(4K) : ALIGN(4K)
	{
		*(.virtual8086)
	} : virtual

}
