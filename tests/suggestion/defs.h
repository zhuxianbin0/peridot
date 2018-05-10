// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PERIDOT_TESTS_SUGGESTION_DEFS_H_
#define PERIDOT_TESTS_SUGGESTION_DEFS_H_

namespace {

// This is how long we wait for the test to finish before we timeout and tear
// down our test.
constexpr int kTimeoutMilliseconds = 5000;
constexpr char kProposalId[] =
    "file:///system/bin/modular_tests/suggestion_proposal_test#proposal";

}  // namespace

#endif  // PERIDOT_TESTS_SUGGESTION_DEFS_H_