

#ifndef _TSS_
#define _TSS_


#pragma pack(push,1)

typedef struct _hoho{
	unsigned long previus_task;
	unsigned long sp0;		// stack pointer 0
	unsigned long ss0;		// stack selector 0
	unsigned long sp1;		// stack pointer 1
	unsigned long ss1;		// stack selector 1
	unsigned long sp2;		// stack pointer 2
	unsigned long ss2;		// stack selector 2
	unsigned long cr3;		// apunta al directorio de paginas del proceso
	unsigned long eip;		// puntero de instrucciones
	unsigned long eflags;
	unsigned long eax;
	unsigned long ecx;
	unsigned long edx;
	unsigned long ebx;
	unsigned long esp;
	unsigned long ebp;
	unsigned long esi;
	unsigned long edi;
	unsigned long es;
	unsigned long cs;
	unsigned long ss;
	unsigned long ds;
	unsigned long fs;
	unsigned long gs;
	unsigned long ldt_selector;
	unsigned long iomap;
} TaskStateSegment;

#pragma pack(pop)

#endif
