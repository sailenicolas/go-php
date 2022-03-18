// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <errno.h>
#include <stdbool.h>
#include <string.h> // correct header
#include <main/php.h>
#include <main/php_main.h>

#include "include/gophp_object.h"
#include "include/gophp_context.h"

gophp_context *new_context() {
    gophp_context *context;

    // Initialize context.
    context = malloc((sizeof(gophp_context)));
    if (context == NULL) {
        errno = 1;
        return NULL;
    }

    SG(server_context) = context;

    // Initialize request lifecycle.
    if (php_request_startup() == FAILURE) {
        zend_throw_error(NULL, "Something wrong", 0);
        SG(server_context) = NULL;
        free(context);
        errno = 1;
        return NULL;
    }

    errno = 0;
    return context;
}

void context_exec(gophp_context *context, char *filename) {
    int ret = 1;
    // Attempt to execute script file.
    zend_first_try
    {
        zend_file_handle script;
        zend_stream_init_filename(&script, filename);
 		ret = zend_execute_scripts(ZEND_REQUIRE, NULL, 1, &script);
 		if (ret == FAILURE) {
            zend_throw_error(NULL, "Failed to execute PHP script", 0);
        }
        zend_destroy_file_handle(&script);
    }
    zend_catch
    {
        errno = ret;
        zend_throw_error(NULL, "Failed to execute PHP script", 0);
        return;
    }
    zend_end_try();
    errno = ret;
    return;
}

zval *context_eval(gophp_context *context, char *script) {
    zend_string *str = zend_string_init(script, strlen(script), 0);

    // Compile script value.
    uint32_t compiler_options = CG(compiler_options);

    CG(compiler_options) = ZEND_COMPILE_DEFAULT_FOR_EVAL;
    zend_op_array *op = zend_compile_string(str, "gophp-engine");
    CG(compiler_options) = compiler_options;

    zend_string_release(str);
    // Return error if script failed to compile.
    if (!op) {
        zend_throw_error(NULL, "Failed to execute PHP script", 0);
        errno = 1;
        return NULL;
    }
    // Attempt to execute compiled string.	_context_eval(op, &tmp); _context_eval(zend_op_array *op, zval *ret)
    EG(no_extensions) = 1;
    zval tmp;
    zend_try
    {
        ZVAL_NULL(&tmp);
        zend_execute(op, &tmp);
    }
    zend_catch
    {
        destroy_op_array(op);
        efree_size(op, sizeof(zend_op_array));
        zend_bailout();
    }
    zend_end_try();

    destroy_op_array(op);
    efree_size(op, sizeof(zend_op_array));

    EG(no_extensions) = 0;

    // Allocate result value and copy temporary execution result in.
    zval *result = malloc(sizeof(zval));
    gophp_object_copy(result, &tmp);
    errno = 0;
    return result;
}

void context_bind(gophp_context *context, char *name, void *value) {
    gophp_object *v = (gophp_object *) value;
    zend_hash_str_update(&EG(symbol_table), name, strlen(name), v->internal);
}

void context_destroy(gophp_context *context) {
    php_request_shutdown((void*)0);

    SG(server_context) = NULL;
    free(context);
}
