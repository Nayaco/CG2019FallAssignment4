#ifndef UTIL_H
#define UTIL_H
#include <string>
#include <iostream>

inline void errLog(const std::string err) 
{ 
    std::cout<<"[OpenGL Err]"\
    <<err\
    <<std::endl;
}

inline bool isNull(const void* any) 
{ 
    return any == NULL; 
}

inline void queryGLVersion() 
{
    std::string version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    std::string glslver  = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    std::string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

    std::cout << "OpenGL [ " << version << " ]\n"
         << "GLSL [ " << glslver << " ]\n"
         << "Renderer [ " << renderer << " ]\n";
}
#endif