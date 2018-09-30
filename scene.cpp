
#include "stdafx.h"

#include "scene.h"
#include "shader.h"
#include "model.h"


scene_node::scene_node()
  : scene_node("")
{
}

scene_node::scene_node(std::string_view name)
  : name_(name), child_matrix_(matrix::identity())
{
}

void scene_node::append_child(ptr_t p)
{
  child_array_.push_back(p);
}


draw_context::draw_context()
  : current_matrix_(matrix::identity())
{
}

void draw_context::push_matrix(const matrix& mtx)
{
  matrix_stack_.push_back(mtx);
  update_current_matrix();
}

void draw_context::pop_matrix()
{
  matrix_stack_.pop_back();
  update_current_matrix();
}

void draw_context::update_current_matrix()
{
  current_matrix_ = matrix::identity();
  for (const auto& mtx : matrix_stack_) {
    current_matrix_ = concat(mtx, current_matrix_);
  }
}


scene::scene()
  : root_node_(std::make_shared<scene_node>("root"))
{
}

scene_node::ptr_t scene::add_node(scene_node::ptr_t p)
{
  root_node_->append_child(p);
  return p;
}

scene_node::ptr_t scene::add_node(scene_node::ptr_t parent, scene_node::ptr_t p)
{
  parent->append_child(p);
  return p;
}

void scene::delete_node(scene_node::ptr_t p)
{
  p.reset();
}

void scene::draw()
{
  camera_.makeup_matrix();

  draw_context ctx;
  draw_impl(root_node_, &ctx);
}

void scene::draw_impl(scene_node::ptr_t n, draw_context* ctx)
{
  auto& child_array = n->child_array();
  if (child_array.empty()) {
    n->draw(this, ctx);
  } else {
    ctx->push_matrix(n->child_matrix());
    for (auto& c : child_array) {
      draw_impl(c, ctx);
    }
    ctx->pop_matrix();
    n->draw(this, ctx);
  }
}

