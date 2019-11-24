#include "mem.h"


void In(void)
{
	uint8_t* get = NULL;

	get = MemAlloc(26);

	for (char a = 'a'; a <= 'z'; a++)
	{
		*(get + a - 'a') = a;
	}

	MemFree(get);
}

int main()
{
	int32_t in = 0;
	uint8_t* p[100] = { NULL };
	uint8_t* q = NULL;
	uint8_t ndx = 0;

	MemInit();

	for (;;)
	{
		scanf_s("%d", &in);

		if (in <= 0)
		{
			MemFree(q);
		}
		else
		{
			q = MemAlloc(in);

			if (q == NULL)
			{
				printf("NULL");
			}
		}

		in = in;
	}

	return 0;
}