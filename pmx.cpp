
#include "stdafx.h"

#include "pmx.h"
#include "utf.h"
#include "util.h"


namespace
{

bool g_trace_enabled = false;

template<class... Args>
void pmx_trace(const char *s, Args... args)
{
  if (!g_trace_enabled) {
    return;
  }
  char buf[256];
  snprintf(buf, countof(buf), s, args...);
  std::cout << buf;
}


void read_pmx_textbuf(std::ifstream& f, bool utf8, std::string *str)
{
  int32_t len;
  read(f, &len);
  if (utf8) {
    str->assign(len, '\0');
    f.read(&((*str)[0]), len);
  } else {
    std::u16string u16str;
    u16str.assign(len, '\0');
    f.read((char*)&u16str[0], len);
    *str = u8_from_u16(u16str);
  }
}

void read_pmx_variable(std::ifstream& f, int pmx_bytesize, uint32_t *p, int n = 1)
{
  for (int i=0; i<n; ++i) {
    switch (pmx_bytesize) {
    case pmx::ByteSize_Byte: {
      uint8_t v;
      read(f, &v);
      p[i] = v;
      break;
    }
    case pmx::ByteSize_Short: {
      uint16_t v;
      read(f, &v);
      p[i] = v;
      break;
    }
    case pmx::ByteSize_Int:
      read(f, &p[i]);
      break;
    }
  }
}

void read_pmx_variable_signed(std::ifstream& f, int pmx_bytesize, int32_t *p, int n = 1)
{
  for (int i=0; i<n; ++i) {
    switch (pmx_bytesize) {
    case pmx::ByteSize_Byte: {
      int8_t v;
      read(f, &v);
      p[i] = v;
      break;
    }
    case pmx::ByteSize_Short: {
      int16_t v;
      read(f, &v);
      p[i] = v;
      break;
    }
    case pmx::ByteSize_Int:
      read(f, &p[i]);
      break;
    }
  }
}


} // end of anonymus namespace

namespace pmx
{


void enable_trace(bool b)
{
  g_trace_enabled = b;
}

bool data::load_from_file(std::string_view filename)
{
  std::ifstream f;
  f.open(filename, std::ios_base::binary);
  if (f.fail()) {
    return false;
  }

  pmx_trace("pmx load file:%s\n", filename);
  
  // ヘッダ.
  uint8_t sig[4];
  read(f, sig);
  if ((sig[0] != 'P') || (sig[1] != 'M') || (sig[2] != 'X') || (sig[3] != ' ')) {
    return false;
  }

  float version;
  read(f, &version);
  pmx_trace("pmv version:%f\n", version);
  if ((version != 2.0f) && (version != 2.1f)) {
    return false;
  }

  uint8_t info_num;
  read(f, &info_num);
  if (info_num != 8) {
    return false;
  }
  read(f, &info_);
//  pmx_trace("pmx info_:(%d)[%d,%d,%d,%d,%d,%d,%d,%d]\n",
//            info_num, info_.v[0], info_.v[1], info_.v[2], info_.v[3], info_.v[4], info_.v[5], info_.v[6], info_.v[7]);

  // コメント.
  std::string model_name, model_name_eng, comment, comment_eng;
  read_pmx_textbuf(f, info_.encode, &model_name);
  read_pmx_textbuf(f, info_.encode, &model_name_eng);
  read_pmx_textbuf(f, info_.encode, &comment);
  read_pmx_textbuf(f, info_.encode, &comment_eng);
  pmx_trace("pmx model_name:%s\n", model_name.c_str());
  pmx_trace("pmx model_name(eng):%s\n", model_name_eng.c_str());
  pmx_trace("pmx comment:%s\n", comment.c_str());
  pmx_trace("pmx comment(eng):%s\n", comment_eng.c_str());

  // 頂点.
  int32_t vertex_cnt;
  read(f, &vertex_cnt);
  vertex_array_.reserve(vertex_cnt);
  pmx_trace("pmx vertex count:%d\n", vertex_cnt);
  for (int i=0; i<vertex_cnt; ++i) {
    vertex_array_.emplace_back();
    vertex& vtx = vertex_array_.back();
    read(f, &vtx.pos);
    read(f, &vtx.nml);
    read(f, &vtx.uv);
    for (int j=0; j<info_.additional_uv; ++j) {
      read(f, &vtx.additional_uv[j]);
    }
    read(f, &vtx.weight_type);
    switch (vtx.weight_type) {
    case WeightType_BDEF1:
      read_pmx_variable(f, info_.sizeof_bone_index, &vtx.bdef1.bone);
      break;
    case WeightType_BDEF2:
      read_pmx_variable(f, info_.sizeof_bone_index, vtx.bdef2.bone, 2);
      read(f, &vtx.bdef2.weight);
      break;
    case WeightType_BDEF4:
      read_pmx_variable(f, info_.sizeof_bone_index, vtx.bdef4.bone, 4);
      read(f, vtx.bdef4.weight);
      break;
    case WeightType_SDEF:
      read_pmx_variable(f, info_.sizeof_bone_index, vtx.sdef.bone, 2);
      read(f, &vtx.sdef.weight);
      read(f, &vtx.sdef.c);
      read(f, &vtx.sdef.r0);
      read(f, &vtx.sdef.r1);
      break;
    }
    read(f, &vtx.edge_scale);
  }

  // 面
  int32_t index_cnt;
  read(f, &index_cnt);
  index_array_.reserve(index_cnt);
  pmx_trace("pmx face count:%d(%d)\n", index_cnt / 3, index_cnt);
  for (int i=0; i<index_cnt; ++i) {
    uint32_t index;
    read_pmx_variable(f, info_.sizeof_vertex_index, &index);
    index_array_.push_back(index);
  }

  // テクスチャ.
  int32_t texture_cnt;
  read(f, &texture_cnt);
  pmx_trace("pmx texture count:%d\n", texture_cnt);
  texture_path_array_.reserve(texture_cnt);
  for (int i=0; i<texture_cnt; ++i) {
    std::string path;
    read_pmx_textbuf(f, info_.encode, &path);
    texture_path_array_.push_back(path);
  }

  // 材質.
  int32_t material_cnt;
  read(f, &material_cnt);
  pmx_trace("pmx material count:%d\n", material_cnt);
  material_array_.reserve(material_cnt);
  for (int i=0; i<material_cnt; ++i) {
    material mtrl;
    read_pmx_textbuf(f, info_.encode, &mtrl.name);
    read_pmx_textbuf(f, info_.encode, &mtrl.name_eng);
    read(f, &mtrl.diffuse);
    read(f, &mtrl.specular);
    read(f, as_array(mtrl.ambient), 3);
    mtrl.ambient.a = 0.f;
    read(f, &mtrl.flags);
    read(f, &mtrl.edge_color);
    read(f, &mtrl.edge_size);
    read_pmx_variable_signed(f, info_.sizeof_texture_index, &mtrl.texid);
    read_pmx_variable_signed(f, info_.sizeof_texture_index, &mtrl.sphere_texid);
    uint8_t sphere_mode;
    read(f, &sphere_mode);
    mtrl.sphere_mode = (SphereMode)sphere_mode;
    uint8_t is_common_toon_tex;
    read(f, &is_common_toon_tex);
    mtrl.is_common_toon_tex = is_common_toon_tex == 1;
    if (mtrl.is_common_toon_tex) {
      uint8_t toon_texid;
      read(f, &toon_texid);
      mtrl.toon_texid = toon_texid;
    } else {
      read_pmx_variable_signed(f, info_.sizeof_texture_index, &mtrl.toon_texid);
    }
    read_pmx_textbuf(f, info_.encode, &mtrl.memo);
    read(f, &mtrl.index_count);
    material_array_.push_back(mtrl);
  }

  // ボーン.
  int32_t bone_cnt;
  read(f, &bone_cnt);
  pmx_trace("pmx bone count:%d\n", bone_cnt);
  bone_array_.reserve(bone_cnt);
  for (int i=0; i<bone_cnt; ++i) {
    bone bne;
    read_pmx_textbuf(f, info_.encode, &bne.name);
    read_pmx_textbuf(f, info_.encode, &bne.name_eng);
    read(f, &bne.pos);
    read_pmx_variable_signed(f, info_.sizeof_bone_index, &bne.parent);
    read(f, &bne.level);
    read(f, &bne.flags.val);
    if (bne.flags.connect == 0) {
      read(f, &bne.connect_offset);
    } else {
      read_pmx_variable_signed(f, info_.sizeof_bone_index, &bne.connect_bone);
    }
    if (bne.flags.assign_rotate || bne.flags.assign_translate) {
      read_pmx_variable_signed(f, info_.sizeof_bone_index, &bne.assign_bone);
      read(f, &bne.assign_rate);
    }
    if (bne.flags.fix_axis) {
      read(f, &bne.fix_axis);
    }
    if (bne.flags.local_axis) {
      read(f, &bne.local_axis_x);
      read(f, &bne.local_axis_z);
    }
    if (bne.flags.external) {
      read(f, &bne.external_key);
    }
    if (bne.flags.enable_ik) {
      read_pmx_variable_signed(f, info_.sizeof_bone_index, &bne.ik_target_bone);
      read(f, &bne.ik_loop_count);
      read(f, &bne.ik_loop_limit);
      int32_t ik_link_count;
      read(f, &ik_link_count);
      bne.ik_link_array.reserve(ik_link_count);
      for (int i=0; i<ik_link_count; ++i) {
        bone::ik_link link;
        read_pmx_variable_signed(f, info_.sizeof_bone_index, &link.bone);
        read(f, &link.angle_limit);
        if (link.angle_limit) {
          read(f, &link.min_angle);
          read(f, &link.max_angle);
        }
        bne.ik_link_array.push_back(link);
      }
    }
    bone_array_.push_back(bne);
  }

  // モーフ.
  int32_t morph_cnt;
  read(f, &morph_cnt);
  morph_array_.reserve(morph_cnt);
  for (int i=0; i<morph_cnt; ++i) {
    std::string name, name_eng;
    read_pmx_textbuf(f, info_.encode, &name);
    read_pmx_textbuf(f, info_.encode, &name_eng);
    uint8_t panel, type;
    read(f, &panel);
    read(f, &type);
    int32_t offset_num;
    read(f, &offset_num);
    std::shared_ptr<morph_base> morph_base;
    switch (type) {
    case Morph_Group: {
      auto morph = std::make_shared<morph_group>();
      morph->offset_array.reserve(offset_num);
      for (int i=0; i<offset_num; ++i) {
        morph_group::offset ofs;
        read_pmx_variable_signed(f, info_.sizeof_morph_index, &ofs.index);
        read(f, &ofs.rate);
        morph->offset_array.push_back(ofs);
      }
      morph_base = morph;
      break;
    }
    case Morph_Vertex: {
      auto morph = std::make_shared<morph_vertex>();
      morph->offset_array.reserve(offset_num);
      for (int i=0; i<offset_num; ++i) {
        morph_vertex::offset ofs;
        read_pmx_variable(f, info_.sizeof_vertex_index, &ofs.index);
        read(f, &ofs.translate);
        morph->offset_array.push_back(ofs);
      }
      morph_base = morph;
      break;
    }
    case Morph_Bone: {
      auto morph = std::make_shared<morph_bone>();
      morph->offset_array.reserve(offset_num);
      for (int i=0; i<offset_num; ++i) {
        morph_bone::offset ofs;
        read_pmx_variable_signed(f, info_.sizeof_bone_index, &ofs.index);
        read(f, &ofs.translate);
        read(f, &ofs.quaternion);
        morph->offset_array.push_back(ofs);
      }
      morph_base = morph;
      break;
    }
    case Morph_UV:
    case Morph_UV_1:
    case Morph_UV_2:
    case Morph_UV_3:
    case Morph_UV_4: {
      auto morph = std::make_shared<morph_uv>();
      morph->offset_array.reserve(offset_num);
      for (int i=0; i<offset_num; ++i) {
        morph_uv::offset ofs;
        read_pmx_variable(f, info_.sizeof_vertex_index, &ofs.index);
        read(f, &ofs.translate);
        morph->offset_array.push_back(ofs);
      }
      morph_base = morph;
      break;
    }
    case Morph_Material: {
      auto morph = std::make_shared<morph_material>();
      morph->offset_array.reserve(offset_num);
      for (int i=0; i<offset_num; ++i) {
        morph_material::offset ofs;
        read_pmx_variable_signed(f, info_.sizeof_material_index, &ofs.index);
        read(f, &ofs.op);
        read(f, &ofs.diffuse);
        read(f, &ofs.specular);
        read(f, &ofs.ambient);
        read(f, &ofs.edge_color);
        read(f, &ofs.edge_size);
        read(f, &ofs.tex_coef);
        read(f, &ofs.spehre_tex_coef);
        read(f, &ofs.toon_tex_coef);
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
    morph_array_.push_back(morph_base);
  }
  f.close();
  
  return true;
}


} // end of namespace pmx
