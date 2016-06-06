#include <assert.h>
#include <algorithm> // only for std::min. (WTF, VC?)
#include "glSurface.h"
#include "log.h"


namespace qmol{

render_target::render_target(void) : format(GL_NONE), flags(0), id(0), owned(true)
{
    ;
}

render_target::render_target(GLenum t_format) : format(t_format), flags(rt_default), id(0), owned(true)
{
    ;
}

render_target & render_target::operator = (GLenum t_format)
{
    this->format = t_format;
    this->flags  = rt_default;
    this->id     = 0;
    this->owned  = true;
    return (*this);
}

bool render_target::operator != (const render_target & b) const{
    return format!=b.format || flags!=b.flags;
}


glSurface::glSurface(void)
{
    this->init_state();
}

glSurface::~glSurface(void)
{
    this->term_state();
}

bool glSurface::setup(int width, int height, int layers, const render_target & color_target, const render_target & depth_stencil_target)
{
    std::vector<render_target> color_targets;
    if (color_target.format != GL_NONE) color_targets.push_back(color_target);
    return this->setup(width, height, layers, color_targets, depth_stencil_target);
}

bool glSurface::setup(int width, int height, int layers, const std::vector<render_target> & color_targets, const render_target & depth_stencil_target)
{
#if assert_SETUP_CLEANUP
    assert(this->m_fbo == 0);
#endif

    assert(width  >  0);
    assert(height >  0);
    assert(layers >= 0);

    this->silent_cleanup();
    this->m_log.clear();

    this->m_width  = width;
    this->m_height = height;
    this->m_layers = layers;

    this->m_level_width  = width;
    this->m_level_height = height;

    GLint curr_fbo          = 0;
    GLint curr_tex_2D       = 0;
    GLint curr_tex_2D_array = 0;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &curr_fbo);
    if (layers > 0)
    {
        glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &curr_tex_2D_array);
    }
    else
    {
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_tex_2D);
    }

    this->m_dst = depth_stencil_target;
    if (this->m_dst.owned)
    {
        if (this->m_dst.format != GL_NONE)
        {
            this->m_dst.id = this->create_texture(this->m_width, this->m_height, this->m_layers, this->m_dst.format, this->m_dst.flags, true);
        }
    }

    this->m_ct = color_targets;
    for (size_t i=0; i<this->m_ct.size(); ++i)
    {
        if (!this->m_ct[i].owned) continue;

        this->m_ct[i].id = this->create_texture(this->m_width, this->m_height, this->m_layers, this->m_ct[i].format, this->m_ct[i].flags, false);
    }

    glGenFramebuffers(1, &(this->m_fbo));

    glBindFramebuffer(GL_FRAMEBUFFER, this->m_fbo);

    if (this->m_dst.format != GL_NONE)
    {
        if (layers > 0)
        {
            glFramebufferTextureARB(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->m_dst.id, 0);
        }
        else
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->m_dst.id, 0);
        }
    }

    std::vector<GLenum> draw_buffers(this->m_ct.size());
    if (this->m_ct.empty())
    {
        draw_buffers.push_back(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    else
    {
        for (size_t i=0; i<this->m_ct.size(); ++i)
        {
            const GLenum att = GL_COLOR_ATTACHMENT0 + GLenum(i);
            draw_buffers[i] = att;

            if (layers > 0)
            {
                glFramebufferTextureARB(GL_FRAMEBUFFER, att, this->m_ct[i].id, 0);
            }
            else
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, att, GL_TEXTURE_2D, this->m_ct[i].id, 0);
            }
        }
    }
    glDrawBuffersARB(GLsizei(draw_buffers.size()), &(draw_buffers[0]));

    const GLenum fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    this->log_status(fb_status);
    //assert(fb_status == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, GLuint(curr_fbo));
    if (layers > 0)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, GLuint(curr_tex_2D_array));
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, GLuint(curr_tex_2D));
    }

    if (fb_status != GL_FRAMEBUFFER_COMPLETE)
    {
        //this->silent_cleanup();
        return false;
    }

    return true;
}

void glSurface::cleanup(void)
{
#if assert_SETUP_CLEANUP
    assert(this->m_fbo != 0);
#endif

    this->silent_cleanup();
}

bool glSurface::valid(void) const
{
    return (this->m_fbo != 0);
}

void glSurface::reset(void)
{
    this->silent_cleanup();
    this->m_log.clear();
}

std::string & glSurface::log(void)
{
    return this->m_log;
}

void glSurface::push(void)
{
    assert(this->m_fbo != 0);
    assert(!this->m_pushed);

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, ((GLint *)(&(this->m_pushed_fbo))));
    glGetIntegerv(GL_VIEWPORT,            this->m_pushed_viewport);

    glBindFramebuffer(GL_FRAMEBUFFER, this->m_fbo);
    glViewport(GLint(0), GLint(0), GLsizei(this->m_width), GLsizei(this->m_height));

    this->m_pushed = true;
}

void glSurface::pop(void)
{
    assert(this->m_fbo != 0);
    assert(this->m_pushed);

    glBindFramebuffer(GL_FRAMEBUFFER, this->m_pushed_fbo);
    glViewport(GLint(this->m_pushed_viewport[0]), GLint(this->m_pushed_viewport[1]), GLsizei(this->m_pushed_viewport[2]), GLsizei(this->m_pushed_viewport[3]));

    this->m_pushed = false;
}

void glSurface::select_layer(int i)
{
    assert(this->m_fbo != 0);
    assert((this->m_layers > 0) && (i < this->m_layers));

    if (this->m_layers <= 0) return;
    if (i >= this->m_layers) return;

    if (i < 0) i = -1;
    this->m_active_layer = i;

    this->reattach();
}

int glSurface::selected_layer(void) const
{
    return this->m_active_layer;
}

void glSurface::select_level(int i)
{
    assert(this->m_fbo != 0);

    GLint curr_tex_id = 0;
    const GLenum binding_point = (this->m_layers > 0) ? (GL_TEXTURE_BINDING_2D_ARRAY) : (GL_TEXTURE_BINDING_2D);
    const GLenum target        = (this->m_layers > 0) ? (GL_TEXTURE_2D_ARRAY) : (GL_TEXTURE_2D);
    glGetIntegerv(binding_point, &curr_tex_id);

    GLint tex_id = 0;
    if (this->m_ct.empty())
    {
        tex_id = this->m_dst.id;
    }
    else
    {
        tex_id = this->m_ct[0].id;
    }

    GLint sz[2] = { 0, 0 };
    glBindTexture(target, tex_id);
    glGetTexLevelParameteriv(target, GLint(i), GL_TEXTURE_WIDTH,  &(sz[0]));
    glGetTexLevelParameteriv(target, GLint(i), GL_TEXTURE_HEIGHT, &(sz[1]));
    glBindTexture(target, curr_tex_id);

    this->m_active_level = i;
    this->m_level_width  = int(sz[0]);
    this->m_level_height = int(sz[1]);

    this->reattach();
}

int glSurface::selected_level(void) const
{
    return this->m_active_level;
}

void glSurface::generate_mipmaps(void)
{
    assert(this->m_fbo != 0);

    GLenum target = (this->m_layers > 0) ? (GL_TEXTURE_2D_ARRAY) : (GL_TEXTURE_2D);

    if (this->m_dst.id != 0)
    {
        glBindTexture(target, this->m_dst.id);
        glGenerateMipmap(target);
    }

    for (size_t i=0; i<this->m_ct.size(); ++i)
    {
        glBindTexture(target, this->m_ct[i].id);
        glGenerateMipmap(target);
    }

    glBindTexture(target, 0);
}

int glSurface::width() const
{
    return this->m_level_width;
}

int glSurface::height() const
{
    return this->m_level_height;
}

int glSurface::layers() const
{
    return this->m_layers;
}


render_target glSurface::color_target(void) const{
    if (m_ct.size()>0) return m_ct[0];
    return render_target();
}

bool glSurface::resize(int width, int height, int layers)
{
    assert(this->m_fbo != 0);

    if ((width == this->m_width) && (height == this->m_height) && (layers == this->m_layers)) return true;

    render_target_vector tmp_ct  = this->m_ct;
    render_target        tmp_dst = this->m_dst;

    return this->setup(width, height, layers, tmp_ct, tmp_dst);
}

GLuint glSurface::id(void) const
{
    return this->framebuffer_id();
}

GLuint glSurface::framebuffer_id(void) const
{
    return this->m_fbo;
}

int glSurface::targets_count(void) const
{
    return int(this->m_ct.size());
}

GLuint glSurface::target_texture_id(int i) const
{
    assert(i < int(this->m_ct.size()));

    return ((i < 0) ? (this->m_dst.id) : (this->m_ct[i].id));
}

GLenum glSurface::target_format(int i) const
{
    assert(i < int(this->m_ct.size()));

    return ((i < 0) ? (this->m_dst.format) : (this->m_ct[i].format));
}

unsigned int glSurface::target_flags(int i) const
{
    assert(i < int(this->m_ct.size()));

    return ((i < 0) ? (this->m_dst.flags) : (this->m_ct[i].flags));
}

unsigned int& glSurface::target_flags(int i)
{
  assert(i < int(this->m_ct.size()));

  return ((i < 0) ? (this->m_dst.flags) : (this->m_ct[i].flags));
}

void glSurface::init_state(void)
{
    this->m_fbo = 0;

    this->m_ct.clear();

    this->m_dst.format = GL_NONE;
    this->m_dst.flags  = 0;
    this->m_dst.id     = 0;
    this->m_dst.owned  = true;

    this->m_width  = 0;
    this->m_height = 0;
    this->m_layers = 0;

    this->m_level_width  = 0;
    this->m_level_height = 0;

    this->m_active_layer = -1;
    this->m_active_level = 0;

    this->m_pushed_fbo = 0;

    this->m_pushed_viewport[0] = 0;
    this->m_pushed_viewport[1] = 0;
    this->m_pushed_viewport[2] = 1;
    this->m_pushed_viewport[3] = 1;

    this->m_pushed = false;
}

void glSurface::printAll(){
//    debug("Surface data (%X):",(int)this);
//    debug(" --m_fbo = %d",m_fbo);
//    debug(" --m_ct_size = %d",(int)m_ct.size());
//    for (int i=0;i<std::min(3,(int)m_ct.size()); i++) {
//        debug(" --m_ct[%d] = %d",i,m_ct[i].id);
//    }
//    if (m_dst.owned)
//      debug(" --m_dst = %d",(int)m_dst.id);
}

void glSurface::term_state(void){
   // debug("Deleting buffers...");
    if (this->m_fbo != 0)
    {
       // debug("a) delete %d...",this->m_fbo);
        glDeleteFramebuffers(1, &(this->m_fbo));
    }

    assert( this->m_ct.size()<=1);
    for (size_t i=0; i<this->m_ct.size(); i++)
    {
       // debug("2... (%d/%d)",i,(int)this->m_ct.size());
        if (!this->m_ct[i].owned || (this->m_ct[i].id == 0)) continue;
        glDeleteTextures(1, &(this->m_ct[i].id));
    }
    this->m_ct.clear();

    if (this->m_dst.owned && (this->m_dst.id != 0))
    {
       // debug("3...");
        glDeleteTextures(1, &(this->m_dst.id));
    }
    //debug("Done...");
}

void glSurface::silent_cleanup(void)
{
    this->term_state();
    this->init_state();
}

GLuint glSurface::create_texture(int width, int height, int layers, GLenum internal_format, unsigned int flags, bool depth)
{
    const GLenum target = ((layers > 0) ? (GL_TEXTURE_2D_ARRAY) : (GL_TEXTURE_2D));

    GLuint tex = 0;

    glGenTextures(1, &tex);

    glBindTexture(target, tex);

    if ((flags & rt_nearest) != 0)
    {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else if ((flags & rt_linear) != 0)
    {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else if ((flags & rt_mipmap) != 0)
    {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    if ((flags & rt_clamp) != 0)
    {
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else if ((flags & rt_repeat) != 0)
    {
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    GLenum fmt, type;

    if (depth)
    {
        if ((flags & rt_shadow) != 0)
        {
            glTexParameteri(target, GL_DEPTH_TEXTURE_MODE_ARB,   GL_LUMINANCE);
            glTexParameteri(target, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
            glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
        }

        if ((flags & rt_integer) != 0)
        {
            fmt  = GL_DEPTH_COMPONENT;
            type = GL_UNSIGNED_BYTE;
        }
        else
        {
            fmt  = GL_DEPTH_COMPONENT;
            type = GL_FLOAT;
        }

        if (layers > 0)
        {
            glTexImage3D(target, 0, internal_format, GLsizei(width), GLsizei(height), GLsizei(layers), 0, fmt, type, 0);
        }
        else
        {
            glTexImage2D(target, 0, internal_format, GLsizei(width), GLsizei(height), 0, fmt, type, 0);
        }
    }
    else
    {
        if ((flags & rt_integer) != 0)
        {
            fmt  = GL_LUMINANCE_INTEGER_EXT;
            type = GL_UNSIGNED_INT;
        }
        else
        {
            fmt  = GL_LUMINANCE;
            type = GL_UNSIGNED_BYTE;
        }

        if (layers > 0)
        {
            glTexImage3D(target, 0, internal_format, GLsizei(width), GLsizei(height), GLsizei(layers), 0, fmt, type, 0);
        }
        else
        {
            glTexImage2D(target, 0, internal_format, GLsizei(width), GLsizei(height), 0, fmt, type, 0);
        }
    }

    if ((flags & rt_mipmap) != 0)
    {
        glGenerateMipmap(target);
    }

    return tex;
}

void glSurface::reattach(void)
{
    if (this->m_dst.format != GL_NONE)
    {
        if (this->m_layers <= 0)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->m_dst.id, GLint(this->m_active_level));
        }
        else
        {
            if (this->m_active_layer < 0)
            {
                glFramebufferTextureARB(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->m_dst.id, GLint(this->m_active_level));
            }
            else
            {
                glFramebufferTextureLayerARB(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->m_dst.id, GLint(this->m_active_level), GLint(this->m_active_layer));
            }
        }
    }

    for (size_t t=0; t<this->m_ct.size(); ++t)
    {
        const GLenum att = GL_COLOR_ATTACHMENT0 + GLenum(t);

        if (this->m_layers <= 0)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, att, GL_TEXTURE_2D, this->m_ct[t].id, GLint(this->m_active_level));
        }
        else
        {
            if (this->m_active_layer < 0)
            {
                glFramebufferTextureARB(GL_FRAMEBUFFER, att, this->m_ct[t].id, GLint(this->m_active_level));
            }
            else
            {
                glFramebufferTextureLayerARB(GL_FRAMEBUFFER, att, this->m_ct[t].id, GLint(this->m_active_level), GLint(this->m_active_layer));
            }
        }
    }
}

void glSurface::log_status(GLenum status)
{
    switch (status)
    {
    case GL_FRAMEBUFFER_COMPLETE:
        this->m_log += "GL_FRAMEBUFFER_COMPLETE";
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        this->m_log += "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        this->m_log += "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        this->m_log += "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        this->m_log += "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        this->m_log += "GL_FRAMEBUFFER_UNSUPPORTED";
        break;

    default:
        this->m_log += "UNKNOWN_ERROR";
        break;
    }
}

bool has_surface(void)
{
    if (GLEW_ARB_framebuffer_object) return true;
    return false;
}

void setup_surface_viewport(glSurface & s)
{
    assert(s.valid());
    glViewport(GLint(0), GLint(0), GLsizei(s.width()), GLsizei(s.height()));
}

void bind_surface(glSurface & s, bool setup_viewport)
{
    GLuint obj = s.framebuffer_id();
    assert(obj != 0);
    glBindFramebuffer(GL_FRAMEBUFFER, obj);

    if (setup_viewport)
    {
        setup_surface_viewport(s);
    }
}

void bind_surface(glSurface * s, bool setup_viewport)
{
    if (s != 0)
    {
        bind_surface(*s, setup_viewport);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void bind_surface_target(glSurface & s, int i)
{
    GLuint obj = s.target_texture_id(i);
    assert(obj != 0);
    GLenum target = (s.layers() > 0) ? (GL_TEXTURE_2D_ARRAY) : (GL_TEXTURE_2D);
    glBindTexture(target, obj);
}

render_target rt_format(GLenum t_format, unsigned int t_flags)
{
    render_target rt;

    rt.format = t_format;
    rt.flags  = t_flags;
    rt.id     = 0;
    rt.owned  = true;

    return rt;
}

render_target rt_tex2D(GLuint t_id)
{
    GLint curr_tex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_tex);

    GLint fmt = 0;
    glBindTexture(GL_TEXTURE_2D, t_id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &fmt);
    glBindTexture(GL_TEXTURE_2D, curr_tex);

    render_target rt;

    rt.format = GLenum(fmt);
    rt.flags  = 0;
    rt.id     = t_id;
    rt.owned  = false;

    return rt;
}

render_target rt_tex2Darr(GLuint t_id)
{
    GLint curr_tex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &curr_tex);

    GLint fmt = 0;
    glBindTexture(GL_TEXTURE_2D_ARRAY, t_id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_INTERNAL_FORMAT, &fmt);
    glBindTexture(GL_TEXTURE_2D_ARRAY, curr_tex);

    render_target rt;

    rt.format = GLenum(fmt);
    rt.flags  = 0;
    rt.id     = t_id;
    rt.owned  = false;

    return rt;
}

render_target rt_target(glSurface & t_srf, int t_target)
{
    render_target rt;

    rt.format = t_srf.target_format(t_target);
    rt.flags  = t_srf.target_flags(t_target);
    rt.id     = t_srf.target_texture_id(t_target);
    rt.owned  = false;

    return rt;
}

}
