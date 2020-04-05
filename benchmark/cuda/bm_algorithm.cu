
#include "../bm_algorithm.hpp"

void bm_cuda_cudaMemcpy(benchmark::State& state) {
    int ts_size = state.range(0);
    cuda::tensor<float, 1> cts_src(ts_size);
    cuda::tensor<float, 1> cts_dst(ts_size);

    while (state.KeepRunning()) {
        cudaMemcpy(cts_dst.data(), cts_src.data(), sizeof(cts_src[0]) * cts_src.size(),
                   cudaMemcpyDefault);
    }

    state.SetBytesProcessed(state.iterations() * static_cast<size_t>(cts_src.size()) *
                            sizeof(float));
}

auto bm_cuda_tensor1f_copy = bm_tensor_copy<cuda::tensor<float, 1>>;
auto bm_cuda_tensor2f_copy = bm_tensor_copy<cuda::tensor<float, 2>>;
BENCHMARK(bm_cuda_tensor1f_copy)->Arg(1_G);
BENCHMARK(bm_cuda_tensor2f_copy)->Arg(32_K);

auto bm_cuda_tensor2f_fill = bm_tensor_fill<cuda::tensor<float, 2>>;
BENCHMARK(bm_cuda_tensor2f_fill)->Arg(32_K);

auto bm_cuda_tensor2f_for_each = bm_tensor_for_each<cuda::tensor<float, 2>>;
BENCHMARK(bm_cuda_tensor2f_for_each)->Arg(32_K);

auto bm_cuda_tensor2f_transform = bm_tensor_transform<cuda::tensor<float, 2>>;
BENCHMARK(bm_cuda_tensor2f_transform)->Arg(32_K);

BENCHMARK_MAIN();