struct FragmentInput {
   float4 position : SV_Position;
};

struct FragmentOutput {
   float4 color : SV_Target0;
};

[shader("fragment")]
FragmentOutput fragmentMain(FragmentInput input) {
   FragmentOutput output;
   output.color = float4(1.0, 0.0, 0.0, 1.0);
   return output;
}
