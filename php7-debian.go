// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
// Build tags specific to Debian (and Debian-derived, such as Ubuntu)
// distributions. Debian builds its PHP7 packages with non-standard naming
// conventions for include and library paths, so we need a specific build tag
// for building against those packages.
//
//go:build Debian && php7 && !static
// +build Debian,php7,!static

package gophp

// #cgo CFLAGS: -I/usr/include/php/20190902 -Isrc/ -Iinclude/
// #cgo CFLAGS: -I/usr/include/php/20190902/main -I/usr/include/php/20190902/Zend
// #cgo CFLAGS: -I/usr/include/php/20190902/TSRM
// #cgo LDFLAGS: -lphp8
import "C"