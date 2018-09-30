
#pragma once


enum Semantics
{
  Semantics_Position,
  Semantics_Normal,
  Semantics_Color,
  Semantics_TexCoord_0,
  Semantics_TexCoord_1,
  Semantics_TexCoord_2,
  Semantics_TexCoord_3,
  Semantics_TexCoord_4,
  Semantics_TexCoord_5,
  Semantics_TexCoord_6,
  Semantics_TexCoord_7,

  Semantics_Num
};


// 頂点宣言.
struct vertex_decl
{
  Semantics semantics;
  GLenum type;
  GLint size;
  size_t offset;
  GLsizei stride;
};
typedef std::vector<vertex_decl> vertex_decl_array_t;


class vertex_shader
{
public:
  vertex_shader();
  ~vertex_shader();

  bool compile_from_source_file(const char *filename);

  GLuint globj() { return shader_; }
  
private:
  GLuint shader_;

};


class fragment_shader
{
public:
  fragment_shader();
  ~fragment_shader();

  bool compile_from_source_file(const char *filename);

  GLuint globj() { return shader_; }

private:
  GLuint shader_;
};


class model;

class shader_program
{
public:
  shader_program();
  ~shader_program();

  void attach(vertex_shader*);
  void attach(fragment_shader*);

  void link();

  GLint uniform_location(const char*);
  GLint attrib_location(const char*);
  GLint attrib_location(Semantics);
  
  void use();

  GLuint globj() { return program_; }

private:
  GLuint program_;
};


class shader
{
public:
  shader() {}
  virtual ~shader() =default;

  virtual void use();

  bool compile_from_source_file(const char *vs, const char *fs);

  void set_attrib(const vertex_decl&);
  void set_attrib(const char *name,
                  GLint size,
                  GLenum type,
                  GLsizei stride,
                  size_t offset);

  void validate();

  GLuint globj() { return shader_program_.globj(); }

  shader_program *get_shader_program() { return &shader_program_; }
  vertex_shader *get_vertex_shader() { return &vertex_shader_; }
  fragment_shader *get_fragment_shader() { return &fragment_shader_; }

  
#define DECL_SET_UNIFORM(type, vectype) \
  void set_uniform(const char *name, type); \
  void set_uniform(const char *name, type, type); \
  void set_uniform(const char *name, type, type, type); \
  void set_uniform(const char *name, type, type, type, type); \
  void set_uniform(const char *name, GLsizei, const type*); \
  void set_uniform(const char *name, GLsizei, int dim, const type*); \
  void set_uniform(const char *name, const vectype ## 2&); \
  void set_uniform(const char *name, GLsizei, const vectype ## 2*); \
  void set_uniform(const char *name, const vectype ## 3&); \
  void set_uniform(const char *name, GLsizei, const vectype ## 3*); \
  void set_uniform(const char *name, const vectype ## 4&); \
  void set_uniform(const char *name, GLsizei, const vectype ## 4*); \
  void set_uniform(GLint loc, type); \
  void set_uniform(GLint loc, type, type); \
  void set_uniform(GLint loc, type, type, type); \
  void set_uniform(GLint loc, type, type, type, type); \
  void set_uniform(GLint loc, GLsizei, const type*); \
  void set_uniform(GLint loc, GLsizei, int dim, const type*); \
  void set_uniform(GLint loc, const vectype ## 2&); \
  void set_uniform(GLint loc, GLsizei, const vectype ## 2*); \
  void set_uniform(GLint loc, const vectype ## 3&); \
  void set_uniform(GLint loc, GLsizei, const vectype ## 3*); \
  void set_uniform(GLint loc, const vectype ## 4&); \
  void set_uniform(GLint loc, GLsizei, const vectype ## 4*)

  DECL_SET_UNIFORM(float, vec);
  DECL_SET_UNIFORM(int, ivec);
  DECL_SET_UNIFORM(unsigned, uvec);

#undef DECL_SET_UNIFORM

  void set_uniform(const char *name, const matrix&);
  void set_uniform(const char *name, GLsizei, const matrix*);
  void set_uniform(GLint loc, const matrix&);
  void set_uniform(GLint loc, GLsizei, const matrix*);
  void set_uniform(const char *name, const color&);
  void set_uniform(const char *name, GLsizei, const color*);
  void set_uniform(GLint loc, const color&);
  void set_uniform(GLint loc, GLsizei, const color*);

  
private:
  vertex_shader vertex_shader_;
  fragment_shader fragment_shader_;
  shader_program shader_program_;
};

