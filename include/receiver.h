// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __RECEIVER_H__
#define __RECEIVER_H__

typedef struct _engine_receiver {
	zend_object obj;
} engine_receiver;

void receiver_define(char *name);
void receiver_destroy(char *name);
/*
static zval * receiver_get(zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv);
static zval * receiver_set(zend_object *object, zend_string *member, zval *value, void **cache_slot);
static int receiver_exists(zend_object *object, zend_string *member, int has_set_exists, void **cache_slot);

static int _receiver_method_call(zend_string *method, zend_object *object, INTERNAL_FUNCTION_PARAMETERS);
static zend_function * _receiver_method_get(zend_object **object, zend_string *name, const zval *key);
static zend_function *_receiver_constructor_get(zend_object *object);

static void receiver_free(zend_object *object);
static zend_object *receiver_init(zend_class_entry *class_type);
static void _receiver_destroy(char *name);

static engine_receiver * receiver_this(zend_object *object);
static void receiver_handlers_set(zend_object_handlers *handlers);

static void handlers_init();
*/

char *_receiver_get_name(engine_receiver *rcvr);
#endif

