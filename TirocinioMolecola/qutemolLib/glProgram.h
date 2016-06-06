#ifndef GLW_PROGRAM_H
#define GLW_PROGRAM_H

#include <string>
#include <vector>

#include "baseTypes.h"
#include "log.h"

namespace qmol {

struct DefineIde{
    const char *str;
    int loc;
    explicit DefineIde( const char* name ): str(name) {
        static int count = 0;
        loc = count++;
        //debug("Registered macro '%s'",str);
    }
};

struct UniformIde{
    const char *str;
    int loc;
    explicit UniformIde( const char* name ): str(name) {
        static int count = 0;
        loc = count++;
        //debug("Registered uniform '%s'",str);
    }
};

struct AttributeIde{
    const char *str;
    int loc;
    explicit AttributeIde( const char* name ): str(name) {
        static int count = 1; // not 0 (in opengl valid attribute locs are >0)
        loc = count++;
        //debug("Registered attribute '%s'",str);
    }
};


#define declare_shader_macro( X ) static DefineIde X( #X )
#define declare_shader_uniform( X ) static UniformIde X( #X )
#define declare_shader_attribute( X ) static AttributeIde X( #X )

class glProgram
{
    DISABLE_COPY(glProgram)

public:

    glProgram ();
    ~glProgram ();

    // loads fragment and vertex shader sources
    void loadSources( const char* source_prefix );

    bool buildAll();

    bool isBuilt() const {return allBuilt; }

    GLuint id() const;

    /* set methods, to set uniforms and defines */

    void set(const UniformIde&, int v0);
    void set(const UniformIde&, float v0);
    void set(const UniformIde&, float v0, float v1);
    void set(const UniformIde&, const Vec &v);
    void set(const UniformIde&, const Vec4 &v);
    void set(const UniformIde&, const Matrix &m );
    void set(const DefineIde&, bool yesOrNo );
    void set3x3(const UniformIde&, const float * values );

    /* uses method, to declare which attribute are used */

    void uses( const AttributeIde& i );

    void cleanup();
    bool is_valid() const;
    void reset();

    std::string & log();

protected:

    GLint attribute_location(const char * name);
    void  attribute_source  (const char * name, unsigned int index);

    GLint fragment_location (const char * name);
    void  fragment_target   (const char * name, unsigned int color_index);

    bool allBuilt;
    GLuint m_vshd;
    GLuint m_gshd;
    GLuint m_fshd;

    GLuint m_prog;

    std::string m_log;
    std::string make_prefix(int &nlines);
    std::string vertex_source;
    std::string fragment_source;
    std::string geometry_source;
    std::string name_on_disk;

    std::vector< const char* > defineName;
    std::vector< bool > defineValue;
    std::vector< int > uniformLoc;
    std::vector< AttributeIde > usedAttributes;

    int locOf( const UniformIde &ide );
    void forgetUniformLocations();
    void imposeAttributeLocations();
    GLint uniform_location (const char * name);

    void init_state ();
    void term_state ();

    void silent_cleanup ();

    bool do_setup (const char * vertex_src, const char * geometry_src, const char * fragment_src, GLenum primitive_input_type, GLenum primitive_output_type, int max_output_vertices);

    GLuint create_shader  (GLenum type, const char * prefix, const char * src, std::string & compile_log, int n_prefix_lines);
    GLuint create_program (GLuint vs, GLuint gs, GLuint fs, GLenum primitive_input_type, GLenum primitive_output_type, int max_output_vertices, std::string & compile_log);

    std::string read_text_file(const char * file_name);
};

bool programs_are_supported();
void bind_program(glProgram & p);
void bind_program(glProgram * p);

void glVertexAttr(const AttributeIde &i, float );
void glVertexAttr(const AttributeIde &i, float, float);
void glVertexAttr(const AttributeIde &i, const Vec& );
void glVertexAttrByte(const AttributeIde &i, const Vec& );
void glVertexAttrShort(const AttributeIde &i, float );
void glVertexAttrShort(const AttributeIde &i, float, float);

} // end namespace
/**********************************************************/

#endif // GLW_PROGRAM_H
