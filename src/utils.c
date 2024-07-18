#include "swcl.h"

// Create new dynamic array with given initial capacity.
SWCLArray swcl_array_new(int initial_capacity) {
  SWCLArray a = {
      .length = 0,
      .capacity = initial_capacity,
      .items = (void **)malloc(sizeof(void *) * initial_capacity),
  };
  return a;
};

// Add item to the end of the array, resizing it if needed.
void swcl_array_append(SWCLArray *array, void *item) {
  if (array->length + 1 > array->capacity) {
    void **new_array =
        (void **)realloc(array->items, ++array->capacity * sizeof(void *));
    array->items = new_array;
  }
  array->items[array->length++] = item;
}

// Remove item from the array
void swcl_array_remove(SWCLArray *array, void *item) {}

// Destroy array
void swcl_array_free(SWCLArray *array) {}
