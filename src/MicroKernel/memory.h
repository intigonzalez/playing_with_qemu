#ifndef __MEMORY_STUFF__
#define __MEMORY_STUFF__

/*
 * Autor : Inti (no faltaba mas)
 * 
 * bien este archivo tiene los relacionado con la inicializacion y gestion de 
 * la memoria en el sistema
 * 
 * */

#include "includes/global.h"
#include "kernel.h"

#include "mKMM.h"

#define MICROKERNEL_BASE_ADDRESS				0x400000		// 4 mb
#define MICROKERNEL_ADDRESS						0xC0000000		// 3 gb
#define KERNEL_PAGE_TABLE_ADDRESS				0x500000		// 5 mb
#define MICROKERNEL_HEAP2								0x900000		// 9 mb
// la direccion base de los segmentos de usuario es 12 mb (debe ser multiplo de 4)
#define KERNEL_BASE_ADDRESS							0xC00000		// 12 mb
#define USER_BASE_ADDRESS  							0xC00000		// 12 mb


extern pHeap kHeap;
extern pPageAllocator kAllocator;

/**
 * Esta funcion es la responzable de inicializar el sistema para la gestion de 
 * memoria en el microkernel
 * - Inicia el sistema de paginación
 * - Crea los heap del microkernel para poder usar memoria virtual
 * */
void InitMemory(PHARDWARE_PARAMETERS hwParameters);

#endif