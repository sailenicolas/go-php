// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __VALUE_H__
#define __VALUE_H__

typedef struct _engine_value {
	zval *internal;
	int  kind;
} engine_value;

enum {
	KIND_NULL,
	KIND_LONG,
	KIND_DOUBLE,
	KIND_BOOL,
	KIND_STRING,
	KIND_ARRAY,
	KIND_MAP,
	KIND_OBJECT
};

// Creates a new value and initializes type to null.
engine_value *value_new();

// Creates a complete copy of a zval.
// The destination zval needs to be correctly initialized before use.
void value_copy(zval *dst, zval *src);

// Returns engine value type. Usually compared against KIND_* constants, defined
// in the `value.h` header file.
int value_kind(engine_value *val);

// Set type and value to null.
void value_set_null(engine_value *val);
// Set type and value to integer.
void value_set_long(engine_value *val, long int num);
// Set type and value to floating point.
void value_set_double(engine_value *val, double num);
// Set type and value to boolean.
void value_set_bool(engine_value *val, bool status);

//void value_set_string(zval **val, char *str);
// Set type and value to string.
void value_set_string(engine_value *val, char *str);

// Set type and value to array with a preset initial size.
void value_set_array(engine_value *val, unsigned int size);

// Set type and value to object.
void value_set_object(engine_value *val);

// Set type and value from zval. The source zval is copied and is otherwise not
// affected.
void value_set_zval(engine_value *val, zval *src);

// Set next index of array or map value.
void value_array_next_set(engine_value *arr, engine_value *val);

void value_array_index_set(engine_value *arr, unsigned long idx, engine_value *val);

void value_array_key_set(engine_value *arr, const char *key, engine_value *val);

void value_object_property_set(engine_value *obj, const char *key, engine_value *val);

int value_get_long(engine_value *val) ;

double value_get_double(engine_value *val);

bool value_get_bool(engine_value *val);

char *value_get_string(engine_value *val);

unsigned int value_array_size(engine_value *arr);
engine_value *value_array_keys(engine_value *arr);
void value_array_reset(engine_value *arr);

engine_value *value_array_next_get(engine_value *arr);

engine_value *value_array_index_get(engine_value *arr, unsigned long idx);

engine_value *value_array_key_get(engine_value *arr, char *key);
// Destroy and free engine value.
void value_destroy(engine_value *val);

int value_truth(zval *val);

static int value_current_key_get(HashTable *ht, zend_string **str_index, zend_ulong *num_index);

static void value_current_key_set(HashTable *ht, engine_value *val);

#endif

