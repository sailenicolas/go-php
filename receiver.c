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

// Fetch and return field for method receiver.
static engine_value *receiver_get(zend_object *object, zend_string *member) {
	engine_receiver *this = _receiver_this(object);
	return engineReceiverGet(this, ZSTR_VAL(member));
}

// Set field for method receiver.
static zval * receiver_set(zend_object *object, zend_string *member, zval *value) {
	engine_receiver *this = _receiver_this(object);
	engineReceiverSet(this, ZSTR_VAL(member), (void *) value);
}

// Check if field exists for method receiver.
static int receiver_exists(zend_object *object, zend_string *member, int check) {
	engine_receiver *this = _receiver_this(object);

	if (!engineReceiverExists(this, ZSTR_VAL(member))) {
		// Value does not exist.
		return 0;
	} else if (check == 2) {
		// Value exists.
		return 1;
	}

	int result = 0;
	engine_value *val = engineReceiverGet(this, ZSTR_VAL(member));

	if (check == 1) {
		// Value exists and is "truthy".
		convert_to_boolean(val->internal);
		result = _value_truth(val->internal);
	} else if (check == 0) {
		// Value exists and is not null.
		result = (val->kind != KIND_NULL) ? 1 : 0;
	} else {
		// Check value is invalid.
		result = 0;
	}

	_value_destroy(val);
	return result;
}

// Call function with arguments passed and return value (if any).
static int receiver_method_call(char *name, INTERNAL_FUNCTION_PARAMETERS) {
	zval args;
	engine_receiver *this = _receiver_this(Z_OBJ_P(ZEND_THIS));

	array_init_size(&args, ZEND_NUM_ARGS());

	if (zend_copy_parameters_array(ZEND_NUM_ARGS(), &args) == FAILURE) {
		RETVAL_NULL();
	} else {
		engine_value *result = engineReceiverCall(this, name, (void *) &args);
		if (result == NULL) {
			RETVAL_NULL();
		} else {
			value_copy(return_value, result->internal);
			_value_destroy(result);
		}
	}

	zval_dtor(&args);
}


// Create new method receiver instance and attach to instantiated PHP object.
// Returns an exception if method receiver failed to initialize for any reason.
static void receiver_new(INTERNAL_FUNCTION_PARAMETERS) {
	zval args;
	engine_receiver *this = _receiver_this(Z_OBJ_P(ZEND_THIS));

	array_init_size(&args, ZEND_NUM_ARGS());

	if (zend_copy_parameters_array(ZEND_NUM_ARGS(), &args) == FAILURE) {
		zend_throw_exception(NULL, "Could not parse parameters for method receiver", 0);
	} else {
		// Create receiver instance. Throws an exception if creation fails.
		int result = engineReceiverNew(this, (void *) &args);
		if (result != 0) {
			zend_throw_exception(NULL, "Failed to instantiate method receiver", 0);
		}
	}

	zval_dtor(&args);
}

// Fetch and return function definition for method receiver. The method call
// happens in the method handler, as returned by this function.
static zend_internal_function *receiver_method_get(zend_object *object) {
	zend_internal_function *func = emalloc(sizeof(zend_internal_function));

	func->type     = ZEND_INTERNAL_FUNCTION;
	func->handler  = NULL;
	func->arg_info = NULL;
	func->num_args = 0;
	func->scope    = object->ce;
	func->fn_flags = ZEND_ACC_CALL_VIA_HANDLER;

	return func;
}

// Fetch and return constructor function definition for method receiver. The
// construct call happens in the constructor handler, as returned by this
// function.
static zend_internal_function *receiver_constructor_get(zend_object *object) {
	static zend_internal_function func;

	func.type     = ZEND_INTERNAL_FUNCTION;
	func.handler  = receiver_new;
	func.arg_info = NULL;
	func.num_args = 0;
	func.scope    = object->ce;
	func.fn_flags = 0;
	func.function_name = object->ce->name;

	return &func;
}
static zend_object_handlers receiver_handlers;

void receiver_define(char *name) {
	zend_class_entry tmp;
	INIT_CLASS_ENTRY_EX(tmp, name, strlen(name), NULL);

	zend_class_entry *this = zend_register_internal_class(&tmp);

	this->create_object = _receiver_init;
	this->ce_flags |= ZEND_ACC_FINAL;

/* Table of handler functions for method receivers.
static zend_object_handlers receiver_handlers = {
MACRO_INT,
free_obj,
zend_object_dtor_obj_t,
zend_object_clone_obj_t,
_receiver_get,
_receiver_set,
read_dimension,
zend_object_write_dimension_t			write_dimension,
zend_object_get_property_ptr_ptr_t		get_property_ptr_ptr, /* required /
zend_object_get_t						get,                  /* optional /
zend_object_set_t						set,                  /* optional /
zend_object_has_property_t				_receiver_exists,         /* requied /
zend_object_unset_property_t			unset_property,       /* required /
zend_object_has_dimension_t				has_dimension,       /* required /
zend_object_unset_dimension_t			unset_dimension,      /* required /
zend_object_get_properties_t			get_properties,       /* required /
zend_object_get_method_t				_receiver_method_get,           /* required /
zend_object_call_method_t				_receiver_method_call,          /* optional /
zend_object_get_constructor_t			get_constructor,      /* required /
zend_object_get_class_name_t			get_class_name,       /* required /
zend_object_compare_t					compare_objects,      /* optional /
zend_object_cast_t						cast_object,         /* optional /
zend_object_count_elements_t			count_elements,       /* optional /
zend_object_get_debug_info_t			get_debug_info,       /* optional /
zend_object_get_closure_t				get_closure,          /* optional /
zend_object_get_gc_t					get_gc,               /* required /
zend_object_do_operation_t				do_operation,         /* optional /
zend_object_compare_zvals_t				compare,              /* optional /
zend_object_get_properties_for_t		get_properties_for   /* optional /
 };
*/

	memcpy(&receiver_handlers, &std_object_handlers, sizeof(receiver_handlers));
	receiver_handlers.get_method = _receiver_method_get;
	receiver_handlers.read_property = _receiver_get;
	receiver_handlers.write_property = _receiver_set;
	receiver_handlers.has_property = _receiver_exists;
	receiver_handlers.get_constructor = _receiver_constructor_get;
	//receiver_handlers.call_method = _receiver_method_call;
	// Set standard handlers for receiver.
	receiver_handlers.free_obj = _receiver_free;
}

void receiver_destroy(char *name) {
	name = php_strtolower(name, strlen(name));
	_receiver_destroy(name);
}

#include "_receiver.c"
