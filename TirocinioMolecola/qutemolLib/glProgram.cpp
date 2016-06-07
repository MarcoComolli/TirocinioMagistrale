#include <string>
#include <vector>
#include <sstream>

#include <stdio.h>
#include <assert.h>

#include <GL/glew.h>

#include "glProgram.h"
#include "log.h"

namespace qmol {

glProgram::glProgram(void)
{
    this->init_state();
}

glProgram::~glProgram(void)
{
    this->term_state();
}

void glProgram::loadSources( const char* source_prefix ){
    std::string prefix(source_prefix);
    name_on_disk = prefix;
    vertex_source = read_text_file( (prefix + ".vert.glsl").c_str() );
    fragment_source = read_text_file( (prefix + ".frag.glsl").c_str() );
    allBuilt = false;
}


void glProgram::cleanup(void)
{
#if assert_SETUP_CLEANUP
    assert(this->m_prog != 0);
#endif

    this->silent_cleanup();
}

bool glProgram::is_valid(void) const
{
    return (this->m_prog != 0);
}

void glProgram::reset(void)
{
    this->silent_cleanup();
    this->m_log.clear();
}

bool glProgram::buildAll(void){

    const bool r = this->do_setup(
        vertex_source.c_str(),
        0,
        fragment_source.c_str(),
        GL_TRIANGLES, GL_TRIANGLE_STRIP, 3
    );

    //debug("LOG:\n\n%s",this->log().c_str());

    //if (!r) debug("ERROR CREATING PROGRAM");
    return r;

}

std::string & glProgram::log(void)
{
    return this->m_log;
}

GLuint glProgram::id(void) const
{
    return this->m_prog;
}

GLint glProgram::attribute_location(const char * name)
{
    assert(this->m_prog != 0);
    assert(name         != 0);

    const GLint loc = glGetAttribLocation(this->m_prog, name);
    return loc;
}

void glProgram::attribute_source(const char * name, unsigned int index)
{
    assert(this->m_prog != 0);
    assert(name         != 0);

    glBindAttribLocation(this->m_prog, GLuint(index), name);
}

void glProgram::uses(const AttributeIde &i){
    usedAttributes.push_back( i );
}

GLint glProgram::fragment_location(const char * name)
{
    assert(this->m_prog != 0);
    assert(name         != 0);

    const GLint loc = glGetFragDataLocation(this->m_prog, name);
    return loc;
}

void glProgram::fragment_target(const char * name, unsigned int color_index)
{
    assert(this->m_prog != 0);
    assert(name         != 0);

    glBindFragDataLocationEXT(this->m_prog, GLuint(color_index), name);
}

GLint glProgram::uniform_location(const char * name)
{
    assert(this->m_prog != 0);
    assert(name         != 0);

    const GLint loc = glGetUniformLocation(this->m_prog, name);
    return loc;
}

int glProgram::locOf( const UniformIde &ide ){
    if (ide.loc>=(int)uniformLoc.size()) {
        uniformLoc.resize( ide.loc+1, -1 );
    }
    if (uniformLoc[ide.loc]==-1) uniformLoc[ide.loc] = uniform_location( ide.str );
    return uniformLoc[ide.loc];
}

void glProgram::imposeAttributeLocations(){
    for (uint i=0; i<usedAttributes.size(); i++)
        attribute_source( usedAttributes[i].str, usedAttributes[i].loc  );
}

void glProgram::forgetUniformLocations(){
    for (uint i=0; i<uniformLoc.size(); i++)  uniformLoc[i] = -1;
}

void glProgram::set(const UniformIde &t, int v0)
{
    assert(this->m_prog != 0);

    glUniform1i( locOf(t), GLint(v0));
}

void glProgram::set(const UniformIde &t, float v0)
{
    assert(this->m_prog != 0);

    glUniform1f( locOf(t), GLfloat(v0));
}

void glProgram::set(const UniformIde &t, float v0, float v1)
{
    assert(this->m_prog != 0);

    glUniform2f( locOf(t), GLfloat(v0), GLfloat(v1));
}

void glProgram::set(const UniformIde &t, const Vec &v)
{
    assert(this->m_prog != 0);

    glUniform3fv( locOf(t), 1, v.V());

}

void glProgram::set(const UniformIde &t, const Vec4 &v)
{
    assert(this->m_prog != 0);
    glUniform4fv( locOf(t), 1, v.V() );
}

void glProgram::set3x3(const UniformIde &t, const float * v )
{
    assert(this->m_prog != 0);
    assert(v            != 0);
    glUniformMatrix3fv( locOf(t), 1, GL_TRUE, (const GLfloat *)(v));
}

void glProgram::set(const UniformIde &t, const Matrix& m )
{
    assert(this->m_prog != 0);

    glUniformMatrix4fv( locOf(t), 1, GL_TRUE, m.V() );
}

void glProgram::set(const DefineIde &t, bool yesOrNo){
    if ((int)defineName.size()<=t.loc) {
        defineName.resize(t.loc+1,NULL);
        defineValue.resize(t.loc+1,true);
        allBuilt = false;
    }
    defineName[t.loc] = t.str;
    if (defineValue[t.loc]!=yesOrNo) allBuilt = false;
    defineValue[t.loc] = yesOrNo;
}

void glProgram::init_state(void)
{
    allBuilt = false;
    this->m_vshd = 0;
    this->m_gshd = 0;
    this->m_fshd = 0;
    this->m_prog = 0;
}

std::string glProgram::make_prefix(int& n_lines ){
    std::string result;
    char str[1024];
    n_lines = 4;

    for (unsigned int i=0; i<defineName.size(); i++) {
        if (defineName[i]) {
            sprintf(str,"#define %s %c\n",defineName[i],(defineValue[i])?'1':'0' );
            result.append(str);
            n_lines++;
        }
    }
    return result;
}

void glProgram::term_state(void)
{
    if (this->m_prog != 0)
    {
        glDeleteProgram(this->m_prog);
        this->m_prog = 0;
    }

    if (this->m_fshd != 0)
    {
        glDeleteShader(this->m_fshd);
        this->m_fshd = 0;
    }

    if (this->m_gshd != 0)
    {
        glDeleteShader(this->m_gshd);
        this->m_gshd = 0;
    }

    if (this->m_vshd != 0)
    {
        glDeleteShader(this->m_vshd);
        this->m_vshd = 0;
    }
}

void glProgram::silent_cleanup(void)
{
    this->term_state();
    this->init_state();
}

bool glProgram::do_setup(const char * vertex_src, const char * geometry_src, const char * fragment_src, GLenum primitive_input_type, GLenum primitive_output_type, int max_output_vertices)
{
#if assert_SETUP_CLEANUP
    assert(this->m_prog == 0);
#endif


    this->silent_cleanup();

    this->m_log.clear();
    this->m_log += "----------------------------\n";
    this->m_log += "Shader: '" + name_on_disk + "'\n";
    this->m_log += "\n";

    int n_lines;
    std::string prefix = make_prefix( n_lines ).c_str();

    this->m_vshd = this->create_shader(GL_VERTEX_SHADER, prefix.c_str(), vertex_src, m_log, n_lines);

    if (geometry_src != 0)
    {

        this->m_gshd = this->create_shader(GL_GEOMETRY_SHADER_EXT, prefix.c_str(), geometry_src, m_log, n_lines);
    }

    this->m_fshd = this->create_shader(GL_FRAGMENT_SHADER, prefix.c_str(), fragment_src, m_log,n_lines);

    this->m_prog = this->create_program(this->m_vshd, this->m_gshd, this->m_fshd, primitive_input_type, primitive_output_type, max_output_vertices, this->m_log);

    this->m_log += "----------------------------\n";
    this->m_log += "\n";

    if (this->m_prog == 0)
    {
        //debug("SURCES:\n\n%s\n\n\n",vertex_src);
        //debug("%s",m_log.c_str());
        this->silent_cleanup();
        return false;
    }

    return true;
}


GLuint glProgram::create_shader(GLenum type, const char * prefix, const char * src, std::string & compile_log, int n_prefix_lines)
{

    GLuint shd = glCreateShader(type);

    std::string ss;

    switch (type)
    {
    case GL_VERTEX_SHADER:
        ss +=
            "#version 130\n"
            "#define attribute in\n"
            "#define varying out\n";
        break;

    case GL_FRAGMENT_SHADER:
        ss +=
            "#version 130\n"
            "#define varying in\n"
            "\n";
        break;
    }

    if (prefix != 0)
    {
        ss += prefix;
        ss += "\n\n";
    }
    ss += src;

    const char * s = ss.c_str();

    glShaderSource(shd, 1, &s, 0);
    glCompileShader(shd);

    compile_log += "[";
    switch (type)
    {
    case GL_VERTEX_SHADER:
        compile_log += "Vertex";
        break;

    case GL_GEOMETRY_SHADER_EXT:
        compile_log += "Geometry";
        break;

    case GL_FRAGMENT_SHADER:
        compile_log += "Fragment";
        break;

    default:
        compile_log += "Unknown";
        break;
    }
    compile_log += " Shader Log]\n";

    GLint cs = 0;
    glGetShaderiv(shd, GL_COMPILE_STATUS, &cs);
    if (cs != GL_FALSE)
    {
        compile_log += "SUCCESS";
    }
    else
    {
        compile_log += "FAILED (Line 0 at "+ std::to_string(n_prefix_lines) +")";
    }
    compile_log += "\n";

    GLint len = 0;
    glGetShaderiv(shd, GL_INFO_LOG_LENGTH, &len);
    if (len > 0)
    {
        char * cl = new char[len + 1];
        glGetShaderInfoLog(shd, len, &len, cl);
        if (len > 0)
        {
            cl[len-1] = '\0';
            compile_log += cl;
        }
        delete [] cl;
    }
    else
    {
        compile_log += "--- NO LOG ---";
    }
    compile_log += "\n";

    //compile_log += "*********************\n";
    compile_log += "\n";

    if (cs == GL_FALSE)
    {
        glDeleteShader(shd);
        return 0;
    }

    return shd;
}

GLuint glProgram::create_program(GLuint vs, GLuint gs, GLuint fs, GLenum primitive_input_type, GLenum primitive_output_type, int max_output_vertices, std::string & compile_log)
{
    GLuint prog = glCreateProgram();

    if (vs != 0) glAttachShader(prog, vs);
    if (gs != 0) glAttachShader(prog, gs);
    if (fs != 0) glAttachShader(prog, fs);

    if (gs != 0)
    {
        glProgramParameteriEXT(prog, GL_GEOMETRY_INPUT_TYPE_EXT,   GLint(primitive_input_type));
        glProgramParameteriEXT(prog, GL_GEOMETRY_OUTPUT_TYPE_EXT,  GLint(primitive_output_type));
        glProgramParameteriEXT(prog, GL_GEOMETRY_VERTICES_OUT_EXT, GLint(max_output_vertices));
    }

    m_prog = prog;

    imposeAttributeLocations();
    glLinkProgram(prog);
    forgetUniformLocations();

    allBuilt = true;

    //compile_log += "*********************\n";
    compile_log += "[Program Log]\n";

    GLint ls = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ls);
    if (ls != GL_FALSE)
    {
        compile_log += "SUCCESS";
    }
    else
    {
        compile_log += "FAILED";
    }
    compile_log += "\n";

    GLint len = 0;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    if (len > 0)
    {
        char * cl = new char[len + 1];
        glGetProgramInfoLog(prog, len, &len, cl);
        if (len > 0)
        {
            cl[len-1] = '\0';
            compile_log += cl;
        }
        delete [] cl;
    }
    else
    {
        compile_log += "--- NO LOG ---";
    }
    compile_log += "\n";

    //compile_log += "*********************\n";
    compile_log += "\n";

    if (ls == GL_FALSE)
    {
        glDeleteProgram(prog);
        return 0;
    }

    return prog;
}

bool programs_are_supported(void)
{
    if (GLEW_VERSION_2_0) return true;
    return false;
}

void bind_program(glProgram & p)
{
    GLuint obj = p.id();
    assert(obj != 0);
    glUseProgram(obj);
}

void bind_program(glProgram * p)
{
    if (p != 0)
    {
        bind_program(*p);
    }
    else
    {
        glUseProgram(0);
    }
}

std::string glProgram::read_text_file(const char * file_name)
{
    assert(file_name != 0);

    std::string r;

    std::cout << "file name " << file_name << std::endl;


    ::FILE * f = fopen(file_name, "rb");
    assert( f!=0 && "File Not Found");

    if (f == 0) return r;

    fseek(f, 0, SEEK_END);
    const size_t sz = size_t(ftell(f));
    rewind(f);

    char * str = new char [sz + 1];
    fread(str, sizeof(char), sz / sizeof(char), f);
    fclose(f);

    str[sz] = '\0';
    r = str;
    delete [] str;

    return r;
}



void glVertexAttr(const AttributeIde& i, float f){
    glVertexAttrib1f( i.loc, f);
}

void glVertexAttr(const AttributeIde& i, float f0, float f1){
    glVertexAttrib2f( i.loc, f0,f1);
}

void glVertexAttr(const AttributeIde& i, const Vec &v){
    glVertexAttrib3fv( i.loc, v.V() );
}

void glVertexAttrShort(const AttributeIde& i, float f){
    glVertexAttrib1s( i.loc, (int)(f*4096));
}

void glVertexAttrShort(const AttributeIde& i, float f0, float f1){
    glVertexAttrib2s( i.loc, (int)(f0*(16384)),(int)(f1*16384));
}


void glVertexAttrByte(const AttributeIde& i, const Vec &v){
    static unsigned char tmp[4] = {0,0,0,0};
    tmp[0] = int(v[0]*255.0);
    tmp[1] = int(v[1]*255.0);
    tmp[2] = int(v[2]*255.0);
    glVertexAttrib4ubv( i.loc, tmp );
}


} // end namespace



