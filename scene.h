
#pragma once

#include "camera.h"
#include "shader.h"

class scene;
class draw_context;

class scene_node
{
public:
  typedef std::shared_ptr<scene_node> ptr_t;
  typedef std::vector<ptr_t> ptr_array_t;
  
public:
  scene_node();
  scene_node(std::string_view);

  void append_child(ptr_t);
  ptr_array_t& child_array() { return child_array_; }

  const matrix& child_matrix() { return child_matrix_; }
  void set_child_matrix(const matrix& m) { child_matrix_ = m; }

  virtual void draw(scene*, draw_context*) {}

  template<class NodeT, class... Args>
  static typename NodeT::ptr_t make(Args... args)
  {
    return std::make_shared<NodeT>(args...);
  }

private:
  std::string name_;
  ptr_t parent_;
  ptr_array_t child_array_;
  matrix child_matrix_;
};

class draw_context
{
public:
  typedef std::vector<matrix> matrix_stack_t;

public:
  draw_context();

  void push_matrix(const matrix&);
  void pop_matrix();
  const matrix& current_matrix() const { return current_matrix_; }

private:
  void update_current_matrix();

private:
  matrix_stack_t matrix_stack_;
  matrix current_matrix_;
};


class scene
{
public:
  typedef std::shared_ptr<scene> ptr_t;

public:
  scene();
  
  scene_node::ptr_t add_node(scene_node::ptr_t p);
  scene_node::ptr_t add_node(scene_node::ptr_t parent, scene_node::ptr_t p);
  void delete_node(scene_node::ptr_t);

  camera& root_camera() { return camera_; }
  scene_node::ptr_t root_node() { return root_node_; }

  void draw();

  template<class FuncT>
  void traverse_depth_first(FuncT);
  template<class FuncT>
  void traverse_breadth_first(FuncT);

private:
  void draw_impl(scene_node::ptr_t, draw_context*);

  template<class FuncT>
  void traverse_depth_first_impl(scene_node::ptr_t, FuncT);
  template<class FuncT>
  void traverse_breadth_first_impl(scene_node::ptr_t, FuncT);

private:
  scene_node::ptr_t root_node_;
  camera camera_;
};


template<class FuncT>
inline void scene::traverse_depth_first(FuncT func)
{
  traverse_depth_first_impl(root_node_, func);
}

template<class FuncT>
inline void scene::traverse_breadth_first(FuncT func)
{
  traverse_breadth_first_impl(root_node_, func);
}

template<class FuncT>
inline void scene::traverse_depth_first_impl(scene_node::ptr_t p, FuncT func)
{
  auto& child_array = p->child_array();
  if (child_array.empty()) {
    func(p);
  } else {
    for (auto node : child_array) {
      traverse_depth_first_impl(node, func);
    }
  }
}

template<class FuncT>
inline void scene::traverse_breadth_first_impl(scene_node::ptr_t p, FuncT func)
{
  auto& child_array = p->child_array();
  for (auto node : child_array) {
    traverse_breadth_first_impl(node, func);
  }

  func(p);
}

