#ifndef GLSURFACE_H
#define GLSURFACE_H

#include <vector>
#include <string>
#include <GL/glew.h>

namespace qmol {


class glSurface;

enum render_target_flags
{
    rt_nearest = (1 << 0),
    rt_linear  = (1 << 1),
    rt_mipmap  = (1 << 2),
    rt_clamp   = (1 << 3),
    rt_repeat  = (1 << 4),
    rt_shadow  = (1 << 5),
    rt_integer = (1 << 6),
    rt_default = (rt_nearest | rt_clamp)
};

class render_target;

render_target rt_format   (GLenum t_format, unsigned int t_flags = rt_default);
render_target rt_tex2D    (GLuint t_id);
render_target rt_tex2Darr (GLuint t_id);
render_target rt_target   (glSurface & t_srf, int t_target);

class render_target
{
public:

    typedef render_target this_type;
    typedef void          base_type;

    render_target(void);
    render_target(GLenum format);

    this_type & operator = (GLenum format);
    bool operator != (const this_type & b) const;

protected:

    friend class glSurface;

    friend render_target rt_format   (GLenum t_format, unsigned int t_flags);
    friend render_target rt_tex2D    (GLuint t_id);
    friend render_target rt_tex2Darr (GLuint t_id);
    friend render_target rt_target   (glSurface & t_srf, int t_target);

    GLenum       format;
    unsigned int flags;
    GLuint       id;
    bool         owned;
};

typedef std::vector<render_target> render_target_vector;

class glSurface
{
    //GLW_DISABLE_COPY(surface)

public:


    typedef glSurface this_type;
    typedef void    base_type;

     glSurface (void);
    ~glSurface (void);

    bool setup   (int width, int height, int layers, const render_target &        color_target,  const render_target & depth_stencil_target);
    bool setup   (int width, int height, int layers, const render_target_vector & color_targets, const render_target & depth_stencil_target);
    void cleanup (void);
    bool valid   (void) const;
    void reset   (void);

    std::string & log(void);

    void push (void);
    void pop  (void);

    void select_layer   (int i);
    int  selected_layer (void) const;

    void select_level   (int i);
    int  selected_level (void) const;

    void generate_mipmaps (void);

    int width () const;
    int height() const;
    int layers() const;
    render_target color_target() const;


    bool resize (int width, int height, int layers);

    GLuint id (void) const;

    GLuint framebuffer_id (void) const;

    int          targets_count     (void)  const;
    GLuint       target_texture_id (int i) const;
    GLenum       target_format     (int i) const;
    unsigned int target_flags      (int i) const;
    unsigned int &      target_flags      (int i) ;

    void printAll();

protected:

    GLuint m_fbo;

    render_target_vector m_ct;
    render_target        m_dst;

    int m_width;
    int m_height;
    int m_layers;

    int m_level_width;
    int m_level_height;

    int m_active_layer;
    int m_active_level;

    std::string m_log;

    GLuint m_pushed_fbo;
    GLint  m_pushed_viewport[4];
    bool   m_pushed;

    void init_state (void);
    void term_state (void);
    void silent_cleanup (void);

    GLuint create_texture (int width, int height, int layers, GLenum internal_format, unsigned int flags, bool depth);
    void   reattach (void);
    void   log_status (GLenum status);
};

bool has_surface (void);

void setup_surface_viewport (glSurface & s);

void bind_surface (glSurface & s, bool setup_viewport = true);
void bind_surface (glSurface * s, bool setup_viewport = true);

void bind_surface_target (glSurface & s, int i);

} // end namespace


#endif // GLSURFACE_H
