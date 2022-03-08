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
		result = value_truth(val->internal);
	} else if (check == 0) {
		// Value exists and is not null.
		result = (val->kind != KIND_NULL) ? 1 : 0;
	} else {
		// Check value is invalid.
		result = 0;
	}

	value_destroy(val);
	return result;
}

// Call function with arguments passed and return value (if any).
static int receiver_method_call(char *name, INTERNAL_FUNCTION_PARAMETERS) {
	zval args;
	engine_receiver *this = _receiver_this(Z_OBJ_P(getThis()));

	array_init_size(&args, ZEND_NUM_ARGS());

	if (zend_copy_parameters_array(ZEND_NUM_ARGS(), &args) == FAILURE) {
		RETVAL_NULL();
	} else {
		engine_value *result = engineReceiverCall(this, name, (void *) &args);
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
void handlers_init(){
	zend_object_handlers rh;
    memcpy(&rh, &std_object_handlers, sizeof(std_object_handlers));
	rh.get_method = _receiver_method_get;
    rh.read_property = _receiver_get;
    rh.write_property = _receiver_set;
    rh.has_property = _receiver_exists;
    rh.get_constructor = _receiver_constructor_get;
	rh.free_obj = _receiver_free;
	rh.get_class_name  = std_object_handlers.get_class_name;
}


/* 	memcpy(&receiver_handlers, &std_object_handlers, sizeof(std_object_handlers));
	receiver_handlers.offset  = 0;
	receiver_handlers.get_method = _receiver_method_get;
	receiver_handlers.read_property = _receiver_get;
	receiver_handlers.write_property = _receiver_set;
	receiver_handlers.has_property = _receiver_exists;
	receiver_handlers.get_constructor = _receiver_constructor_get;
*
static zend_object_handlers receiver_handlers = 	{
0, 								/* offset of real object header (usually zero)  /
NULL,           				/* free_obj;             required 				/
NULL,           				/* dtor_obj;             required 				/
NULL,          					/* clone_obj;            optional 				/
_receiver_get,      			/* read_property;        required 				/
_receiver_set,     				/* write_property;       required 				/
NULL,							/* read_dimension;       required 				/
NULL,    						/* write_dimension;      required 				/
NULL, 							/* get_property_ptr_ptr; required 				/
_receiver_exists,   			/* has_property;         required 				/
NULL,     						/* unset_property;       required 				/
NULL,      						/* has_dimension;        required 				/
NULL,    						/* unset_dimension;      required 				/
NULL,    						/* get_properties;       required 				/
_receiver_method_get,  			/* get_method;           required 				/
_receiver_constructor_get,		/* get_constructor;      required 				/
NULL,    						/* get_class_name;       required 				/
NULL,    						/* cast_object;          required 				/
NULL,    						/* count_elements;       optional 				/
NULL,    						/* get_debug_info;       optional 				/
NULL,    						/* get_closure;          optional 				/
NULL,    						/* get_gc;               required 				/
NULL,    						/* do_operation;         optional 				/
NULL,    						/* compare;              required 				/
NULL 							/* get_properties_for;   optional 				/
};  */


void receiver_define(char *name) {
	zend_class_entry tmp;
	INIT_CLASS_ENTRY(tmp, name, NULL);
	handlers_init();
	zend_class_entry *this = zend_register_internal_class(&tmp);
    // Set standard handlers for receiver.
   	this->create_object = _receiver_init;
	this->ce_flags |= ZEND_ACC_FINAL;
}

void receiver_destroy(char *name) {
	name = php_strtolower(name, strlen(name));
	_receiver_destroy(name);
}

static zval *_receiver_get(zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv) {
	engine_value *result = receiver_get(object, member);
	if (result == NULL) {
		ZVAL_NULL(rv);
		return rv;
	}

	value_copy(rv, result->internal);
	value_destroy(result);

	return rv;
}

static zval * _receiver_set(zend_object *object, zend_string *member, zval *value, void **cache_slot) {
	receiver_set(object, member, value);
}

static int _receiver_exists(zend_object *object, zend_string *member, int has_set_exists, void **cache_slot) {
	return receiver_exists(object, member, has_set_exists);
}

static int _receiver_method_call(zend_string *method, zend_object *object, INTERNAL_FUNCTION_PARAMETERS) {
	return receiver_method_call(method->val, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static zend_function *_receiver_method_get(zend_object **object, zend_string *name, const zval *key) {
	zend_internal_function *func = receiver_method_get(*object);

	func->function_name = zend_string_copy(name);
	zend_set_function_arg_flags((zend_function *) func);

	return (zend_function *) func;
}

static zend_function *_receiver_constructor_get(zend_object *object) {
	zend_internal_function *func = receiver_constructor_get(object);
	zend_set_function_arg_flags((zend_function *) func);

	return (zend_function *) func;
}

// Free storage for allocated method receiver instance.
static void _receiver_free(zend_object *object) {
	engine_receiver *this = (engine_receiver *) object;
	zend_object_std_dtor(&(this->obj));
}

// Initialize instance of method receiver object. The method receiver itself is
// attached in the constructor function call.
static zend_object *_receiver_init(zend_class_entry *class_type) {

	engine_receiver *this;
	memset(this, 0, sizeof(engine_receiver));

	zend_object_std_init(&(this->obj), class_type);
	object_properties_init(&(this->obj), class_type);
	this->obj.handlers = &receiver_handlers;

	return &(this->obj);
}

static void _receiver_destroy(char *name) {
	zval *class = zend_hash_str_find(CG(class_table), name, strlen(name));

	if (class != NULL) {
		destroy_zend_class(class);
		zend_hash_str_del(CG(class_table), name, strlen(name));
	}
}

static engine_receiver *_receiver_this(zend_object *object) {
	return (engine_receiver *) object;
}

// Return class name for method receiver.
char *_receiver_get_name(engine_receiver *rcvr) {
	return rcvr->obj.ce->name->val;
}
