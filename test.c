#include <stdio.h>
#include <stddef.h>
#include <string.h>

struct test
{
	int a;
	int b;
};

struct map
{
	char *name;
	size_t offset;
};


static struct map test_map[] = {
	{"a", offsetof(struct test, a)},
	{"b", offsetof(struct test, b)},
	{NULL},
};

void setValue(void *obj, char *name, int value) 
{
	size_t offset = 0;
	size_t i = 0;
	for (i = 0; test_map[i].name; i++)
	{
		if (strcmp(test_map[i].name, name) == 0)
		{
			offset = test_map[i].offset;
			break;
		}	
	}
	if (!test_map[i].name)
		return ;

	*((int *)(obj + offset)) = value;
}

int main()
{
	struct test t = {0, 0};

	setValue(&t, "a", 10);

	printf("a: %d, b: %d\n", t.a, t.b);
}
