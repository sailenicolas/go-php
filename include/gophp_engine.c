// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <stdio.h>
#include <errno.h>

#include <main/php.h>
#include <main/SAPI.h>
#include <main/php_main.h>
#include <main/php_variables.h>
#include <zend_exceptions.h>

#include "include/gophp_receiver.h"

#include "include/gophp_context.h"
#include "include/gophp_engine.h"
#include "_cgo_export.h"


// The php.ini defaults for the Go-PHP engine.
const char engine_ini_defaults[] = {
        "expose_php = 0\n"
        "html_errors = 0\n"
        "log_errors = 1\n"
        "display_errors = 1\n"
        "error_reporting = E_ALL\n"
        "implicit_flush = 1\n"
        "output_buffering = 0\n"
        "max_execution_time = 0\n"
        "max_input_time = -1\n\0"
};

static size_t engine_ub_write(const char *str, size_t len) {
    gophp_context *context = SG(server_context);
    int written = engineWriteOut(context, (void *) str, len);
    if (written != len) {
        php_handle_aborted_connection();
    }
    return len;
}

static int
engine_header_handler(sapi_header_struct *sapi_header, sapi_header_op_enum op, sapi_headers_struct *sapi_headers) {
    gophp_context *context = SG(server_context);
    switch (op) {
        case SAPI_HEADER_REPLACE:
        case SAPI_HEADER_ADD:
        case SAPI_HEADER_DELETE:
        case SAPI_HEADER_DELETE_ALL:
        case SAPI_HEADER_SET_STATUS:
            engineSetHeader(context, op, (void *) sapi_header->header, sapi_header->header_len);
            break;
    }
    return 0;
}

static void engine_send_header(sapi_header_struct *sapi_header, void *server_context) {
    // Do nothing.
}

static char *engine_read_cookies() {
    return NULL;
}

static void engine_register_variables(zval *track_vars_array) {
    php_import_environment_variables(track_vars_array);
}

static void engine_log_message(const char *str, int syslog_type_int) {
    gophp_context *context = SG(server_context);
    engineWriteLog(context, (void *) str, strlen(str));
}

static int engine_deactivate(void) {
    fflush(stdout);
    return SUCCESS;
}

/* Module initialization (MINIT) */
static int engine_startup(sapi_module_struct *sapi_module) {
    if (php_module_startup(sapi_module, NULL, 0) == FAILURE) {
        return FAILURE;
    }
    return SUCCESS;
}

static void engine_flush(void *server_context) {
    if (fflush(stdout) == EOF) {
        php_handle_aborted_connection();
    }
}

static sapi_module_struct engine_module = {
        "gophp-engine",              // Name
        "Go PHP Engine Library",     // Pretty Name
        engine_startup,              // Startup
        php_module_shutdown_wrapper, // Shutdown
        NULL,                        // Activate
        engine_deactivate,           // Deactivate
        engine_ub_write,             // Unbuffered Write
        engine_flush,                // Flush
        NULL,                        // Get UID
        NULL,                        // Getenv
        php_error,                   // Error Handler
        engine_header_handler,       // Header Handler
        NULL,                        // Send Headers Handler
        engine_send_header,          // Send Header Handler
        NULL,                        // Read POST Data
        engine_read_cookies,         // Read Cookies
        engine_register_variables,   // Register Server Variables
        engine_log_message,          // Log Message
        NULL,                        // Get Request Time
        NULL,                        // Child Terminate

        STANDARD_SAPI_MODULE_PROPERTIES
};


php_engine *engine_init(void) {
#if defined(SIGPIPE) && defined(SIG_IGN)
    signal(SIGPIPE, SIG_IGN); /* ignore SIGPIPE in standalone mode so
								 that sockets created via fsockopen()
								 don't kill PHP if the remote site
								 closes it.  in apache|apxs mode apache
								 does that for us!  thies@thieso.net
								 20000419 */
#endif

    zend_signal_startup();
    sapi_startup(&engine_module);

    engine_module.ini_entries = malloc(sizeof(engine_ini_defaults));
    memcpy(engine_module.ini_entries, engine_ini_defaults, sizeof(engine_ini_defaults));
    /* Module initialization (MINIT) */
    if (engine_module.startup(&engine_module) == FAILURE) {
        sapi_shutdown();
        errno = 1;
        return NULL;
    }
    if (php_request_startup() == FAILURE) {
        php_module_shutdown();
        errno = 2;
        return NULL;
    }
    SG(headers_sent) = 1;
    SG(request_info).no_headers = 1;
    php_register_variable("PHP_SELF", "-", NULL);
    php_engine *engine = malloc(sizeof(php_engine));
    errno = 0;
    return engine;
}


void engine_shutdown() {
    /* Request shutdown (RSHUTDOWN) */
    //php_request_shutdown((void *) 0);

    /* Module shutdown (MSHUTDOWN) */
    php_module_shutdown();

    /* SAPI shutdown (SSHUTDOWN) */
    sapi_shutdown();

#ifdef ZTS
    tsrm_shutdown();
#endif
    if (engine_module.ini_entries) {
        free(engine_module.ini_entries);
        engine_module.ini_entries = NULL;
    }
}
