// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "peridot/bin/ledger/tests/benchmark/disk_space/disk_space.h"

#include <iostream>

#include <lib/async/cpp/task.h>
#include <lib/zx/time.h>
#include <trace/event.h>

#include "lib/callback/waiter.h"
#include "lib/fxl/command_line.h"
#include "lib/fxl/files/directory.h"
#include "lib/fxl/logging.h"
#include "lib/fxl/strings/string_number_conversions.h"
#include "peridot/bin/ledger/filesystem/get_directory_content_size.h"
#include "peridot/bin/ledger/testing/get_ledger.h"
#include "peridot/bin/ledger/testing/quit_on_error.h"
#include "peridot/bin/ledger/testing/run_with_tracing.h"

namespace {
constexpr fxl::StringView kStoragePath = "/data/benchmark/ledger/disk_space";
constexpr fxl::StringView kPageCountFlag = "page-count";
constexpr fxl::StringView kUniqueKeyCountFlag = "unique-key-count";
constexpr fxl::StringView kCommitCountFlag = "commit-count";
constexpr fxl::StringView kKeySizeFlag = "key-size";
constexpr fxl::StringView kValueSizeFlag = "value-size";

void PrintUsage(const char* executable_name) {
  std::cout << "Usage: trace record --categories=benchmark,ledger "
            << executable_name << " --" << kPageCountFlag << "=<int>"
            << " --" << kUniqueKeyCountFlag << "=<int>"
            << " --" << kCommitCountFlag << "=<int>"
            << " --" << kKeySizeFlag << "=<int>"
            << " --" << kValueSizeFlag << "=<int>" << std::endl;
}

}  // namespace

namespace test {
namespace benchmark {

DiskSpaceBenchmark::DiskSpaceBenchmark(async::Loop* loop, size_t page_count,
                                       size_t unique_key_count,
                                       size_t commit_count, size_t key_size,
                                       size_t value_size)
    : loop_(loop),
      tmp_dir_(kStoragePath),
      application_context_(
          component::ApplicationContext::CreateFromStartupInfo()),
      page_count_(page_count),
      unique_key_count_(unique_key_count),
      commit_count_(commit_count),
      key_size_(key_size),
      value_size_(value_size) {
  FXL_DCHECK(loop_);
  FXL_DCHECK(page_count_ >= 0);
  FXL_DCHECK(unique_key_count_ >= 0);
  FXL_DCHECK(commit_count_ >= 0);
  FXL_DCHECK(key_size_ > 0);
  FXL_DCHECK(value_size_ > 0);
}

void DiskSpaceBenchmark::Run() {
  ledger::LedgerPtr ledger;
  ledger::Status status =
      test::GetLedger([this] { loop_->Quit(); }, application_context_.get(),
                      &component_controller_, nullptr, "disk_space",
                      tmp_dir_.path(), &ledger);
  QuitOnError([this] { loop_->Quit(); }, status, "GetLedger");

  for (size_t page_number = 0; page_number < page_count_; page_number++) {
    ledger::PagePtr page;
    status = test::GetPageEnsureInitialized([this] { loop_->Quit(); }, &ledger,
                                            nullptr, &page, nullptr);
    pages_.push_back(std::move(page));
  }
  if (benchmark::QuitOnError([this] { loop_->Quit(); }, status,
                             "GetPageEnsureInitialized")) {
    return;
  }
  if (commit_count_ == 0) {
    ShutDownAndRecord();
    return;
  }
  Populate();
}

void DiskSpaceBenchmark::Populate() {
  int transaction_size = static_cast<int>(
      ceil(static_cast<double>(unique_key_count_) / commit_count_));
  int insertions = std::max(unique_key_count_, commit_count_);
  FXL_LOG(INFO) << "Transaction size: " << transaction_size
                << ", insertions: " << insertions << ".";
  auto waiter =
      callback::StatusWaiter<ledger::Status>::Create(ledger::Status::OK);
  for (auto& page : pages_) {
    auto keys = generator_.MakeKeys(insertions, key_size_, unique_key_count_);
    page_data_generator_.Populate(
        &page, std::move(keys), value_size_, transaction_size,
        test::benchmark::PageDataGenerator::ReferenceStrategy::REFERENCE,
        ledger::Priority::EAGER, waiter->NewCallback());
  }
  waiter->Finalize([this](ledger::Status status) {
    if (status != ledger::Status::OK) {
      benchmark::QuitOnError([this] { loop_->Quit(); }, status,
                             "PageGenerator::Populate");
      return;
    }
    ShutDownAndRecord();
  });
}

void DiskSpaceBenchmark::ShutDownAndRecord() {
  component_controller_->Kill();
  component_controller_.WaitForResponseUntil(zx::deadline_after(zx::sec(5)));
  loop_->Quit();

  uint64_t tmp_dir_size = 0;
  FXL_CHECK(ledger::GetDirectoryContentSize(tmp_dir_.path(), &tmp_dir_size));
  TRACE_COUNTER("benchmark", "ledger_directory_size", 0, "directory_size",
                TA_UINT64(tmp_dir_size));
}

}  // namespace benchmark
}  // namespace test

int main(int argc, const char** argv) {
  fxl::CommandLine command_line = fxl::CommandLineFromArgcArgv(argc, argv);

  std::string page_count_str;
  size_t page_count;
  std::string unique_key_count_str;
  size_t unique_key_count;
  std::string commit_count_str;
  size_t commit_count;
  std::string key_size_str;
  size_t key_size;
  std::string value_size_str;
  size_t value_size;
  if (!command_line.GetOptionValue(kPageCountFlag.ToString(),
                                   &page_count_str) ||
      !fxl::StringToNumberWithError(page_count_str, &page_count) ||
      !command_line.GetOptionValue(kUniqueKeyCountFlag.ToString(),
                                   &unique_key_count_str) ||
      !fxl::StringToNumberWithError(unique_key_count_str, &unique_key_count) ||
      !command_line.GetOptionValue(kCommitCountFlag.ToString(),
                                   &commit_count_str) ||
      !fxl::StringToNumberWithError(commit_count_str, &commit_count) ||
      !command_line.GetOptionValue(kKeySizeFlag.ToString(), &key_size_str) ||
      !fxl::StringToNumberWithError(key_size_str, &key_size) || key_size == 0 ||
      !command_line.GetOptionValue(kValueSizeFlag.ToString(),
                                   &value_size_str) ||
      !fxl::StringToNumberWithError(value_size_str, &value_size) ||
      value_size == 0) {
    PrintUsage(argv[0]);
    return -1;
  }

  async::Loop loop(&kAsyncLoopConfigMakeDefault);
  test::benchmark::DiskSpaceBenchmark app(&loop, page_count, unique_key_count,
                                          commit_count, key_size, value_size);

  return test::benchmark::RunWithTracing(&loop, [&app] { app.Run(); });
}