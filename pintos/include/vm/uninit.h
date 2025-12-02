#ifndef VM_UNINIT_H
#define VM_UNINIT_H
#include "vm/vm.h"

struct page;
enum vm_type;

typedef bool vm_initializer(struct page *, void *aux);

// 아직 데이터가 할당되지 않은 페이지. "Lazy loading" 구현을 위한 페이지 타입/
struct uninit_page {
	/* Initiate the contets of the page */
	vm_initializer *init;
	enum vm_type type;
	void *aux;
	/* Initiate the struct page and maps the pa to the va */
	bool (*page_initializer)(struct page *, enum vm_type, void *kva);
};

void uninit_new(struct page *page, void *va, vm_initializer *init, enum vm_type type, void *aux,
				bool (*initializer)(struct page *, enum vm_type, void *kva));
#endif
