// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __VALUE_H__
#define __VALUE_H__
typedef enum {
    KIND_NULL,
    KIND_LONG,
    KIND_DOUBLE,
    KIND_BOOL,
    KIND_STRING,
    KIND_ARRAY,
    KIND_MAP,
    KIND_OBJECT
} types;

typedef struct _gophp_object {
    zval *internal;
    types kind;
} gophp_object;



// Creates a new value and initializes type to null.
 gophp_object *new_gophp_object();

// Creates a complete copy of a zval.
// The destination zval needs to be correctly initialized before use.
void gophp_object_copy(zval *dst, zval *src);

// Returns engine value type. Usually compared against KIND_* constants, defined
// in the `value.h` header file.
int gophp_object_get_kind( gophp_object *val);

// Set type and value to null.
void gophp_object_set_null( gophp_object *val);

// Set type and value to integer.
void gophp_object_set_long( gophp_object *val, long int num);

// Set type and value to floating point.
void gophp_object_set_double( gophp_object *val, double num);

// Set type and value to boolean.
void gophp_object_set_bool( gophp_object *val, bool status);

//void value_set_string(zval **val, char *str);
// Set type and value to string.
void gophp_object_set_string( gophp_object *val, char *str);

// Set type and value to array with a preset initial size.
void gophp_object_set_array( gophp_object *val, unsigned int size);

// Set type and value to object.
void gophp_object_set_object( gophp_object *val);

// Set type and value from zval. The source zval is copied and is otherwise not
// affected.
void gophp_object_set_zval( gophp_object *val, zval *src);

// Set next index of array or map value.
void gophp_object_set_array_next( gophp_object *arr, gophp_object *val);

void gophp_object_set_array_index( gophp_object *arr, unsigned long idx, gophp_object *val);

void gophp_object_set_array_key( gophp_object *arr, const char *key, gophp_object *val);

void gophp_object_set_object_property( gophp_object *obj, const char *key, gophp_object *val);

int gophp_object_get_long( gophp_object *val);

double gophp_object_get_double( gophp_object *val);

bool gophp_object_get_bool( gophp_object *val);

char * gophp_object_get_string( gophp_object *val);

unsigned int gophp_object_array_size( gophp_object *arr);

 gophp_object * gophp_object_array_keys( gophp_object *arr);

void gophp_object_array_reset( gophp_object *arr);

 gophp_object * gophp_object_get_array_next( gophp_object *arr);

 gophp_object * gophp_object_get_array_index( gophp_object *arr, unsigned long idx);

 gophp_object * gophp_object_get_array_key( gophp_object *arr, char *key);

// Destroy and free engine value.
void destroy_gophp_object( gophp_object *val);

int gophp_object_get_truth(zval *val);

//static int value_current_key_get(HashTable *ht, zend_string **str_index, zend_ulong *num_index);

//static void value_current_key_set(HashTable *ht, gophp_object *val);

#endif

