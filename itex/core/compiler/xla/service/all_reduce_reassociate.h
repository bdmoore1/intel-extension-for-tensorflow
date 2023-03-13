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

#ifndef ITEX_CORE_COMPILER_XLA_SERVICE_ALL_REDUCE_REASSOCIATE_H_
#define ITEX_CORE_COMPILER_XLA_SERVICE_ALL_REDUCE_REASSOCIATE_H_

#include "itex/core/compiler/xla/service/hlo_module.h"
#include "itex/core/compiler/xla/service/hlo_pass_interface.h"
#include "itex/core/compiler/xla/statusor.h"

namespace itex_xla {

// A pass that reassociates all-reduce feeding into compatible elementwise
// operations. As an example: add(all-reduce(x), all-reduce(y)) will be replaced
// with all-reduce(add(x,y)). Mathematically, this is replacing
//   add(x0, x1, ... xk) + add(y0, y1, ... yk) with
//   add((x0+y0), (x1+y), ... (xk+yk)
//
//  i.e., reassociating the reduction operation.

class AllReduceReassociate : public HloModulePass {
 public:
  absl::string_view name() const override { return "all-reduce-reassociate"; }

  StatusOr<bool> Run(HloModule* module) override;
};

}  // namespace itex_xla

#endif  // ITEX_CORE_COMPILER_XLA_SERVICE_ALL_REDUCE_REASSOCIATE_H_