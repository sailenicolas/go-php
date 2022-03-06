// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

static zval *_receiver_get(zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv) {
	engine_value *result = receiver_get(object, member);
	if (result == NULL) {
		ZVAL_NULL(rv);
		return rv;
	}

	value_copy(rv, result->internal);
	_value_destroy(result);

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
	engine_receiver *this = emalloc(sizeof(engine_receiver));
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
