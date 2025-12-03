#include "userprog/validate.h"

#include "threads/thread.h"
#include "threads/vaddr.h"

static int64_t get_user(const uint8_t *uaddr);
static int64_t put_user(uint8_t *udst, uint8_t byte);

bool check_buffer(const void *uaddr, size_t size, bool write)
{
	void *start = pg_round_down(uaddr);
	void *end = pg_round_down(uaddr + size - 1);

	for (void *p = start; p <= end; p += PGSIZE) {
		if (!valid_address(p, write)) {
			return false;
		}
	}

	return true;
}

bool valid_address(const void *uaddr, bool write)
{
	if (uaddr == NULL || !is_user_vaddr(uaddr))
		return false;

	int result = get_user(uaddr);
	if (result == -1)
		return false;

	if (write) {
		if (put_user(uaddr, (uint8_t)result) == -1)
			return false;
	}

	return true;
}

bool valid_address_test(const void *uaddr, bool write)
{
	if (uaddr == NULL || !is_user_vaddr(uaddr))
		return false;
	return (write ? put_user(uaddr, 0) : get_user(uaddr)) != -1;
}

static int64_t get_user(const uint8_t *uaddr)
{
	int64_t result;
	__asm __volatile("movabsq $done_get, %0\n"
					 "movzbq %1, %0\n"
					 "done_get:\n"
					 : "=&a"(result)
					 : "m"(*uaddr));
	return result;
}

static int64_t put_user(uint8_t *udst, uint8_t byte)
{
	int64_t error_code;
	__asm __volatile("movabsq $done_put, %0\n"
					 "movb %b2, %1\n"
					 "done_put:\n"
					 : "=&a"(error_code), "=m"(*udst)
					 : "q"(byte));
	return error_code;
}