//===----------------------------------------------------------------------===//
//
// Part of libcu++, the C++ Standard Library for your entire system,
// under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES.
// SPDX-FileCopyrightText: Modifications Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#ifndef _NV__PREPROCESSOR
#define _NV__PREPROCESSOR

#if defined(__GNUC__)
#pragma GCC system_header
#endif

// For all compilers and dialects this header defines:
//  _NV_CONCAT_EVAL
//  _NV_EVAL
//  _NV_FIRST_ARG
//  _NV_IF
//  _NV_REMOVE_PAREN
//  _NV_STRIP_PAREN

#define _NV_EVAL1(...) __VA_ARGS__
#define _NV_EVAL(...) _NV_EVAL1(__VA_ARGS__)

#define _NV_CONCAT_EVAL1(l, r) _NV_EVAL(l##r)
#define _NV_CONCAT_EVAL(l, r) _NV_CONCAT_EVAL1(l, r)

#define _NV_IF_0(t, f) f
#define _NV_IF_1(t, f) t

#define _NV_IF_BIT(b) _NV_EVAL(_NV_IF_##b)
#define _NV_IF__EVAL(fn, t, f) _NV_EVAL(fn(t, f))
#define _NV_IF_EVAL(cond, t, f) _NV_IF__EVAL(_NV_IF_BIT(cond), t, f)

#define _NV_IF1(cond, t, f) _NV_IF_EVAL(cond, t, f)
#define _NV_IF(cond, t, f) _NV_IF1(_NV_EVAL(cond), _NV_EVAL(t), _NV_EVAL(f))

// The below mechanisms were derived from: https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/

#define _NV_REMOVE_PAREN(...) _NV_REMOVE_PAREN1(__VA_ARGS__)
#define _NV_REMOVE_PAREN1(...) \
    _NV_STRIP_PAREN(           \
        _NV_IF(_NV_TEST_PAREN(__VA_ARGS__), (_NV_STRIP_PAREN(__VA_ARGS__)), (__VA_ARGS__)))

#define _NV_STRIP_PAREN2(...) __VA_ARGS__
#define _NV_STRIP_PAREN1(...) _NV_STRIP_PAREN2 __VA_ARGS__
#define _NV_STRIP_PAREN(...) _NV_STRIP_PAREN1(__VA_ARGS__)

#define _NV_TEST_PAREN(...) _NV_TEST_PAREN1(__VA_ARGS__)
#define _NV_TEST_PAREN1(...) _NV_TEST_PAREN2(_NV_TEST_PAREN_DUMMY __VA_ARGS__)
#define _NV_TEST_PAREN2(...) _NV_TEST_PAREN3(_NV_CONCAT_EVAL(_, __VA_ARGS__))
#define _NV_TEST_PAREN3(...) _NV_EVAL(_NV_FIRST_ARG(__VA_ARGS__))

#define __NV_PAREN_YES 1
#define __NV_PAREN_NO 0

#define _NV_TEST_PAREN_DUMMY(...) _NV_PAREN_YES
#define __NV_TEST_PAREN_DUMMY __NV_PAREN_NO,

#define _NV_FIRST_ARG1(x, ...) x
#define _NV_FIRST_ARG(x, ...) _NV_FIRST_ARG1(x)

#define _NV_REMOVE_FIRST_ARGS1(...) __VA_ARGS__
#define _NV_REMOVE_FIRST_ARGS(x, ...) _NV_REMOVE_FIRST_ARGS1(__VA_ARGS__)

#endif // _NV__PREPROCESSOR
