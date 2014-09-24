#include "video.h"
#include "memory.h"
#include "BasicMachine.h"

extern unsigned _end;
// Este es el directorio de paginas del kernel
static unsigned char page_directory[4096*2];
// He aqui el mitico heap, modesto y con necesidades de muchos arreglos, pero funcional
static Heap _kHeap;
// Este no es mas que un espejo del Heap, para las masas
pHeap kHeap;

static PageAllocator _kAllocator;
pPageAllocator kAllocator;

/**
 * \brief Esta es la operacion para tomar memoria
 * \param t
 */
void* operator new(size_t t)
{
	return kmalloc(kHeap,t);
}

/**
 * \brief Esta es la operacion para tomar memoria
 * \param t
 */
void* operator new[](size_t t)
{
	return kmalloc(kHeap,t);
}

/**
 * \brief 
 * \param p
 */
void operator delete(void *p)
{
    kfree(kHeap,p);
}
 
/**
 * \brief 
 * \param p
 */
void operator delete[](void *p)
{
    kfree(kHeap,p);
}

/**
 * Activa la paginacion en el sistema
 * @param hwParameters - Son los parametros de hardware, solo se usa la cantidad de memoria
 * @return La direccion del fin de la estructura de paginas usada por el kernel
 */
static void init32bitPagingMode(PHARDWARE_PARAMETERS hwParameters)
{
	int* tmp = (int*)hwParameters->kernel_page_directory;
    hwParameters->kernel_page_directory = (((unsigned int)&page_directory[0]) & 0xFFFFF000) + 4096;
	int * base = (int*)(hwParameters->kernel_page_directory);
	for(int j=0; j<1024; j++)
		base[j] = tmp[j];
		
	base[1020] = base[0];
	base[1021] = base[1];
	base[1022] = base[2];
	unsigned xxx = (unsigned)base - MICROKERNEL_ADDRESS + MICROKERNEL_BASE_ADDRESS;
	
	// esto tiene que ser asi porque lo usa el ensamblador
	hwParameters->kernel_page_directory = xxx;
	// activo la paginacion
	__asm__("movl %0,%%cr3;"::"r"(xxx):);
}

/**
 * Esta funcion es la responzable de inicializar el sistema para la gestion de 
 * memoria en el microkernel
 * - Inicia el sistema de paginación
 * - Crea los heap del microkernel para poder usar memoria virtual
 * */
void InitMemory(PHARDWARE_PARAMETERS hwParameters)
{
	// activamos la paginación
	init32bitPagingMode(hwParameters);
	
	// ahora el Heap 1 (todo tipo de estructuras)
	kHeap = & _kHeap;
	
	unsigned temp = (unsigned)&_end; // esta es la direccion final de la imagen del microkernel, se declara en ldScript.ld
	temp = (temp & 0xFFFFFFFFD) + 4;
	unsigned size = MICROKERNEL_ADDRESS + 0x100000 - temp;
	printf("\nEl Heap 1 del kernel es de:%d kb y %d bytes\n",size / 1024, size % 1024);
	// estoy diciendo que el heap empieza a paritr de los 512 kb despues de la direccion del microkernel
	CreateHeap(kHeap, (void*)temp ,(void*)(MICROKERNEL_ADDRESS + 0x100000),1024);
	// ahora el Heap 2 (para solicitar paginas completas)
	kAllocator = & _kAllocator;
	CreatePageAllocator(kAllocator,(void*)(MICROKERNEL_ADDRESS + MICROKERNEL_HEAP2 ),(void*)(MICROKERNEL_ADDRESS + KERNEL_BASE_ADDRESS));
}
