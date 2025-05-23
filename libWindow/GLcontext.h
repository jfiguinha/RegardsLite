#pragma once
#include <epoxy/gl.h>
#ifdef WIN32
#include <epoxy/wgl.h>
#endif
#ifdef __WXGTK__
#include <epoxy/glx.h>
#endif


#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <wx/glcanvas.h>

