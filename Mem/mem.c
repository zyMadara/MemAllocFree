#include "mem.h"


#define _MEM_SIZE_					((10u << 10) + (_MEM_HEAD_OFSET_ << 2))

#define _MEM_IN_USE_				(0x8000u)
#define _MEM_MIN_SIZE_				(4u)
#define _MEM_HEAD_OFSET_			(4u) //(sizeof(MemHdr_t))

#define _MEM_ROUND_(x)				((((x) + _MEM_HEAD_OFSET_ - 1) / _MEM_HEAD_OFSET_) * _MEM_HEAD_OFSET_)

// small
#define _MEM_SMALL_CNT_				(128u)
#define _MEM_SMALL_SIZE_			_MEM_ROUND_(32u)
#define _MEM_SMALLBLK_BUCKET_		(_MEM_SMALL_CNT_ * _MEM_SMALL_SIZE_)

// big
#define _MEM_BIG_CNT_				(24u)
#define _MEM_BIG_SIZE_				_MEM_ROUND_(256u)
#define _MEM_BIGBLK_BUCKET_			(_MEM_BIG_CNT_ * _MEM_BIG_SIZE_)


#if ((_MEM_BIGBLK_BUCKET_ + _MEM_SMALLBLK_BUCKET_) != (_MEM_SIZE_ - (_MEM_HEAD_OFSET_ << 2)))
#error "Try to make best use of Heap"
#endif

#if (_MEM_SIZE_ & _MEM_IN_USE_)
#error "Heap is too big to use"
#endif

#define _MEM_SMALL_LASTBLK_IDX_		(_MEM_SMALLBLK_BUCKET_ / _MEM_HEAD_OFSET_ + 1)
#define _MEM_BIG_FIRSTBLK_IDX_		(_MEM_SMALL_LASTBLK_IDX_ + 1)
#define _MEM_HEAP_LASTBLK_IDX_		(_MEM_SIZE_ / _MEM_HEAD_OFSET_ - 1)

#define _INIT_AFTER_ALLOC_			(1)

#define _MEM_FLAG_INIT_              'X'
#define _MEM_FLAG_ALOC_              'A'
#define _MEM_FLAG_REIN_              'F'

typedef uint32_t memDummy_t;

struct hdr_t
{
	uint32_t len : 15;
	uint32_t inUse : 1;
};

typedef union
{
	memDummy_t dummy;
	uint16_t val;
	struct hdr_t hdr;
}MemHdr_t;

MemHdr_t Heap[_MEM_SIZE_ / _MEM_HEAD_OFSET_];


void MemInit(void)
{
	memset(Heap, _MEM_FLAG_INIT_, sizeof(Heap));
	Heap[0].val = _MEM_SMALLBLK_BUCKET_;
	Heap[_MEM_SMALL_LASTBLK_IDX_].val = 0;
	Heap[_MEM_BIG_FIRSTBLK_IDX_].val = _MEM_BIGBLK_BUCKET_;
	Heap[_MEM_HEAP_LASTBLK_IDX_].val = 0;
}

void* MemAlloc(uint16_t size)
{
	MemHdr_t* memHdr;
	MemHdr_t* prevHdr = NULL;
	uint8_t coal = 0;
	uint8_t mod = size % _MEM_HEAD_OFSET_;

	if (mod != 0)
	{
		size += (_MEM_HEAD_OFSET_ - (mod));
	}

	if (size <= _MEM_SMALL_SIZE_)
	{
		memHdr = Heap;
	}
	else
	{
		memHdr = Heap + _MEM_BIG_FIRSTBLK_IDX_;
	}

	do
	{
		if (memHdr->hdr.inUse)
		{
			coal = 0;
		}
		else
		{
			if (coal != 0)
			{
				prevHdr->hdr.len += memHdr->hdr.len;
				memHdr->hdr.len = 0;
				if (prevHdr->hdr.len >= size)
				{
					memHdr = prevHdr;
					break;
				}
			}
			else
			{
				if (memHdr->hdr.len >= size)
				{
					break;
				}

				coal = 1;
				prevHdr = memHdr;
			}
		}

		memHdr = (MemHdr_t*)((uint8_t*)memHdr + memHdr->hdr.len + _MEM_HEAD_OFSET_);

		if (memHdr->val == 0)
		{
			memHdr = NULL;
			break;
		}
	} while (1);

	if (memHdr != NULL)
	{
		uint16_t temp = memHdr->hdr.len - size;

		if (temp >= _MEM_MIN_SIZE_)
		{
			MemHdr_t* next = (MemHdr_t*)((uint8_t*)memHdr + size + _MEM_HEAD_OFSET_);

			next->val = temp - _MEM_HEAD_OFSET_;
			memHdr->val = (size | _MEM_IN_USE_);
		}
		else
		{
			memHdr->hdr.inUse = 1;
		}

		memHdr++;

#if _INIT_AFTER_ALLOC_
		memset((void*)memHdr, _MEM_FLAG_ALOC_, size);
#endif

	}

	return (void*)memHdr;
}


void MemFree(void* p)
{
	MemHdr_t* hdr = (MemHdr_t*)p - 1;

	if (hdr->hdr.inUse == 0)
	{
		return;
	}

	hdr->hdr.inUse = 0;

#if _INIT_AFTER_ALLOC_
	memset((void*)(hdr + 1), _MEM_FLAG_REIN_, hdr->hdr.len);
#endif
}