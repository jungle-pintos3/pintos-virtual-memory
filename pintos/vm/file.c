/* file.c: Implementation of memory backed file object (mmaped object). */

#include <string.h>

#include "vm/vm.h"
#include "threads/vaddr.h"
#include "threads/mmu.h"
#include "userprog/process.h"
#include "userprog/syscall.h"

static bool file_backed_swap_in(struct page *page, void *kva);
static bool file_backed_swap_out(struct page *page);
static void file_backed_destroy(struct page *page);

static bool lazy_load_file(struct page *page, void *aux);

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void vm_file_init(void)
{
}

/* Initialize the file backed page */
bool file_backed_initializer(struct page *page, enum vm_type type, void *kva)
{
	if (page == NULL || kva == NULL || type != VM_FILE)
		return false;

	// 1. VM_FILE에 맞게 operations 변경
	page->operations = &file_ops;

	// 2. file_page 구조체 초기화
	struct file_page *aux = page->uninit.aux;
	struct file_page *file_page = &page->file;

	*file_page = (struct file_page){
		.offset = aux->offset,
		.file = aux->file,
		.page_read_bytes = aux->page_read_bytes,
	};

	return true;
}

/* Swap in the page by read contents from the file. */
static bool file_backed_swap_in(struct page *page, void *kva)
{
	if (page == NULL || kva == NULL)
		return false;

	struct file_page *file_page = &page->file;

	struct file *file = file_page->file;
	off_t ofs = file_page->offset;
	size_t page_read_bytes = file_page->page_read_bytes;

	lock_acquire(&file_lock);
	int read_result = file_read_at(file, page->frame->kva, page_read_bytes, ofs);
	lock_release(&file_lock);

	if (read_result != (int)page_read_bytes)
		PANIC("PANIC: VM_FILE swap in, 예상보다 적게 읽었는데 어떻게 하지?\n");

	memset(page->frame->kva + page_read_bytes, 0, PGSIZE - page_read_bytes);
	return true;
}

/* Swap out the page by writeback contents to the file. */
static bool file_backed_swap_out(struct page *page)
{
	if (page == NULL)
		return false;

	struct file_page *file_page = &page->file;

	if (pml4_is_dirty(thread_current()->pml4, page->va)) {
		/* write back 수행 */

		struct file *file = file_page->file;
		off_t ofs = file_page->offset;
		size_t page_read_bytes = file_page->page_read_bytes;

		lock_acquire(&file_lock);
		int write_result = file_write_at(file, page->frame->kva, page_read_bytes, ofs);
		lock_release(&file_lock);

		if (write_result != (int)page_read_bytes)
			PANIC("PANIC: VM_FILE swap in, 예상보다 적게 썼는데 어떻게 하지?\n");
	}

	return true;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void file_backed_destroy(struct page *page)
{
	struct file_page *file_page = &page->file;

	if (page->frame != NULL) {
		// pte에서 매핑 제거
		pml4_clear_page(thread_current()->pml4, page->va);

		// 물리메모리도 제거
		palloc_free_page(page->frame->kva);

		// frame 구조체 해제
		free(page->frame);
		page->frame = NULL;

		// TLB 플러시 (변경사항 적용)
		pml4_activate(thread_current()->pml4);
	}
}

/* Do the mmap */
void *do_mmap(void *addr, size_t length, int writable, struct file *file, off_t offset)
{
	/*
	1. length보다 크거나 같을 때까지 += PGSIZE 해주면서 페이지 늘려주기
	2. 페이지마다 만들고, vm_alloc_page_with_initializer() 호출해서 파일 & offset 전달하기 (offset
	업데이트, reopen하기)
	3. offset & read_bytes 값 업데이트하기
	4. init_aux로 file_page struct 전달할거고, init 함수는 lazy_load_segment, VM_TYPE은 VM_FILE로,
	upage는 page 시작 주소, writable 같이 전달하기
	*/

	file = file_reopen(file);

	void *addr_copy = addr;
	size_t read_bytes = length;
	struct page *current_page = NULL;

	while (read_bytes > 0) {
		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;

		struct file_page *aux = malloc(sizeof *aux);
		if (!aux)
			return NULL;

		*aux = (struct file_page){
			.file = file,
			.offset = offset,
			.page_read_bytes = page_read_bytes,
		};

		if (!vm_alloc_page_with_initializer(VM_FILE, addr_copy, writable, lazy_load_file, aux)) {
			free(aux);
			aux = NULL;
			return NULL;
		}

		struct page *page = spt_find_page(&thread_current()->spt, addr_copy);
		if (current_page != NULL)
			current_page->next_page = page;
		current_page = page;

		addr_copy += PGSIZE;
		offset += page_read_bytes;
		read_bytes -= page_read_bytes;
	}

	return addr;
}

/* Do the munmap */
void do_munmap(void *addr)
{
	/*
	1. addr로 page 검색해서 찾기
	2. spt_remove_page 호출해서 해당 페이지 삭제하기
	*/

	struct page *page = spt_find_page(&thread_current()->spt, addr);
	if (!page)
		return;

	if (page->next_page != NULL)
		do_munmap(page->next_page->va);

	// printf("munmap addr: %p, page: %p, type: %d\n", addr, page, page->operations->type);
	spt_remove_page(&thread_current()->spt, page);
}

static bool lazy_load_file(struct page *page, void *aux)
{
	struct file_page *vm_load_aux = (struct file_page *)aux;
	struct file *file = vm_load_aux->file;
	off_t ofs = vm_load_aux->offset;
	size_t page_read_bytes = vm_load_aux->page_read_bytes;

	lock_acquire(&file_lock);
	int read_result = file_read_at(file, page->frame->kva, page_read_bytes, ofs);
	lock_release(&file_lock);

	page->file.page_read_bytes = read_result;
	memset(page->frame->kva + page_read_bytes, 0, PGSIZE - page_read_bytes);
	free(aux);

	return true;
}
