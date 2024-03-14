#pragma once

#ifndef USE_GLEW
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#else
#include <GL/glew.h>
#endif

#include <cstddef>
#include <vector>

/*!
        \file

        OpenGL RAII wrappers and convenience functions.
*/

#define GLDEBUG_S(x) #x
#define GLDEBUG_S_(x) GLDEBUG_S(x)
#define GLDEBUGSTRING "GL: err " __FILE__ ", line " GLDEBUG_S_(__LINE__)
#define GLDEBUG GL::get_error(GLDEBUGSTRING, __PRETTY_FUNCTION__)

namespace GL {
//! Buffer RAII wrapper
struct Buffer {
  inline Buffer() noexcept { glGenBuffers(1, &id); };
  inline ~Buffer() noexcept { glDeleteBuffers(1, &id); };
  //! move ctor
  Buffer(Buffer &&b) noexcept : id(b.id) { b.id = 0; };

  Buffer(const Buffer &) = delete;        //!< disable copying
  Buffer &operator=(Buffer &&) = default; //!< move assignment

  /*!
          Binds the buffer to the GL_ARRAY_BUFFER target.
  */
  inline void bind() const noexcept { glBindBuffer(GL_ARRAY_BUFFER, id); };

  inline void tfbind() {
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, id);
  };

  /*!
          Binds the buffer to the GL_ARRAY_BUFFER target.
  */
  inline void operator()() const noexcept { bind(); };

  /*!
          Binds the buffer to the specified target.
          \param target the target to bind to
  */
  inline void bind(GLenum target) const noexcept { glBindBuffer(target, id); };
  /*!
          Binds the buffer to the specified target.
          \param target the target to bind to
  */
  inline void operator()(GLenum target) const noexcept { bind(target); };

  /*!
          Binds the default buffer to the GL_ARRAY_BUFFER target.
  */
  static inline void unbind() noexcept { glBindBuffer(GL_ARRAY_BUFFER, 0); };

  /*!
          Binds the default buffer to the specified target.
          \param target the target to bind to
  */
  static inline void unbind(GLenum target) noexcept {
    glBindBuffer(target, 0);
  };

  GLuint id; //!< buffer handle
};

//! VAO RAII wrapper
struct VAO {
  inline VAO() noexcept { glGenVertexArrays(1, &id); };
  inline ~VAO() noexcept { glDeleteVertexArrays(1, &id); };
  //! move ctor
  VAO(VAO &&v) noexcept : id(v.id) { v.id = 0; };

  VAO(const VAO &) = delete;        //!< disable copying
  VAO &operator=(VAO &&) = default; //!< move assignment

  inline void bind() const noexcept { glBindVertexArray(id); };
  //! bind VAO
  inline void operator()() const noexcept { bind(); };

  //! unbind VAO
  static inline void unbind() noexcept { glBindVertexArray(0); };

  GLuint id; //!< VAO handle
};

//! Shader RAII wrapper
struct Shader {
public:
  Shader(const char *code, GLuint type);
  inline ~Shader() noexcept { glDeleteShader(id); };
  //! move ctor
  Shader(Shader &&s) noexcept : id(s.id) { s.id = 0; };

  Shader(const Shader &) = delete;        //!< disable copying
  Shader &operator=(Shader &&) = default; //!< move assignment

  GLuint id; //!< handle
};

//! Shader Program RAII wrapper
class Program {
public:
  Program();
  inline ~Program() noexcept { glDeleteProgram(id); };
  //! move ctor
  Program(Program &&p) : id(p.id) { p.id = 0; };

  Program(const Program &) = delete;        //!< disable copying
  Program &operator=(Program &&) = default; //!< move assignment

  /*!
          Link the specified shaders to the Program.
          \param shs shaders to link
  */
  template <typename... T> void link(T &...shs) {
    attach(shs...);
    glLinkProgram(id);
    detach(shs...);
    check_link_status();
  }

  template <typename... T>
  void link_TF(const size_t n, const char **varyings, T &...shs) {
    glTransformFeedbackVaryings(id, n, varyings, GL_INTERLEAVED_ATTRIBS);
    link(shs...);
  }

  /*!
          Link the specified shaders to the Program.
          \param shs shaders to link
  */
  void link_vector(const std::vector<Shader> &shs) {
    for (auto &sh : shs) {
      attach(sh);
    }

    glLinkProgram(id);

    for (auto &sh : shs) {
      detach(sh);
    }

    check_link_status();
  }

  /*!
          Check if the Program is linked correctly. Throws std::invalid_argument
     on failure.
  */
  void check_link_status();

  /*!
          Attach multiple shaders to program.
          \param shs shaders to attach
  */
  template <typename... T> inline void attach(const Shader &sh, T &...shs) {
    attach(sh);
    attach(shs...);
  };
  /*!
          Attach specified shader to program.
          \param sh shader to attach
  */
  inline void attach(const Shader &sh) { glAttachShader(id, sh.id); };

  /*!
          Detach multiple shaders from the program.
          \param shs shaders to detach
  */
  template <typename... T> inline void detach(const Shader &sh, T &...shs) {
    detach(sh);
    detach(shs...);
  };
  /*!
          Detach specified shader from the program.
          \param sh shader to detach
  */
  inline void detach(const Shader &sh) { glDetachShader(id, sh.id); };

  //! use shader
  inline void use() const noexcept { glUseProgram(id); };
  //! use shader
  inline void operator()() const noexcept { use(); };

  /*!
          Get program id.
          \return Program handle
  */
  inline GLuint get_id() const noexcept { return id; };

  /*!
          Get the uniform location of the specified uniform name.
          \param name Uniform name
          \return Uniform location
  */
  inline GLint get_uniform(const char *name) const {
    return glGetUniformLocation(id, name);
  };
  /*!
          Get the attribute location of the specified attribute name.
          \param name Attribute name
          \return Attribute location
  */
  inline GLint get_attrib(const char *name) const {
    return glGetAttribLocation(id, name);
  };

private:
  GLuint id; //!< Program handle
};

//! Texture RAII wrapper
struct Texture {
  inline Texture() noexcept { glGenTextures(1, &id); };
  inline ~Texture() noexcept { glDeleteTextures(1, &id); };
  //! move ctor
  Texture(Texture &&t) noexcept : id(t.id) { t.id = 0; };

  Texture(const Texture &) = delete;        //!< disable copying
  Texture &operator=(Texture &&) = default; //!< move assignment

  inline void bind(GLenum target) const noexcept { glBindTexture(target, id); };
  inline void operator()(GLenum target) const noexcept { bind(target); };

  static inline void unbind(GLenum target) noexcept {
    glBindTexture(target, 0);
  };

  GLuint id; //!< handle
};

//! Framebuffer RAII wrapper
struct FBO {
  inline FBO() noexcept { glGenFramebuffers(1, &id); };
  inline ~FBO() noexcept { glDeleteFramebuffers(1, &id); };
  //! move ctor
  FBO(FBO &&t) noexcept : id(t.id) { t.id = 0; };

  FBO(const FBO &) = delete;        //!< disable copying
  FBO &operator=(FBO &&) = default; //!< move assignment

  inline void bind() const noexcept { bind(GL_FRAMEBUFFER); };
  inline void operator()() const noexcept { bind(); };

  inline void bind(GLenum target) const noexcept {
    glBindFramebuffer(target, id);
  };
  inline void operator()(GLenum target) const noexcept { bind(target); };

  static inline void unbind() noexcept { unbind(GL_FRAMEBUFFER); };
  static inline void unbind(GLenum target) noexcept {
    glBindFramebuffer(target, 0);
  };

  GLuint id; //!< handle
};

/*!
        Print error message if an opengl error occured.
        \param str error message that gets printed
*/
void get_error(const char *, const char *function = nullptr);

//! Initialize GL context
void init();
} // namespace GL
