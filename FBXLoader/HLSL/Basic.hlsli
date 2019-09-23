#include "LightingHelper.hlsli"


cbuffer CBPerObject
{
    float4x4 world;
    float4x4 world_inv_trans;
    float4x4 world_view_proj;
    float4x4 world_view_proj_tex;
    
    float4x4 tex_transform;
    Material material;
};

Texture2D diffuse_map;
Texture2D shadow_map;
Texture2D ssao_map;
TextureCube cube_map;


//
// Constant buffers.
//

cbuffer CBPerObject
{
    float4x4 world;
    float4x4 world_inv_trans;

    float4x4 tex_mapping;

	// For skinned objects.
    float4x4 bone_transforms[96];
};

cbuffer CBPerFrame
{
    float3 eye_pos;
    float4x4 view;
    float4x4 shadow_transform;
};

cbuffer CBPerResize
{
    float4x4 proj;
};

cbuffer CBChangeRarely
{
    DirectionalLight directional_lights[3];

    float fog_start;
    float fog_range;
    float4 fog_color;

    float shadow_map_size;
};