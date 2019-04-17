/**
 * @file
 *
 * @date Apr 16, 2019
 * @author Anton Bondarev
 */

#ifndef SRC_ARCH_E2K_INCLUDE_MMU_REGS_H_
#define SRC_ARCH_E2K_INCLUDE_MMU_REGS_H_

#include <e2k_api.h>

#ifndef __ASSEMBLY__
typedef unsigned long      mmu_reg_t;
#endif /* __ASSEMBLY__ */

#define MMU_REG_CR          0x00  /* Control register */
#define MMU_REG_CONT        0x10  /* Context register */
#define MMU_REG_CR3_RG      0x20  /* CR3 register for INTEL only */
#define MMU_REG_ELB_PTB     0x30  /* ELBRUS page table virtual base */
#define MMU_REG_ROOT_PTB    0x40  /* Root Page Table Base register */
#define MMU_REG_TRAP_POINT  0x50  /* Trap Pointer register */
#define MMU_REG_TRAP_COUNT  0x60  /* Trap Counter register */

/*
 * Write MMU register
 */
#define WRITE_MMU_REG(addr_val, reg_val) \
		E2K_WRITE_MAS_D((addr_val), (reg_val), MAS_MMU_REG)

/*
 * Read MMU register
 */
#define READ_MMU_REG(addr_val)	\
		E2K_READ_MAS_D((addr_val), MAS_MMU_REG)

/*
 * Read MMU Control register
 */
#define READ_MMU_CR() \
		READ_MMU_REG(MMU_REG_CR)

/*
 * Write MMU Control register
 */
#define WRITE_MMU_CR(mmu_cr) \
		WRITE_MMU_REG(MMU_REG_CR, mmu_cr)

/*
 * Read MMU Control register
 */
#define READ_MMU_CONT() \
		READ_MMU_REG(MMU_REG_CONT)

/*
 * Write MMU Context register
 */
#define WRITE_MMU_CONT(mmu_cont) \
		WRITE_MMU_REG(MMU_REG_CONT, mmu_cont)

/*
 * Read MMU Control Register of secondary space table
 */
#define READ_MMU_CR3_RG() \
		READ_MMU_REG(MMU_REG_CR3_RG)

/*
 * Write MMU Control Register of secondary space table
 */
#define WRITE_MMU_CR3_RG(mmu_page_dir) \
		WRITE_MMU_REG(MMU_REG_CR3_RG,mmu_page_dir)

/*
 * Read MMU ELBRUS page table base register
 */
#define READ_MMU_ELB_PTB() \
		READ_MMU_REG(MMU_REG_ELB_PTB)

/*
 * Write MMU ELBRUS page table base register
 */
#define WRITE_MMU_ELB_PTB(mmu_elb_ptb) \
		WRITE_MMU_REG(MMU_REG_ELB_PTB, mmu_elb_ptb)

/*
 * Read MMU root page table base register
 */
#define READ_MMU_ROOT_PTB() \
		READ_MMU_REG(MMU_REG_ROOT_PTB)

/*
 * Write MMU root page table base register
 */
#define WRITE_MMU_ROOT_PTB(mmu_root_ptb) \
		WRITE_MMU_REG(MMU_REG_ROOT_PTB, mmu_root_ptb)

#endif /* SRC_ARCH_E2K_INCLUDE_MMU_REGS_H_ */
