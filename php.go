// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//

package gophp

// #cgo CFLAGS: -I/usr/include/php/phpsrc
// #cgo CFLAGS: -I/usr/include/php/phpsrc/main -I/usr/include/php/phpsrc/Zend
// #cgo CFLAGS: -I/usr/include/php/phpsrc/TSRM
// #cgo LDFLAGS: -lphp8
import "C"
