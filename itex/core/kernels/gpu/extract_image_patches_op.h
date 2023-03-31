/* Copyright (c) 2021-2022 Intel Corporation

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

#ifndef ITEX_CORE_KERNELS_GPU_EXTRACT_IMAGE_PATCHES_OP_H_
#define ITEX_CORE_KERNELS_GPU_EXTRACT_IMAGE_PATCHES_OP_H_

#include <algorithm>
#include <limits>

#include "itex/core/utils/tensor_shape.h"
#include "itex/core/utils/tensor_types.h"
#include "third_party/eigen3/unsupported/Eigen/CXX11/Tensor"

namespace itex {
namespace functor {

template <typename Device, typename T>
struct ExtractImagePatchesForward {
  void operator()(const Device& d, typename TTypes<T, 4>::ConstTensor input,
                  int patch_rows, int patch_cols, int stride_rows,
                  int stride_cols, int rate_rows, int rate_cols,
                  const Eigen::PaddingType& padding,
                  typename TTypes<T, 4>::Tensor output) {
    // Need to swap row/col when calling Eigen, because our data is in
    // NHWC format while Eigen assumes NWHC format.
    const int64 N = std::max(input.size(), output.size());
    if (N <= std::numeric_limits<Index32>::max()) {
      auto output_32bit = To32Bit(output);
      output_32bit.device(d) =
          To32Bit(input)
              .extract_image_patches(patch_cols, patch_rows, stride_cols,
                                     stride_rows, rate_cols, rate_rows, padding)
              .reshape(output_32bit.dimensions());
    } else {
      output.device(d) =
          input
              .extract_image_patches(patch_cols, patch_rows, stride_cols,
                                     stride_rows, rate_cols, rate_rows, padding)
              .reshape(output.dimensions());
    }
  }
};

}  // namespace functor
}  // namespace itex

#endif  // ITEX_CORE_KERNELS_GPU_EXTRACT_IMAGE_PATCHES_OP_H_