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

#include "value.h"
#include "receiver.h"
#include "_cgo_export.h"

/*
static engine_receiver *receiver_this(zend_object *object) {
	return (engine_receiver *) object;
}
*/

// Call function with arguments passed and return value (if any).
static int receiver_method_call(char *name, INTERNAL_FUNCTION_PARAMETERS) {
	zval args;
	array_init_size(&args, ZEND_NUM_ARGS());
	if (zend_copy_parameters_array(ZEND_NUM_ARGS(), &args) == FAILURE) {
		RETVAL_NULL();
	} else {
		engine_value *result = engineReceiverCall(Z_OBJ_P(getThis()), name, (void *) &args);
		if (result == NULL) {
			RETVAL_NULL();
		} else {
			value_copy(return_value, result->internal);
			value_destroy(result);
		}
	}

	zval_dtor(&args);
}


// Create new method receiver instance and attach to instantiated PHP object.
// Returns an exception if method receiver failed to initialize for any reason.
static void receiver_new(INTERNAL_FUNCTION_PARAMETERS) {
	zval args;

	array_init_size(&args, ZEND_NUM_ARGS());

	if (zend_copy_parameters_array(ZEND_NUM_ARGS(), &args) == FAILURE) {
		zend_throw_error(NULL, "Could not parse parameters for method receiver", 0);
	} else {
		// Create receiver instance. Throws an exception if creation fails.
		int result = engineReceiverNew(Z_OBJ_P(getThis()), (void *) &args);
		if (result != 0) {
			zend_throw_error(NULL, "Failed to instantiate method receiver", 0);
		}
	}

	zval_dtor(&args);
}




static const zend_function_entry receiver_define_methods[] = {
	ZEND_FE_END
};



void receiver_destroy(char *name) {
	name = php_strtolower(name, strlen(name));
	zval *zclass = zend_hash_str_find(CG(class_table), name, strlen(name));
	if (zclass != NULL) {
		destroy_zend_class(zclass);
		zend_hash_str_del(CG(class_table), name, strlen(name));
	}
}

static zval *receiver_read_property(zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv) {
	engine_value *result = engineReceiverGet(object, ZSTR_VAL(member));;
	if (result == NULL) {
		ZVAL_NULL(rv);
		return rv;
	}

	ZVAL_COPY(rv, result->internal);
	value_destroy(result);
	return rv;
}

static zval * receiver_write_property(zend_object *object, zend_string *member, zval *value, void **cache_slot) {
    engineReceiverSet(object, ZSTR_VAL(member), (void *) value);
    return value;
}

static int receiver_has_property(zend_object *object, zend_string *member, int has_set_exists, void **cache_slot) {
   if (!engineReceiverExists(object, ZSTR_VAL(member))) {
    	// Value does not exist.
   		return 0;
   } else if (has_set_exists == 2) {
   	// Value exists.
   		return 1;
   }

   int result = 0;
   engine_value *val = engineReceiverGet(object, ZSTR_VAL(member));
  	if (has_set_exists == 1) {
   		// Value exists and is "truthy".
   		convert_to_boolean(val->internal);
   		result = value_truth(val->internal);
    } else if (has_set_exists == 0) {
   		// Value exists and is not null.
    	result = (val->kind != KIND_NULL) ? 1 : 0;
   	} else {
   		// Check value is invalid.
   		result = 0;
   	}

   	value_destroy(val);
    return result;
}

static int _receiver_method_call(zend_string *method, zend_object *object, INTERNAL_FUNCTION_PARAMETERS) {
return receiver_method_call(method->val, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
// Fetch and return function definition for method receiver. The method call
// happens in the method handler, as returned by this function.

static zend_function *receiver_get_method(zend_object **object, zend_string *name, const zval *key) {
	zend_internal_function *func;
	func = emalloc(sizeof(zend_internal_function));
 	memset(func, 0, sizeof(zend_internal_function));
 	func->type     = ZEND_INTERNAL_FUNCTION;
    func->handler  = receiver_new;
    func->arg_info = NULL;
    func->num_args = 0;
    func->scope    = (*object)->ce;
    func->fn_flags = ZEND_ACC_CALL_VIA_HANDLER;

	func->function_name = zend_string_copy(name);
	return (zend_function *) func;
}
// Fetch and return constructor function definition for method receiver. The
// construct call happens in the constructor handler, as returned by this
// function.

static zend_function *receiver_get_constructor(zend_object *object, zend_string *name) {
	zend_internal_function *func;
	if (EXPECTED(EG(trampoline).common.function_name == NULL)) {
	   func = (zend_internal_function *) &EG(trampoline);
	} else {
	    func = emalloc(sizeof(zend_internal_function));
	}
	memset(func, 0, sizeof(zend_internal_function));
	func->type				= ZEND_INTERNAL_FUNCTION;
	func->handler			= receiver_new;
	func->arg_info			= NULL;
	func->num_args			= 0;
	func->scope				= object->ce;
	func->fn_flags = ZEND_ACC_CALL_VIA_HANDLER;
   	func->function_name = zend_string_copy(name);

	return (zend_function *) func;
}

// Free storage for allocated method receiver instance.
static void receiver_free_obj(zend_object *object) {
zend_object_std_dtor(object);
}



// Return class name for method receiver.
char *_receiver_get_name(zend_object *rcvr) {
	return rcvr->ce->name->val;
}

static zval *receiver_read_dimension(zend_object * object, zval *value, int val, zval * dimension){

}
static void receiver_write_dimension(){

}
static zval *receiver_get_property_ptr_ptr(zend_object *object, zend_string *prop, int type,  void **cache_slot){

}

static int receiver_has_dimension(zend_object *object, zval *value, int type){

}
static void receiver_unset_property(){

}
static HashTable *receiver_get_properties(zend_object *object){

}
static zend_string *receiver_get_class_name(const zend_object *object){

}
static int receiver_cast_object(zend_object *object, zval *val, int type){

}
static void receiver_destroy_object(){

}
static void receiver_unset_dimension(){

}
static HashTable *receiver_get_gc(zend_object *object, zval **val, int *type){

}
static int receiver_compare(zval *value, zval *val){

}

static zend_object_handlers receiver_handlers;
// Initialize instance of method receiver object. The method receiver itself is
// attached in the constructor function call.
static zend_object * receiver_init(zend_class_entry *class_type) {
	zend_object *obj = zend_objects_new(class_type);
	object_properties_init(obj, class_type);
	obj->handlers = &receiver_handlers;
	return obj;
}
void receiver_define(char *name) {
	memcpy(&receiver_handlers, zend_get_std_object_handlers(),
		sizeof(zend_object_handlers));

	zend_class_entry ce;

	INIT_CLASS_ENTRY_EX(ce, *name, strlen(*name) - 1, NULL);

	zend_class_entry *class_t = zend_register_internal_class(&ce);

   	class_t->create_object = receiver_init;

	class_t->ce_flags |= ZEND_ACC_FINAL;
}
