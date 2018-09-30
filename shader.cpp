
#include "stdafx.h"

#include "shader.h"
#include "util.h"


namespace {

int get_shader_info_log(GLuint shader, std::string *s)
{
  // サイズを取得
  GLint log_length;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
  // 文字列を取得
  s->assign(log_length, '\0');
  GLsizei length;
  glGetShaderInfoLog(shader, log_length, &length, &((*s)[0]));

  return log_length;
}

int get_program_info_log(GLuint program, std::string *s)
{
  // サイズを取得
  GLint log_length;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
  // 文字列を取得
  s->assign(log_length, '\0');
  GLsizei length;
  glGetProgramInfoLog(program, log_length, &length, &((*s)[0]));

  return log_length;
}


bool get_shader_error(GLuint shader, std::string *s) {
  GLint result;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

  // ログを取得
  if (result == GL_FALSE) {
    get_shader_info_log(shader, s);

    return true;
  }

  return false;
}

bool compile_shader_from_source_file(GLuint shader, const char *filename)
{
  std::string source = read_file_all(filename);
  const char *s = source.c_str();
  glShaderSource(shader, 1, &s, 0);
  glCompileShader(shader);

  std::string error;
  if (get_shader_error(shader, &error)) {
    std::cerr << filename << std::endl;
    std::cerr << error << std::endl;
    return false;
  }
  return true;
}


const char *get_semantics_attrib_name(Semantics semantics)
{
  static const char *semantics_attrib_name[] = {
    "vPos",
    "vNormal",
    "vColor",
    "vTexCoord_0",
    "vTexCoord_1",
    "vTexCoord_2",
    "vTexCoord_3",
    "vTexCoord_4",
    "vTexCoord_5",
    "vTexCoord_6",
    "vTexCoord_7",
  };
  static_assert(countof(semantics_attrib_name) == Semantics_Num);

  return semantics_attrib_name[semantics];
}

}	// end of anonymus namespace


vertex_shader::vertex_shader()
  : shader_(0)
{
  shader_ = glCreateShader(GL_VERTEX_SHADER);
}

vertex_shader::~vertex_shader()
{
  if (shader_) {
    glDeleteShader(shader_);
  }
}

bool vertex_shader::compile_from_source_file(const char *filename)
{
  return compile_shader_from_source_file(shader_, filename);
}


fragment_shader::fragment_shader()
  : shader_(0)
{
  shader_ = glCreateShader(GL_FRAGMENT_SHADER);
}

fragment_shader::~fragment_shader()
{
  if (shader_) {
    glDeleteShader(shader_);
  }
}

bool fragment_shader::compile_from_source_file(const char *filename)
{
  return compile_shader_from_source_file(shader_, filename);
}


shader_program::shader_program()
  : program_(0)
{
  program_ = glCreateProgram();
}

shader_program::~shader_program()
{
  if (program_) {
    glDeleteProgram(program_);
  }
}

void shader_program::attach(vertex_shader* vs)
{
  glAttachShader(program_, vs->globj());
}

void shader_program::attach(fragment_shader* fs)
{
  glAttachShader(program_, fs->globj());
}

void shader_program::link()
{
  glLinkProgram(program_);
  std::string info_log;
  GLint success;
  glGetProgramiv(globj(), GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    if (get_program_info_log(globj(), &info_log)) {
      std::cout << "shader(" << globj() << ") validation log:" << std::endl;
      std::cout << info_log << std::endl;
    }
  }
}

GLint shader_program::uniform_location(const char *name)
{
  return glGetUniformLocation(program_, name);
}

GLint shader_program::attrib_location(const char *name)
{
  return glGetAttribLocation(program_, name);
}

GLint shader_program::attrib_location(Semantics semantics)
{
  return attrib_location(get_semantics_attrib_name(semantics));
}


void shader_program::use()
{
  glUseProgram(program_);
}

void shader::use()
{
  get_shader_program()->use();
}

bool shader::compile_from_source_file(const char *vs, const char *fs)
{
  if (!vertex_shader_.compile_from_source_file(vs)) {
    return false;
  }
  if (!fragment_shader_.compile_from_source_file(fs)) {
    return false;
  }
  shader_program_.attach(&vertex_shader_);
  shader_program_.attach(&fragment_shader_);
  shader_program_.link();

  return true;
}


void shader::set_attrib(const vertex_decl& decl)
{
  set_attrib(get_semantics_attrib_name(decl.semantics),
             decl.size,
             decl.type,
             decl.stride,
             decl.offset);
}

void shader::set_attrib(const char *name,
                        GLint size,
                        GLenum type,
                        GLsizei stride,
                        size_t offset)
{
  int loc = get_shader_program()->attrib_location(name);
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, size, type, GL_FALSE, stride, (void*)offset);
}

void shader::validate()
{
  glValidateProgram(globj());
  std::string info_log;
  GLint success;
  glGetProgramiv(globj(), GL_VALIDATE_STATUS, &success);
  if (success == GL_FALSE) {
    if (get_program_info_log(globj(), &info_log)) {
      std::cout << "shader(" << globj() << ") validation log:" << std::endl;
      std::cout << info_log << std::endl;
    }
  }
}

#define IMPL_SET_UNIFORM(type, vectype, glpostfix) \
void shader::set_uniform(const char *name, type v0) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), v0); \
} \
void shader::set_uniform(const char *name, type v0, type v1) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), v0, v1); \
} \
void shader::set_uniform(const char *name, type v0, type v1, type v2) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), v0, v1, v2); \
} \
void shader::set_uniform(const char *name, type v0, type v1, type v2, type v3) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), v0, v1, v2, v3); \
} \
void shader::set_uniform(const char *name, GLsizei num, const type *v) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), num, v); \
} \
void shader::set_uniform(const char *name, GLsizei num, int dim, const type *v) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), num, dim, v); \
} \
void shader::set_uniform(const char *name, const vectype ## 2& v) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), v); \
} \
void shader::set_uniform(const char *name, GLsizei num, const vectype ## 2 *v) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), num, v); \
} \
void shader::set_uniform(const char *name, const vectype ## 3& v) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), v); \
} \
void shader::set_uniform(const char *name, GLsizei num, const vectype ## 3 *v) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), num, v); \
} \
void shader::set_uniform(const char *name, const vectype ## 4& v) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), v); \
} \
void shader::set_uniform(const char *name, GLsizei num, const vectype ## 4 *v) \
{ \
  set_uniform(get_shader_program()->uniform_location(name), num, v); \
} \
void shader::set_uniform(GLint loc, type v0) \
{ \
  glUniform1 ## glpostfix (loc, v0); \
} \
void shader::set_uniform(GLint loc, type v0, type v1) \
{ \
  glUniform2 ## glpostfix (loc, v0, v1); \
} \
void shader::set_uniform(GLint loc, type v0, type v1, type v2) \
{ \
  glUniform3 ## glpostfix (loc, v0, v1, v2); \
} \
void shader::set_uniform(GLint loc, type v0, type v1, type v2, type v3) \
{ \
  glUniform4 ## glpostfix (loc, v0, v1, v2, v3); \
} \
void shader::set_uniform(GLint loc, GLsizei num, const type *v) \
{ \
  glUniform1 ## glpostfix ## v (loc, num, v); \
} \
void shader::set_uniform(GLint loc, GLsizei num, int dim, const type *v) \
{ \
  switch (num) { \
  case 1: glUniform1 ## glpostfix ## v (loc, num, v); break; \
  case 2: glUniform2 ## glpostfix ## v (loc, num, v); break; \
  case 3: glUniform3 ## glpostfix ## v (loc, num, v); break; \
  case 4: glUniform4 ## glpostfix ## v (loc, num, v); break; \
  } \
} \
void shader::set_uniform(GLint loc, const vectype ## 2& v) \
{ \
  glUniform2 ## glpostfix (loc, v.x, v.y); \
} \
void shader::set_uniform(GLint loc, GLsizei num, const vectype ## 2 *v) \
{ \
  glUniform2 ## glpostfix ## v (loc, num, as_array(*v)); \
} \
void shader::set_uniform(GLint loc, const vectype ## 3& v) \
{ \
  glUniform3 ## glpostfix (loc, v.x, v.y, v.z); \
} \
void shader::set_uniform(GLint loc, GLsizei num, const vectype ## 3 *v) \
{ \
  glUniform3 ## glpostfix ## v (loc, num, as_array(*v)); \
} \
void shader::set_uniform(GLint loc, const vectype ## 4& v) \
{ \
  glUniform4 ## glpostfix (loc, v.x, v.y, v.z, v.w); \
} \
void shader::set_uniform(GLint loc, GLsizei num, const vectype ## 4 *v) \
{ \
  glUniform4 ## glpostfix ## v (loc, num, as_array(*v)); \
}

IMPL_SET_UNIFORM(float, vec, f)
IMPL_SET_UNIFORM(int, ivec, i)
IMPL_SET_UNIFORM(unsigned, uvec, ui)

#undef IMPL_SET_UNIFORM

void shader::set_uniform(const char *name, const matrix& v)
{
  set_uniform(get_shader_program()->uniform_location(name), v);
}
void shader::set_uniform(const char *name, GLsizei size, const matrix *v)
{
  set_uniform(get_shader_program()->uniform_location(name), size, v);
}
void shader::set_uniform(GLint loc, const matrix& v)
{
  glUniformMatrix4fv(loc, 1, GL_FALSE, as_array(v));
}
void shader::set_uniform(GLint loc, GLsizei num,  const matrix *v)
{
  glUniformMatrix4fv(loc, num, GL_FALSE, as_array(*v));
}

void shader::set_uniform(const char *name, const color& v)
{
  set_uniform(get_shader_program()->uniform_location(name), v);
}
void shader::set_uniform(const char *name, GLsizei size, const color *v)
{
  set_uniform(get_shader_program()->uniform_location(name), size, v);
}
void shader::set_uniform(GLint loc, const color& v)
{
  glUniform4f(loc, v.r, v.g, v.b, v.a);
}
void shader::set_uniform(GLint loc, GLsizei num, const color *v)
{
  glUniform4fv(loc, num, as_array(*v));
}

