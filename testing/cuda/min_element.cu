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
#include <thrust/extrema.h>
#include <thrust/execution_policy.h>


#ifdef THRUST_TEST_DEVICE_SIDE
template<typename ExecutionPolicy, typename Iterator, typename Iterator2>
__global__
void min_element_kernel(ExecutionPolicy exec, Iterator first, Iterator last, Iterator2 result)
{
  *result = thrust::min_element(exec, first, last);
}


template<typename ExecutionPolicy, typename Iterator, typename BinaryPredicate, typename Iterator2>
__global__
void min_element_kernel(ExecutionPolicy exec, Iterator first, Iterator last, BinaryPredicate pred, Iterator2 result)
{
  *result = thrust::min_element(exec, first, last, pred);
}


template<typename ExecutionPolicy>
void TestMinElementDevice(ExecutionPolicy exec)
{
  size_t n = 1000;
  thrust::host_vector<int> h_data = unittest::random_samples<int>(n);
  thrust::device_vector<int> d_data = h_data;

  using iter_type = typename thrust::device_vector<int>::iterator;

  thrust::device_vector<iter_type> d_result(1);
  
  typename thrust::host_vector<int>::iterator   h_min = thrust::min_element(h_data.begin(), h_data.end());

  min_element_kernel<<<1,1>>>(exec, d_data.begin(), d_data.end(), d_result.begin());
  {
    cudaError_t const err = cudaDeviceSynchronize();
    ASSERT_EQUAL(cudaSuccess, err);
  }

  ASSERT_EQUAL(h_min - h_data.begin(), (iter_type)d_result[0] - d_data.begin());

  typename thrust::host_vector<int>::iterator   h_max = thrust::min_element(h_data.begin(), h_data.end(), thrust::greater<int>());

  min_element_kernel<<<1,1>>>(exec, d_data.begin(), d_data.end(), thrust::greater<int>(), d_result.begin());
  {
    cudaError_t const err = cudaDeviceSynchronize();
    ASSERT_EQUAL(cudaSuccess, err);
  }

  ASSERT_EQUAL(h_max - h_data.begin(), (iter_type)d_result[0] - d_data.begin());
}


void TestMinElementDeviceSeq()
{
  TestMinElementDevice(thrust::seq);
}
DECLARE_UNITTEST(TestMinElementDeviceSeq);


void TestMinElementDeviceDevice()
{
  TestMinElementDevice(thrust::device);
}
DECLARE_UNITTEST(TestMinElementDeviceDevice);
#endif


void TestMinElementCudaStreams()
{
  using Vector = thrust::device_vector<int>;
  using T      = Vector::value_type;

  Vector data(6);
  data[0] = 3;
  data[1] = 5;
  data[2] = 1;
  data[3] = 2;
  data[4] = 5;
  data[5] = 1;

  cudaStream_t s;
  cudaStreamCreate(&s);

  ASSERT_EQUAL( *thrust::min_element(thrust::cuda::par.on(s), data.begin(), data.end()), 1);
  ASSERT_EQUAL( thrust::min_element(thrust::cuda::par.on(s), data.begin(), data.end()) - data.begin(), 2);
  
  ASSERT_EQUAL( *thrust::min_element(thrust::cuda::par.on(s), data.begin(), data.end(), thrust::greater<T>()), 5);
  ASSERT_EQUAL( thrust::min_element(thrust::cuda::par.on(s), data.begin(), data.end(), thrust::greater<T>()) - data.begin(), 1);

  cudaStreamDestroy(s);
}
DECLARE_UNITTEST(TestMinElementCudaStreams);

void TestMinElementDevicePointer()
{
  using Vector = thrust::device_vector<int>;
  using T      = Vector::value_type;

  Vector data(6);
  data[0] = 3;
  data[1] = 5;
  data[2] = 1;
  data[3] = 2;
  data[4] = 5;
  data[5] = 1;

  T* raw_ptr = thrust::raw_pointer_cast(data.data());
  size_t n = data.size();
  ASSERT_EQUAL( thrust::min_element(thrust::device, raw_ptr, raw_ptr+n) - raw_ptr, 2);
  ASSERT_EQUAL( thrust::min_element(thrust::device, raw_ptr, raw_ptr+n, thrust::greater<T>()) - raw_ptr, 1);
}
DECLARE_UNITTEST(TestMinElementDevicePointer);
