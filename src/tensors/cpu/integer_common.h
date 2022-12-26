#pragma once

#include "tensors/tensor_allocator.h"
#include "tensors/tensor_operators.h"
#include "tensors/cpu/aligned.h"
#include "common/io_item.h"
#include "3rd_party/intgemm/intgemm.h"
#if defined(WASM)
#include "wasm_intgemm_interface.h"
#endif

#include <emmintrin.h>
#include <immintrin.h>
#include <tmmintrin.h>
#include <xmmintrin.h>
#include <cassert>
#include <cstddef>

namespace marian {
namespace cpu {
namespace integer {

//Convenient function to get rows and columns of a tensor, shadowed by namespace.
inline int cols(Tensor& tensor) { return tensor->shape()[-1]; }
inline int rows(Tensor& tensor) { return tensor->shape().elements() / cols(tensor); }

inline int cols(Shape& shape) { return shape[-1]; }
inline int rows(Shape& shape) { return shape.elements() / cols(shape); }

template<Type type> struct intgemm_;
template <> struct intgemm_<Type::int8> {using width = intgemm::Int8;
                                         using type = int8_t;
                                         constexpr static const Type intgemmType = Type::intgemm8;};
template <> struct intgemm_<Type::int16> {using width = intgemm::Int16;
                                          using type = int16_t;
                                          constexpr static const Type intgemmType = Type::intgemm16;};

// This operates on floats after processing so doesn't care about int8_t vs int16_t.
void AddBias(marian::Tensor C, const marian::Tensor Bias);

// For loading architecture agnostic models. We do PrepareAndTranpose, because we already transposed
// in our binary format. Then we copy the quantizationMultiplier information at the end
template<Type vtype>
void prepareAndTransposeB(io::Item& item, const char * input) {
    #ifdef COMPILE_CPU
    typedef typename intgemm_<vtype>::type Integer;
    Integer * output_tensor = reinterpret_cast<Integer *>(&(*item.bytes.begin()));
    // Sometimes we will end up with misaligned intput (and output) so we can't use them directly.
    // If this is the case, we will need to temporary allocate aligned memory, copy the results, and then free it
    if (reinterpret_cast<uintptr_t>(input) % 64 == 0 && reinterpret_cast<uintptr_t>(output_tensor) % 64 == 0) {
    #if defined(WASM)
        ABORT_IF(intgemm_<vtype>::intgemmType == Type::intgemm16,
                "Int16::PrepareBQuantizedTransposed is not implemented for wasm.");
        int8PrepareBFromQuantizedTransposed(reinterpret_cast<const int8_t *>(input),
                                        (Index)rows(item.shape),  //Since we only transposed, but didn't update the shape when constructing the binary 
                                        (Index)cols(item.shape), //rows here returns the columns of the transposed input matrix, and cols -> the rows
                                        (int8_t *)output_tensor);
    #else
        intgemm_<vtype>::width::PrepareBQuantizedTransposed(reinterpret_cast<const Integer *>(input),
                                                   output_tensor,
                                                   rows(item.shape),  //Since we only transposed, but didn't update the shape when constructing the binary, 
                                                   cols(item.shape)); //rows here returns the columns of the transposed input matrix, and cols -> the rows
    #endif
    } else {
        Integer * aligned_input = reinterpret_cast<Integer *>(genericMalloc(512, rows(item.shape)*cols(item.shape)*sizeof(Integer)));
        std::copy(input, input + rows(item.shape)*cols(item.shape), aligned_input);
        Integer * aligned_output = reinterpret_cast<Integer *>(genericMalloc(512, rows(item.shape)*cols(item.shape)*sizeof(Integer)));
    #if defined(WASM)
        ABORT_IF(intgemm_<vtype>::intgemmType == Type::intgemm16,
                "Int16::PrepareBQuantizedTransposed is not implemented for wasm.");
        int8PrepareBFromQuantizedTransposed(reinterpret_cast<const int8_t *>(aligned_input),
                                        (Index)rows(item.shape),  //Since we only transposed, but didn't update the shape when constructing the binary, 
                                        (Index)cols(item.shape), //rows here returns the columns of the transposed input matrix, and cols -> the rows
                                        reinterpret_cast<int8_t *>(aligned_output));
    #else
        intgemm_<vtype>::width::PrepareBQuantizedTransposed(reinterpret_cast<const Integer *>(aligned_input),
                                                   reinterpret_cast<Integer *>(aligned_output),
                                                   rows(item.shape),  //Since we only transposed, but didn't update the shape when constructing the binary, 
                                                   cols(item.shape)); //rows here returns the columns of the transposed input matrix, and cols -> the rows
    #endif
        // Copy to output tensor
        std::copy(aligned_output, aligned_output + rows(item.shape)*cols(item.shape), output_tensor);
        genericFree(aligned_input);
        genericFree(aligned_output);
    }
    //Copy the quantMult
    float quantMult = *(reinterpret_cast<const float *>(reinterpret_cast<const Integer *>(input) + item.shape.elements()));
    *(reinterpret_cast<float *>(&(*(output_tensor + item.shape.elements())))) = quantMult;
    #else
    ABORT("Using intgemm models is supported only with -DCOMPILE_CPU=on");
    #endif
}

template<Type vtype>
void unquantizeWemb(io::Item& item, const char * input) {
    typedef typename intgemm_<vtype>::type Integer;
    float quantMult = *(reinterpret_cast<const float *>(reinterpret_cast<const Integer *>(input) + item.shape.elements()));
    float * output_tensor = reinterpret_cast<float *>(&(*item.bytes.begin()));
    // Explicitly calculate n once beforehand because the compiler does not pick up on its
    // static nature, and will end up calling marian::Shape::dim() a lot.
    const size_t n = rows(item.shape) * cols(item.shape);
    for (size_t i = 0; i < n; i++) {
        output_tensor[i] = reinterpret_cast<const Integer *>(input)[i]*(1/quantMult);
    }
}

} //integer
} //cpu
} //marian