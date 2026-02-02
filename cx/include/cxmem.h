/*! \file cxmem.h
	\brief Memory management
*/

#if !CXMEM_VERSION

	#define CXMEM_VERSION 1000000L

	#include <stdlib.h>

	#include <cxerror.h>
	#include <cxinter.h>
	#include <cxdef.h>
	#include <cxrt_mutex.h>

	#define CXMEM_DEFAULT_STORAGE_CAP 8

	//#define cxreturnall return cxpushall(), CXRT_RETURN(),

	#define cxstatic (&CXMEM_STATIC)
	#define cxthread_local (&CXRUNTIME.storage)

	/// @private
	typedef struct cxmem_alloc
	{
		cxinter_t *inter;
		cxaddress_t addr;
		cxsize_t cap;
	}
	cxmem_alloc_t;

	typedef struct cxmem_storage
	{
		cxmem_alloc_t *allocs;
		cxsize_t size;
		cxsize_t cap;

		cxrt_mutex_t *mutex;
	}
	cxmem_storage_t;

	/// @private
	int cxmem_storage_construct(cxmem_storage_t *storage);
	/// @private
	int cxmem_storage_destruct(cxmem_storage_t *storage);
	/// @private
	int cxmem_storage_get(cxmem_storage_t *storage, cxmem_alloc_t *alloc, cxaddress_t addr);
	/// @private
	int cxmem_storage_add(cxmem_storage_t *storage, cxmem_alloc_t alloc);
	/// @private
	int cxmem_storage_remove(cxmem_storage_t *storage, cxmem_alloc_t *alloc, cxaddress_t addr);
	/// @private
	int cxmem_storage_replace(cxmem_storage_t *storage, cxmem_alloc_t alloc, cxaddress_t addr);
	/// @private
	int cxmem_storage_merge(cxmem_storage_t *new_storage, cxmem_storage_t *old_storage);
	/// @private
	int cxmem_storage_of(cxmem_storage_t* *storage, cxmem_alloc_t *alloc, cxaddress_t addr);
	/// @private
	int cxmem_storage_get_traversal(cxaddress_t entry, cxaddress_t arg, cxaddress_t ret);

	cxaddress_t cxmalloc(cxmem_storage_t *storage, cxsize_t cap);
	cxaddress_t cxrealloc(cxaddress_t addr, cxsize_t cap);
	void cxfree(cxaddress_t addr);

	cxaddress_t cxnew(cxmem_storage_t *storage, cxinter_t *inter, ...);
	void cxdelete(cxaddress_t addr);

	cxaddress_t cxpush(cxaddress_t address);
	void cxpushall();

	cxaddress_t cxrestore(cxmem_storage_t *storage, cxaddress_t address);
	void cxrestoreall(cxmem_storage_t *new_storage, cxmem_storage_t *old_storage);

	extern cxmem_storage_t CXMEM_STATIC;

	extern cxexception_t CXException_Memory;
	extern cxexception_t CXException_Memory_NoMemory;
	extern cxexception_t CXException_Memory_InvalidAddress;
	extern cxexception_t CXException_Memory_InvalidPush;
	extern cxexception_t CXException_Memory_InvalidArg;

#endif
