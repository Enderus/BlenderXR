/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2019 by Blender Foundation.
 * All rights reserved.
 *
 * Contributor(s): MARUI-PlugIn, Multiplexed Reality
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/vr/intern/vr_main.c
 *  \ingroup vr
 *
 * Main VR module. Also handles loading/unloading of VR device shared libraries.
 */

#include "vr_build.h"

#include "vr_main.h"

#include "BLI_math.h"

#include "DNA_camera_types.h"
#include "DNA_screen_types.h"
#include "DNA_windowmanager_types.h"

#include "BKE_camera.h"
#include "BKE_library.h"
#include "BKE_library_query.h"
#include "BKE_main.h"
#include "BKE_object.h"

#include "DEG_depsgraph_build.h"

#include "ED_object.h"

#include "GPU_framebuffer.h"
#include "GPU_viewport.h"

#include "draw_manager.h"
#include "wm_draw.h"

#include "WM_api.h"
#include "WM_types.h"

#ifdef WIN32
#include "BLI_winstuff.h"
#else
#include <dlfcn.h>
#include <unistd.h>
#include <GL/glxew.h>
#endif

#ifdef __APPLE__
#include <libproc.h>
#endif

#include "vr_api.h"

#ifndef WIN32
/* Remove __stdcall for the dll imports. */
#define __stdcall
#endif

/* VR shared library functions. */
typedef int(__stdcall *c_createVR)(void);	/* Create the internal VR object. Must be called before the functions below. */
#ifdef WIN32
typedef int(__stdcall *c_initVR)(void* device, void* context);	/*Initialize the internal VR object (OpenGL). */
#else
typedef int(__stdcall *c_initVR)(void* display, void* drawable, void* context);	/* Initialize the internal object (OpenGL). */
#endif
typedef int(__stdcall *c_getHMDType)(int* type);	/* Get the type of HMD used for VR. */
typedef int(__stdcall *c_setEyeParams)(int side, float fx, float fy, float cx, float cy);	/* Set rendering parameters. */
typedef int(__stdcall *c_getDefaultEyeParams)(int side, float* fx, float* fy, float* cx, float* cy);	/* Get the HMD's default parameters. */
typedef int(__stdcall *c_getDefaultEyeTexSize)(int* w, int* h, int side);	/* Get the default eye texture size. */
typedef int(__stdcall *c_updateTrackingVR)(void);	/* Update the t_eye positions based on latest tracking data. */
typedef int(__stdcall *c_getEyePositions)(float t_eye[VR_SIDES][4][4]);	/* Last tracked position of the eyes. */
typedef int(__stdcall *c_getHMDPosition)(float t_hmd[4][4]);	/* Last tracked position of the HMD. */
typedef int(__stdcall *c_getControllerPositions)(float t_controller[VR_MAX_CONTROLLERS][4][4]);	/* Last tracked position of the controllers. */
typedef int(__stdcall *c_getControllerStates)(void* controller_states[VR_MAX_CONTROLLERS]);	/* Last tracked button states of the controllers. */
typedef int(__stdcall *c_blitEye)(int side, void* texture_resource, const float* aperture_u, const float* aperture_v);	/* Blit a rendered image into the internal eye texture. */
typedef int(__stdcall *c_blitEyes)(void* texture_resource_left, void* texture_resource_right, const float* aperture_u, const float* aperture_v);	/* Blit rendered images into the internal eye textures. */
typedef int(__stdcall *c_submitFrame)(void);	/* Submit frame to the HMD. */
typedef int(__stdcall *c_uninitVR)(void);	/* Un-initialize the internal object. */

static c_createVR vr_dll_create_vr;
static c_initVR vr_dll_init_vr;
static c_getHMDType vr_dll_get_hmd_type;
static c_setEyeParams vr_dll_set_eye_params;
static c_getDefaultEyeParams vr_dll_get_default_eye_params;
static c_getDefaultEyeTexSize vr_dll_get_default_eye_tex_size;
static c_updateTrackingVR vr_dll_update_tracking_vr;
static c_getEyePositions vr_dll_get_eye_positions;
static c_getHMDPosition vr_dll_get_hmd_position;
static c_getControllerPositions vr_dll_get_controller_positions;
static c_getControllerStates vr_dll_get_controller_states;
static c_blitEye vr_dll_blit_eye;
static c_blitEyes vr_dll_blit_eyes;
static c_submitFrame vr_dll_submit_frame;
static c_uninitVR vr_dll_uninit_vr;

/* VR module object (singleton). */
static VR vr;
VR *vr_get_obj() { return &vr; }

/* Temporary VR camera to use if scene does not contain a camera. */
static Object *vr_temp_cam = NULL;

/* The active VR dll (if any). */
#ifdef WIN32
static HINSTANCE vr_dll;
#else
static void *vr_dll;
#endif

/* Unload shared library functions. */
static int vr_unload_dll_functions(void)
{
  if (!vr_dll) {
    return 0;
  }

#ifdef WIN32
	int success = FreeLibrary(vr_dll);
	if (success) {
		vr_dll = 0;
		return 0;
	}
#else
	int error = dlclose(vr_dll);
	if (!error) {
		vr_dll = 0;
		return 0;
	}
#endif

	return -1;
}

/* Load shared library functions and set VR type. */
static int vr_load_dll_functions(VR_Type type)
{
  if (vr_dll) {
    vr_unload_dll_functions();
  }

  /* The shared library must be in the folder that contains the Blender executable.
   * "BlenderXR_OpenXR.dll/.so" also requires "openxr-loader-1_0.dll/.so" to be present.
   * "BlenderXR_SteamVR.dll/.so" also requires "openvr_api.dll/.so" to be present.
   * "BlenderXR_Fove.dll" also requires "FoveClient.dll". */
  switch (type) {
  case VR_TYPE_MAGICLEAP: {
    return -1;
  }
  case VR_TYPE_OPENXR: {
#ifdef WIN32
    vr_dll = LoadLibrary("BlenderXR_OpenXR.dll");
#elif defined __linux__
    static char proc_path[PATH_MAX];
    static char path[PATH_MAX];
    memset(path, 0, sizeof(path));
    pid_t pid = getpid();
    sprintf(proc_path, "/proc/%d/exe", pid);
    ssize_t len = readlink(proc_path, path, PATH_MAX);
    if (len != -1) {
      /* Replace the "blender" tail */
      sprintf(&path[len - 7], "libBlenderXR_OpenXR.so");
      vr_dll = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    }
#elif defined __APPLE__
    static char proc_path[PROC_PIDPATHINFO_MAXSIZE];
    static char path[PATH_MAX];
    memset(path, 0, sizeof(path));
    pid_t pid = getpid();
    int len = proc_pidpath(pid, path, PATH_MAX);
    if (len > 0) {
      /* Replace the "blender.app/Contents/MacOS/blender" tail */
      sprintf(&path[len - 34], "libBlenderXR_OpenXR.dylib");
      vr_dll = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    }
#endif
    if (vr_dll) {
      vr.type = VR_TYPE_OPENXR;
    }
    break;
  }
  case VR_TYPE_STEAM: {
#ifdef WIN32
    vr_dll = LoadLibrary("BlenderXR_SteamVR.dll");
#elif defined __linux__
    static char proc_path[PATH_MAX];
    static char path[PATH_MAX];
    memset(path, 0, sizeof(path));
    pid_t pid = getpid();
    sprintf(proc_path, "/proc/%d/exe", pid);
    ssize_t len = readlink(proc_path, path, PATH_MAX);
    if (len != -1) {
      /* Replace the "blender" tail */
      sprintf(&path[len - 7], "libBlenderXR_SteamVR.so");
      vr_dll = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    }
#elif defined __APPLE__
    static char proc_path[PROC_PIDPATHINFO_MAXSIZE];
    static char path[PATH_MAX];
    memset(path, 0, sizeof(path));
    pid_t pid = getpid();
    int len = proc_pidpath(pid, path, PATH_MAX);
    if (len > 0) {
      /* Replace the "blender.app/Contents/MacOS/blender" tail */
      sprintf(&path[len - 34], "libBlenderXR_SteamVR.dylib");
      vr_dll = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    }
#endif
    if (vr_dll) {
      vr.type = VR_TYPE_STEAM;
    }
    break;
  }
  case VR_TYPE_OCULUS: {
#ifdef WIN32
    vr_dll = LoadLibrary("BlenderXR_Oculus.dll");
    if (vr_dll) {
      vr.type = VR_TYPE_OCULUS;
    }
#endif
    break;
  }
  case VR_TYPE_FOVE: {
#ifdef WIN32
    vr_dll = LoadLibrary("BlenderXR_Fove.dll");
    if (vr_dll) {
      vr.type = VR_TYPE_FOVE;
    }
#endif
    break;
  }
  case VR_TYPE_NULL:
  default: {
    for (int i = 1; i < VR_TYPES; ++i) {
      if (i == VR_TYPE_MAGICLEAP) {
        continue;
      }
      else if (i == VR_TYPE_OPENXR) {
        continue;
      }
      else if (i == VR_TYPE_STEAM) {
#ifdef WIN32
        vr_dll = LoadLibrary("BlenderXR_SteamVR.dll");
#elif defined __linux__
        static char proc_path[PATH_MAX];
        static char path[PATH_MAX];
        memset(path, 0, sizeof(path));
        pid_t pid = getpid();
        sprintf(proc_path, "/proc/%d/exe", pid);
        ssize_t len = readlink(proc_path, path, PATH_MAX);
        if (len != -1) {
          /* Replace the "blender" tail */
          sprintf(&path[len - 7], "libBlenderXR_SteamVR.so");
          vr_dll = dlopen(path, RTLD_NOW | RTLD_LOCAL);
        }
#elif defined __APPLE__
        static char proc_path[PROC_PIDPATHINFO_MAXSIZE];
        static char path[PATH_MAX];
        memset(path, 0, sizeof(path));
        pid_t pid = getpid();
        int len = proc_pidpath(pid, path, PATH_MAX);
        if (len > 0) {
          /* Replace the "blender.app/Contents/MacOS/blender" tail */
          sprintf(&path[len - 34], "libBlenderXR_SteamVR.dylib");
          vr_dll = dlopen(path, RTLD_NOW | RTLD_LOCAL);
        }
#endif
        if (vr_dll) {
          vr.type = VR_TYPE_STEAM;
          break;
        }
      }
#ifdef WIN32
      else if (i == VR_TYPE_OCULUS) {
        vr_dll = LoadLibrary("BlenderXR_Oculus.dll");
        if (vr_dll) {
          vr.type = VR_TYPE_OCULUS;
          break;
        }
      }
      else if (i == VR_TYPE_FOVE) {
        vr_dll = LoadLibrary("BlenderXR_Fove.dll");
        if (vr_dll) {
          vr.type = VR_TYPE_FOVE;
          break;
        }
      }
#endif
    }
    break;
  }
  }
	if (!vr_dll) {
		return -1;
	}

#ifdef WIN32
	vr_dll_create_vr = (c_createVR)GetProcAddress(vr_dll, "c_createVR");
	if (!vr_dll_create_vr) {
		return -1;
	}
	vr_dll_init_vr = (c_initVR)GetProcAddress(vr_dll, "c_initVR");
	if (!vr_dll_init_vr) {
		return -1;
	}
	vr_dll_get_hmd_type = (c_getHMDType)GetProcAddress(vr_dll, "c_getHMDType");
	if (!vr_dll_get_hmd_type) {
		return -1;
	}
	vr_dll_set_eye_params = (c_setEyeParams)GetProcAddress(vr_dll, "c_setEyeParams");
	if (!vr_dll_set_eye_params) {
		return -1;
	}
	vr_dll_get_default_eye_params = (c_getDefaultEyeParams)GetProcAddress(vr_dll, "c_getDefaultEyeParams");
	if (!vr_dll_get_default_eye_params) {
		return -1;
	}
	vr_dll_get_default_eye_tex_size = (c_getDefaultEyeTexSize)GetProcAddress(vr_dll, "c_getDefaultEyeTexSize");
	if (!vr_dll_get_default_eye_tex_size) {
		return -1;
	}
	vr_dll_update_tracking_vr = (c_updateTrackingVR)GetProcAddress(vr_dll, "c_updateTrackingVR");
	if (!vr_dll_update_tracking_vr) {
		return -1;
	}
	vr_dll_get_eye_positions = (c_getEyePositions)GetProcAddress(vr_dll, "c_getEyePositions");
	if (!vr_dll_get_eye_positions) {
		return -1;
	}
	vr_dll_get_hmd_position = (c_getHMDPosition)GetProcAddress(vr_dll, "c_getHMDPosition");
	if (!vr_dll_get_hmd_position) {
		return -1;
	}
	vr_dll_get_controller_positions = (c_getControllerPositions)GetProcAddress(vr_dll, "c_getControllerPositions");
	if (!vr_dll_get_controller_positions) {
		return -1;
	}
	vr_dll_get_controller_states = (c_getControllerStates)GetProcAddress(vr_dll, "c_getControllerStates");
	if (!vr_dll_get_controller_states) {
		return -1;
	}
	vr_dll_blit_eye = (c_blitEye)GetProcAddress(vr_dll, "c_blitEye");
	if (!vr_dll_blit_eye) {
		return -1;
	}
	vr_dll_blit_eyes = (c_blitEyes)GetProcAddress(vr_dll, "c_blitEyes");
	if (!vr_dll_blit_eyes) {
		return -1;
	}
	vr_dll_submit_frame = (c_submitFrame)GetProcAddress(vr_dll, "c_submitFrame");
	if (!vr_dll_submit_frame) {
		return -1;
	}
	vr_dll_uninit_vr = (c_uninitVR)GetProcAddress(vr_dll, "c_uninitVR");
	if (!vr_dll_uninit_vr) {
		return -1;
	}
#else
	vr_dll_create_vr = (c_createVR)dlsym(vr_dll, "c_createVR");
	if (!vr_dll_create_vr) {
		return -1;
	}
	vr_dll_init_vr = (c_initVR)dlsym(vr_dll, "c_initVR");
	if (!vr_dll_init_vr) {
		return -1;
	}
	vr_dll_get_hmd_type = (c_getHMDType)dlsym(vr_dll, "c_getHMDType");
	if (!vr_dll_get_hmd_type) {
		return -1;
	}
	vr_dll_set_eye_params = (c_setEyeParams)dlsym(vr_dll, "c_setEyeParams");
	if (!vr_dll_set_eye_params) {
		return -1;
	}
	vr_dll_get_default_eye_params = (c_getDefaultEyeParams)dlsym(vr_dll, "c_getDefaultEyeParams");
	if (!vr_dll_get_default_eye_params) {
		return -1;
	}
	vr_dll_get_default_eye_tex_size = (c_getDefaultEyeTexSize)dlsym(vr_dll, "c_getDefaultEyeTexSize");
	if (!vr_dll_get_default_eye_tex_size) {
		return -1;
	}
	vr_dll_update_tracking_vr = (c_updateTrackingVR)dlsym(vr_dll, "c_updateTrackingVR");
	if (!vr_dll_update_tracking_vr) {
		return -1;
	}
	vr_dll_get_eye_positions = (c_getEyePositions)dlsym(vr_dll, "c_getEyePositions");
	if (!vr_dll_get_eye_positions) {
		return -1;
	}
	vr_dll_get_hmd_position = (c_getHMDPosition)dlsym(vr_dll, "c_getHMDPosition");
	if (!vr_dll_get_hmd_position) {
		return -1;
	}
	vr_dll_get_controller_positions = (c_getControllerPositions)dlsym(vr_dll, "c_getControllerPositions");
	if (!vr_dll_get_controller_positions) {
		return -1;
	}
	vr_dll_get_controller_states = (c_getControllerStates)dlsym(vr_dll, "c_getControllerStates");
	if (!vr_dll_get_controller_states) {
		return -1;
	}
	vr_dll_blit_eye = (c_blitEye)dlsym(vr_dll, "c_blitEye");
	if (!vr_dll_blit_eye) {
		return -1;
	}
	vr_dll_blit_eyes = (c_blitEyes)dlsym(vr_dll, "c_blitEyes");
	if (!vr_dll_blit_eyes) {
		return -1;
	}
	vr_dll_submit_frame = (c_submitFrame)dlsym(vr_dll, "c_submitFrame");
	if (!vr_dll_submit_frame) {
		return -1;
	}
	vr_dll_uninit_vr = (c_uninitVR)dlsym(vr_dll, "c_uninitVR");
	if (!vr_dll_uninit_vr) {
		return -1;
	}
#endif

	return 0;
}

/* Create temporary camera. */
static int vr_create_temp_camera(View3D *v3d)
{
  /* Adapted from bc_add_object() in collada_utils.cpp */
  bContext *C = vr_get_obj()->ctx;
  Main *main = CTX_data_main(C);
  ViewLayer *view_layer = CTX_data_view_layer(C);

  Object *ob = BKE_object_add_only_object(main, OB_CAMERA, NULL);
  if (!ob) {
    return -1;
  }

  ob->data = BKE_object_obdata_add_from_type(main, OB_CAMERA, NULL);
  DEG_id_tag_update(&ob->id, ID_RECALC_TRANSFORM | ID_RECALC_GEOMETRY | ID_RECALC_ANIMATION);

  LayerCollection *layer_collection = BKE_layer_collection_get_active(view_layer);
  BKE_collection_object_add(main, layer_collection->collection, ob);

  Base *base = BKE_view_layer_base_find(view_layer, ob);
  BKE_view_layer_base_select_and_set_active(view_layer, base);

  v3d->camera = vr_temp_cam = ob;

  return 0;
}

/* Delete temporary camera. */
static int vr_delete_temp_camera(void)
{
  bContext *C = vr_get_obj()->ctx;
  Main *bmain = CTX_data_main(C);
  Scene *scene = CTX_data_scene(C);
  wmWindowManager *wm = CTX_wm_manager(C);
  wmWindow *win;
  bool changed = false;
  bool use_global = true;

  if (!vr_temp_cam) {
    return 0;
  }
  Object *ob = vr_temp_cam;

  const bool is_indirectly_used = BKE_library_ID_is_indirectly_used(bmain, ob);
  if (ob->id.tag & LIB_TAG_INDIRECT) {
    /* Can this case ever happen? */
    return -1;
  }
  else if (is_indirectly_used && ID_REAL_USERS(ob) <= 1 && ID_EXTRA_USERS(ob) == 0) {
    return -1;
  }

  /* This is sort of a quick hack to address T51243 - Proper thing to do here would be to nuke most of all this
    * custom scene/object/base handling, and use generic lib remap/query for that.
    * But this is for later (aka 2.8, once layers & co are settled and working).
    */
  if (use_global && ob->id.lib == NULL) {
    /* We want to nuke the object, let's nuke it the easy way (not for linked data though)... */
    BKE_id_delete(bmain, &ob->id);
    changed = true;
  }
  else {
    /* remove from current scene only */
    ED_object_base_free_and_unlink(bmain, scene, ob);
    changed = true;
  }

  if (use_global) {
    Scene *scene_iter;
    for (scene_iter = (Scene*)bmain->scenes.first; scene_iter; scene_iter = (Scene*)scene_iter->id.next) {
      if (scene_iter != scene && !ID_IS_LINKED(scene_iter)) {
        if (is_indirectly_used && ID_REAL_USERS(ob) <= 1 && ID_EXTRA_USERS(ob) == 0) {
          break;
        }
        ED_object_base_free_and_unlink(bmain, scene_iter, ob);
      }
    }
  }

  if (!changed) {
    return -1;
  }

  /* delete has to handle all open scenes */
  BKE_main_id_tag_listbase(&bmain->scenes, LIB_TAG_DOIT, true);
  for (win = (wmWindow*)wm->windows.first; win; win = win->next) {
    scene = WM_window_get_active_scene(win);

    if (scene->id.tag & LIB_TAG_DOIT) {
      scene->id.tag &= ~LIB_TAG_DOIT;

      DEG_relations_tag_update(bmain);

      DEG_id_tag_update(&scene->id, ID_RECALC_SELECT);
      WM_event_add_notifier(C, NC_SCENE | ND_OB_ACTIVE, scene);
      WM_event_add_notifier(C, NC_SCENE | ND_LAYER_CONTENT, scene);
    }
  }
  //ED_undo_push(C, "Delete");

  vr_temp_cam = NULL;

  return 0;
}

/* Copy-pasted from wm_draw_offscreen_texture_parameters(). */
static void vr_draw_offscreen_texture_parameters(GPUOffScreen *offscreen)
{
	/* Setup offscreen color texture for drawing. */
	GPUTexture *texture = GPU_offscreen_color_texture(offscreen);

	/* We don't support multisample textures here. */
	BLI_assert(GPU_texture_target(texture) == GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, GPU_texture_opengl_bindcode(texture));

	/* No mipmaps or filtering. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	/* GL_TEXTURE_BASE_LEVEL = 0 by default */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
}

int vr_init(bContext *C)
{
  memset(&vr, 0, sizeof(vr));

  int type;
  int autodetect = (U.vr_device == 0);

  if (U.vr_openxr) {
    switch ((VR_Device_Type)U.vr_device) {
    case VR_DEVICE_TYPE_MAGICLEAP: {
      type = VR_TYPE_MAGICLEAP;
      break;
    }
    default: {
      type = VR_TYPE_OPENXR;
      autodetect = 0;
      break;
    }
    }
  }
  else {
    if (autodetect) {
      /* Autodetect hardware */
      type = 1;
    }
    else {
      /* Test for specified device */
      switch ((VR_Device_Type)U.vr_device) {
      case VR_DEVICE_TYPE_OCULUS: {
        type = VR_TYPE_OCULUS;
        break;
      }
      case VR_DEVICE_TYPE_VIVE:
      case VR_DEVICE_TYPE_WINDOWSMR:
      case VR_DEVICE_TYPE_PIMAX:
      case VR_DEVICE_TYPE_INDEX: {
        type = VR_TYPE_STEAM;
        break;
      }
      case VR_DEVICE_TYPE_FOVE: {
        type = VR_TYPE_FOVE;
        break;
      }
      case VR_DEVICE_TYPE_MAGICLEAP: {
        type = VR_TYPE_MAGICLEAP;
        break;
      }
      case VR_DEVICE_TYPE_NULL:
      default: {
        printf("Invalid VR device.");
        return -1;
      }
      }
    }
  }

  if (type == VR_TYPE_MAGICLEAP) {
    vr.type = VR_TYPE_MAGICLEAP;
    /* Try to start remote session. */
    int error = vr_api_init_remote(5);
    if (!error) {
      /* Get VR params. */
      vr_api_get_params_remote();
      vr.aperture_u = 1.0f;
      vr.aperture_v = 1.0f;
      vr.clip_sta = VR_CLIP_NEAR;
      vr.clip_end = VR_CLIP_FAR;
      vr_api_get_transforms_remote();

      vr.ctx = C;
      vr.initialized = 1;
      return 0;
    }
    else {
      return -1;
    }
  }

  for (; type < VR_TYPES; ++type) {
    int error = vr_load_dll_functions((VR_Type)type);

    if (!error) {
      vr_dll_create_vr();
  #ifdef WIN32
      HDC device = wglGetCurrentDC();
      HGLRC context = wglGetCurrentContext();
      error = vr_dll_init_vr((void*)device, (void*)context);
  #else
  #ifdef __APPLE__
        Display *display = NULL;
  #else
        Display *display = glXGetCurrentDisplay();
  #endif
        GLXDrawable drawable = glXGetCurrentDrawable();
        GLXContext context = glXGetCurrentContext();
        error = vr_dll_init_vr((void*)display, (void*)&drawable, (void*)&context);
  #endif
      if (!error) {
        /* Get VR params. */
        vr_dll_get_default_eye_params(0, &vr.fx[0], &vr.fy[0], &vr.cx[0], &vr.cy[0]);
        vr_dll_get_default_eye_params(1, &vr.fx[1], &vr.fy[1], &vr.cx[1], &vr.cy[1]);
        vr_dll_get_default_eye_tex_size(&vr.tex_width, &vr.tex_height, 0);
        vr.aperture_u = 1.0f;
        vr.aperture_v = 1.0f;
        vr.clip_sta = VR_CLIP_NEAR;
        vr.clip_end = VR_CLIP_FAR;
        vr_dll_get_eye_positions(vr.t_eye[VR_SPACE_REAL]);
        vr_dll_get_hmd_position(vr.t_hmd[VR_SPACE_REAL]);
        vr_dll_get_controller_positions(vr.t_controller[VR_SPACE_REAL]);

        vr.ctx = C;
        vr.initialized = 1;
      }
      else {
        vr_dll_uninit_vr();
      }
    }

    if (!vr.initialized) {
      if (!autodetect || (type == (VR_TYPES - 1))) {
        vr.type = VR_TYPE_NULL;
        printf("Failed to initialize VR.");
        return -1;
      }
    }
    else {
      break;
    }
  }

	return 0;
}

int vr_init_ui(void)
{
	BLI_assert(vr.initialized);

	int error;

  if (vr.type == VR_TYPE_MAGICLEAP) {
    vr.device_type = VR_DEVICE_TYPE_MAGICLEAP;
  }
  else {
    /* Assign the UI type based on the HMD type.
     * This is important when the VR type differs from
     * the HMD type (i.e. running WindowsMR through SteamVR. */
    vr_dll_get_hmd_type((int*)&vr.device_type);
  }

	vr_api_create_ui();
#ifdef WIN32
	HDC device = wglGetCurrentDC();
	HGLRC context = wglGetCurrentContext();
	error = vr_api_init_ui((void*)device, (void*)context);
#else
#ifdef __APPLE__
		Display *display = NULL;
#else
		Display *display = glXGetCurrentDisplay();
#endif
		GLXDrawable drawable = glXGetCurrentDrawable();
		GLXContext context = glXGetCurrentContext();
		error = vr_api_init_ui((void*)display, (void*)&drawable, (void*)&context);
#endif
	if (!error) {
    /* Allocate controller structs. */
    for (int i = 0; i < VR_MAX_CONTROLLERS; ++i) {
      vr.controller[i] = MEM_callocN(sizeof(VR_Controller), "VR_Controller");
    }
    if (vr.type == VR_TYPE_MAGICLEAP) {
      vr_api_get_controller_states_remote();
    }
    else {
      vr_dll_get_controller_states(vr.controller);
    } 
		vr.ui_initialized = 1;
	}

	if (!vr.ui_initialized) {
    vr.device_type = VR_DEVICE_TYPE_NULL;
    printf("vr_init_ui() : Failed to initialize VR UI.");
		return -1;
	}

	return 0;
}

int vr_uninit(void)
{
	BLI_assert(vr.initialized);

	if (vr.ui_initialized) {
		vr_api_uninit_ui();
		/* Free controller structs. */
		for (int i = 0; i < VR_MAX_CONTROLLERS; ++i) {
			if (vr.controller[i]) {
				MEM_freeN(vr.controller[i]);
				vr.controller[i] = NULL;
			}
		}

		vr.ui_initialized = 0;
	}
  if (vr.type == VR_TYPE_MAGICLEAP) {
    vr_api_uninit_remote(0);
  }
  else {
    vr_dll_uninit_vr();
  }

  /* Free viewports. */
  //vr_free_viewports();

  if (vr_temp_cam) {
    /* Delete temporary camera. */
    vr_delete_temp_camera();
  }

  vr.ctx = NULL;
  vr.initialized = 0;
  vr.type = VR_TYPE_NULL;
  vr.device_type = VR_DEVICE_TYPE_NULL;

	int error = vr_unload_dll_functions();
	if (error) {
		return -1;
	}

	return 0;
}

int vr_create_viewports(struct ARegion* ar)
{
	BLI_assert(vr.initialized);

	if (!ar->draw_buffer) {
		ar->draw_buffer = MEM_callocN(sizeof(wmDrawBuffer), "wmDrawBuffer");

		for (int i = 0; i < 2; ++i) {
			GPUOffScreen *offscreen = GPU_offscreen_create(vr.tex_width, vr.tex_height, 0, true, true, NULL);
			if (!offscreen) {
				printf("vr_create_viewports() : Could not create offscreen buffers.");
				return -1;
			}
			vr_draw_offscreen_texture_parameters(offscreen);

			vr.offscreen[i] = ar->draw_buffer->offscreen[i] = offscreen;
			vr.viewport[i] = ar->draw_buffer->viewport[i] = GPU_viewport_create_from_offscreen(vr.offscreen[i]);
		}

		RegionView3D *rv3d = ar->regiondata;
		if (!rv3d) {
			return -1;
		}
		/* Set region view to perspective. */
		rv3d->is_persp = 1;
		rv3d->persp = RV3D_PERSP;
#if WITH_VR
		rv3d->rflag |= RV3D_IS_VR;
#endif
	}

	return 0;
}

void vr_free_viewports(ARegion *ar)
{
	if (ar->draw_buffer) {
		for (int side = 0; side < 2; ++side) {
			if (vr.offscreen[side]) {
				GPU_offscreen_free(vr.offscreen[side]);
				vr.offscreen[side] = NULL;
			}
			if (vr.viewport[side]) {
				GPU_viewport_free(vr.viewport[side]);
				vr.viewport[side] = NULL;
			}
		}

		MEM_freeN(ar->draw_buffer);
		ar->draw_buffer = NULL;
	}
}

void vr_draw_region_bind(ARegion *ar, int side)
{
	BLI_assert(vr.initialized);

	if (!vr.viewport[side]) {
		return;
	}

	/* Render with VR dimensions, regardless of window size. */
	rcti rect;
	rect.xmin = 0;
	rect.xmax = vr.tex_width;
	rect.ymin = 0;
	rect.ymax = vr.tex_height;

	GPU_viewport_bind(vr.viewport[side], &rect);

	ar->draw_buffer->bound_view = side;
}

void vr_draw_region_unbind(ARegion *ar, int side)
{
	BLI_assert(vr.initialized);

	if (!vr.viewport[side]) {
		return;
	}

	ar->draw_buffer->bound_view = -1;

	GPU_viewport_unbind(vr.viewport[side]);
}

int vr_update_tracking(void)
{
	BLI_assert(vr.initialized);

  int error;

  if (vr.type == VR_TYPE_MAGICLEAP) {
    vr_api_get_transforms_remote();
  }
  else {
    error = vr_dll_update_tracking_vr();

    /* Get hmd and eye positions. */
    vr_dll_get_hmd_position(vr.t_hmd[VR_SPACE_REAL]);
    vr_dll_get_eye_positions(vr.t_eye[VR_SPACE_REAL]);

    /* Get controller positions. */
    vr_dll_get_controller_positions(vr.t_controller[VR_SPACE_REAL]);
  }

	if (vr.ui_initialized) {
		/* Get controller button states. */
    if (vr.type == VR_TYPE_MAGICLEAP) {
      vr_api_get_controller_states_remote();
    }
    else {
      vr_dll_get_controller_states(vr.controller);
    }

		/* Update the UI. */
		error = vr_api_update_tracking_ui();
	}

	if (error) {
		vr.tracking = 0;
	}
	else {
		vr.tracking = 1;
	}

	return error;
}

int vr_blit(void)
{
	BLI_assert(vr.initialized);

  if (vr.type == VR_TYPE_MAGICLEAP) {
    return 0;
  }

	int error;

#if WITH_VR
	vr_dll_blit_eyes((void*)(&(vr.viewport[VR_SIDE_LEFT]->fbl->default_fb->attachments[2].tex->bindcode)),
			     	 (void*)(&(vr.viewport[VR_SIDE_RIGHT]->fbl->default_fb->attachments[2].tex->bindcode)),
					 &vr.aperture_u, &vr.aperture_v);
#endif
	error = vr_dll_submit_frame();

	return error;
}

void vr_pre_scene_render(int side)
{
	BLI_assert(vr.ui_initialized);

	vr_api_pre_render(side);
}
void vr_post_scene_render(int side)
{
	BLI_assert(vr.ui_initialized);

	vr_api_post_render(side);
}

void vr_do_interaction(void)
{
	BLI_assert(vr.ui_initialized);

	vr_api_execute_operations();
}

void vr_do_post_render_interaction(void)
{
	BLI_assert(vr.ui_initialized);

	vr_api_execute_post_render_operations();
}

void vr_update_view_matrix(int side, const float view[4][4])
{
	BLI_assert(vr.ui_initialized);

	/* Take navigation into account. */
	const float (*navinv)[4] = (float(*)[4])vr_api_get_navigation_matrix(1);
	_va_mul_m4_series_3(vr.t_eye[VR_SPACE_REAL][side], navinv, view);
	invert_m4_m4(vr.t_eye_inv[VR_SPACE_REAL][side], vr.t_eye[VR_SPACE_REAL][side]);
	vr_api_update_view_matrix(vr.t_eye_inv[VR_SPACE_REAL][side]);
}

void vr_update_projection_matrix(int side, const float projection[4][4])
{
	BLI_assert(vr.ui_initialized);

	vr_api_update_projection_matrix(side, projection);
}

void vr_update_viewport_bounds(const rcti *bounds)
{
	BLI_assert(vr.ui_initialized);

	vr_api_update_viewport_bounds(bounds);
}

void vr_compute_viewplane(const View3D *v3d, CameraParams *params, int winx, int winy)
{
	BLI_assert(vr.initialized);
	BLI_assert(params);

	int side = v3d->multiview_eye;
	rctf viewplane;
	float xasp, yasp, pixsize, viewfac, sensor_size, dx, dy;

	// float navscale = vr_api_get_navigation_scale();
	params->clip_start = vr.clip_sta; // * navscale; Don't need to apply, because the scale factor on the view matrix affects all transformations.
	params->clip_end = vr.clip_end; // * navscale;
	//params->zoom = 2.0f;

	xasp = vr.aperture_u;
	yasp = vr.aperture_v;
	params->ycor = xasp / yasp;

	if (params->is_ortho) {
		/* orthographic camera */
		/* scale == 1.0 means exact 1 to 1 mapping */
		pixsize = params->ortho_scale;
	}
	else {
		/* perspective camera */
		switch (params->sensor_fit) {
			case CAMERA_SENSOR_FIT_AUTO:
			case CAMERA_SENSOR_FIT_HOR: {
				sensor_size = params->sensor_x;
				params->lens = vr.fx[side] * params->zoom * params->sensor_x;
				break;
			}
			case CAMERA_SENSOR_FIT_VERT: {
				sensor_size = params->sensor_y;
				params->lens = vr.fy[side] * params->zoom * params->sensor_y;
				break;
			}
		}
		pixsize = (sensor_size * params->clip_start) / params->lens;
	}

	switch (params->sensor_fit) {
		case CAMERA_SENSOR_FIT_AUTO: {
			if (xasp * vr.tex_width >= yasp *vr.tex_height) {
				viewfac = vr.tex_width;
			}
			else {
				viewfac = params->ycor * vr.tex_height;
			}
			break;
		}
		case CAMERA_SENSOR_FIT_HOR: {
			viewfac = vr.tex_width;
			break;
		}
		case CAMERA_SENSOR_FIT_VERT: {
			viewfac = params->ycor * vr.tex_height;
			break;
		}
	}
	pixsize /= viewfac;

	/* extra zoom factor */
	pixsize *= params->zoom;

	/* lens shift and offset */
	params->offsetx = (vr.cx[side] - 0.5f)*2.0f * xasp;
	params->offsety = (vr.cy[side] - 0.5f)*2.0f * yasp;

	dx = params->shiftx * viewfac + vr.tex_width * params->offsetx;
	dy = params->shifty * viewfac + vr.tex_height * params->offsety;

	/* Compute view plane:
     * centered and at distance 1.0: */
	float res_x = (float)vr.tex_width;
	float res_y = (float)vr.tex_height;
	float pfx = vr.fx[side] * res_x;
	float pfy = vr.fy[side] * res_y;
	float pcx = vr.cx[side] * res_x;
	float pcy = (1.0f - vr.cy[side]) * res_y;
	viewplane.xmax = ((res_x - pcx) / pfx) * params->clip_start;
	viewplane.xmin = (-pcx / pfx) * params->clip_start;
	viewplane.ymax = ((res_y - pcy) / pfy) * params->clip_start;
	viewplane.ymin = (-pcy / pfy) * params->clip_start;

	/* Used for rendering (offset by near-clip with perspective views), passed to RE_SetPixelSize.
	 * For viewport drawing 'RegionView3D.pixsize'. */
	params->viewdx = pixsize;
	params->viewdy = params->ycor * pixsize;
	params->viewplane = viewplane;

	/*
	 * OVERRIDE: due to the navigation, the clipping distance (in Blender coordinates) may change between frames.
	 * Some tools rely on the View3D having the correct clipping distances set, so we have to override this value here.
	 */
	{
		View3D *_v3d = (View3D*)v3d;
		_v3d->clip_start = params->clip_start;
		_v3d->clip_end  = params->clip_end;
		_v3d->lens = params->lens;

		if (_v3d->camera && _v3d->camera->data) {
			Camera *cam = (Camera*)v3d->camera->data;
			/* cam->type = CAM_PERSP; */
			cam->lens = params->lens;
			/* cam->ortho_scale = params->ortho_scale; */
			/* params->shiftx = cam->shiftx; */
			/* params->shifty = cam->shifty; */
			/* params->sensor_x = cam->sensor_x; */
			/* params->sensor_y = cam->sensor_y; */
			/* params->sensor_fit = cam->sensor_fit; */
			cam->clip_start = params->clip_start;
			cam->clip_end = params->clip_end;
		}
    else {
      vr_create_temp_camera(_v3d);
    }
	}
}

void vr_compute_viewmat(int side, float viewmat_out[4][4])
{
	BLI_assert(vr.initialized);

	if (vr.ui_initialized) {
		/* Take navigation into account. */
		const float(*navmat)[4] = (float(*)[4])vr_api_get_navigation_matrix(0);
		_va_mul_m4_series_3(vr.t_eye[VR_SPACE_BLENDER][side], navmat, vr.t_eye[VR_SPACE_REAL][side]);
		invert_m4_m4(viewmat_out, vr.t_eye[VR_SPACE_BLENDER][side]);
	}
	else {
		invert_m4_m4(viewmat_out, vr.t_eye[VR_SPACE_REAL][side]);
	}
}
