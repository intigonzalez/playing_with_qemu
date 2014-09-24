#include "video.h"
#include "mKMM.h"


#define MAGIC_NUMBER	0xFEFAFEA1

#define FOOTER(X)	((pblock_footer)((uint)X+X->size-sizeof(block_footer)))
#define HEADER(X) ((pblock_header)((uint)X-sizeof(block_header)))
#define BLOCKSIZE(X) (X->size - sizeof(block_footer) - sizeof(block_header))
#define USEFUL_ADDRESS(X) ((uint)X+sizeof(block_header))
#define NEXT_HEADER(X) ((pblock_header)((uint)X + X->size))
#define PREVIOUS_FOOTER(X) ((pblock_footer)((uint)X-sizeof(block_footer)) )
#define IS_VALID(H,B) (H->realInit<=B && B<H->endAddress)

typedef struct 
{
	uint	magic;
	bool	hole;
	size_t size;
} block_header, *pblock_header;

typedef struct
{
	uint magic;
	pblock_header myHeader;
} block_footer, *pblock_footer;

static bool BloqueMasChiquito(void* b1, void* b2)
{
	return ((pblock_header)b1)->size < ((pblock_header)b2)->size;
}

// solo para DEBUG
static void PrintBlockInfo(pblock_header h)
{
	printf("Header %x", h);
	printf(" Size %x", h->size);
	printf(" Magic %d", h->magic == MAGIC_NUMBER);
	printf(" Hole %d\n", h->hole);
}

// solo para debug
static void printAllInfo(pHeap heap)
{
	pblock_header h,n,p;
	pblock_footer pf;
	int i = 0; 
	while ((i < heap->list.length))
	{
		printf("Bloque %d: \n",i);
		h = (pblock_header)heap->list.data[i];
		PrintBlockInfo(h);
		printf("NEXT %d: \n",i);
		n = NEXT_HEADER(h);
		if (IS_VALID(heap,n))
			PrintBlockInfo(n);
		else printf("None\n"); 
		printf("PREVIOUS %d: \n",i);
		pf = PREVIOUS_FOOTER(h);
		if (IS_VALID(heap,pf))
			PrintBlockInfo(pf->myHeader);
		else printf("None\n"); 
		
		i++;
	}
}

bool CreateHeap(pHeap heap, void* begin, void* end, uint maxEntryCount){
	heap->initAddress = begin;
	heap->endAddress = end;
	heap->realInit = begin + maxEntryCount*4; // tendremos hasta 1024 bloques libres, eso es poco pero probemos
	lessThan* t = (lessThan*)(&BloqueMasChiquito); // OJO
	CreateSortedList(&heap->list,heap->initAddress,heap->realInit,t);
	
	// ahora creamos un bloque con toda la memoria
	pblock_header header = (pblock_header)heap->realInit;
	header->magic = MAGIC_NUMBER;
	header->hole = true;
	header->size = (uint)((uint)heap->endAddress - (uint)heap->realInit);
	pblock_footer footer = FOOTER(header);
	footer->magic = MAGIC_NUMBER;
	footer->myHeader = header;
	AddToList(&heap->list,header);
	return true;
}

void* kmalloc(pHeap heap, size_t amount){
	// primero, redondeemos el amount a multiplos de 4, el x86 funciona mejor
	//amount = (amount & 0xFFFFFFFD) + ((amount | 0x3)?4:0);
	// busquemos donde hay espacio
	pblock_header h;
	int i = 0; 
	bool b = false;
	while ((i < heap->list.length) && !b)
	{
		h = (pblock_header)ValueAt(&heap->list,i);
		b = (BLOCKSIZE(h) > amount);
		if (!b)
			i++;
	}
	
	if (b)
	{
		// encontre espacio, reservemos
		h = (pblock_header)ValueAt(&heap->list,i);
		RemoveFromListAt(&heap->list,i);
		// si al quitar este queda espacio para mas entonces creo uno nuevo
		if (BLOCKSIZE(h) - amount > sizeof(block_footer) + sizeof(block_header) + 4)
		{
			size_t s = h->size;
			
			h->size = amount + sizeof(block_header) + sizeof(block_footer);
			
			pblock_header nextH = NEXT_HEADER(h); // no puede dar error
			nextH->hole = true;
			nextH->magic = MAGIC_NUMBER;
			nextH->size = s - h->size;
			
			pblock_footer footer = FOOTER(h);
			footer->magic = MAGIC_NUMBER;
			footer->myHeader = h;
			
			footer = FOOTER(nextH);
			footer->magic = MAGIC_NUMBER;
			footer->myHeader = nextH;
			
			// veamos algo, siempre que se hace esta llamada ocurrio previamente 
			// un Remove, por tanto hay espacio y siempre se podra adicionar
			AddToList(&heap->list,nextH);
		} 
		h->hole = false;
		return (pblock_header)USEFUL_ADDRESS(h);
	} 
	else 
		{
			
			printf("KERNEL OUT OF MEMORY %s in %s\n",__FUNCTION__, __FILE__);
			while (1);
			return 0; 
		}
}

bool kfree(pHeap heap, void* address)
{
	int index;
	pblock_footer footer;
	pblock_header h = HEADER(address);
	if (h->magic == MAGIC_NUMBER && !h->hole)
	{
		// es un bloque y ademas ocupado, liberemoslo
		h->hole = true;
		// veamos si tenemos que unirlo con el de la derecha
		pblock_header nh = NEXT_HEADER(h);
		if (IS_VALID(heap,nh) && nh->hole)
		{
			// lo buscamos en la tabla
			index = IndexOf(&heap->list,nh);
			if (index != -1)
				RemoveFromListAt(&heap->list,index);
			h->size += nh->size;
			footer = FOOTER(h);
			footer->magic = MAGIC_NUMBER;
			footer->myHeader = h;
		}
		// veamos si tenemos que unirlo con el de la izquierda
		pblock_footer pf = PREVIOUS_FOOTER(h);
		pblock_header ph = pf->myHeader;
		if (IS_VALID(heap,pf) && IS_VALID(heap,ph) && ph->hole)
		{
			// lo buscamos en la tabla
			index = IndexOf(&heap->list,ph);
			if (index != -1)
				RemoveFromListAt(&heap->list,index);
			ph->size += h->size;
			footer = FOOTER(ph);
			footer->magic = MAGIC_NUMBER;
			footer->myHeader = ph;
			h = ph;
		}
		// Este es el único lugar donde existe la posibilidad de que no haya espacio 
		// en la lista de huecos. Si ocurre, nada, sencillamente este tipo no cae en 
		// la lista de espacio y no se puede usar hasta nuevo aviso, en este caso
		// el aviso es que el se reduzca por otro lado, asi que sin pánico
		bool b = AddToList(&heap->list,h);
			
		return b;
	}
	return false;
}

bool CreatePageAllocator(pPageAllocator allocator, void* begin, void* end)
{
	// determinos las direcciones posibles de inicio y fin
	allocator->initAddress = (void*)(((uint)begin & 0xFFFFF000) + ((((uint)begin & 0xFFF) | 0x000)?4096:0));
	allocator->endAddress = (void*)((uint)end & 0xFFFFF000);
	// cuantas paginas hay
	allocator->pageCount = ((uint)allocator->endAddress - (uint)allocator->initAddress) / 4096 ;
	// ahora determinemos cuantas paginas pierdo para administrar
	int count = 0;
	while (count * 1024 < allocator->pageCount)
	{
		count++;
		allocator->pageCount--;
	}
	allocator->dataAddress = (void*)((uint)allocator->initAddress + 4096*count);
	int * p = (int*)allocator->initAddress;
	for (count = 0 ; count < allocator->pageCount ; count++)
		p[count] = count;
	allocator->index = 0;
	//printf("%x %x %x %d\n",allocator->initAddress,allocator->endAddress,allocator->dataAddress,allocator->pageCount);
}

void* AllocPage(pPageAllocator allocator)
{
	if (allocator->index < allocator->pageCount)
	{
		int * p = (int*)allocator->initAddress;
		void * address = (void*)(p[allocator->index]*4096 + allocator->dataAddress);
		allocator->index++;
		return address;
	}
	else return 0;
}

bool DeallocPage(pPageAllocator allocator, void * page)
{
	if (!(((uint)page & 0xFFF) | 0x000) && (page >=allocator->dataAddress) 
							&& (page<allocator->endAddress))
							{
								page = page - (uint)allocator->dataAddress;
								uint i = (int)page / 4096;
								int * p = (int*)allocator->initAddress;
								allocator->index--;
								p[allocator->index] = i;
								return true;
							} else return false;
}



