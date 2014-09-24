#ifndef KERNEL_H_
#define KERNEL_H_


#include <system_calls.h>

typedef unsigned int size_t;


typedef struct _HARDWARE_PARAMETERS
{
    size_t memoryCount;     // offset 0

    unsigned kernel_page_directory; // offset 4 
		
	void* modules[3];

} HARDWARE_PARAMETERS, *PHARDWARE_PARAMETERS;

extern "C"
{
	void lock_interrupts();
	void unlock_interrupts();

	void set_new_gdt(void * gdt_desc);
	void set_new_idt(void * idt_desc);
	void set_new_tss(int tss_descriptor);

	void low_level_sys_call();

	void interrupt_test();
	
	void idle_thread_entry();

	void activate_multitasking();
	void active32bit_pagination(void* page_directory);
	void FastSysCall();

	//void _restore1();
	
	void low_level_clock_handler();
	void high_level_handler(unsigned number);
	void high_level_handler_faults(unsigned number, unsigned cs, unsigned eip, unsigned error_code);
	void high_level_clock_handler();
	void microkernel_syscall(unsigned type);
	
	void RegistreThreadToInterrupt(const unsigned char interrupt, TID thread);
	int DoThreadInterrupt(const unsigned char value);
	
	void printf2(const char* format, int* first);
	
	void low_level_0();
	void low_level_1();
	void low_level_2();
	void low_level_3();
	void low_level_4();
	void low_level_5();
	void low_level_6();
	void low_level_7();
	void low_level_8();
	void low_level_9();
	void low_level_10();
	void low_level_11();
	void low_level_12();
	void low_level_13();
	void low_level_14();
	void low_level_15();
	void low_level_16();
	void low_level_17();
	void low_level_18();
	void low_level_19();
	
	void* GetBasicHandler(unsigned number);
	
	
	void _save();
	void _restore();
	void _dir1();
}


void do_test();


extern HARDWARE_PARAMETERS _hwParameters;

#define PAGE_FAULT          14
#define GENERAL_PROTECTION  13

// LAS CONSTANTES DE INICIALIZACION DE LA MEMORIA EN EL KERNEL
#define GLOBAL_PAGE	0x100
#define PAGE_ACCESSED	0x20
#define USER_PAGE	0x04
#define READ_WRITE_PAGE	0x02
#define PAGE_DIRTY	0x40
#define PAGE_PRESENT	0x01

// LAS CONSTANTES GLOBALES DEL KERNEL
#define KERNEL_CODE_SELECTOR 0x08 // 1, GDT , RPL = 0
#define KERNEL_DATA_SELECTOR 0x10	// 1, GDT , RPL = 0
#define USER_CODE_SELECTOR	0x1B	// 3, GDT , RPL = 3
#define USER_DATA_SELECTOR	0x23	// 4, GDT , RPL = 3
#define TSS_SELECTOR		0x2B			// 5, GDT , RPL = 3 
#define UTCB_SEGMENT		0x33			// 6, GDT , RPL = 3
#define IDLE_CODE_SELECTOR 0x39		// 7, GDT , RPL = 1
#define IDLE_DATA_SELECTOR 0x41		// 8, GDT , RPL = 1

#define IRQ0_VECTOR		0x30 
#define IRQ8_VECTOR		0x38
#define SYS_CALL_VECTOR	0x64	// 100 en decimal

// identifica los numeros de las interrupciones
#define CLOCK_IRQ          0
#define KEYBOARD_IRQ       1
#define CASCADE_IRQ        2	/* cascade enable for 2nd AT controller */
#define ETHER_IRQ          3	/* default ethernet interrupt vector */
#define SECONDARY_IRQ      3	/* RS232 interrupt vector for port 2 */
#define RS232_IRQ          4	/* RS232 interrupt vector for port 1 */
#define XT_WINI_IRQ        5	/* xt winchester */
#define FLOPPY_IRQ         6	/* floppy disk */
#define PRINTER_IRQ        7
#define AT_WINI_IRQ       14	/* at winchester */


#define HZ 60


#endif /*KERNEL_H_*/
