/* uninit.c: Implementation of uninitialized page.
 *
 * All of the pages are born as uninit page. When the first page fault occurs,
 * the handler chain calls uninit_initialize (page->operations.swap_in).
 * The uninit_initialize function transmutes the page into the specific page
 * object (anon, file, page_cache), by initializing the page object,and calls
 * initialization callback that passed from vm_alloc_page_with_initializer
 * function.
 * */

#include "vm/vm.h"
#include "vm/uninit.h"

static bool uninit_initialize(struct page *page, void *kva);
static void uninit_destroy(struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations uninit_ops = {
	.swap_in = uninit_initialize,
	.swap_out = NULL,
	.destroy = uninit_destroy,
	.type = VM_UNINIT,
};

// page_initializer을 설정하는 역할을 한다
void uninit_new(struct page *page, void *va, vm_initializer *init, enum vm_type type, void *aux,
				bool (*initializer)(struct page *, enum vm_type, void *))
{
	ASSERT(page != NULL);

	*page = (struct page){.operations = &uninit_ops,
						  .va = va,
						  .frame = NULL, /* no frame for now */
						  .uninit = (struct uninit_page){
							  .init = init,
							  .type = type,
							  .aux = aux,
							  .page_initializer = initializer,
						  }};
}

// 페이지 폴트 발생시 페이지를 초기화하는 함수
static bool uninit_initialize(struct page *page, void *kva)
{
	struct uninit_page *uninit = &page->uninit;

	// 1. 저장된 정보를 꺼낸다
	vm_initializer *init = uninit->init; // lazy_load_segment
	void *aux = uninit->aux;			 // vm_file_aux
	enum vm_type type = uninit->type;

	// 2. 타입 변환	(uninit_new에서 설정해둔 Initializer가 실행된다)
	bool success = uninit->page_initializer(page, type, kva);
	if (!success)
		return false;

	// 3. 실제 데이터 로드 (lazy_load_segment 실행된다)
	if (init) {
		return init(page, aux);
	}

	return true;
}

/* Free the resources hold by uninit_page. Although most of pages are transmuted
 * to other page objects, it is possible to have uninit pages when the process
 * exit, which are never referenced during the execution.
 * PAGE will be freed by the caller. */
static void uninit_destroy(struct page *page)
{
	struct uninit_page *uninit UNUSED = &page->uninit;
	/* TODO: Fill this function.
	 * TODO: If you don't have anything to do, just return. */
}
