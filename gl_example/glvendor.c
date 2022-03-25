#include <GL/glew.h>  
  
#include <GL/glut.h>  
#include <stdio.h>  
  
GLuint vShader,fShader;//顶点着色器对象  
  
//顶点位置数组  
float positionData[] = {  
    -0.8f, -0.8f, 0.0f,  
    0.8f, -0.8f, 0.0f,  
    0.0f,  0.8f, 0.0f };  
//颜色数组  
float colorData[] = {  
        1.0f, 0.0f, 0.0f,  
        0.0f, 1.0f, 0.0f,  
        0.0f, 0.0f, 1.0f };  
  
GLuint vaoHandle;//vertex array object  
  
void main(const char *VShaderFile,const char *FShaderFile)  
{  
    //1、查看GLSL和OpenGL的版本  
    const GLubyte *renderer = glGetString( GL_RENDERER );  
    const GLubyte *vendor = glGetString( GL_VENDOR );  
    const GLubyte *version = glGetString( GL_VERSION );  
    const GLubyte *glslVersion =   
        glGetString( GL_SHADING_LANGUAGE_VERSION );  
    GLint major, minor;  
    glGetIntegerv(GL_MAJOR_VERSION, &major);  
    glGetIntegerv(GL_MINOR_VERSION, &minor);  
    printf("GL Vendor    :%s\n",vendor);  
    printf( "GL Renderer  : %s\n",renderer); 
    printf("GL Version (string)  : %s\n",version );
    printf("GL Version (integer) : %d.%d\n" , major , minor );
    printf( "GLSL Version : %s\n" ,glslVersion );
}
