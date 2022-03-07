// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __ENGINE_H__
#define __ENGINE_H__

typedef struct _php_engine {
} php_engine;

php_engine *engine_init(void);
void engine_shutdown(php_engine *engine);
static size_t _engine_ub_write(const char *str, size_t len);

#ifdef PHP_VERSION_ID
#if PHP_VERSION_ID < 70400 && PHP_VERSION_ID > 70000


#elif PHP_VERSION_ID >= 70400


#endif

#endif

#endif
