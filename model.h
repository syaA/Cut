
#pragma once

#include "shader.h"
#include "texture.h"
#include "scene.h"


// 頂点ストリーム.
class vertex_stream_base
{
public:
  typedef std::shared_ptr<vertex_stream_base> ptr_t;

public:
  vertex_stream_base(const vertex_decl_array_t& vertex_decl_arary);
  virtual ~vertex_stream_base();

  virtual void *vertex_array() { return 0; }
  virtual size_t vertex_count() { return 0; }
  virtual size_t vertex_buffer_size() { return 0; }

  vertex_decl_array_t& vertex_decl_array() { return vertex_decl_array_; }
  const vertex_decl_array_t& vertex_decl_array() const { return vertex_decl_array_; }

  GLuint globj_vertex_buffer() { return vertex_buffer_; }

protected:
  void setup_buffer();

private:
  vertex_decl_array_t vertex_decl_array_;
  GLuint vertex_buffer_;

public:
  template<class VertexArrayT>
  static auto make(const vertex_decl_array_t&, const VertexArrayT&);
};

template<class VertexT>
class vertex_stream : public vertex_stream_base
{
public:
  typedef vertex_stream_base base_t;
  typedef VertexT vertex_t;
  typedef std::vector<vertex_t> vertex_array_t;
  
public:
  vertex_stream(const vertex_decl_array_t& vertex_decl_arary, const vertex_array_t& vertex_array)
    : vertex_stream_base(vertex_decl_arary), vertex_array_(vertex_array)
  {
    base_t::setup_buffer();
  }

  virtual void *vertex_array() { return vertex_array_.empty() ? 0 : &vertex_array_[0]; }
  virtual size_t vertex_count() { return vertex_array_.size(); }
  virtual size_t vertex_buffer_size() { return vertex_array_.size() * sizeof(vertex_t); }
  
private:
  vertex_array_t vertex_array_;
};

template<class VertexArrayT>
static inline auto vertex_stream_base::make(
  const vertex_decl_array_t& vertex_decl_array, const VertexArrayT& vertex_array)
{
  typedef VertexArrayT::value_type vertex_t;
  return std::make_shared<vertex_stream<vertex_t>>(vertex_decl_array, vertex_array);
}


// ジオメトリ.
class geometry
{
public:
  typedef std::vector<std::uint32_t> index_array_t;
  typedef std::shared_ptr<geometry> ptr_t;

public:
  geometry(vertex_stream_base::ptr_t, const index_array_t&);
  ~geometry();

  void *index_array() { return &index_array_[0]; }
  size_t index_count() { return index_array_.size(); }
  size_t index_buffer_size() { return index_count() * 4; }

  vertex_stream_base::ptr_t vertex_stream() { return vertex_stream_; }

  GLuint globj_index_buffer() { return index_buffer_; }

private:
  vertex_stream_base::ptr_t vertex_stream_;
  index_array_t index_array_;
  GLuint index_buffer_;

public:
  static auto make(vertex_stream_base::ptr_t vertex_stream, const index_array_t& index_array)
  {
    return std::make_shared<geometry>(vertex_stream, index_array);
  }

};


// マテリアル.
class material
{
public:
  typedef std::shared_ptr<material> ptr_t;

  class parameter_t
  {
  public:
    enum Type
    {
      Type_None,
      Type_Float,
      Type_Int,
      Type_Uint,
      Type_Color,
      Type_Matrix,
    };

  public:
#define DECL_PARAMETER_T_CTOR_TYPE(type, dim, Type) \
    parameter_t(const type& v) : parameter_t(Type, 1, dim, sizeof(type), &v) {} \
    parameter_t(size_t num, const type *v) : parameter_t(Type, num, dim, sizeof(type), v) {} \
    parameter_t(std::initializer_list<type> init) \
      : parameter_t(Type, init.size(), dim, sizeof(type), init.begin()) {}

#define DECL_PARAMETER_T_CTOR(type, vectype, Type) \
    DECL_PARAMETER_T_CTOR_TYPE(type, 1, Type) \
    DECL_PARAMETER_T_CTOR_TYPE(vectype ## 2, 2, Type) \
    DECL_PARAMETER_T_CTOR_TYPE(vectype ## 3, 3, Type) \
    DECL_PARAMETER_T_CTOR_TYPE(vectype ## 4, 4, Type) \
    parameter_t(type v0, type v1) : parameter_t(1, {v0, v1}) {} \
    parameter_t(type v0, type v1, type v2) : parameter_t(1, {v0, v1, v2}) {} \
    parameter_t(type v0, type v1, type v2, type v3) : parameter_t(1, {v0, v1, v2, v3}) {} \
    parameter_t(size_t num, int dim, const type *v) : parameter_t(Type, num, dim, sizeof(type), v) {} \
    parameter_t(int dim, std::initializer_list<type> init) \
      : parameter_t(Type, init.size(), dim, sizeof(type), init.begin()) {}

    DECL_PARAMETER_T_CTOR(float, vec, Type_Float)
    DECL_PARAMETER_T_CTOR(int, ivec, Type_Int)
    DECL_PARAMETER_T_CTOR(unsigned, uvec, Type_Uint)

    DECL_PARAMETER_T_CTOR_TYPE(color, 1, Type_Color)
    DECL_PARAMETER_T_CTOR_TYPE(matrix, 1, Type_Matrix)

#undef DECL_PARAMETER_T_CTOR
#undef DECL_PARAMETER_T_CTOR_TYPE

    parameter_t() : type_(Type_None) {}
    parameter_t(Type type, size_t num, int dim, size_t size, const void*);
    parameter_t(parameter_t&& o)
      : type_(o.type_), num_(o.num_), dim_(o.dim_), storage_(std::move(o.storage_)) {}
    parameter_t& operator=(parameter_t&& o)
    {
      type_ = o.type_;
      num_ = o.num_;
      dim_ = o.dim_;
      storage_ = std::move(o.storage_);
      return *this;
    }

    void set_to(const char *name, shader*) const;

  private:
    Type type_;
    size_t num_;
    int dim_;
    std::unique_ptr<uint8_t[]> storage_;
  };

  typedef std::unordered_map<std::string, parameter_t> parameter_map_t;
  typedef std::unordered_map<std::string, texture::ptr_t> texture_map_t;

public:
  material();
  ~material();

  template<class... Args>
  void set_parameter(const char *name, Args&&... args)
  {
    std::string key(name);
    if (parameter_map_.count(key)) {
      parameter_map_[key] = parameter_t(args...);
    } else {
      parameter_map_.try_emplace(name, args...);
    }
  }

  void set_texture(const char *name, texture::ptr_t tex)
  {
    texture_map_[name] = tex;
  }

  const parameter_map_t& parameter_map() const { return parameter_map_; }
  const texture_map_t& texture_map() const { return texture_map_; }

private:
  parameter_map_t parameter_map_;
  texture_map_t texture_map_;

  
public:
  static auto make() {
    return std::make_shared<material>();
  }
};


// モデル.
class model
{
public:
  typedef std::vector<vertex_stream_base::ptr_t> vertex_stream_array_t;
  typedef std::vector<geometry::ptr_t> geometry_array_t;
  typedef std::vector<material::ptr_t> material_array_t;
  typedef std::vector<texture::ptr_t> texture_array_t;

  struct section
  {
    geometry::ptr_t geom;
    material::ptr_t mtrl;
  };
  typedef std::vector<section> section_array_t;

public:
  model();
  ~model();

  void push(geometry::ptr_t, material::ptr_t);

  const section_array_t& section_array() { return section_array_; }

private:
  vertex_stream_array_t vertex_stream_array_;
  geometry_array_t geometry_array_;
  material_array_t material_array_;
  texture_array_t texture_array_;
  section_array_t section_array_;
};


class model_node : public scene_node
{
public:
  model_node(std::shared_ptr<model> model, std::shared_ptr<shader> shader);
  virtual void draw(scene*, draw_context*);

  const matrix& world_matrix() const { return mtx_; }
  void set_world_matrix(const matrix& m) { mtx_ = m; }

private:
  std::shared_ptr<model> model_;
  std::shared_ptr<shader> shader_;
  matrix mtx_;
};
