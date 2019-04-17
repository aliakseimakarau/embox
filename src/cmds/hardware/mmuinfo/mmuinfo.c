/**
 * @file
 *
 * @date Apr 13, 2019
 * @author: Anton Bondarev
 */


#include <stdio.h>

#include <mmu_regs.h>

int main(int argc, char **argv) {
	mmu_reg_t mmu_cr;
	mmu_reg_t mmu_cont;
	mmu_reg_t mmu_elb_ptb;
	mmu_reg_t mmu_root_ptb;

	mmu_cr = READ_MMU_CR();
	mmu_cont = READ_MMU_CONT();
	mmu_elb_ptb = READ_MMU_ELB_PTB();
	mmu_root_ptb = READ_MMU_ROOT_PTB();

	printf("\n\tmmu_cr  %x\n\tmmu_cont %x\n\tmmu_elb_ptb %x\n\tmmu_root_ptb %x\n", mmu_cr, mmu_cont, mmu_elb_ptb, mmu_root_ptb);

	return 0;
}
