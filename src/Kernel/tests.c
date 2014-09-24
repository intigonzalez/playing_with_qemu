#include <system_calls.h>
#include <calls.h>

#include "kerneltasks.h"


typedef struct
{
	int offset;
	int len;
	char name[100];
} Entrada,*pEntrada;

static void leerDiscoRam1(void * addr)
{
	unsigned* n = (unsigned *)addr;
	
	pEntrada entry = (pEntrada)&n[1];
	printf("NOMBRE    OFFSET     SIZE\n");
	printf("=========================\n");
	for (int i = 0 ; i < n[0] ; i++)
		
		printf("%s      %d       %d\n", entry[i].name, entry[i].offset, entry[i].len);
	printf("=========================\n");
}

static void leerHastaEnter(char* buff)
{
	int i = 0;
	i = read(0,buff,100);
	buff[i] = 0;
}

/**
 * \brief Un hilo para probar elementos del sistema
 * \param utcb
 */
void Test_Thread(void * utcb)
{
	char buff[101];
	
	//leerDiscoRam1(_bootInfo.ramdisk1);
	
	while (1)
	{
		printf("root@ubuntu # ");
		//printf("Direccion : %x\n",(unsigned)&PassToKernelMode);
		//leerHastaEnter(buff);
		PassToKernelMode((unsigned)&EtiquetaDeRetorno);
	}
}

// Pila de este hilo
static char pila[1024];
// Area utcb de este hilo
static char utcb[UTCB_AREA_SIZE];

/**
 * \brief Comenzamos el hilo de pruebas
 */
void Init_Test_Thread()
{
	ThreadControl(&_test_thread,&pila[1024],&utcb[0]);
}
