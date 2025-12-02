#ifndef VM_FILE_H
#define VM_FILE_H
#include "filesys/file.h"
#include "vm/vm.h"

struct page;
enum vm_type;

struct file_page {
	/* 어떤 파일인가?
	  파일의 어느 위치인가?
	  얼마나 읽을/쓸 것인가?*/
	struct file *file; // 연결된 파일
	off_t ofs;		   // 파일 오프셋
	size_t read_bytes; // 읽을 바이트 수
	size_t zero_bytes; // 0으로 채울 바이트 수
};

void vm_file_init(void);
bool file_backed_initializer(struct page *page, enum vm_type type, void *kva);
void *do_mmap(void *addr, size_t length, int writable, struct file *file, off_t offset);
void do_munmap(void *va);
#endif
