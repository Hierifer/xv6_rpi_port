/*****************************************************************
*	entry.s
*	by Zhiyi Huang, hzy@cs.otago.ac.nz
*	University of Otago
*
********************************************************************/

.section .init
.globl _start
_start:

b entry  /* branch to the actual entry code */

.section .data

.align 4
.globl font
font:
	.incbin "font1.bin"

.align 4
.global _binary_initcode_start
_binary_initcode_start:
	.incbin "initcode"
.global _binary_initcode_end
_binary_initcode_end:

.align 4
.global _binary_fs_img_start
_binary_fs_img_start:
        .incbin "fs.img"
.global _binary_fs_img_end
_binary_fs_img_end:


.section .text

entry:

/* interrupts disabled, SVC mode by setting PSR_DISABLE_IRQ|PSR_DISABLE_FIQ|PSR_MODE_SVC */
mov r1, #0x00000080 /* PSR_DISABLE_IRQ */
orr r1, #0x00000040 /* PSR_DISABLE_FIQ */
orr r1, #0x00000013 /* PSR_MODE_SVC */
msr cpsr, r1

mov sp, #0x3000 

//---------------------------------------------------------
mov r5, #0
mov r6, #0
mov r7, #0
mov r8, #0
	
// store multiple at r4.
stmia r4!, {r5-r8}

//Disable MMU
mrc p15, 0, r1, c1, c0, 0
bic r1, r1, #0x1
mcr p15, 0, r1, c1, c0, 0

//Disable L1 Caches
mrc p15, 0, r1, c1, c0, 0
bic r1, r1, #(0x1 << 12)
bic r1, r1, #(0x1 << 2)
mcr p15, 0, r1, c1, c0, 0
	
//Invalidate L1 & Instruction cache
mov r1, #0
mcr p15, 0, r1, c7, c5, 0

//Invalidate Data cache
mrc p15, 1, r0, c0, c0, 0
ldr r3,=0x1ff
and r0, r3, r0, lsr #13

mov r1, #0

way_loop:
	mov r3, #0
set_loop:
	mov r2, r1, lsl #30
	orr r2, r3, lsl #5
	mcr p15, 0, r2, c7, c6, 2
	add r3, r3, #1
	cmp r0, r3
	bgt set_loop
	add r1, r1, #1
	cmp r1, #4
	bne way_loop


//Invalidate TLB
mcr p15, 0, r1, c8, c7, 0


//Branch Prediction Enable
mov r1, #0
mrc p15, 0, r1, c1, c0, 0
orr r1, r1, #(0x1 << 11)
	mcr p15, 0, r1, c1, c0, 0

//Create Translation Table
//Enable D-side Prefetch
mrc p15, 0, r1, c1, c0, 1
orr r1, r1, #(0x1 << 2)
mcr p15, 0, r1, c1, c0, 1
dsb
isb

bl mmuinit0       

//Initialize MMU
mov r1, #0x0
mcr p15, 0, r1, c2, c0, 2
ldr r1, =0x4000
mcr p15, 0, r1, c2, c0, 0

//Set up domain access control register
ldr r1, =0x55555555
mcr p15, 0, r1, c3, c0, 0

//Enable MMU 
mrc p15, 0, r1, c1, c0, 0
orr r1, r1, #0x1
    orr r1, #0x00002000     //Tell where is the vector table
    orr r1, #0x00000004     //Data Cache Enable
    orr r1, #0x00001000     //Instruction Cache Enable
mcr p15, 0, r1, c1, c0, 0
//---------------------------------------------------------

/* switch SP and PC into KZERO space */
mov r1, sp
add r1, #0x80000000
mov sp, r1

ldr r1, =_pagingstart
bx r1

.global _pagingstart
_pagingstart:
bl cmain  /* call C functions now */
//bl NotOkLoop

.global dsb_barrier
dsb_barrier:
	dsb
	bx lr
.global flush_dcache_all
flush_dcache_all:
	mov r0, #0
	mcr p15, 0, r0, c7, c10, 4 /* dsb */
	mov r0, #0
	mcr p15, 0, r0, c7, c14, 0 /* invalidate d-cache */
	bx lr
.global flush_idcache
flush_idcache:
	mov r0, #0
	mcr p15, 0, r0, c7, c10, 4 /* dsb */
	mov r0, #0
	mcr p15, 0, r0, c7, c14, 0 /* invalidate d-cache */
	mov r0, #0
	mcr p15, 0, r0, c7, c5, 0 /* invalidate i-cache */
	bx lr
.global flush_tlb
flush_tlb:
	mov r0, #0
	mcr p15, 0, r0, c8, c7, 0
	dsb
	bx lr
.global flush_dcache /* flush a range of data cache flush_dcache(va1, va2) */
flush_dcache:
	mcrr p15, 0, r0, r1, c14
	bx lr
.global set_pgtbase /* set the page table base set_pgtbase(base) */
set_pgtbase:
	mcr p15, 0, r0, c2, c0
	bx lr

.global getsystemtime
getsystemtime:
	ldr r0, =0xFE003004 /* addr of the time-stamp lower 32 bits */
	ldrd r0, r1, [r0]
	bx lr


