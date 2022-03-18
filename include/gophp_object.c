// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <errno.h>
#include <stdbool.h>
#include <main/php.h>
#include <zend_exceptions.h>

#include "include/gophp_object.h"

// Creates a new value and initializes type to null.
gophp_object *new_gophp_object() {
    gophp_object *val = malloc(sizeof(gophp_object));
    val->internal = malloc(sizeof(zval));
    gophp_object_set_null(val);
    errno = 0;
    return val;
}

// Creates a complete copy of a zval.
// The destination zval needs to be correctly initialized before use.
void gophp_object_copy(zval *dst, zval *src) {
    ZVAL_COPY(dst, src);
    zval_copy_ctor(dst);
}

// Returns engine value type. Usually compared against KIND_* constants, defined
// in the `value.h` header file.
int gophp_object_get_kind(gophp_object *val) {
    return val->kind;
}

// Set type and value to null.
void gophp_object_set_null(gophp_object *val) {
    ZVAL_NULL(val->internal);
    val->kind = KIND_NULL;
}

// Set type and value to integer.
void gophp_object_set_long(gophp_object *val, long int num) {
    ZVAL_LONG(val->internal, num);
    val->kind = KIND_LONG;
}

// Set type and value to floating point.
void gophp_object_set_double(gophp_object *val, double num) {
    ZVAL_DOUBLE(val->internal, num);
    val->kind = KIND_DOUBLE;
}

// Set type and value to boolean.
void gophp_object_set_bool(gophp_object *val, bool status) {
    ZVAL_BOOL(val->internal, status);
    val->kind = KIND_BOOL;
}

// Set type and value to string.
void gophp_object_set_string(gophp_object *val, char *str) {
    ZVAL_STRING(val->internal, str);
    val->kind = KIND_STRING;
}

// Set type and value to array with a preset initial size.
void gophp_object_set_array(gophp_object *val, unsigned int size) {
    array_init_size(val->internal, size);
    val->kind = KIND_ARRAY;
}

// Set type and value to object.
void gophp_object_set_object(gophp_object *val) {
    object_init(val->internal);
    val->kind = KIND_OBJECT;
}

// Set type and value from zval. The source zval is copied and is otherwise not
// affected.
void gophp_object_set_zval(gophp_object *val, zval *src) {
    int kind;
    // Determine concrete type from source zval.
    switch (Z_TYPE_P(src)) {
        case IS_NULL:
            kind = KIND_NULL;
            break;
        case IS_LONG:
            kind = KIND_LONG;
            break;
        case IS_DOUBLE:
            kind = KIND_DOUBLE;
            break;
        case IS_STRING:
            kind = KIND_STRING;
            break;
        case IS_OBJECT:
            kind = KIND_OBJECT;
            break;
        case IS_ARRAY:
            kind = KIND_ARRAY;
            HashTable *h = (Z_ARRVAL_P(src));

            // Determine if array is associative or indexed. In the simplest case, a
            // associative array will have different values for the number of elements
            // and the index of the next free element. In cases where the number of
            // elements and the next free index is equal, we must iterate through
            // the hash table and check the keys themselves.
            if (h->nNumOfElements != h->nNextFreeElement) {
                kind = KIND_MAP;
                break;
            }

            unsigned long i = 0;

            for (zend_hash_internal_pointer_reset(h); i < h->nNumOfElements; i++) {
                unsigned long index;
                int type = zend_hash_get_current_key(h, NULL, &index);
                if (type == HASH_KEY_IS_STRING || index != i) {
                    kind = KIND_MAP;
                    break;
                }

                zend_hash_move_forward(h);
            }

            break;
        default:
            // Booleans need special handling for different PHP versions.
            if (gophp_object_get_truth(src) != -1) {
                kind = KIND_BOOL;
                break;
            }
            printf("%d\n", Z_TYPE_P(src));
            errno = 1231313131;
            return;
    }
    gophp_object_copy(val->internal, src);
    val->kind = kind;
    errno = 0;
}

// Set next index of array or map value.
void gophp_object_set_array_next(gophp_object *arr, gophp_object *val) {
    add_next_index_zval(arr->internal, val->internal);
}

void gophp_object_set_array_index(gophp_object *arr, unsigned long idx, gophp_object *val) {
    add_index_zval(arr->internal, idx, val->internal);
    arr->kind = KIND_MAP;
}

void gophp_object_set_array_key(gophp_object *arr, const char *key, gophp_object *val) {
    add_assoc_zval(arr->internal, key, val->internal);
    arr->kind = KIND_MAP;
}

void gophp_object_set_object_property(gophp_object *obj, const char *key, gophp_object *val) {
    add_property_zval(obj->internal, key, val->internal);
}

int gophp_object_get_long(gophp_object *val) {
    zval tmp;

    // Return value directly if already in correct type.
    if (val->kind == KIND_LONG) {
        return Z_LVAL_P(val->internal);
    }

    gophp_object_copy(&tmp, val->internal);
    convert_to_long(&tmp);

    return Z_LVAL(tmp);
}

double gophp_object_get_double(gophp_object *val) {
    zval tmp;

    // Return value directly if already in correct type.
    if (val->kind == KIND_DOUBLE) {
        return Z_DVAL_P(val->internal);
    }

    gophp_object_copy(&tmp, val->internal);
    convert_to_double(&tmp);

    return Z_DVAL(tmp);
}

bool gophp_object_get_bool(gophp_object *val) {
    zval tmp;

    // Return value directly if already in correct type.
    if (val->kind == KIND_BOOL) {
        return gophp_object_get_truth(val->internal);
    }

    gophp_object_copy(&tmp, val->internal);
    convert_to_boolean(&tmp);

    return gophp_object_get_truth(&tmp);
}

char *gophp_object_get_string(gophp_object *val) {
    zval tmp;
    int result;
    switch (val->kind) {
        case KIND_STRING:
            gophp_object_copy(&tmp, val->internal);
            break;
        case KIND_OBJECT:
            result = zend_std_cast_object_tostring(Z_OBJ_P(val->internal), &tmp, IS_STRING);
            if (result == FAILURE) {
                ZVAL_EMPTY_STRING(&tmp);
            }
            break;
        default:
            gophp_object_copy(&tmp, val->internal);
            convert_to_string(&tmp);
    }

    int len = Z_STRLEN(tmp) + 1;
    char *str = malloc(len);
    memcpy(str, Z_STRVAL(tmp), len);

    zval_dtor(&tmp);

    return str;
}

unsigned int gophp_object_array_size(gophp_object *arr) {
    switch (arr->kind) {
        case KIND_ARRAY:
        case KIND_MAP:
            return Z_ARRVAL_P(arr->internal)->nNumOfElements;
        case KIND_OBJECT:
            // Object size is determined by the number of properties, regardless of
            // visibility.
            return Z_OBJPROP_P(arr->internal)->nNumOfElements;
        case KIND_NULL:
            // Null values are considered empty.
            return 0;
    }

    // Non-array or object values are considered to be single-value arrays.
    return 1;
}

static void gophp_object_set_current_key(HashTable *ht, gophp_object *val) {
    zval tmp;
    zend_hash_get_current_key_zval(ht, &tmp);
    add_next_index_zval(val->internal, &tmp);
}

gophp_object *gophp_object_array_keys(gophp_object *arr) {
    HashTable *h = NULL;
    gophp_object *keys = new_gophp_object();

    gophp_object_set_array(keys, gophp_object_array_size(arr));

    switch (arr->kind) {
        case KIND_ARRAY:
        case KIND_MAP:
        case KIND_OBJECT:
            if (arr->kind == KIND_OBJECT) {
                h = Z_OBJPROP_P(arr->internal);
            } else {
                h = Z_ARRVAL_P(arr->internal);
            }

            unsigned long i = 0;

            for (zend_hash_internal_pointer_reset(h); i < h->nNumOfElements; i++) {
                gophp_object_set_current_key(h, keys);
                zend_hash_move_forward(h);
            }

            break;
        case KIND_NULL:
            // Null values are considered empty.
            break;
        default:
            // Non-array or object values are considered to contain a single key, '0'.
            add_next_index_long(keys->internal, 0);
    }

    return keys;
}

void gophp_object_array_reset(gophp_object *arr) {
    HashTable *h = NULL;

    switch (arr->kind) {
        case KIND_ARRAY:
        case KIND_MAP:
            h = Z_ARRVAL_P(arr->internal);
            break;
        case KIND_OBJECT:
            h = Z_OBJPROP_P(arr->internal);
            break;
        default:
            return;
    }

    zend_hash_internal_pointer_reset(h);
}

gophp_object *gophp_object_get_array_next(gophp_object *arr) {
    HashTable *ht = NULL;
    gophp_object *val = new_gophp_object();

    switch (arr->kind) {
        case KIND_ARRAY:
        case KIND_MAP:
            ht = Z_ARRVAL_P(arr->internal);
            break;
        case KIND_OBJECT:
            ht = Z_OBJPROP_P(arr->internal);
            break;
        default:
            // Attempting to return the next index of a non-array value will return
            // the value itself, allowing for implicit conversions of scalar values
            // to arrays.
            gophp_object_set_zval(val, arr->internal);
            return val;
    }

    zval *tmp = NULL;
    if ((tmp = zend_hash_get_current_data(ht)) != NULL) {
        gophp_object_set_zval(val, tmp);
        zend_hash_move_forward(ht);
    }
    return val;
}

gophp_object *gophp_object_get_array_index(gophp_object *arr, unsigned long idx) {
    HashTable *ht = NULL;
    gophp_object *val = new_gophp_object();
    switch (arr->kind) {
        case KIND_ARRAY:
        case KIND_MAP:
            ht = Z_ARRVAL_P(arr->internal);
            break;
        case KIND_OBJECT:
            ht = Z_OBJPROP_P(arr->internal);
            break;
        default:
            // Attempting to return the first index of a non-array value will return
            // the value itself, allowing for implicit conversions of scalar values
            // to arrays.
            if (idx == 0) {
                gophp_object_set_zval(val, arr->internal);
                return val;
            }

            return val;
    }
    zval *tmp = NULL;
    if ((tmp = zend_hash_index_find(ht, idx)) != NULL) {
        gophp_object_set_zval(val, tmp);
    }
    return val;
}

gophp_object *gophp_object_get_array_key(gophp_object *arr, char *key) {
    HashTable *ht = NULL;
    gophp_object *val = new_gophp_object();

    switch (arr->kind) {
        case KIND_ARRAY:
        case KIND_MAP:
            ht = Z_ARRVAL_P(arr->internal);
            break;
        case KIND_OBJECT:
            ht = Z_OBJPROP_P(arr->internal);
            break;
        default:
            return val;
    }
    zval *tmp = NULL;
    zend_string *str = zend_string_init(key, strlen(key), 0);
    if ((tmp = zend_hash_find(ht, str)) != NULL) {
        gophp_object_set_zval(val, tmp);
    }
    zend_string_release(str);
    return val;
}

// Destroy and free engine value.
void destroy_gophp_object(gophp_object *val) {
    zval_dtor(val->internal);
    free(val->internal);
    free(val);
}

int gophp_object_get_truth(zval *val) {
    return (Z_TYPE_P(val) == IS_TRUE) ? 1 : ((Z_TYPE_P(val) == IS_FALSE) ? 0 : -1);
}

