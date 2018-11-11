
#pragma once

#include "texture.h"


namespace pmx
{

enum ByteSize
{
  ByteSize_Byte = 1,
  ByteSize_Short = 2,
  ByteSize_Int = 4,
};


enum WeightType
{
  WeightType_BDEF1,
  WeightType_BDEF2,
  WeightType_BDEF4,
  WeightType_SDEF,
};


enum MaterialFlag
{
  CullNone = 0x01,
  ShadowCasterGround = 0x02,
  ShadowCaster = 0x04,
  ShadowReceiver = 0x08,
  Edge = 0x10,
};


enum SphereMode
{
  SphereMode_None,
  SphereMode_Mult,
  SphereMode_Add,
  SphereMode_SubTexture,
};


enum Morph
{
  Morph_Group,
  Morph_Vertex,
  Morph_Bone,
  Morph_UV,
  Morph_UV_1,
  Morph_UV_2,
  Morph_UV_3,
  Morph_UV_4,
  Morph_Material,
};

struct info
{
  uint8_t encode;	// 0: utf16, 1:utf8
  uint8_t additional_uv;
  uint8_t sizeof_vertex_index;
  uint8_t sizeof_texture_index;
  uint8_t sizeof_material_index;
  uint8_t sizeof_bone_index;
  uint8_t sizeof_morph_index;
  uint8_t sizeof_rigid_index;
};

struct vertex
{
  vec3 pos;
  vec3 nml;
  vec2 uv;
  vec4 additional_uv[4];
  uint8_t weight_type;
  union {
    struct {
      uint32_t bone;
    } bdef1;
    struct {
      uint32_t bone[2];
      float weight;
    } bdef2;
    struct {
      uint32_t bone[4];
      float weight[4];
    } bdef4;
    struct {
      uint32_t bone[2];
      float weight;
      vec3 c;
      vec3 r0;
      vec3 r1;
    } sdef;
  };
  float edge_scale;
};


struct material
{
  std::string name;
  std::string name_eng;
  color diffuse;
  color specular;
  color ambient;
  uint8_t flags;
  color edge_color;
  float edge_size;
  int32_t texid;
  int32_t sphere_texid;
  SphereMode sphere_mode;
  bool is_common_toon_tex;
  int32_t toon_texid;
  std::string memo;
  int32_t index_count;
};


struct bone
{
  std::string name;
  std::string name_eng;
  vec3 pos;
  int32_t parent;
  int32_t level;
  union {
    uint16_t val;
    struct {
      unsigned connect : 1;
      unsigned enable_rotate : 1;
      unsigned enable_translate : 1;
      unsigned show : 1;
      unsigned enable_control : 1;
      unsigned enable_ik : 1;
      unsigned reserved : 2;
      unsigned assign_rotate : 1;
      unsigned assign_translate : 1;
      unsigned fix_axis : 1;
      unsigned local_axis : 1;
      unsigned after_physics : 1;
      unsigned external : 1;
    };
  } flags;
  vec3 connect_offset;
  int32_t connect_bone;
  int32_t assign_bone;
  float assign_rate;
  vec3 fix_axis;
  vec3 local_axis_x;
  vec3 local_axis_z;
  int32_t external_key;
  int32_t ik_target_bone;
  int32_t ik_loop_count;
  float ik_loop_limit;
  struct ik_link
  {
    int32_t bone;
    int8_t angle_limit;
    vec3 min_angle;
    vec3 max_angle;
  };
  std::vector<ik_link> ik_link_array;
};


struct morph_base
{
  virtual ~morph_base() =default;
  std::string name;
  std::string name_eng;
  int8_t panel;
  int8_t type;
};

struct morph_group : morph_base
{
  struct offset
  {
    int32_t index;
    float rate;
  };
  std::vector<offset> offset_array;
};

struct morph_vertex : morph_base
{
  struct offset
  {
    uint32_t index;
    vec3 translate;
  };
  std::vector<offset> offset_array;
};

struct morph_bone : morph_base
{
  struct offset
  {
    int32_t index;
    vec3 translate;
    vec4 quaternion;
  };
  std::vector<offset> offset_array;

};

struct morph_uv : morph_base
{
  struct offset
  {
    uint32_t index;
    vec4 translate;
  };
  std::vector<offset> offset_array;
};

struct morph_material : morph_base
{
  struct offset
  {
    int32_t index;
    uint8_t op;
    vec4 diffuse;
    vec4 specular;
    vec3 ambient;
    vec4 edge_color;
    float edge_size;
    vec4 tex_coef;
    vec4 spehre_tex_coef;
    vec4 toon_tex_coef;
  };
  std::vector<offset> offset_array;
};



void enable_trace(bool);

class data
{
public:
  typedef std::vector<vertex> vertex_array_t;
  typedef std::vector<uint32_t> index_array_t;
  typedef std::vector<std::string> texture_path_array_t;
  typedef std::vector<material> material_array_t;
  typedef std::vector<bone> bone_array_t;
  typedef std::vector<std::shared_ptr<morph_base>> morph_array_t;

public:
  bool load_from_file(std::string_view filename);

private:
  float version_;;
  info info_;
  vertex_array_t vertex_array_;
  index_array_t index_array_;
  texture_path_array_t texture_path_array_;
  material_array_t material_array_;
  bone_array_t bone_array_;
  morph_array_t morph_array_;
};

} // end of namespace pmx
