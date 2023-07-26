/* Copyright (c) 2023 Intel Corporation

Copyright 2021 The TensorFlow Authors. All Rights Reserved.

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

#ifndef ITEX_CORE_COMPILER_XLA_SERVICE_RESULT_CASTER_H_
#define ITEX_CORE_COMPILER_XLA_SERVICE_RESULT_CASTER_H_

#include <utility>

#include "itex/core/compiler/xla/service/hlo_module.h"
#include "itex/core/compiler/xla/service/op_expander_pass.h"

namespace itex_xla {

// Inserts Convert to result of instructions to the preferred element type
// specified by the instructions when direct accumulation of that type isn't
// supported by the backend. This pass should run after OperandUpcaster.
class ResultCaster : public OpExpanderPass {
 public:
  explicit ResultCaster(PatternExtraFilter extra_filter = nullptr)
      : OpExpanderPass(std::move(extra_filter)) {}

  absl::string_view name() const override { return "result_caster"; }

 protected:
  bool InstructionMatchesPattern(HloInstruction* instruction) override;

  StatusOr<HloInstruction*> ExpandInstruction(
      HloInstruction* instruction) override;
};

}  // namespace itex_xla

#endif  // ITEX_CORE_COMPILER_XLA_SERVICE_RESULT_CASTER_H_