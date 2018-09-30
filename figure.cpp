
#pragma once

#include "stdafx.h"

#include "figure.h"
#include "util.h"



namespace figure
{

const vertex_decl FIGURE_VERTEX_DECL[] = {
  { Semantics_Position, GL_FLOAT, 3, offsetof(manager::vertex_t, pos),   sizeof(manager::vertex_t) },
  { Semantics_Color,    GL_FLOAT, 4, offsetof(manager::vertex_t, clr), sizeof(manager::vertex_t) },
};

void line::draw(scene *scn, draw_context *ctx, manager *m)
{
  manager::vertex_t vert[] = {
    { s, clr },
    { e, clr },
  };

  m->draw(scn, ctx, matrix::identity(), GL_LINES, vert, countof(vert));
}

void coordinator::draw(scene *scn, draw_context *ctx, manager *m)
{
  vec3 o(mtx._03, mtx._13, mtx._23);
  vec3 x(mtx._00, mtx._10, mtx._20);
  vec3 y(mtx._01, mtx._11, mtx._21);
  vec3 z(mtx._02, mtx._12, mtx._22);
  manager::vertex_t vert[] = {
    { o,     color(1.f, 0.f, 0.f, 1.f) },
    { o + x, color(1.f, 0.f, 0.f, 1.f) },
    { o,     color(0.f, 1.f, 0.f, 1.f) },
    { o + y, color(0.f, 1.f, 0.f, 1.f) },
    { o,     color(0.f, 0.f, 1.f, 1.f) },
    { o + z, color(0.f, 0.f, 1.f, 1.f) },
  };
  m->draw(scn, ctx, matrix::identity(), GL_LINES, vert, countof(vert));
}


manager::manager()
{
  shader_.compile_from_source_file("assets/shader/simple.vsh", "assets/shader/simple.fsh");
  glGenBuffers(1, &vertex_buffer_);
}

manager::~manager()
{
  glDeleteBuffers(1, &vertex_buffer_);
}

void manager::draw(scene *scn, draw_context *ctx, const matrix& mtx, GLenum mode, const vertex_t *vertices, GLsizei count)
{
  shader_.use();

  matrix m = concat(ctx->current_matrix(), mtx);
  matrix mv = concat(scn->root_camera().view_matrix(), m);
  matrix mvp = concat(scn->root_camera().projection_matrix(), mv);

  shader_.set_uniform("MVP", mvp);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * count, vertices, GL_STATIC_DRAW);
  
  for (const auto& decl : FIGURE_VERTEX_DECL) {
    shader_.set_attrib(decl);
  }

  glDrawArrays(mode, 0, count);
  glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
}

}	// end of namespace figure
