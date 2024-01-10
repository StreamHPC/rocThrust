/******************************************************************************
 * Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
 * Modifications Copyright (c) 2019-2024, Advanced Micro Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
#pragma once

#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HIP
#include <thrust/system/hip/config.h>

#include <thrust/detail/cstdint.h>
#include <thrust/detail/temporary_array.h>
#include <thrust/system/hip/detail/par_to_seq.h>
#include <thrust/system/hip/detail/transform.h>
#include <thrust/system/hip/detail/util.h>
#include <thrust/functional.h>
#include <thrust/distance.h>
#include <thrust/detail/mpl/math.h>
#include <thrust/detail/minmax.h>

// rocprim include
#include <rocprim/rocprim.hpp>

THRUST_NAMESPACE_BEGIN

template <typename DerivedPolicy,
          typename InputIterator,
          typename OutputIterator,
          typename BinaryFunction>
__host__ __device__ OutputIterator
adjacent_difference(const thrust::detail::execution_policy_base<DerivedPolicy>& exec,
                    InputIterator                                               first,
                    InputIterator                                               last,
                    OutputIterator                                              result,
                    BinaryFunction                                              binary_op);

namespace hip_rocprim
{
namespace __adjacent_difference
{

    template <typename Derived,
              typename InputIt,
              typename OutputIt,
              typename BinaryOp>
    static OutputIt THRUST_HIP_RUNTIME_FUNCTION
    adjacent_difference(execution_policy<Derived>& policy,
                        InputIt                    first,
                        InputIt                    last,
                        OutputIt                   result,
                        BinaryOp                   binary_op)
    {
        typedef typename iterator_traits<InputIt>::difference_type size_type;

        size_type   num_items    = thrust::distance(first, last);
        size_t      storage_size = 0;
        hipStream_t stream       = hip_rocprim::stream(policy);
        bool        debug_sync   = THRUST_HIP_DEBUG_SYNC_FLAG;

        if(num_items <= 0)
        {
            return result;
        }

        // Determine temporary device storage requirements.
        hip_rocprim::throw_on_error(
            rocprim::adjacent_difference_inplace(nullptr,
                                                 storage_size,
                                                 first,
                                                 result,
                                                 static_cast<size_t>(num_items),
                                                 binary_op,
                                                 stream,
                                                 debug_sync),
            "adjacent_difference failed on 1st step");

        // Allocate temporary storage.
        thrust::detail::temporary_array<thrust::detail::uint8_t, Derived>
            tmp(policy, storage_size);
        void *ptr = static_cast<void*>(tmp.data().get());

        hip_rocprim::throw_on_error(
            rocprim::adjacent_difference_inplace(ptr,
                                                 storage_size,
                                                 first,
                                                 result,
                                                 static_cast<size_t>(num_items),
                                                 binary_op,
                                                 stream,
                                                 debug_sync),
            "adjacent_difference failed on 2nd step");

        hip_rocprim::throw_on_error(hip_rocprim::synchronize_optional(policy));

        return result + num_items;
    }

} // namespace __adjacent_difference

//-------------------------
// Thrust API entry points
//-------------------------

template <class Derived, class InputIt, class OutputIt, class BinaryOp>
OutputIt THRUST_HIP_FUNCTION
adjacent_difference(execution_policy<Derived>& policy,
                    InputIt                    first,
                    InputIt                    last,
                    OutputIt                   result,
                    BinaryOp                   binary_op)
{
    // struct workaround is required for HIP-clang
    struct workaround
    {
        __host__
        static void par(execution_policy<Derived>& policy,
                        InputIt                    first,
                        InputIt                    last,
                        OutputIt&                  result,
                        BinaryOp                   binary_op)
        {
            result = __adjacent_difference::adjacent_difference(
                policy, first, last, result, binary_op);
        }
        __device__
        static void seq(execution_policy<Derived>& policy,
                        InputIt                    first,
                        InputIt                    last,
                        OutputIt&                  result,
                        BinaryOp                   binary_op)
        {
            result = thrust::adjacent_difference(
               cvt_to_seq(derived_cast(policy)),
               first,
               last,
               result,
               binary_op
            );
        }
    };
    #if __THRUST_HAS_HIPRT__
    workaround::par(policy, first, last, result, binary_op);
    #else
    workaround::seq(policy, first, last, result, binary_op);
    #endif

    return result;
}

template <class Derived, class InputIt, class OutputIt>
OutputIt THRUST_HIP_FUNCTION
adjacent_difference(execution_policy<Derived>& policy,
                    InputIt                    first,
                    InputIt                    last,
                    OutputIt                   result)
{
    typedef typename iterator_traits<InputIt>::value_type input_type;
    return hip_rocprim::adjacent_difference(policy, first, last, result, minus<input_type>());
}

} // namespace hip_rocprim
THRUST_NAMESPACE_END

//
#endif // THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HIP
