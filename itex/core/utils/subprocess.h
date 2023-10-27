/* Copyright (c) 2023 Intel Corporation

Copyright 2016 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef ITEX_CORE_UTILS_SUBPROCESS_H_
#define ITEX_CORE_UTILS_SUBPROCESS_H_

#include <errno.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <vector>

#include "itex/core/utils/types.h"

namespace itex {

// Channel identifiers.
enum Channel {
  CHAN_STDIN = 0,
  CHAN_STDOUT = 1,
  CHAN_STDERR = 2,
};

// Specify how a channel is handled.
enum ChannelAction {
  // Close the file descriptor when the process starts.
  // This is the default behavior.
  ACTION_CLOSE,

  // Make a pipe to the channel.  It is used in the Communicate() method to
  // transfer data between the parent and child processes.
  ACTION_PIPE,

  // Duplicate the parent's file descriptor. Useful if stdout/stderr should
  // go to the same place that the parent writes it.
  ACTION_DUPPARENT,
};

// Supports spawning and killing child processes.
class SubProcess;

// Returns an object that represents a child process that will be
// launched with the given command-line arguments `argv`. The process
// must be explicitly started by calling the Start() method on the
// returned object.
std::unique_ptr<SubProcess> CreateSubProcess(
    const std::vector<std::string>& argv);

}  // namespace itex

#include "itex/core/utils/macros.h"
#include "itex/core/utils/mutex.h"
#include "itex/core/utils/platform.h"

namespace itex {

class SubProcess {
 public:
  // SubProcess()
  //    nfds: The number of file descriptors to use.
  explicit SubProcess(int nfds = 3);

  // Virtual for backwards compatibility; do not create new subclasses.
  // It is illegal to delete the SubProcess within its exit callback.
  virtual ~SubProcess();

  // SetChannelAction()
  //    Set how to handle a channel.  The default action is ACTION_CLOSE.
  //    The action is set for all subsequent processes, until SetChannel()
  //    is called again.
  //
  //    SetChannel may not be called while the process is running.
  //
  //    chan: Which channel this applies to.
  //    action: What to do with the channel.
  // Virtual for backwards compatibility; do not create new subclasses.
  virtual void SetChannelAction(Channel chan, ChannelAction action);

  // SetProgram()
  //    Set up a program and argument list for execution, with the full
  //    "raw" argument list passed as a vector of strings.  argv[0]
  //    should be the program name, just as in execv().
  //
  //    file: The file containing the program.  This must be an absolute path
  //          name - $PATH is not searched.
  //    argv: The argument list.
  virtual void SetProgram(const std::string& file,
                          const std::vector<std::string>& argv);

  // Start()
  //    Run the command that was previously set up with SetProgram().
  //    The following are fatal programming errors:
  //       * Attempting to start when a process is already running.
  //       * Attempting to start without first setting the command.
  //    Note, however, that Start() does not try to validate that the binary
  //    does anything reasonable (e.g. exists or can execute); as such, you can
  //    specify a non-existent binary and Start() will still return true.  You
  //    will get a failure from the process, but only after Start() returns.
  //
  //    Return true normally, or false if the program couldn't be started
  //    because of some error.
  // Virtual for backwards compatibility; do not create new subclasses.
  virtual bool Start();

  // Kill()
  //    Send the given signal to the process.
  //    Return true normally, or false if we couldn't send the signal - likely
  //    because the process doesn't exist.
  virtual bool Kill(int signal);

  // Wait()
  //    Block until the process exits.
  //    Return true normally, or false if the process wasn't running.
  virtual bool Wait();

  // Communicate()
  //    Read from stdout and stderr and writes to stdin until all pipes have
  //    closed, then waits for the process to exit.
  //    Note: Do NOT call Wait() after calling Communicate as it will always
  //     fail, since Communicate calls Wait() internally.
  //    'stdin_input', 'stdout_output', and 'stderr_output' may be NULL.
  //    If this process is not configured to send stdout or stderr to pipes,
  //     the output strings will not be modified.
  //    If this process is not configured to take stdin from a pipe, stdin_input
  //     will be ignored.
  //    Returns the command's exit status.
  virtual int Communicate(const std::string* stdin_input,
                          std::string* stdout_output,
                          std::string* stderr_output);

 private:
  static constexpr int kNFds = 3;
  static bool chan_valid(int chan) { return ((chan >= 0) && (chan < kNFds)); }
  static bool retry(int e) {
    return ((e == EINTR) || (e == EAGAIN) || (e == EWOULDBLOCK));
  }
  void FreeArgs() TF_EXCLUSIVE_LOCKS_REQUIRED(data_mu_);
  void ClosePipes() TF_EXCLUSIVE_LOCKS_REQUIRED(data_mu_);
  bool WaitInternal(int* status);

  // The separation between proc_mu_ and data_mu_ mutexes allows Kill() to be
  // called by a thread while another thread is inside Wait() or Communicate().
  mutable mutex proc_mu_;
  bool running_ TF_GUARDED_BY(proc_mu_);
  pid_t pid_ TF_GUARDED_BY(proc_mu_);

  mutable mutex data_mu_ TF_ACQUIRED_AFTER(proc_mu_);
  char* exec_path_ TF_GUARDED_BY(data_mu_);
  char** exec_argv_ TF_GUARDED_BY(data_mu_);
  ChannelAction action_[kNFds] TF_GUARDED_BY(data_mu_);
  int parent_pipe_[kNFds] TF_GUARDED_BY(data_mu_);
  int child_pipe_[kNFds] TF_GUARDED_BY(data_mu_);

  TF_DISALLOW_COPY_AND_ASSIGN(SubProcess);
};

}  // namespace itex

#endif  // ITEX_CORE_UTILS_SUBPROCESS_H_