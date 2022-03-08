// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
//go:build !debian && !php7
// +build !debian,!php7

package gophp

// #cgo CFLAGS: -Iinclude/php8 -Isrc/php8
// #cgo LDFLAGS: -lphp8.1
import "C"
