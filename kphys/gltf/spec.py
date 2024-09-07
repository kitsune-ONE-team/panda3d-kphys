from panda3d import core as p3d


COMPONENT_TYPE_MAP = {
    5120: p3d.GeomEnums.NT_int8,
    5121: p3d.GeomEnums.NT_uint8,
    5122: p3d.GeomEnums.NT_int16,
    5123: p3d.GeomEnums.NT_uint16,
    5124: p3d.GeomEnums.NT_int32,
    5125: p3d.GeomEnums.NT_uint32,
    5126: p3d.GeomEnums.NT_float32,
}

COMPONENT_SIZE_MAP = {
    5120: 1,
    5121: 1,
    5122: 2,
    5123: 2,
    5124: 4,
    5125: 4,
    5126: 4,
}

COMPONENT_NUM_MAP = {
    'MAT4': 16,
    'VEC4': 4,
    'VEC3': 3,
    'VEC2': 2,
    'SCALAR': 1,
}

ATTRIB_CONTENT_MAP = {
    'vertex': p3d.GeomEnums.C_point,
    'normal': p3d.GeomEnums.C_normal,
    'tangent': p3d.GeomEnums.C_other,
    'texcoord': p3d.GeomEnums.C_texcoord,
    'color': p3d.GeomEnums.C_color,
    'transform_weight': p3d.GeomEnums.C_other,
    'transform_index': p3d.GeomEnums.C_index,
}

ATTRIB_NAME_MAP = {
    'position': p3d.InternalName.get_vertex().get_name(),
    'weights': p3d.InternalName.get_transform_weight().get_name(),
    'joints': p3d.InternalName.get_transform_index().get_name(),
}

PRIMITIVE_MODE_MAP = {
    0: p3d.GeomPoints,
    1: p3d.GeomLines,
    3: p3d.GeomLinestrips,
    4: p3d.GeomTriangles,
    5: p3d.GeomTristrips,
    6: p3d.GeomTrifans,
}

SAMPLER_STATE_FT_MAP = {
    9728: p3d.SamplerState.FT_nearest,
    9729: p3d.SamplerState.FT_linear,
    9984: p3d.SamplerState.FT_nearest_mipmap_nearest,
    9985: p3d.SamplerState.FT_linear_mipmap_nearest,
    9986: p3d.SamplerState.FT_nearest_mipmap_linear,
    9987: p3d.SamplerState.FT_linear_mipmap_linear,
}

SAMPLER_STATE_WM_MAP = {
    33071: p3d.SamplerState.WM_clamp,
    33648: p3d.SamplerState.WM_mirror,
    10497: p3d.SamplerState.WM_repeat,
}
