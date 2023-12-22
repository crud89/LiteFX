#pragma pack_matrix(row_major)

#define KERNEL_SIZE 8  // NOTE: Must be even!
#define SIGMA       10

RWTexture2D<float4> FrameBuffer : register(u0, space0);

// Evaluates gaussian bell curve for a value n.
float gauss(float sigma, int n)
{
    return exp(-pow(n, 2.0) / (2.0 * pow(sigma, 2.0)));
}

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{   
    int width, height;
    FrameBuffer.GetDimensions(width, height);
    
    // Compute 1D gaussian kernel.
    float kernel[KERNEL_SIZE + 1];
    float weight = 0.0;
    
    [unroll((KERNEL_SIZE / 2) + 1)]
    for (int n = 0; n <= KERNEL_SIZE / 2; ++n)
        kernel[KERNEL_SIZE / 2 + n] = kernel[KERNEL_SIZE / 2 - n] = gauss(SIGMA, n);
    
    [unroll(KERNEL_SIZE + 1)]
    for (int s = 0; s < KERNEL_SIZE + 1; ++s)
        weight += kernel[s];
    
    // Compute weighted color.
    float3 color = float3(0.0, 0.0, 0.0);
    
    for (int x = -KERNEL_SIZE / 2; x <= KERNEL_SIZE / 2; ++x)
    {
        [unroll(KERNEL_SIZE + 1)]
        for (int y = -KERNEL_SIZE / 2; y <= KERNEL_SIZE / 2; ++y)
        {
            // NOTE: This samples pixels on the edges of the frame buffer multiple times, which is inaccurate but serves the purpose of demonstration.
            float coefficient = kernel[(KERNEL_SIZE / 2) + x] * kernel[(KERNEL_SIZE / 2) + y];
            int2 sampleLocation = int2(max(min(id.x + x, width), 0), max(min(id.y + y, height), 0));
            color += FrameBuffer.Load(sampleLocation).rgb;
        }
    }
    
    FrameBuffer[id.xy] = float4(color / pow(weight, 2.0), 1.0);
}