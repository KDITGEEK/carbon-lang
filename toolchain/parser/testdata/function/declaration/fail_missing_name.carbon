// Part of the Carbon Language project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// AUTOUPDATE
// CHECK:STDOUT: [
// CHECK:STDOUT:   {kind: 'FunctionIntroducer', text: 'fn'},
// CHECK:STDOUT:   {kind: 'InvalidParse', text: '('},
// CHECK:STDOUT: {kind: 'FunctionDeclaration', text: ';', has_error: yes, subtree_size: 3},
// CHECK:STDOUT: {kind: 'FileEnd', text: ''},
// CHECK:STDOUT: ]

// CHECK:STDERR: fail_missing_name.carbon:[[@LINE+3]]:4: `fn` introducer should be followed by a name.
// CHECK:STDERR: fn ();
// CHECK:STDERR:    ^
fn ();
