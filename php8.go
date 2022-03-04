// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
//go:build !php7
// +build !php7

package gophp

// #cgo CFLAGS: -Iincludes/php8
// #cgo LDFLAGS: -lphp8
import "C"
