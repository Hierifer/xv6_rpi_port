/*****************************************************************
*       mmu.c
*       by Zhiyi Huang, hzy@cs.otago.ac.nz
*       University of Otago
*
********************************************************************/


#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "mmu.h"
#include "synchronize.h"

void mmuinit0(void)
{
    pde_t *l1;
	pte_t *l2;
    uint pa, va, *p;


	for(p=(uint *)0x2000; p<(uint *)0x8000; p++) *p = 0;

    l1 = (pde_t *) K_PDX_BASE;
    l2 = (pte_t *) K_PTX_BASE;

        // map all of ram at KERNBASE
	va = KERNBASE;
	for(pa = PA_START; pa < PA_START+RAMSIZE; pa += MBYTE){
        l1[PDX(va)] = pa|DOMAIN0|PDX_AP(K_RW)|SECTION|CACHED|BUFFERED;
        va += MBYTE;
    }

    // identity map first MB of ram so mmu can be enabled
    l1[PDX(PA_START)] = PA_START|DOMAIN0|PDX_AP(K_RW)|SECTION|CACHED|BUFFERED;

    // map IO region
    va = DEVSPACE;
    for(pa = PHYSIO; pa < PHYSIO+IOSIZE; pa += MBYTE){
        l1[PDX(va)] = pa|DOMAIN0|PDX_AP(K_RW)|SECTION;
        va += MBYTE;
    }

	// map GPU memory
	va = GPUMEMBASE;
	for(pa = GPUMEMBASE; pa < (uint)GPUMEMBASE+(uint)GPUMEMSIZE; pa += MBYTE){
		l1[PDX(va)] = pa|DOMAIN0|PDX_AP(K_RW)|SECTION;
		va += MBYTE;
	}

    // double map exception vectors at top of virtual memory
    va = HVECTORS;
    l1[PDX(va)] = (uint)l2|DOMAIN0|COARSE;
    l2[PTX(va)] = PA_START|PTX_AP(K_RW)|SMALL;

}

void
mmuinit1(void)
{
    pde_t *l1;
	
    l1 = (pde_t*)(K_PDX_BASE);

    // undo identity map of first MB of ram
    l1[PDX(PA_START)] = 0;

    dsb_barrier();
    CleanDataCache();
    InvalidateDataCache();
	

    // invalidate TLB; DSB barrier used
	flush_tlb();

}

