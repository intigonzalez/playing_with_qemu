#pragma pack (push, 1)

typedef struct _MODULE_ENTRY {
    unsigned int moduleStart;
    unsigned int moduleEnd;
    char string[8];

} MODULE_ENTRY, *PMODULE_ENTRY;

typedef struct _DRIVE_ENTRY {
    unsigned int size; //size of structure
    unsigned char driveNumber;
    unsigned char driveMode;
    unsigned short driveCylinders;
    unsigned char driveHeads;
    unsigned char driveSectors;
    unsigned char ports [0]; //can be any number of elements

} DRIVE_ENTRY, *PDRIVE_ENTRY;

typedef struct _MULTIBOOT_INFO {
    unsigned int flags; //required
    unsigned int memLower; //if bit 0 in flags are set
    unsigned int memUpper; //if bit 0 in flags are set
    unsigned int bootDevice; //if bit 1 in flags are set
    unsigned int commandLine; //if bit 2 in flags are set
    unsigned int moduleCount; //if bit 3 in flags are set
    PMODULE_ENTRY moduleAddress; //if bit 3 in flags are set
    unsigned int syms[4]; //if bits 4 or 5 in flags are set
    unsigned int memMapLength; //if bit 6 in flags is set
    unsigned int memMapAddress; //if bit 6 in flags is set
    unsigned int drivesLength; //if bit 7 in flags is set
    PDRIVE_ENTRY drivesAddress; //if bit 7 in flags is set
    unsigned int configTable; //if bit 8 in flags is set
    unsigned int apmTable; //if bit 9 in flags is set
    unsigned int vbeControlInfo; //if bit 10 in flags is set
    unsigned int vbeModeInfo; //if bit 11 in flags is set
    unsigned int vbeMode; // all vbe_* set if bit 12 in flags are set
    unsigned int vbeInterfaceSeg;
    unsigned int vbeInterfaceOff;
    unsigned int vbeInterfaceLength;

} MULTIBOOT_INFO, *PMULTIBOOT_INFO;

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

#define MICROKERNEL_BASE_ADDRESS				0x400000		// 4 mb
#define KERNEL_PAGE_TABLE_ADDRESS				0x500000		// 5 mb

// LAS CONSTANTES DE INICIALIZACION DE LA MEMORIA EN EL KERNEL
#define GLOBAL_PAGE	0x100
#define PAGE_ACCESSED	0x20
#define USER_PAGE	0x04
#define READ_WRITE_PAGE	0x02
#define PAGE_DIRTY	0x40
#define PAGE_PRESENT	0x01

PMULTIBOOT_INFO pMultibootInfo;
// Este es el directorio de paginas del kernel
static unsigned char page_directory[4096*2];

extern "C" {
	void active32bit_pagination(void* page_directory);
}

/**
 * \brief GRUB carga los modulos a continuacion de la imagen del kernel o en 
 * general donde quiere, no es aceptable dejarlos en un lugar inadecuado.
 * Esta funcion las mueve hacia una direccion especificada.
 */
static void MoveModule(unsigned addr, unsigned module, unsigned* root_begin, unsigned* root_end)
{
	char * base = (char*) addr;
	
	char * start = (char*) pMultibootInfo->moduleAddress[module].moduleStart;
	char * end = (char*) pMultibootInfo->moduleAddress[module].moduleEnd;
	int count = (int)(end - start);
	
	*root_begin = addr;
	*root_end = addr + count;
	
	// ahora copio
	int iDst = 0;
	for (int iSrc = 0; iSrc < count ; iSrc++,iDst++)
		base[iDst] = start[iSrc];
}



/**
 * Activa la paginacion en el sistema
 * @param hwParameters - Son los parametros de hardware, solo se usa la cantidad de memoria
 * @return La direccion del fin de la estructura de paginas usada por el kernel
 */
static int* init32bitPagingMode()
{
    unsigned int kernel_page_directory = (((unsigned int)&page_directory[0]) & 0xFFFFF000) + 4096;

    int * base = (int*)(kernel_page_directory);
    /*
     * Necesitamos por fuerza un directorio de pagina
     * Debemos calcular cuantas entradas del directorio de pagina
     * se usaran:
     *  Numero Entradas = (MemoriaEnMB)/4Mb
     * El directorio de pagina necesita de 4kb y cada entrada que se use 4kb
     * La cantidad total de memoria a usar sera:
     *  Cantidad a Usar = (1 + Numero Entradas) * 4096 
     */
    int NumeroEntradas = (pMultibootInfo->memUpper/1024)/4;
    //printf("Numero de entradas es : %d\n",NumeroEntradas);
    unsigned int addr = KERNEL_PAGE_TABLE_ADDRESS; // la primera tabla de paginas va aqui
    unsigned int realAddr = 0;
	unsigned int * pageTable;
    for (int i = 0 ; i < NumeroEntradas ; i++)
    {
        // recorro las 1024 entradas de la tabla de paginas
        base[i] = (addr) | (PAGE_PRESENT | READ_WRITE_PAGE | USER_PAGE);
		pageTable = (unsigned int*) addr;
        for (int j = 0 ; j < 1024 ; j++) 
        {
            pageTable[j] = realAddr | (PAGE_PRESENT | READ_WRITE_PAGE | USER_PAGE);
            realAddr += 4096;
        }
        //printf("Directorio de pagina %d terminado\n",i);
        addr += 4096;
    }
    // las entradas que sobran las marco como no presentes
    for (int i = NumeroEntradas ; i < 1024 ; i++)
        base[i] = 0; // paginas no presentes

	// ahora a partir de los 3 Gigabytes debo poner 4 MB a apuntar a los 4 megas de la memoria real
	realAddr = 0x400000;
	for (int i = 768 ; i <= 770 ; i++)
    {
        // recorro las 1024 entradas de la tabla de paginas
        base[i] = (addr) | (PAGE_PRESENT | READ_WRITE_PAGE | USER_PAGE);
		pageTable = (unsigned int*) addr;
        for (int j = 0 ; j < 1024 ; j++) 
        {
            pageTable[j] = realAddr | (PAGE_PRESENT | READ_WRITE_PAGE | USER_PAGE);
            realAddr += 4096;
        }
        //printf("Directorio de pagina %d terminado\n",i);
        addr += 4096;
    }
	
	for (int i = 0 ; i < 3 ; i++)
		base[1020 + i] = base[i];

    // activo la paginacion
    active32bit_pagination(base);

    return base;
}

unsigned LoadMicrokernel(unsigned fileAddrStart)
{
    // copio la imagen para el espacio de direcciones
    PELF_HEADER header = (PELF_HEADER)fileAddrStart;
    int start = (int) fileAddrStart;
    int programEntries = header->phCount;

    int p = start + header->programHeaderOffset; // la primera entrada en esta tabla

    for (int i = 0 ; i < programEntries; i++)
    {
        PPROGRAM_HEADER pHeader = (PPROGRAM_HEADER)p;
        if (pHeader->type == 1)
        {
            // es un pedazo que debo cargar
            // copio para el lugar correcto
            char* src = (char*)(start + pHeader->offset);
            char* dst = (char*)(pHeader->vaddr);
            
            int count = pHeader->fsize;
            int j ;
            for (j = 0 ; j < count ; j++)
                dst[j] = src[j];

            while (j < pHeader->msize)
                dst[j++] = 0;
        }
        p += header->phentrySize; // voy a la siguiente entrada en la tabla
    }
	return header->entry_point;
}

/**
 * El cargardor es un ejecutable multiboot que espera lo siguiente
 * 
 * El primer modulo sera tratado como el kernel, este kernel va a ser cargado en la posicion 4 MB de la memoria real,
 * y se vera a partir de los 3 GB de la memoria logica, este kernel no puede usar direccion prefijadas de mas de 4 MB.
 * 
 * El segundo modulo va a ser el root server del sistema operativo, es como quiera
 * 
 * El resto de los modulos es cualquier cosa que quieras
 */
int main(int argc, char **argv)
{
	
	// activamos la paginacion y creamos una tabla de paginas que permita cargar el microkernel a partir de 3GB
	int* addr_page_directory = init32bitPagingMode();
	
	unsigned addr1,addr2,addr3,addr4;
	// copio el microkernel para la posicion 20 mb
	MoveModule(20*1024*1024, 0, &addr1,&addr2); 
	MoveModule(addr2+2,1,&addr2,&addr3);
	MoveModule(addr3+2,2,&addr3,&addr4);
	
	// ahora vamos a lo de verdad, es decir, a saltar al kernel
	unsigned int entry_point = LoadMicrokernel(addr1);
	unsigned int memory = pMultibootInfo->memUpper;
	unsigned int addrOS = addr2;
	unsigned int addrRD = addr3;
	__asm__("push %1; \
			 push %2; \
			 push %3; \
			 push %4; \
			 jmp %0;"
			:
			:"r"(entry_point),"r"(addrRD),"r"(addrOS),"r"(memory),"r"(addr_page_directory)
			:);

	while(1);

	return 0;
}
