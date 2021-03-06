/**
 * @file
 * @brief
 *
 * @date 09.11.12
 * @author Anton Bulychev
 */

#ifndef MEM_MMAP_MMU_H_
#define MEM_MMAP_MMU_H_

#include <stdint.h>
#include <util/dlist.h>
#include <hal/mmu.h>

struct marea {
	uintptr_t start;
	uintptr_t end;
	uint32_t flags;
	uint32_t is_allocated;

	struct dlist_head mmap_link;
};

struct phy_page {
	void *page;
	size_t page_number;

	struct dlist_head page_link;
};

struct emmap {
	void *brk;

	mmu_ctx_t ctx;

	struct dlist_head marea_list;
	struct dlist_head page_list;
};

extern void mmap_add_marea(struct emmap *mmap, struct marea *marea);
extern void mmap_del_marea(struct marea *marea);
extern struct marea *mmap_find_marea(struct emmap *mmap, mmu_vaddr_t vaddr);
extern struct marea *mmap_find_marea_next(struct emmap *mmap, mmu_vaddr_t vaddr,
		struct marea *prev);
extern int mmap_kernel_inited(void);
extern struct emmap *mmap_early_emmap(void);
extern int mmap_mapping(struct emmap *emmap);

extern void mmap_add_phy_page(struct emmap *mmap, struct phy_page *phy_page);
extern void mmap_del_phy_page(struct phy_page *phy_page);
extern struct phy_page *mmap_find_phy_page(struct emmap *mmap, void *start);

#endif /* MEM_MMAP_MMU_H_ */
