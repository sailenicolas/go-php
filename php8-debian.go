// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
// Build tags specific to Debian (and Debian-derived, such as Ubuntu)
// distributions. Debian builds its PHP7 packages with non-standard naming
// conventions for include and library paths, so we need a specific build tag
// for building against those packages.
//
//go:build debian && !php7
// +build debian,!php7

package gophp

// #cgo CFLAGS: -I/usr/include/php/20210902 -Iinclude/php8 -Isrc/php8
// #cgo CFLAGS: -I/usr/include/php/20210902/main -I/usr/include/php/20210902/Zend
// #cgo CFLAGS: -I/usr/include/php/20210902/TSRM
// #cgo LDFLAGS: -lphp8
import "C"