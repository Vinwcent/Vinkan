struct PushConstant {
    uint value;
};

[[vk::push_constant]] 
ConstantBuffer<PushConstant> pushConstant;

RWStructuredBuffer<uint> buffer_data;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) 
{
    uint index = dispatchThreadID.x;
    
    if (index >= buffer_data.getCount()) {
        return;
    }
		uint32_t *data = &buffer_data[index];
    
    *data += pushConstant.value;
}
