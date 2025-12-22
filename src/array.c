#include <xmllib.h>

void xl_array_push(int** array, int value) {
	if((*array) == NULL) {
		int* raw = malloc(sizeof(*raw) * 2);

		*array = &raw[1];

		raw[0] = 1;
	} else {
		int* old = &(*array)[-1];
		int* raw = malloc(sizeof(*raw) * (old[0] + 1 + 1));
		int  i;

		for(i = 0; i < old[0] + 1; i++) {
			raw[i] = old[i];
		}

		raw[0]++;

		*array = &raw[1];

		free(old);
	}

	(*array)[xl_array_length(array) - 1] = value;
}

int xl_array_length(int** array) {
	if((*array) == NULL) return 0;

	return (*array)[-1];
}

void xl_array_pop(int** array) {
	int* old;
	int* raw;
	int  i;

	if((*array) == NULL) return;

	old = &(*array)[-1];

	(*array)[-1]--;

	raw = malloc(sizeof(*raw) * (old[0] + 1));
	for(i = 0; i < old[0] + 1; i++) {
		raw[i] = old[i];
	}

	free(old);

	*array = &raw[1];
}

void xl_array_free(int** array) {
	if((*array) == NULL) return;

	free(&(*array)[-1]);

	*array = NULL;
}
