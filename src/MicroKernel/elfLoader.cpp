
#include "elfLoader.h"
#include "ThreadControl.h"
#include "memory.h"

#pragma pack(push,1)

typedef struct _ELF_HEADER {
    unsigned char ident[16];
    unsigned short type; // tipo de archivo
    unsigned short machine; // maquina objetivo
    unsigned int version; // normalmente 1
    unsigned int entry_point; // punto de entrada al programa
    unsigned int programHeaderOffset; // lugar donde esta la tabla del programa
    unsigned int sectionHeaderOffset;
    unsigned int flags; // banderas
    unsigned short ehsize; // el size de esta tabla
    unsigned short phentrySize; // el size de una entrada en la tabla de programas
    unsigned short phCount; // numero de entradas en esa tabla
    unsigned short shentrySize; // igual que arriba pero para la otra tabla
    unsigned short shCount; // igual
    unsigned short shsrndx; // no hace falta, no te sulfates
} ELF_HEADER,*PELF_HEADER;

typedef struct _PROGRAM_HEADER
{
    unsigned int type;
    unsigned int offset; // donde esta en el archivo
    unsigned int vaddr; // donde va en la memoria
    unsigned int paddr; //
    unsigned int fsize; // tamaño en el archivo
    unsigned int msize; // tamaño en la memoria
    unsigned int flags; // algunos atributos
    unsigned int aling; // alineacion que debe tener la seccion
} PROGRAM_HEADER,*PPROGRAM_HEADER;

#pragma pack(pop)


/*
 * Proptotipo de funcion main
 */

typedef int (*MAIN)(int argc,char* argv[]);


/**
 * Esta funcion carga el servidor raiz, con un hilo y en un espacio de direcciones
 * @param fileAddrStart
 */
void LoadRootServer(void* fileAddrStart)
{

    threadControl.InitThreadTable();

    // creo el espacio de direccion inicial
    // este comienza en USER_BASE_ADDRESS
    // Debo definir un directorio de paginas para el servicio raiz
    int* kernelPD = (int*)_hwParameters.kernel_page_directory;
	int* rootPD = (int*) (AllocPage(kAllocator) - (MICROKERNEL_ADDRESS));
	printf("Address root page directory %x\n",rootPD);
	/*
	 * El hilo raiz esta compilado con direccion 0 como base
	 * Sin embargo no lo puedo poner en la direccion fisica 0, tampoco puedo ponerlo en la direccion
	 * logica 0 utilizando la direccion base de lo segmentos para trasldarlo porque entonces no es 
	 * un modelo FLAT y las instrucciones sysenter y sysexit no funcionan (nosotros queremos llamadas rapidas)
	 * La solucione es hacer un mapeo con la memoria de la siguiente forma:
	 * La direccion logica 0 del programa se mapea a KERNEL_BASE_ADDRESS y las direcciones fisicas de 0 hacia adelante se mapean
	 * en 4080  mb logicos, hay que tener en cuenta que a partir de los 3 GB los descriptores son iguales
	 * a los del microkernel
	 */
	for (int i = 0 ; i < 1024 ; i++)
		rootPD[i] = kernelPD[i];
	int npaginas = _hwParameters.memoryCount/4; // es el numero de paginas
	int entradas = npaginas / 1024 - 3;
	printf("Numero de entradas a copiar son %d\n",entradas);
	for (int i = 0 ; i < entradas ; i++)
		rootPD[i] = kernelPD[i + 3];
	for (int i = entradas ; i < 1024 ; i++) // las que sobran no existen
		rootPD[i] = 0;
	for (int i = 768 ; i <= 770 ; i++)
		rootPD[i] = kernelPD[i];
	
	
	// el hilo raiz vera en la posicion 4080 mb la memoria fisica que empieza en 0
	rootPD[1020] = kernelPD[0];
	rootPD[1021] = kernelPD[1];
	rootPD[1022] = kernelPD[2];
	
	
    // copio la imagen para el espacio de direcciones
    PELF_HEADER header = (PELF_HEADER)fileAddrStart;
    int start = (int) fileAddrStart;
    int programEntries = header->phCount;

    int p = start + header->programHeaderOffset; // la primera entrada en esta tabla

    unsigned esp = 0;
    
    for (int i = 0 ; i < programEntries; i++)
    {
        PPROGRAM_HEADER pHeader = (PPROGRAM_HEADER)p;
        if (pHeader->type == 1)
        {
            // es un pedazo que debo cargar 
            
            // copio para el lugar correcto
            char* src = (char*)(start + pHeader->offset);
            char* dst = (char*)(KERNEL_BASE_ADDRESS + pHeader->vaddr);

            if (((unsigned)dst + pHeader->msize + 1) > esp)
                esp = (unsigned)dst + pHeader->msize + 1;
            
            int count = pHeader->fsize;
			// mostramos informaciones
            printf("V. Address: %x\n",pHeader->vaddr);
            int j ;
            for (j = 0 ; j < count ; j++)
                dst[j] = src[j];

            while (j < pHeader->msize)
                dst[j++] = 0;
        }
        p += header->phentrySize; // voy a la siguiente entrada en la tabla
    }
	printf("Entry Point : %x\n",header->entry_point);
    // TODO: Esto del utcb del root server es pura mierda como se encuentra implementado
	esp += 64 + 4 + sizeof(BootInfo) + 4; // le damos 64 bytes de pila al root server, solo la usara un instante
	unsigned utcb = esp; // vamos a darle algo de utcb
	
	int * pointer= (int *)(esp - 4 - sizeof(BootInfo)-4);
	*pointer = sizeof(BootInfo)/4 + 1;
	pBootInfo pbi = (pBootInfo)(esp - sizeof(BootInfo)-4);
	
	pbi->hardwareMemory = (void*)((4080)*1024*1024);
	pbi->videoMemory = (void*)((4080)*1024*1024 + 0xb8000);
	pbi->memory = _hwParameters.memoryCount;
	
	pbi->ramdisk1 = (void*)((unsigned)(_hwParameters.modules[1]) - KERNEL_BASE_ADDRESS);
	//pbi->ramdisk2 = (void*)((unsigned)(_hwParameters.modules[2])/* - KERNEL_BASE_ADDRESS*/);
	
	esp = esp - (4 + sizeof(BootInfo) + 4);
	
    // creamos el nuevo hilo con su espacio de direcciones
	// TODO : Arreglar el valor que se le pasa a UTCB
    threadControl.CreateThread(header->entry_point, esp-KERNEL_BASE_ADDRESS, esp/*-KERNEL_BASE_ADDRESS*/,(unsigned)rootPD);
}


