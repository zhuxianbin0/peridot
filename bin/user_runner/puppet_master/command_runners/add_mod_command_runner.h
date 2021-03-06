// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PERIDOT_BIN_USER_RUNNER_PUPPET_MASTER_COMMAND_RUNNERS_ADD_MOD_COMMAND_RUNNER_H_
#define PERIDOT_BIN_USER_RUNNER_PUPPET_MASTER_COMMAND_RUNNERS_ADD_MOD_COMMAND_RUNNER_H_

#include <fuchsia/modular/cpp/fidl.h>

#include "peridot/bin/user_runner/puppet_master/command_runners/command_runner.h"

namespace modular {

class AddModCommandRunner : public CommandRunner {
 public:
  AddModCommandRunner(SessionStorage* const session_storage);
  ~AddModCommandRunner();

  void Execute(
      fidl::StringPtr story_id, fuchsia::modular::StoryCommand command,
      std::function<void(fuchsia::modular::ExecuteResult)> done) override;
};

}  // namespace modular

#endif  // PERIDOT_BIN_USER_RUNNER_PUPPET_MASTER_COMMAND_RUNNERS_ADD_MOD_COMMAND_RUNNER_H_
