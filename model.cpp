
#include "stdafx.h"

#include "model.h"


namespace {

template<class ContT>
void push_back_unique(ContT& cont, const typename ContT::value_type& v)
{
  if (std::find(cont.begin(), cont.end(), v) == cont.end()) {
    cont.push_back(v);
  }
}

} // end of anonymus namespace


vertex_stream_base::vertex_stream_base(const vertex_decl_array_t& vertex_decl_arary)
  : vertex_decl_array_(vertex_decl_arary), vertex_buffer_(0)
{
  glGenBuffers(1, &vertex_buffer_);
}
  
vertex_stream_base::~vertex_stream_base()
{
  glDeleteBuffers(1, &vertex_buffer_);
}

void vertex_stream_base::setup_buffer()
{
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size(), vertex_array(), GL_STATIC_DRAW);
}


geometry::geometry(vertex_stream_base::ptr_t vertex_stream, const index_array_t& index_array)
  : vertex_stream_(vertex_stream), index_array_(index_array), index_buffer_(0)
{
  glGenBuffers(1, &index_buffer_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size(), this->index_array(), GL_STATIC_DRAW);
}


geometry::~geometry()
{
  glDeleteBuffers(1, &index_buffer_);
}


material::parameter_t::parameter_t(Type type, size_t num, int dim, size_t size, const void *p)
  : type_(type), num_(num), dim_(dim), storage_(std::make_unique<uint8_t[]>(num * dim * size))
{
  std::memcpy(storage_.get(), p, num * dim * size);
}

void material::parameter_t::set_to(const char *name, shader *shdr) const
{
  switch (type_) {
  case Type_Float: shdr->set_uniform(name, (GLsizei)num_, dim_, (const float*)storage_.get()); break;
  case Type_Int: shdr->set_uniform(name, (GLsizei)num_, dim_, (const int*)storage_.get()); break;
  case Type_Uint: shdr->set_uniform(name, (GLsizei)num_, dim_, (const unsigned*)storage_.get()); break;
  case Type_Color: shdr->set_uniform(name, (GLsizei)num_, (const color*)storage_.get()); break;
  case Type_Matrix: shdr->set_uniform(name, (GLsizei)num_, (const matrix*)storage_.get()); break;
  }
}



material::material()
{
}

material::~material()
{
}


model::model()
{
}

model::~model()
{
}

void model::push(geometry::ptr_t geom, material::ptr_t mtrl)
{
  push_back_unique(vertex_stream_array_, geom->vertex_stream());
  geometry_array_.push_back(geom);
  material_array_.push_back(mtrl);
  for (const auto& [key, tex] : mtrl->texture_map()) {
    push_back_unique(texture_array_, tex);
  }

  section_array_.push_back({geom, mtrl});
}



model_node::model_node(std::shared_ptr<model> model, std::shared_ptr<shader> shader)
  : model_(model), shader_(shader), mtx_(matrix::identity())
{
}

void model_node::draw(scene *scn, draw_context *ctx)
{
  shader_->use();

  matrix m = concat(ctx->current_matrix(), mtx_);
  matrix mv = concat(scn->root_camera().view_matrix(), m);
  matrix mvp = concat(scn->root_camera().projection_matrix(), mv);

  shader_->set_uniform("MVP", mvp);
  
  for (auto [geom, mtrl] : model_->section_array()) {
    vertex_stream_base::ptr_t vtxstm = geom->vertex_stream();
    glBindBuffer(GL_ARRAY_BUFFER, vtxstm->globj_vertex_buffer());
    for (const auto& decl: vtxstm->vertex_decl_array()) {
      shader_->set_attrib(decl);
    }
    for (const auto& [name, param] : mtrl->parameter_map()) {
      param.set_to(name.c_str(), shader_.get());
    }
    int texture_index = 0;
    for (const auto& [name, tex] : mtrl->texture_map()) {
      glActiveTexture(GL_TEXTURE0 + texture_index);
      glBindTexture(GL_TEXTURE_2D, tex->texture_globj());
      glBindSampler(texture_index, tex->sampler_globj());
      shader_->set_uniform(name.c_str(), texture_index);
      ++texture_index;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->globj_index_buffer());
    glDrawElements(GL_TRIANGLES,
                   (GLsizei)geom->index_count(),
                   GL_UNSIGNED_INT,
                   0);
  }
}

