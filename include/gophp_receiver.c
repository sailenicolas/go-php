// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <stdio.h>
#include <stdbool.h>

#include <main/php.h>
#include <zend_exceptions.h>
#include <ext/standard/php_string.h>
#include <Zend/zend_hash.h>
#include "include/gophp_object.h"
#include "include/gophp_receiver.h"
#include "_cgo_export.h"


static zend_object_handlers receiver_handlers;

// Call function with arguments passed and return value (if any).
static int call_receiver_method(char *name, INTERNAL_FUNCTION_PARAMETERS) {
  zval args;
    array_init_size(&args, ZEND_NUM_ARGS());
    if (zend_copy_parameters_array(ZEND_NUM_ARGS(), &args) == FAILURE) {
        RETVAL_NULL();
    } else {
        gophp_object *result = engineReceiverCall(Z_OBJ_P(getThis()), name, (void *) &args);
        if (result == NULL) {
            RETVAL_NULL();
        } else {
            gophp_object_copy(return_value, result->internal);
            destroy_gophp_object(result);
        }
    }

    zval_dtor(&args);
}



static zval *read_receiver_property(zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv) {
   gophp_object *result = engineReceiverGet(object, ZSTR_VAL(member));;
    if (result == NULL) {
        ZVAL_NULL(rv);
        return rv;
    }

    gophp_object_copy(rv, result->internal);
    destroy_gophp_object(result);
    return rv;
}

static zval *write_receiver_property(zend_object *object, zend_string *member, zval *value, void **cache_slot) {
  engineReceiverSet(object, ZSTR_VAL(member), (void *) value);
    return value;
}

static int has_receiver_property(zend_object *object, zend_string *member, int has_set_exists, void **cache_slot) {
   if (!engineReceiverExists(object, ZSTR_VAL(member))) {
        // Value does not exist.
        return 0;
    } else if (has_set_exists == 2) {
        // Value exists.
        return 1;
    }

    int result = 0;
    gophp_object *val = engineReceiverGet(object, ZSTR_VAL(member));
    if (has_set_exists == 1) {
        // Value exists and is "truthy".
        convert_to_boolean(val->internal);
        result = gophp_object_get_truth(val->internal);
    } else if (has_set_exists == 0) {
        // Value exists and is not null.
        result = (val->kind != KIND_NULL) ? 1 : 0;
    } else {
        // Check value is invalid.
        result = 0;
    }

    destroy_gophp_object(val);
    return result;
}

static void receiver_method_handler(INTERNAL_FUNCTION_PARAMETERS) {
   /* Cleanup trampoline */
   ZEND_ASSERT(EX(func)->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE);
   zend_free_trampoline(EX(func));
   EX(func) = NULL;
}
// Fetch and return function definition for method receiver. The method call
// happens in the method handler, as returned by this function.
//TODO: BEFORE IT WAS ZEND_OVERLOADED_FUNCTION, NOW IS ZEND_INTERNAL_FUNCTION REVIEW IT.
static zend_function *get_receiver_method(zend_object **object, zend_string *name, const zval *key) {
   zend_internal_function *func;
    func = emalloc(sizeof(zend_internal_function));
    memset(func, 0, sizeof(zend_internal_function));
    func->type = ZEND_INTERNAL_FUNCTION;
    func->handler = receiver_method_handler;
    func->arg_info = NULL;
    func->num_args = 0;
    func->scope = (*object)->ce;
    func->fn_flags = ZEND_ACC_CALL_VIA_HANDLER;

    func->function_name = zend_string_copy(name);
    zend_set_function_arg_flags((zend_function *) func);
    return (zend_function *) func;
}

// Create new method receiver instance and attach to instantiated PHP object.
// Returns an exception if method receiver failed to initialize for any reason.
static void receiver_ctor_handler(INTERNAL_FUNCTION_PARAMETERS) {
    zval args;
    array_init_size(&args, ZEND_NUM_ARGS());

    if (zend_copy_parameters_array(ZEND_NUM_ARGS(), &args) == FAILURE) {
        zend_throw_exception(NULL, "Could not parse parameters for method receiver", 0);
    } else {
        // Create receiver instance. Throws an exception if creation fails.
		if(Z_OBJ_P(getThis()) != NULL){
			int result = engineReceiverNew(Z_OBJ_P(getThis()), (void *) &args);
			if (result != 0) {
				zend_throw_exception(NULL, "Failed to instantiate method receiver",0);
			}
   		 }
    }

    zval_dtor(&args);
   /* Cleanup trampoline */
   ZEND_ASSERT(EX(func)->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE);
   zend_free_trampoline(EX(func));
   EX(func) = NULL;
}


// Fetch and return constructor function definition for method receiver. The
// construct call happens in the constructor handler, as returned by this
// function.
//TODO: REVIEW THIS
static zend_function *get_receiver_constructor(zend_object *object) {
    zend_internal_function *func;
    if (EXPECTED(EG(trampoline).common.function_name == NULL)) {
        func = (zend_internal_function * ) & EG(trampoline);
    } else {
        func = emalloc(sizeof(zend_internal_function));
    }
    memset(func, 0, sizeof(zend_internal_function));
    func->type = ZEND_INTERNAL_FUNCTION;
    func->handler = receiver_ctor_handler;
    func->arg_info = NULL;
    func->num_args = 0;
    func->scope = object->ce;
    func->fn_flags = ZEND_ACC_CALL_VIA_HANDLER;

    return (zend_function *) func;
}

// Free storage for allocated method receiver instance.
static void free_receiver_obj(zend_object *object) {
    zend_object_std_dtor(object);
    zend_string_release(object->ce->name);
}


// Return class name for method receiver.
char *get_receiver_name(zend_object *rcvr) {
    return rcvr->ce->name->val;
}

static zval *read_receiver_dimension(zend_object *object, zval *value, int val, zval *dimension) {

}

static void write_receiver_dimension() {

}

static zval *get_receiver_property_ptr_ptr(zend_object *object, zend_string *prop, int type, void **cache_slot) {

}

static int has_receiver_dimension(zend_object *object, zval *value, int type) {

}

static void unset_receiver_property() {

}

static HashTable *get_receiver_properties(zend_object *object) {

}

static zend_string *get_receiver_class_name(const zend_object *object) {

}

static int receiver_cast_object(zend_object *object, zval *val, int type) {

}

static void destroy_receiver_object() {

}

static void unset_receiver_dimension() {

}

static HashTable *get_receiver_gc(zend_object *object, zval **val, int *type) {

}

static int receiver_compare(zval *value, zval *val) {

}


// Initialize instance of method receiver object. The method receiver itself is
// attached in the constructor function call.
static zend_object *init_receiver(zend_class_entry *class_type) {
	zend_object *obj = emalloc(sizeof(zend_object));
	memset(obj, 0, sizeof(zend_object));

	zend_object_std_init(obj, class_type);
	object_properties_init(obj, class_type);
	obj->handlers = &receiver_handlers;
return obj;
}


void receiver_define(char *name) {
    memcpy(&receiver_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    receiver_handlers.free_obj = free_receiver_obj;
    receiver_handlers.read_property = read_receiver_property;
    receiver_handlers.write_property = write_receiver_property;
    receiver_handlers.has_property = has_receiver_property;
    receiver_handlers.get_method = get_receiver_method;
    receiver_handlers.get_constructor = get_receiver_constructor;

    zend_class_entry orig_class_entry;
    INIT_CLASS_ENTRY_EX(orig_class_entry, name, strlen(name), NULL);
	zend_class_entry *class_entry = malloc(sizeof(zend_class_entry));
	 zend_string *lcname;
	*class_entry = orig_class_entry;
	class_entry->type = ZEND_INTERNAL_CLASS;
    zend_initialize_class_data(class_entry, 0);
	#if PHP_MINOR_VERSION == 1
		zend_alloc_ce_cache(class_entry->name);
	#else

	#endif
	class_entry->info.internal.module = EG(current_module);

		if (class_entry->info.internal.builtin_functions) {
    		zend_register_functions(class_entry, class_entry->info.internal.builtin_functions, &class_entry->function_table, EG(current_module)->type);
    	}

    class_entry->create_object = init_receiver;
    class_entry->ce_flags =  orig_class_entry.ce_flags | ZEND_ACC_CONSTANTS_UPDATED | ZEND_ACC_LINKED | ZEND_ACC_RESOLVED_PARENT |
            ZEND_ACC_RESOLVED_INTERFACES | ZEND_ACC_FINAL;
	lcname = zend_string_tolower_ex(orig_class_entry.name, 1);
	lcname = zend_new_interned_string(lcname);
	zend_hash_update_ptr(CG(class_table), lcname, class_entry);
    zend_string_release_ex(lcname, 0);
}
void destroy_receiver(char *name) {
	name = php_strtolower(name, strlen(name));
	zval *val = zend_hash_str_find_ptr(EG(class_table), name, strlen(name)-1);
	if (val != NULL) {
		destroy_zend_class(val);
		zend_hash_str_del(CG(class_table), name, strlen(name)-1);
	}
}
