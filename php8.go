// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
//go:build !Debian && php8 && !static
// +build !Debian,php8,!static

package gophp

// #cgo CFLAGS: -I/usr/include/php/20210902 -Isrc/  -Iinclude/
// #cgo CFLAGS: -I/usr/include/php/20210902/main -I/usr/include/php/20210902/Zend
// #cgo CFLAGS: -I/usr/include/php/20210902/TSRM
// #cgo LDFLAGS: -lphp8
import "C"
