/*
 *  Copyright 2008-2013 NVIDIA Corporation
 *  Modifications Copyright© 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <unittest/unittest.h>
#include <thrust/execution_policy.h>
#include <thrust/system/cuda/detail/util.h>

#include <thread>

void verify_stream()
{
  auto exec = thrust::device;
  auto stream = thrust::cuda_cub::stream(exec);
  ASSERT_EQUAL(stream, cudaStreamPerThread);
}

void TestPerThreadDefaultStream()
{
  verify_stream();

  std::thread t(verify_stream);
  t.join();
}
DECLARE_UNITTEST(TestPerThreadDefaultStream);
