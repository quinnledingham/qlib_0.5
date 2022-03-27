#ifndef _G_L_T_F_LOADER_H
#define _G_L_T_F_LOADER_H

#include "cgltf.h"

cgltf_data* LoadGLTFFile(const char* path);
void FreeGLTFFile(cgltf_data* handle);

#endif //_G_L_T_F_LOADER_H
