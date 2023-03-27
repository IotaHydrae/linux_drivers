#include <stdio.h>

struct person {
	int age;
	char *name;
};

int main(int argc, char **argv)
{
	struct person somebody = {
		.age = 12,
		.name = "karen",
	};

	printf("%ld\n", &somebody);
	printf("%ld\n", &somebody.age);

	printf("%ld\n", &somebody.name);
	
	printf("%ld\n", &((struct person *)0)->name);
}
