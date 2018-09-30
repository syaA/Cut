
#include "stdafx.h"

#include "pmx_loader.h"

#include "util.h"


namespace {


template<class... Args>
void pmx_trace(const char *s, Args... args)
{
  char buf[256];
  snprintf(buf, countof(buf), s, args...);
  std::cout << buf;
}

std::string utf8_from_utf16(const std::wstring& u16str)
{
  size_t size = ::WideCharToMultiByte(CP_UTF8, 0, u16str.c_str(), (int)u16str.size(), 0, 0, 0, 0);
  std::string r;
  r.assign(size, '\0');
  ::WideCharToMultiByte(CP_UTF8, 0, u16str.c_str(), (int)u16str.size(), &r[0], (int)r.size(), 0, 0);
  return r;
}

void read_pmx_textbuf(std::ifstream& f, bool utf8, std::string *str)
{
  int32_t len;
  read_int32(f, &len);
  if (utf8) {
    str->assign(len, '\0');
    f.read(&((*str)[0]), len);
  } else {
    std::wstring u16str;
    u16str.assign(len, '\0');
    f.read((char*)&u16str[0], len);
    *str = utf8_from_utf16(u16str);
  }
}

enum PMX_ByteSize
{
  PMX_Byte = 1,
  PMX_Short = 2,
  PMX_Int = 4,
};
void read_pmx_variable(std::ifstream& f, int pmx_bytesize, uint32_t *p, int n = 1)
{
  for (int i=0; i<n; ++i) {
    switch (pmx_bytesize) {
    case PMX_Byte: {
      uint8_t v;
      read_uint8(f, &v);
      p[i] = v;
      break;
    }
    case PMX_Short: {
      uint16_t v;
      read_uint16(f, &v);
      p[i] = v;
      break;
    }
    case PMX_Int:
      read_uint32(f, &p[i]);
      break;
    }
  }
}

void read_pmx_variable_signed(std::ifstream& f, int pmx_bytesize, int32_t *p, int n = 1)
{
  for (int i=0; i<n; ++i) {
    switch (pmx_bytesize) {
    case PMX_Byte: {
      int8_t v;
      read_int8(f, &v);
      p[i] = v;
      break;
    }
    case PMX_Short: {
      int16_t v;
      read_int16(f, &v);
      p[i] = v;
      break;
    }
    case PMX_Int:
      read_int32(f, &p[i]);
      break;
    }
  }
}

enum PMXWeightType
{
  PMX_BDEF1,
  PMX_BDEF2,
  PMX_BDEF4,
  PMX_SDEF,
};

struct pmx_vertex
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

  pmx_vertex() {}
  pmx_vertex(const pmx_vertex& o)
  {
    std::memcpy(this, &o, sizeof(*this));
  }
};

struct pmx_model_vertex
{
  vec3 pos;
  vec3 nml;
  vec2 uv;
  uint32_t bone[4];
  float weight[4];
};
vertex_decl pmx_model_vertex_decl[] = {
  { Semantics_Position, GL_FLOAT, 3, offsetof(pmx_model_vertex, pos), sizeof(pmx_model_vertex) },
  { Semantics_Normal, GL_FLOAT, 3, offsetof(pmx_model_vertex, nml), sizeof(pmx_model_vertex) },
  { Semantics_TexCoord_0, GL_FLOAT, 2, offsetof(pmx_model_vertex, uv), sizeof(pmx_model_vertex) },
};
vertex_decl_array_t get_pmx_model_vertex_decl()
{
  return vertex_decl_array_t(std::begin(pmx_model_vertex_decl),
                             std::end(pmx_model_vertex_decl));
}

enum PMXMaterialFlag
{
  PMX_CullNone = 0x01,
  PMX_ShadowCasterGround = 0x02,
  PMX_ShadowCaster = 0x04,
  PMX_ShadowReceiver = 0x08,
  PMX_Edge = 0x10,
};
enum PMXSphereMode
{
  PMX_None,
  PMX_Mult,
  PMX_Add,
  PMC_SubTexture,
};

struct pmx_material
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
  PMXSphereMode sphere_mode;
  bool is_common_toon_tex;
  int32_t toon_texid;
  std::string memo;
  int32_t index_count;
};

struct pmx_bone
{
  pmx_bone() {}
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

enum PMXMorph
{
  PMXMorph_Group,
  PMXMorph_Vertex,
  PMXMorph_Bone,
  PMXMorph_UV,
  PMXMorph_UV_1,
  PMXMorph_UV_2,
  PMXMorph_UV_3,
  PMXMorph_UV_4,
  PMXMorph_Material,
};

struct pmx_morph_base
{
  virtual ~pmx_morph_base() {}
  std::string name;
  std::string name_eng;
  int8_t panel;
  int8_t type;
};
struct pmx_morph_group : pmx_morph_base
{
  struct offset
  {
    int32_t index;
    float rate;
  };
  std::vector<offset> offset_array;
};
struct pmx_morph_vertex : pmx_morph_base
{
  struct offset
  {
    uint32_t index;
    vec3 translate;
  };
  std::vector<offset> offset_array;
};
struct pmx_morph_bone : pmx_morph_base
{
  struct offset
  {
    int32_t index;
    vec3 translate;
    vec4 quaternion;
  };
  std::vector<offset> offset_array;
};
struct pmx_morph_uv : pmx_morph_base
{
  struct offset
  {
    uint32_t index;
    vec4 translate;
  };
  std::vector<offset> offset_array;
};
struct pmx_morph_material : pmx_morph_base
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

} // end of anonymus namespace


bool load_pmx(model *out, const char *filename, resource_repository *rm)
{
  std::ifstream f;
  f.open(filename, std::ios_base::binary);
  if (f.fail()) {
    return false;
  }

  pmx_trace("pmx file:%s\n", filename);
  
  // ヘッダ.
  uint8_t sig[4];
  read_uint8(f, sig, 4);
  if ((sig[0] != 'P') || (sig[1] != 'M') || (sig[2] != 'X') || (sig[3] != ' ')) {
    return false;
  }
  pmx_trace("%c%c%c%c\n", sig[0], sig[1], sig[2], sig[3]);
  float version;
  read_float(f, &version);
  pmx_trace("Ver:%f\n", version);
  if ((version != 2.0f) && (version != 2.1f)) {
    return false;
  }

  uint8_t info_num;
  read_uint8(f, &info_num);
  if (info_num != 8) {
    return false;
  }
  union {
    uint8_t v[8];
    struct {
      uint8_t encode;
      uint8_t additional_uv;
      uint8_t sizeof_vertex_index;
      uint8_t sizeof_texture_index;
      uint8_t sizeof_material_index;
      uint8_t sizeof_bone_index;
      uint8_t sizeof_morph_index;
      uint8_t sizeof_rigid_index;
    };
  } info;
  read_uint8(f, info.v, info_num);
  pmx_trace("info:(%d)[%d,%d,%d,%d,%d,%d,%d,%d]\n",
            info_num, info.v[0], info.v[1], info.v[2], info.v[3], info.v[4], info.v[5], info.v[6], info.v[7]);

  // コメント.
  std::string model_name, model_name_eng, comment, comment_eng;
  read_pmx_textbuf(f, info.encode, &model_name);
  read_pmx_textbuf(f, info.encode, &model_name_eng);
  read_pmx_textbuf(f, info.encode, &comment);
  read_pmx_textbuf(f, info.encode, &comment_eng);
  pmx_trace("ModelName:%s\n", model_name.c_str());
  pmx_trace("ModelName(eng):%s\n", model_name_eng.c_str());
  pmx_trace("Comment:%s\n", comment.c_str());
  pmx_trace("Comment(eng):%s\n", comment.c_str());

  // 頂点.
  std::vector<pmx_vertex> vertex_array;
  int32_t vertex_cnt;
  read_int32(f, &vertex_cnt);
  vertex_array.reserve(vertex_cnt);
  pmx_trace("Vertex:%d\n", vertex_cnt);
  for (int i=0; i<vertex_cnt; ++i) {
    pmx_vertex vtx;
    read_float(f, as_array(vtx.pos), 3);
    read_float(f, as_array(vtx.nml), 3);
    read_float(f, as_array(vtx.uv), 2);
    for (int j=0; j<info.additional_uv; ++j) {
      read_float(f, as_array(vtx.additional_uv[i]), 2);
    }
    read_uint8(f, &vtx.weight_type);
    switch (vtx.weight_type) {
    case PMX_BDEF1:
      read_pmx_variable(f, info.sizeof_bone_index, &vtx.bdef1.bone);
      break;
    case PMX_BDEF2:
      read_pmx_variable(f, info.sizeof_bone_index, vtx.bdef2.bone, 2);
      read_float(f, &vtx.bdef2.weight);
      break;
    case PMX_BDEF4:
      read_pmx_variable(f, info.sizeof_bone_index, vtx.bdef4.bone, 4);
      read_float(f, vtx.bdef4.weight, 4);
      break;
    case PMX_SDEF:
      read_pmx_variable(f, info.sizeof_bone_index, vtx.sdef.bone, 2);
      read_float(f, &vtx.sdef.weight);
      read_float(f, as_array(vtx.sdef.c), 3);
      read_float(f, as_array(vtx.sdef.r0), 3);
      read_float(f, as_array(vtx.sdef.r1), 3);
      break;
    }
    read_float(f, &vtx.edge_scale);
    vertex_array.push_back(vtx);
  }

  // 面
  std::vector<uint32_t> index_array;
  int32_t index_cnt;
  read_int32(f, &index_cnt);
  index_array.reserve(index_cnt);
  pmx_trace("Face:%d(%d)\n", index_cnt / 3, index_cnt);
  for (int i=0; i<index_cnt; ++i) {
    uint32_t vertex_index;
    read_pmx_variable(f, info.sizeof_vertex_index, &vertex_index);
    index_array.push_back(vertex_index);
  }

  // テクスチャ.
  std::vector<std::string> texture_path_array;
  int32_t texture_cnt;
  read_int32(f, &texture_cnt);
  texture_path_array.reserve(texture_cnt);
  for (int i=0; i<texture_cnt; ++i) {
    std::string path;
    read_pmx_textbuf(f, info.encode, &path);
    texture_path_array.push_back(path);
  }

  // 材質.
  std::vector<pmx_material> material_array;
  int32_t material_cnt;
  read_int32(f, &material_cnt);
  for (int i=0; i<material_cnt; ++i) {
    pmx_material mtrl;
    read_pmx_textbuf(f, info.encode, &mtrl.name);
    read_pmx_textbuf(f, info.encode, &mtrl.name_eng);
    read_float(f, as_array(mtrl.diffuse), 4);
    read_float(f, as_array(mtrl.specular), 4);
    read_float(f, as_array(mtrl.ambient), 3);
    mtrl.ambient.a = 0.f;
    read_uint8(f, &mtrl.flags);
    read_float(f, as_array(mtrl.edge_color), 4);
    read_float(f, &mtrl.edge_size);
    read_pmx_variable_signed(f, info.sizeof_texture_index, &mtrl.texid);
    read_pmx_variable_signed(f, info.sizeof_texture_index, &mtrl.sphere_texid);
    uint8_t sphere_mode;
    read_uint8(f, &sphere_mode);
    mtrl.sphere_mode = (PMXSphereMode)sphere_mode;
    uint8_t is_common_toon_tex;
    read_uint8(f, &is_common_toon_tex);
    mtrl.is_common_toon_tex = is_common_toon_tex == 1;
    if (mtrl.is_common_toon_tex) {
      uint8_t toon_texid;
      read_uint8(f, &toon_texid);
      mtrl.toon_texid = toon_texid;
    } else {
      read_pmx_variable_signed(f, info.sizeof_texture_index, &mtrl.toon_texid);
    }
    read_pmx_textbuf(f, info.encode, &mtrl.memo);
    read_int32(f, &mtrl.index_count);
    material_array.push_back(mtrl);
  }

  // ボーン.
  std::vector<pmx_bone> bone_array;
  int32_t bone_cnt;
  read_int32(f, &bone_cnt);
  for (int i=0; i<bone_cnt; ++i) {
    pmx_bone bone;
    read_pmx_textbuf(f, info.encode, &bone.name);
    read_pmx_textbuf(f, info.encode, &bone.name_eng);
    read_float(f, as_array(bone.pos), 3);
    read_pmx_variable_signed(f, info.sizeof_bone_index, &bone.parent);
    read_int32(f, &bone.level);
    read_uint16(f, &bone.flags.val);
    if (bone.flags.connect == 0) {
      read_float(f, as_array(bone.connect_offset), 3);
    } else {
      read_pmx_variable_signed(f, info.sizeof_bone_index, &bone.connect_bone);
    }
    if (bone.flags.assign_rotate || bone.flags.assign_translate) {
      read_pmx_variable_signed(f, info.sizeof_bone_index, &bone.assign_bone);
      read_float(f, &bone.assign_rate);
    }
    if (bone.flags.fix_axis) {
      read_float(f, as_array(bone.fix_axis), 3);
    }
    if (bone.flags.local_axis) {
      read_float(f, as_array(bone.local_axis_x), 3);
      read_float(f, as_array(bone.local_axis_z), 3);
    }
    if (bone.flags.external) {
      read_int32(f, &bone.external_key);
    }
    if (bone.flags.enable_ik) {
      read_pmx_variable_signed(f, info.sizeof_bone_index, &bone.ik_target_bone);
      read_int32(f, &bone.ik_loop_count);
      read_float(f, &bone.ik_loop_limit);
      int32_t ik_link_count;
      read_int32(f, &ik_link_count);
      bone.ik_link_array.reserve(ik_link_count);
      for (int i=0; i<ik_link_count; ++i) {
        pmx_bone::ik_link link;
        read_pmx_variable_signed(f, info.sizeof_bone_index, &link.bone);
        read_int8(f, &link.angle_limit);
        if (link.angle_limit) {
          read_float(f, as_array(link.min_angle), 3);
          read_float(f, as_array(link.max_angle), 3);
        }
        bone.ik_link_array.push_back(link);
      }
    }
    bone_array.push_back(bone);
  }

  // モーフ.
  std::vector<std::shared_ptr<pmx_morph_base>> morph_array;
  int32_t morph_cnt;
  read_int32(f, &morph_cnt);
  morph_array.reserve(morph_cnt);
  for (int i=0; i<morph_cnt; ++i) {
    std::string name, name_eng;
    read_pmx_textbuf(f, info.encode, &name);
    read_pmx_textbuf(f, info.encode, &name_eng);
    uint8_t panel, type;
    read_uint8(f, &panel);
    read_uint8(f, &type);
    int32_t offset_num;
    read_int32(f, &offset_num);
    std::shared_ptr<pmx_morph_base> morph_base;
    switch (type) {
    case PMXMorph_Group: {
      auto morph = std::make_shared<pmx_morph_group>();
      morph->offset_array.reserve(offset_num);
      for (int i=0; i<offset_num; ++i) {
        pmx_morph_group::offset ofs;
        read_pmx_variable_signed(f, info.sizeof_morph_index, &ofs.index);
        read_float(f, &ofs.rate);
        morph->offset_array.push_back(ofs);
      }
      morph_base = morph;
      break;
    }
    case PMXMorph_Vertex: {
      auto morph = std::make_shared<pmx_morph_vertex>();
      morph->offset_array.reserve(offset_num);
      for (int i=0; i<offset_num; ++i) {
        pmx_morph_vertex::offset ofs;
        read_pmx_variable(f, info.sizeof_vertex_index, &ofs.index);
        read_float(f, as_array(ofs.translate), 3);
        morph->offset_array.push_back(ofs);
      }
      morph_base = morph;
      break;
    }
    case PMXMorph_Bone: {
      auto morph = std::make_shared<pmx_morph_bone>();
      morph->offset_array.reserve(offset_num);
      for (int i=0; i<offset_num; ++i) {
        pmx_morph_bone::offset ofs;
        read_pmx_variable_signed(f, info.sizeof_bone_index, &ofs.index);
        read_float(f, as_array(ofs.translate), 3);
        read_float(f, as_array(ofs.quaternion), 4);
        morph->offset_array.push_back(ofs);
      }
      morph_base = morph;
      break;
    }
    case PMXMorph_UV:
    case PMXMorph_UV_1:
    case PMXMorph_UV_2:
    case PMXMorph_UV_3:
    case PMXMorph_UV_4: {
      auto morph = std::make_shared<pmx_morph_uv>();
      morph->offset_array.reserve(offset_num);
      for (int i=0; i<offset_num; ++i) {
        pmx_morph_uv::offset ofs;
        read_pmx_variable(f, info.sizeof_vertex_index, &ofs.index);
        read_float(f, as_array(ofs.translate), 4);
        morph->offset_array.push_back(ofs);
      }
      morph_base = morph;
      break;
    }
    case PMXMorph_Material: {
      auto morph = std::make_shared<pmx_morph_material>();
      morph->offset_array.reserve(offset_num);
      for (int i=0; i<offset_num; ++i) {
        pmx_morph_material::offset ofs;
        read_pmx_variable_signed(f, info.sizeof_material_index, &ofs.index);
        read_uint8(f, &ofs.op);
        read_float(f, as_array(ofs.diffuse), 4);
        read_float(f, as_array(ofs.specular), 4);
        read_float(f, as_array(ofs.ambient), 3);
        read_float(f, as_array(ofs.edge_color), 4);
        read_float(f, &ofs.edge_size);
        read_float(f, as_array(ofs.tex_coef), 4);
        read_float(f, as_array(ofs.spehre_tex_coef), 4);
        read_float(f, as_array(ofs.toon_tex_coef), 4);
        morph->offset_array.push_back(ofs);
      }
      morph_base = morph;
      break;
    }
    default:
      std::cerr << "unknown morph type " << type << "." << std::endl;
      continue;
    }
    morph_base->name = name;
    morph_base->name_eng = name_eng;
    morph_base->panel = panel;
    morph_base->type = type;
    morph_array.push_back(morph_base);
  }
  
  f.close();

  // 出力.
  // 頂点を変換.
  std::vector<pmx_model_vertex> pmx_model_vertex_array;
  for (const auto& pmx_vtx : vertex_array) {
    pmx_model_vertex vtx;
    vtx.pos = pmx_vtx.pos;
    vtx.nml = pmx_vtx.nml;
    vtx.uv = pmx_vtx.uv;
    switch (pmx_vtx.weight_type) {
    case PMX_BDEF1:
      vtx.bone[0] = pmx_vtx.bdef1.bone;
      vtx.bone[1] = vtx.bone[2] = vtx.bone[3] = -1;
      vtx.weight[0] = 1.f;
      vtx.weight[1] = vtx.weight[2] = vtx.weight[3] = 0.f;
      break;
    case PMX_BDEF2:
      vtx.bone[0] = pmx_vtx.bdef2.bone[0];
      vtx.bone[1] = pmx_vtx.bdef2.bone[1];
      vtx.bone[2] = vtx.bone[3] = -1;
      vtx.weight[0] = pmx_vtx.bdef2.weight;
      vtx.weight[1] = 1.f - pmx_vtx.bdef2.weight;
      vtx.weight[2] = vtx.weight[3] = 0.f;
      break;
    case PMX_BDEF4:
      vtx.bone[0] = pmx_vtx.bdef4.bone[0];
      vtx.bone[1] = pmx_vtx.bdef4.bone[1];
      vtx.bone[2] = pmx_vtx.bdef4.bone[2];
      vtx.bone[3] = pmx_vtx.bdef4.bone[3];
      vtx.weight[0] = pmx_vtx.bdef4.weight[0];
      vtx.weight[1] = pmx_vtx.bdef4.weight[1];
      vtx.weight[2] = pmx_vtx.bdef4.weight[2];
      vtx.weight[3] = pmx_vtx.bdef4.weight[3];
      break;
    case PMX_SDEF:
      vtx.bone[0] = pmx_vtx.sdef.bone[0];
      vtx.bone[1] = pmx_vtx.sdef.bone[1];
      vtx.bone[2] = vtx.bone[3] = -1;
      vtx.weight[0] = pmx_vtx.sdef.weight;
      vtx.weight[1] = 1.f - pmx_vtx.sdef.weight;
      vtx.weight[2] = vtx.weight[3] = 0.f;
      break;
    }
    pmx_model_vertex_array.push_back(vtx);
  }

  // テクスチャを作っておく.
  std::filesystem::path base_dir(filename);
  base_dir.remove_filename();
  std::vector<texture::ptr_t> texture_array;
  texture_array.reserve(texture_path_array.size());
  for (const auto& path : texture_path_array) {
    auto tex = texture::make();
    std::string fullpath = (base_dir / path).string();
    if (!texture::load_from_file(tex, fullpath.c_str())) {
      std::cerr << "cannnot load texture. " << fullpath << std::endl;
    }
    texture_array.push_back(tex);
  }

  // 頂点ストリームは一つ.
  auto vtxstm = vertex_stream_base::make(
    get_pmx_model_vertex_decl(), pmx_model_vertex_array);
  int index_array_start_index = 0;
  for (const auto& pmx_mtrl : material_array) {
    int index_array_end_index = index_array_start_index + pmx_mtrl.index_count;
    auto geom = geometry::make(vtxstm,
                               std::vector<uint32_t>(index_array.begin() + index_array_start_index,
                                                     index_array.begin() + index_array_end_index));
    auto mtrl = material::make();
    mtrl->set_parameter("diffuse", pmx_mtrl.diffuse);
    mtrl->set_parameter("specular", pmx_mtrl.specular);
    mtrl->set_parameter("ambient", pmx_mtrl.ambient);

    // テクスチャ.
    // 無い場合はダミーを入れておく.
    if (pmx_mtrl.texid >= 0) {
      mtrl->set_texture("color_sampler", texture_array[pmx_mtrl.texid]);
    } else {
      mtrl->set_texture("color_sampler", rm->get<texture::ptr_t>("tex_black"));
    }
    if (pmx_mtrl.sphere_texid >= 0) {
      mtrl->set_texture("sphere_sampler", texture_array[pmx_mtrl.sphere_texid]);
    } else {
      if (pmx_mtrl.sphere_mode == PMX_Mult) {
        mtrl->set_texture("sphere_sampler", rm->get<texture::ptr_t>("tex_white"));
      } else {
        mtrl->set_texture("sphere_sampler", rm->get<texture::ptr_t>("tex_black"));
      }
    }
    if (pmx_mtrl.is_common_toon_tex) {
      // ...?
      mtrl->set_texture("toon_sampler", rm->get<texture::ptr_t>("tex_white"));
    } else {
      if (pmx_mtrl.toon_texid >= 0) {
        mtrl->set_texture("toon_sampler", texture_array[pmx_mtrl.toon_texid]);
      } else {
        mtrl->set_texture("toon_sampler", rm->get<texture::ptr_t>("tex_white"));
      }
    }
    out->push(geom, mtrl);

    index_array_start_index = index_array_end_index;
  }
  
  return true;
}

