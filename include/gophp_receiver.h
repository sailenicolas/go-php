// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __RECEIVER_H__
#define __RECEIVER_H__

void receiver_define(char *name);

void destroy_receiver(char *name);

/*
static zval * receiver_read_property(zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv);
static zval * receiver_write_property(zend_object *object, zend_string *member, zval *value, void **cache_slot);
static int receiver_exists(zend_object *object, zend_string *member, int has_set_exists, void **cache_slot);

static zend_function *receiver_get_method(zend_object **object, zend_string *name, const zval *key);
static zend_function *receiver_get_constructor(zend_object *object);

static void receiver_free(zend_object *object);
static zend_object *receiver_init(zend_class_entry *class_type);
static void destroy_receiver(char *name);

static void receiver_handlers_set(zend_object_handlers *handlers);

*/

char *get_receiver_name(zend_object *rcvr);
/* PHP 8 class declaration macros */

#define GOPHP_INIT_CLASS_ENTRY_BASE(module, namespace_name, short_name, methods, parent_ce)                               \
    do {                                                                                                               \
        zend_class_entry _##module##_ce = {};                                                                          \
        INIT_CLASS_ENTRY(_##module##_ce, namespace_name, methods);                                                     \
        module##_ce = zend_register_internal_class_ex(&_##module##_ce, parent_ce);                                     \
        if (short_name) SW_CLASS_ALIAS_SHORT_NAME(short_name, module);                                                 \
    } while (0)

#define GOPHP_INIT_CLASS_ENTRY_STD(module, namespace_name, methods)                                                       \
    GOPHP_INIT_CLASS_ENTRY_BASE(module, namespace_name, nullptr, methods, NULL);                                          \
    memcpy(&module##_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers))

#define GOPHP_INIT_CLASS_ENTRY(module, namespace_name, short_name, methods)                                               \
    GOPHP_INIT_CLASS_ENTRY_BASE(module, namespace_name, short_name, methods, NULL);                                       \
    memcpy(&module##_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers))

#define GOPHP_INIT_CLASS_ENTRY_EX(module, namespace_name, short_name, methods, parent_module)                             \
    GOPHP_INIT_CLASS_ENTRY_BASE(module, namespace_name, short_name, methods, parent_module##_ce);                         \
    memcpy(&module##_handlers, &parent_module##_handlers, sizeof(zend_object_handlers))

#define GOPHP_INIT_CLASS_ENTRY_EX2(module, namespace_name, short_name, methods, parent_module_ce, parent_module_handlers) \
    GOPHP_INIT_CLASS_ENTRY_BASE(module, namespace_name, short_name, methods, parent_module_ce);                           \
    memcpy(&module##_handlers, parent_module_handlers, sizeof(zend_object_handlers))

// Data Object: no methods, no parent
#define GOPHP_INIT_CLASS_ENTRY_DATA_OBJECT(module, namespace_name)                                                        \
    GOPHP_INIT_CLASS_ENTRY_BASE(module, namespace_name, NULL, NULL, NULL);                                                \
    memcpy(&module##_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers))

#define SW_CLASS_ALIAS(name, module)                                                                                   \
    do {                                                                                                               \
        if (name) {                                                                                                    \
            sw_zend_register_class_alias(ZEND_STRL(name), module##_ce);                                                \
        }                                                                                                              \
    } while (0)

#define SW_CLASS_ALIAS_SHORT_NAME(short_name, module)                                                                  \
    do {                                                                                                               \
        if (SWOOLE_G(use_shortname)) {                                                                                 \
            SW_CLASS_ALIAS(short_name, module);                                                                        \
        }                                                                                                              \
    } while (0)
#endif

