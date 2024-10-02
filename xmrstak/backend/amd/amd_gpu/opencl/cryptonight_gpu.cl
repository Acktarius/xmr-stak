R"===(

inline __global int4* scratchpad_ptr(uint idx, uint n, __global int *lpad) {
    return (__global int4*)((__global char*)lpad + (idx & MASK) + n * 16);
}

#define fma_break(x) ((x & (float4)(0xFEFFFFFF)) | (float4)(0x00800000))

#define sub_round(n0, n1, n2, n3, rnd_c, n, d, c) \
    do { \
        n1 += *c; \
        float4 nn = n0 * *c; \
        nn = n1 * (nn * nn); \
        nn = fma_break(nn); \
        *n += nn; \
        \
        n3 -= *c; \
        float4 dd = n2 * *c; \
        dd = n3 * (dd * dd); \
        dd = fma_break(dd); \
        *d += dd; \
        \
        *c += rnd_c + (float4)(0.734375f); \
        float4 r = nn + dd; \
        r = (r & (float4)(0x807FFFFF)) | (float4)(0x40000000); \
        *c += r; \
    } while(0)

#define round_compute(n0, n1, n2, n3, rnd_c, c, r) \
    do { \
        float4 n = (float4)(0.0f); \
        float4 d = (float4)(0.0f); \
        \
        for(int i = 0; i < 8; ++i) { \
            sub_round(n0, n1, n2, n3, rnd_c, &n, &d, c); \
            float4 tmp = n0; n0 = n1; n1 = n2; n2 = n3; n3 = tmp; \
        } \
        \
        d = (d & (float4)(0xFF7FFFFF)) | (float4)(0x40000000); \
        *r += n / d; \
    } while(0)

#define single_compute(n0, n1, n2, n3, cnt, rnd_c, sum) \
    ({ \
        float4 c = (float4)(cnt); \
        float4 r = (float4)(0.0f); \
        \
        for(int i = 0; i < 4; ++i) \
            round_compute(n0, n1, n2, n3, rnd_c, &c, &r); \
        \
        r = (r & (float4)(0x807FFFFF)) | (float4)(0x40000000); \
        *sum = r; \
        convert_int4_rte(r * (float4)(536870880.0f)); \
    })

#define single_compute_wrap(rot, v0, v1, v2, v3, cnt, rnd_c, sum, out) \
    do { \
        float4 n0 = convert_float4_rte(v0); \
        float4 n1 = convert_float4_rte(v1); \
        float4 n2 = convert_float4_rte(v2); \
        float4 n3 = convert_float4_rte(v3); \
        \
        int4 r = single_compute(n0, n1, n2, n3, cnt, rnd_c, sum); \
        *out = rot == 0 ? r : (int4)(r.yzwx); \
    } while(0)

)==="
	R"===(

// Move lookup tables to constant memory
__constant uint look[16][4] = {
    {0, 1, 2, 3},
    {0, 2, 3, 1},
    {0, 3, 1, 2},
    {0, 3, 2, 1},
    {1, 0, 2, 3},
    {1, 2, 3, 0},
    {1, 3, 0, 2},
    {1, 3, 2, 0},
    {2, 1, 0, 3},
    {2, 0, 3, 1},
    {2, 3, 1, 0},
    {2, 3, 0, 1},
    {3, 1, 2, 0},
    {3, 2, 0, 1},
    {3, 0, 1, 2},
    {3, 0, 2, 1}
};

__constant float ccnt[16] = {
    1.34375f,
    1.28125f,
    1.359375f,
    1.3671875f,
    1.4296875f,
    1.3984375f,
    1.3828125f,
    1.3046875f,
    1.4140625f,
    1.2734375f,
    1.2578125f,
    1.2890625f,
    1.3203125f,
    1.3515625f,
    1.3359375f,
    1.4609375f
};

struct SharedMemChunk
{
	int4 out[16];
	float4 va[16];
};

__attribute__((reqd_work_group_size(WORKSIZE * 16, 1, 1)))
__kernel void JOIN(cn1_cn_gpu,ALGO)(__global int *lpad_in, __global int *spad, uint numThreads)
{
    const uint gIdx = getIdx();

    #if(COMP_MODE==1)
    if(gIdx/16 >= numThreads)
        return;
    #endif

    uint chunk = get_local_id(0) / 16;

    #if(STRIDED_INDEX==0)
    __global int4* lpad = (__global int4*)((__global char*)lpad_in + MEMORY * (gIdx/16));
    #endif

    __local struct SharedMemChunk {
        int4 out[16];
        float4 va[16];
    } smem_in[WORKSIZE];
    __local struct SharedMemChunk* smem = smem_in + chunk;

    // New local memory cache for frequently accessed data
    __local int4 lpad_cache[32];

    uint tid = get_local_id(0) % 16;

    uint idxHash = gIdx/16;
    uint s = ((__global uint*)spad)[idxHash * 50] >> 8;
    float4 vs = (float4)(0);

    const uint tidd = tid / 4;
    const uint tidm = tid % 4;
    const uint block = tidd * 16 + tidm;

    // Preload frequently accessed data into local memory
    if (tid < 32) {
        lpad_cache[tid] = lpad[tid];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll CN_UNROLL
    for(uint i = 0; i < ITERATIONS; i++)
    {
        barrier(CLK_LOCAL_MEM_FENCE);
        
        smem->out[tid] = scratchpad_ptr(s, tidd, (__global int*)lpad_cache)[tidm];
        
        barrier(CLK_LOCAL_MEM_FENCE);

        single_compute_wrap(
            tidm,
            smem->out[look[tid][0]],
            smem->out[look[tid][1]],
            smem->out[look[tid][2]],
            smem->out[look[tid][3]],
            ccnt[tid], vs, &smem->va[tid],
            &smem->out[tid]
        );

        barrier(CLK_LOCAL_MEM_FENCE);

        // Combine outXor calculation and update of lpad_cache
        int4 outXor = smem->out[block];
        #pragma unroll
        for(uint dd = block + 4; dd < (tidd + 1) * 16; dd += 4)
            outXor ^= smem->out[dd];
        lpad_cache[s % 32] = outXor ^ smem->out[tid];
        smem->out[tid] = outXor;

        // Combine va_tmp calculation and assignment
        smem->va[tid] = (smem->va[block] + smem->va[block + 4]) + (smem->va[block + 8] + smem->va[block + 12]);

        barrier(CLK_LOCAL_MEM_FENCE);

        // Combine out2 and va_tmp2 calculations
        int4 out2 = (smem->out[tid] ^ smem->out[tid + 4]) ^ (smem->out[tid + 8] ^ smem->out[tid + 12]);
        float4 va_tmp2 = fabs((smem->va[block] + smem->va[block + 4]) + (smem->va[block + 8] + smem->va[block + 12]));

        // Combine xx calculation and conversion
        int4 xx_int = convert_int4_rte(va_tmp2 * (float4)(16777216.0f));
        smem->out[tid] = out2 ^ xx_int;
        smem->va[tid] = va_tmp2 * (float4)(0.015625f);  // Vectorized division by 64.0f

        barrier(CLK_LOCAL_MEM_FENCE);

        // Combine vs and s updates
        vs = smem->va[0];
        s = smem->out[0].x ^ smem->out[0].y ^ smem->out[0].z ^ smem->out[0].w;
    }

    // Write back to global memory
    if (tid < 32) {
        lpad[tid] = lpad_cache[tid];
    }
}

)==="
	R"===(

static const __constant uint skip[3] = {
	20,22,22
};

inline void generate_512(uint idx, __local ulong* in, __global ulong* out)
{
	ulong hash[25];

	hash[0] = in[0] ^ idx;
	for(int i = 1; i < 25; ++i)
		hash[i] = in[i];

	for(int a = 0; a < 3;++a)
	{
		keccakf1600_1(hash);
		for(int i = 0; i < skip[a]; ++i)
			out[i] = hash[i];
		out+=skip[a];
	}
}

__attribute__((reqd_work_group_size(8, 8, 1)))
__kernel void JOIN(cn0_cn_gpu,ALGO)(__global ulong *input, __global int *Scratchpad, __global ulong *states, uint Threads)
{
    const uint gIdx = getIdx();
    __local ulong State_buf[8 * 25];
	__local ulong* State = State_buf + get_local_id(0) * 25;

#if(COMP_MODE==1)
    // do not use early return here
	if(gIdx < Threads)
#endif
    {
        states += 25 * gIdx;

#if(STRIDED_INDEX==0)
        Scratchpad = (__global int*)((__global char*)Scratchpad + MEMORY * gIdx);
#endif

        if (get_local_id(1) == 0)
        {

// NVIDIA
#ifdef __NV_CL_C_VERSION
			for(uint i = 0; i < 8; ++i)
				State[i] = input[i];
#else
            ((__local ulong8 *)State)[0] = vload8(0, input);
#endif
            State[8]  = input[8];
            State[9]  = input[9];
            State[10] = input[10];

            ((__local uint *)State)[9]  &= 0x00FFFFFFU;
            ((__local uint *)State)[9]  |= (((uint)get_global_id(0)) & 0xFF) << 24;
            ((__local uint *)State)[10] &= 0xFF000000U;
            /* explicit cast to `uint` is required because some OpenCL implementations (e.g. NVIDIA)
             * handle get_global_id and get_global_offset as signed long long int and add
             * 0xFFFFFFFF... to `get_global_id` if we set on host side a 32bit offset where the first bit is `1`
             * (even if it is correct casted to unsigned on the host)
             */
            ((__local uint *)State)[10] |= (((uint)get_global_id(0) >> 8));

            for (int i = 11; i < 25; ++i) {
                State[i] = 0x00UL;
            }

            // Last bit of padding
            State[16] = 0x8000000000000000UL;

            keccakf1600_2(State);

            #pragma unroll
            for (int i = 0; i < 25; ++i) {
                states[i] = State[i];
            }
        }
	}
}

__attribute__((reqd_work_group_size(64, 1, 1)))
__kernel void JOIN(cn00_cn_gpu,ALGO)(__global int *Scratchpad, __global ulong *states)
{
    const uint gIdx = getIdx() / 64;
    __local ulong State[25];

	states += 25 * gIdx;

#if(STRIDED_INDEX==0)
    Scratchpad = (__global int*)((__global char*)Scratchpad + MEMORY * gIdx);
#endif

	for(int i = get_local_id(0); i < 25; i+=get_local_size(0))
		State[i] = states[i];

	barrier(CLK_LOCAL_MEM_FENCE);


	for(uint i = get_local_id(0); i < MEMORY / 512; i += get_local_size(0))
	{
		generate_512(i, State, (__global ulong*)((__global uchar*)Scratchpad + i*512));
	}
}

)==="