/**
 * @file
 * @brief
 *
 * @date 09.11.12
 * @author Anton Bulychev
 */

#ifndef MEM_MMAP_NOMMU_H_
#define MEM_MMAP_NOMMU_H_

#include <stdint.h>
#include <hal/mmu.h>
#include <util/dlist.h>

struct marea {
	uint32_t start, end;
	uint32_t flags;

	struct dlist_head mmap_link;
	struct dlist_head glob_link;
};

struct emmap {
	struct marea *stack_marea;
	struct marea *heap_marea;
	void *brk;
	mmu_ctx_t ctx;

	struct dlist_head marea_list;
};

extern void mmap_add_marea(struct emmap *mmap, struct marea *marea);
extern void mmap_del_marea(struct marea *marea);
extern struct marea *mmap_find_marea(struct emmap *mmap, mmu_vaddr_t vaddr);
extern struct marea *mmap_find_marea_next(struct emmap *mmap, mmu_vaddr_t vaddr,
		struct marea *prev);
extern int mmap_kernel_inited(void);
extern struct emmap *mmap_early_emmap(void);
extern int mmap_mapping(struct emmap *emmap);

#endif /* MEM_MMAP_NOMMU_H_ */
