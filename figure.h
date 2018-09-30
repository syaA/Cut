
#pragma once

#include "shader.h"
#include "scene.h"
#include "color.h"


namespace figure
{

class manager;

struct line
{
  vec3 s, e;
  color clr;
  
  line(const vec3& s, const vec3& e, const color& clr)
    : s(s), e(e), clr(clr)
  {}

  void draw(scene*, draw_context*, manager*);
};


struct coordinator
{
  matrix mtx;

  coordinator(const matrix& mtx)
    : mtx(mtx)
  {}

  void draw(scene*, draw_context*, manager*);
};


class manager;

template<class FigureT>
class node : public scene_node
{
public:
  typedef std::shared_ptr<node<FigureT>> ptr_t;
  typedef FigureT figure_t;

public:
  node(manager* m, const figure_t& figure)
    : manager_(m), figure_(figure) {}

  virtual void draw(scene *scn, draw_context *ctx)
  {
    figure_.draw(scn, ctx, manager_);
  }

  figure_t& figure() { return figure_; }
  
private:
  manager *manager_;
  figure_t figure_;
};


class manager
{
public:
  typedef std::shared_ptr<manager> ptr_t;

  struct vertex_t
  {
    vec3 pos;
    color clr;
  };

public:
  manager();
  ~manager();

  template<class FigureT>
  typename node<FigureT>::ptr_t append_to(scene::ptr_t, const FigureT&);

  void draw(scene*, draw_context*, const matrix&, GLenum mode, const vertex_t*,  GLsizei count);

private:
  shader shader_;
  GLuint vertex_buffer_;
};

template<class FigureT>
inline typename node<FigureT>::ptr_t manager::append_to(scene::ptr_t scn, const FigureT& figure)
{
  auto n = scene_node::make<node<FigureT>>(this, figure);
  scn->add_node(n);
  return n;
}

} // end of namespace figure
