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

/** \file blender/vr/intern/vr_draw.cpp
*   \ingroup vr
*
* Collection of OpenGL-related utility functionality for drawing VR UI elements.
*/

#include "vr_types.h"
#include "vr_main.h"

#include <glew.h>

#ifdef WIN32
#include <Windows.h>
#else
#include <GL/glxew.h>
#include <string>
#endif
#include <GL/gl.h>

#include "vr_draw.h"
#include "vr_math.h"
#include "vr_ui.h"

#include "png.h"

/* Controller models and textures */
#include "ui_oculus_controller_left.obj.h"
#include "ui_oculus_controller_right.obj.h"
#include "ui_oculus_controller.png.h"

#include "ui_vive_controller.obj.h"
#include "ui_vive_controller.png.h"

#include "ui_windowsmr_controller_left.obj.h"
#include "ui_windowsmr_controller_right.obj.h"
#include "ui_windowsmr_controller.png.h"

#include "ui_index_controller_left.obj.h"
#include "ui_index_controller_right.obj.h"
#include "ui_index_controller.png.h"

#include "ui_magicleap_controller.obj.h"
#include "ui_magicleap_controller.png.h"

#include "ui_cursor.obj.h"
#include "ui_cursor.png.h"
#include "icon_cursor.png.h"
#include "icon_mouse_cursor.png.h"

/* Image textures */
#include "icon_nav_grabair.png.h"
#include "icon_nav_joystick.png.h"
#include "icon_nav_teleport.png.h"
#include "icon_nav_locktrans.png.h"
#include "icon_nav_locktransup.png.h"
#include "icon_nav_lockrot.png.h"
#include "icon_nav_lockrotup.png.h"
#include "icon_nav_lockscale.png.h"
#include "icon_nav_lockscalereal.png.h"
#include "icon_ctrl.png.h"
#include "icon_shift.png.h"
#include "icon_alt.png.h"
#include "icon_select.png.h"
#include "icon_select_raycast.png.h"
#include "icon_select_proximity.png.h"
#include "icon_cursor_teleport.png.h"
#include "icon_cursor_worldorigin.png.h"
#include "icon_cursor_objorigin.png.h"
#include "icon_transform.png.h"
#include "icon_move.png.h"
#include "icon_rotate.png.h"
#include "icon_scale.png.h"
#include "icon_annotate.png.h"
#include "icon_measure.png.h"
#include "icon_mesh.png.h"
#include "icon_mesh_plane.png.h"
#include "icon_mesh_cube.png.h"
#include "icon_mesh_circle.png.h"
#include "icon_mesh_cylinder.png.h"
#include "icon_mesh_cone.png.h"
#include "icon_mesh_grid.png.h"
#include "icon_mesh_monkey.png.h"
#include "icon_mesh_uvsphere.png.h"
#include "icon_mesh_icosphere.png.h"
#include "icon_extrude.png.h"
#include "icon_extrude_individual.png.h"
#include "icon_extrude_normals.png.h"
#include "icon_insetfaces.png.h"
#include "icon_bevel.png.h"
#include "icon_loopcut.png.h"
#include "icon_knife.png.h"
#include "icon_sculpt.png.h"
#include "icon_sculpt_draw.png.h"
#include "icon_sculpt_clay.png.h"
#include "icon_sculpt_claystrips.png.h"
#include "icon_sculpt_layer.png.h"
#include "icon_sculpt_inflate.png.h"
#include "icon_sculpt_blob.png.h"
#include "icon_sculpt_crease.png.h"
#include "icon_sculpt_smooth.png.h"
#include "icon_sculpt_flatten.png.h"
#include "icon_sculpt_fill.png.h"
#include "icon_sculpt_scrape.png.h"
#include "icon_sculpt_pinch.png.h"
#include "icon_sculpt_grab.png.h"
#include "icon_sculpt_snakehook.png.h"
#include "icon_sculpt_thumb.png.h"
#include "icon_sculpt_nudge.png.h"
#include "icon_sculpt_rotate.png.h"
#include "icon_sculpt_mask.png.h"
#include "icon_sculpt_simplify.png.h"
#include "icon_animation.png.h"
#include "icon_delete.png.h"
#include "icon_duplicate.png.h"
#include "icon_join.png.h"
#include "icon_separate.png.h"
#include "icon_undo.png.h"
#include "icon_redo.png.h"
#include "icon_manip_global.png.h"
#include "icon_manip_local.png.h"
#include "icon_manip_normal.png.h"
#include "icon_manip_plus.png.h"
#include "icon_manip_minus.png.h"
#include "icon_objectmode.png.h"
#include "icon_editmode.png.h"
#include "icon_object.png.h"
#include "icon_vertex.png.h"
#include "icon_edge.png.h"
#include "icon_face.png.h"
#include "icon_toolsettings.png.h"
#include "icon_box_empty.png.h"
#include "icon_box_filled.png.h"
#include "icon_plus.png.h"
#include "icon_minus.png.h"
#include "icon_reset.png.h"

/* Menu textures */
#include "menu_background.png.h"
#include "menu_colorwheel.png.h"

/* String textures */
#include "ascii.png.h" /* Image of the ASCII texture file */
#include "str_on.png.h"
#include "str_off.png.h"
#include "str_x.png.h"
#include "str_y.png.h"
#include "str_z.png.h"
#include "str_xy.png.h"
#include "str_yz.png.h"
#include "str_zx.png.h"

bool VR_Draw::initialized(false);

#ifdef WIN32
void *VR_Draw::device(0);
//HDC VR_Draw::device(0);

void *VR_Draw::context(0);
//HGLRC VR_Draw::context(0);
#else
void *VR_Draw::display(0);
//Display* VR_Draw::display(0);

void *VR_Draw::drawable(0);
//GLXDrawable VR_Draw::drawable;

void *VR_Draw::context(0);
//GLXContext VR_Draw::context;
#endif

VR_Draw::Model *VR_Draw::controller_model[VR_SIDES]{ 0 };
VR_Draw::Texture *VR_Draw::controller_tex(0);
VR_Draw::Model *VR_Draw::vr_cursor_model(0);
VR_Draw::Texture *VR_Draw::vr_cursor_tex(0);
VR_Draw::Texture *VR_Draw::cursor_tex(0);
VR_Draw::Texture *VR_Draw::mouse_cursor_tex(0);

VR_Draw::Texture *VR_Draw::nav_grabair_tex(0);
VR_Draw::Texture *VR_Draw::nav_joystick_tex(0);
VR_Draw::Texture *VR_Draw::nav_teleport_tex(0);
VR_Draw::Texture *VR_Draw::nav_locktrans_tex(0);
VR_Draw::Texture *VR_Draw::nav_locktransup_tex(0);
VR_Draw::Texture *VR_Draw::nav_lockrot_tex(0);
VR_Draw::Texture *VR_Draw::nav_lockrotup_tex(0);
VR_Draw::Texture *VR_Draw::nav_lockscale_tex(0);
VR_Draw::Texture *VR_Draw::nav_lockscalereal_tex(0);
VR_Draw::Texture *VR_Draw::ctrl_tex(0);
VR_Draw::Texture *VR_Draw::shift_tex(0);
VR_Draw::Texture *VR_Draw::alt_tex(0);
VR_Draw::Texture *VR_Draw::select_tex(0);
VR_Draw::Texture *VR_Draw::select_raycast_tex(0);
VR_Draw::Texture *VR_Draw::select_proximity_tex(0);
VR_Draw::Texture *VR_Draw::cursor_teleport_tex(0);
VR_Draw::Texture *VR_Draw::cursor_worldorigin_tex(0);
VR_Draw::Texture *VR_Draw::cursor_objorigin_tex(0);
VR_Draw::Texture *VR_Draw::transform_tex(0);
VR_Draw::Texture *VR_Draw::move_tex(0);
VR_Draw::Texture *VR_Draw::rotate_tex(0);
VR_Draw::Texture *VR_Draw::scale_tex(0);
VR_Draw::Texture *VR_Draw::annotate_tex(0);
VR_Draw::Texture *VR_Draw::measure_tex(0);
VR_Draw::Texture *VR_Draw::mesh_tex(0);
VR_Draw::Texture *VR_Draw::mesh_plane_tex(0);
VR_Draw::Texture *VR_Draw::mesh_cube_tex(0);
VR_Draw::Texture *VR_Draw::mesh_circle_tex(0);
VR_Draw::Texture *VR_Draw::mesh_cylinder_tex(0);
VR_Draw::Texture *VR_Draw::mesh_cone_tex(0);
VR_Draw::Texture *VR_Draw::mesh_grid_tex(0);
VR_Draw::Texture *VR_Draw::mesh_monkey_tex(0);
VR_Draw::Texture *VR_Draw::mesh_uvsphere_tex(0);
VR_Draw::Texture *VR_Draw::mesh_icosphere_tex(0);
VR_Draw::Texture *VR_Draw::extrude_tex(0);
VR_Draw::Texture *VR_Draw::extrude_individual_tex(0);
VR_Draw::Texture *VR_Draw::extrude_normals_tex(0);
VR_Draw::Texture *VR_Draw::insetfaces_tex(0);
VR_Draw::Texture *VR_Draw::bevel_tex(0);
VR_Draw::Texture *VR_Draw::loopcut_tex(0);
VR_Draw::Texture *VR_Draw::knife_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_draw_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_clay_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_claystrips_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_layer_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_inflate_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_blob_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_crease_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_smooth_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_flatten_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_fill_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_scrape_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_pinch_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_grab_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_snakehook_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_thumb_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_nudge_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_rotate_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_mask_tex(0);
VR_Draw::Texture *VR_Draw::sculpt_simplify_tex(0);
VR_Draw::Texture *VR_Draw::animation_tex(0);
VR_Draw::Texture *VR_Draw::delete_tex(0);
VR_Draw::Texture *VR_Draw::delete_alt_tex(0);
VR_Draw::Texture *VR_Draw::duplicate_tex(0);
VR_Draw::Texture *VR_Draw::join_tex(0);
VR_Draw::Texture *VR_Draw::separate_tex(0);
VR_Draw::Texture *VR_Draw::undo_tex(0);
VR_Draw::Texture *VR_Draw::redo_tex(0);
VR_Draw::Texture *VR_Draw::manip_global_tex(0);
VR_Draw::Texture *VR_Draw::manip_local_tex(0);
VR_Draw::Texture *VR_Draw::manip_normal_tex(0);
VR_Draw::Texture *VR_Draw::manip_plus_tex(0);
VR_Draw::Texture *VR_Draw::manip_minus_tex(0);
VR_Draw::Texture *VR_Draw::objectmode_tex(0);
VR_Draw::Texture *VR_Draw::editmode_tex(0);
VR_Draw::Texture *VR_Draw::object_tex(0);
VR_Draw::Texture *VR_Draw::vertex_tex(0);
VR_Draw::Texture *VR_Draw::edge_tex(0);
VR_Draw::Texture *VR_Draw::face_tex(0);
VR_Draw::Texture *VR_Draw::toolsettings_tex(0);
VR_Draw::Texture *VR_Draw::box_empty_tex(0);
VR_Draw::Texture *VR_Draw::box_filled_tex(0);
VR_Draw::Texture *VR_Draw::plus_tex(0);
VR_Draw::Texture *VR_Draw::minus_tex(0);
VR_Draw::Texture *VR_Draw::reset_tex(0);

VR_Draw::Texture *VR_Draw::background_menu_tex(0);
VR_Draw::Texture *VR_Draw::colorwheel_menu_tex(0);

VR_Draw::Texture *VR_Draw::ascii_tex(0);
VR_Draw::Texture *VR_Draw::on_str_tex(0);
VR_Draw::Texture *VR_Draw::off_str_tex(0);
VR_Draw::Texture *VR_Draw::x_str_tex(0);
VR_Draw::Texture *VR_Draw::y_str_tex(0);
VR_Draw::Texture *VR_Draw::z_str_tex(0);
VR_Draw::Texture *VR_Draw::xy_str_tex(0);
VR_Draw::Texture *VR_Draw::yz_str_tex(0);
VR_Draw::Texture *VR_Draw::zx_str_tex(0);

Mat44f VR_Draw::model_matrix;
Mat44f VR_Draw::view_matrix;
Mat44f VR_Draw::projection_matrix;
Mat44f VR_Draw::modelview_matrix;
Mat44f VR_Draw::modelview_matrix_inv;
float VR_Draw::color_vector[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

VR_Draw::VR_Draw()
{
	//
}

VR_Draw::~VR_Draw()
{
	//
}

#ifdef WIN32
int VR_Draw::init(void* device, void* context)
#else
int VR_Draw::init(void* display, void* drawable, void* context)
#endif
{
	/* Save GL device, context. */
#ifdef WIN32
	VR_Draw::device = device;
	VR_Draw::context = context;
	//VR_Draw::device = (HDC)device;
	//VR_Draw::context = (HGLRC)context;
#else
	VR_Draw::display = display;
	VR_Draw::drawable = drawable;
	VR_Draw::context = context;
	//VR_Draw::display = (Display*)display;
	//VR_Draw::drawable = (GLXDrawable)(*drawable);
	//VR_Draw::context = (GLXContext)(*context);
#endif

	/* Initialize glew. */
	glewExperimental = GL_TRUE; /* This is required for the glGenFramebuffers call */
	if (glewInit() != GLEW_OK) {
		return -1;
	}

	/* Create cursor model. */
	vr_cursor_model = Model::create(ui_cursor_obj_verts, ui_cursor_obj_nrmls, ui_cursor_obj_uvs, ui_cursor_obj_numverts);
	vr_cursor_tex = new Texture(ui_cursor_png);
	if (vr_cursor_model) {
		vr_cursor_model->texture = vr_cursor_tex;
	}
	cursor_tex = new Texture(icon_cursor_png);
	mouse_cursor_tex = new Texture(icon_mouse_cursor_png);

	/* Create image textures. */
	nav_grabair_tex = new Texture(icon_nav_grabair_png);
	nav_joystick_tex = new Texture(icon_nav_joystick_png);
	nav_teleport_tex = new Texture(icon_nav_teleport_png);
	nav_locktrans_tex = new Texture(icon_nav_locktrans_png);
	nav_locktransup_tex = new Texture(icon_nav_locktransup_png);
	nav_lockrot_tex = new Texture(icon_nav_lockrot_png);
	nav_lockrotup_tex = new Texture(icon_nav_lockrotup_png);
	nav_lockscale_tex = new Texture(icon_nav_lockscale_png);
	nav_lockscalereal_tex = new Texture(icon_nav_lockscalereal_png);
	ctrl_tex = new Texture(icon_ctrl_png);
	shift_tex = new Texture(icon_shift_png);
	alt_tex = new Texture(icon_alt_png);
	select_tex = new Texture(icon_select_png);
	select_raycast_tex = new Texture(icon_select_raycast_png);
	select_proximity_tex = new Texture(icon_select_proximity_png);
	cursor_teleport_tex = new Texture(icon_cursor_teleport_png);
	cursor_worldorigin_tex = new Texture(icon_cursor_worldorigin_png);
	cursor_objorigin_tex = new Texture(icon_cursor_objorigin_png);
	transform_tex = new Texture(icon_transform_png);
	move_tex = new Texture(icon_move_png);
	rotate_tex = new Texture(icon_rotate_png);
	scale_tex = new Texture(icon_scale_png);
	annotate_tex = new Texture(icon_annotate_png);
	measure_tex = new Texture(icon_measure_png);
	mesh_tex = new Texture(icon_mesh_png);
	mesh_plane_tex = new Texture(icon_mesh_plane_png);
	mesh_cube_tex = new Texture(icon_mesh_cube_png);
	mesh_circle_tex = new Texture(icon_mesh_circle_png);
	mesh_cylinder_tex = new Texture(icon_mesh_cylinder_png);
	mesh_cone_tex = new Texture(icon_mesh_cone_png);
	mesh_grid_tex = new Texture(icon_mesh_grid_png);
	mesh_monkey_tex = new Texture(icon_mesh_monkey_png);
	mesh_uvsphere_tex = new Texture(icon_mesh_uvsphere_png);
	mesh_icosphere_tex = new Texture(icon_mesh_icosphere_png);
	extrude_tex = new Texture(icon_extrude_png);
	extrude_individual_tex = new Texture(icon_extrude_individual_png);
	extrude_normals_tex = new Texture(icon_extrude_normals_png);
	insetfaces_tex = new Texture(icon_insetfaces_png);
	bevel_tex = new Texture(icon_bevel_png);
	loopcut_tex = new Texture(icon_loopcut_png);
	knife_tex = new Texture(icon_knife_png);
	sculpt_tex = new Texture(icon_sculpt_png);
	sculpt_draw_tex = new Texture(icon_sculpt_draw_png);
	sculpt_clay_tex = new Texture(icon_sculpt_clay_png);
	sculpt_claystrips_tex = new Texture(icon_sculpt_claystrips_png);
	sculpt_layer_tex = new Texture(icon_sculpt_layer_png);
	sculpt_inflate_tex = new Texture(icon_sculpt_inflate_png);
	sculpt_blob_tex = new Texture(icon_sculpt_blob_png);
	sculpt_crease_tex = new Texture(icon_sculpt_crease_png);
	sculpt_smooth_tex = new Texture(icon_sculpt_smooth_png);
	sculpt_flatten_tex = new Texture(icon_sculpt_flatten_png);
	sculpt_fill_tex = new Texture(icon_sculpt_fill_png);
	sculpt_scrape_tex = new Texture(icon_sculpt_scrape_png);
	sculpt_pinch_tex = new Texture(icon_sculpt_pinch_png);
	sculpt_grab_tex = new Texture(icon_sculpt_grab_png);
	sculpt_snakehook_tex = new Texture(icon_sculpt_snakehook_png);
	sculpt_thumb_tex = new Texture(icon_sculpt_thumb_png);
	sculpt_nudge_tex = new Texture(icon_sculpt_nudge_png);
	sculpt_rotate_tex = new Texture(icon_sculpt_rotate_png);
	sculpt_mask_tex = new Texture(icon_sculpt_mask_png);
	sculpt_simplify_tex = new Texture(icon_sculpt_simplify_png);
	animation_tex = new Texture(icon_animation_png);
	delete_tex = new Texture(icon_delete_png);
	duplicate_tex = new Texture(icon_duplicate_png);
	join_tex = new Texture(icon_join_png);
	separate_tex = new Texture(icon_separate_png);
	undo_tex = new Texture(icon_undo_png);
	redo_tex = new Texture(icon_redo_png);
	manip_global_tex = new Texture(icon_manip_global_png);
	manip_local_tex = new Texture(icon_manip_local_png);
	manip_normal_tex = new Texture(icon_manip_normal_png);
	manip_plus_tex = new Texture(icon_manip_plus_png);
	manip_minus_tex = new Texture(icon_manip_minus_png);
	objectmode_tex = new Texture(icon_objectmode_png);
	editmode_tex = new Texture(icon_editmode_png);
	object_tex = new Texture(icon_object_png);
	vertex_tex = new Texture(icon_vertex_png);
	edge_tex = new Texture(icon_edge_png);
	face_tex = new Texture(icon_face_png);
	toolsettings_tex = new Texture(icon_toolsettings_png);
	box_empty_tex = new Texture(icon_box_empty_png);
	box_filled_tex = new Texture(icon_box_filled_png);
	plus_tex = new Texture(icon_plus_png);
	minus_tex = new Texture(icon_minus_png);
	reset_tex = new Texture(icon_reset_png);

	background_menu_tex = new Texture(menu_background_png);
	colorwheel_menu_tex = new Texture(menu_colorwheel_png);

	ascii_tex = new Texture(ascii_png);
	on_str_tex = new Texture(str_on_png);
	off_str_tex = new Texture(str_off_png);
	x_str_tex = new Texture(str_x_png);
	y_str_tex = new Texture(str_y_png);
	z_str_tex = new Texture(str_z_png);
	xy_str_tex = new Texture(str_xy_png);
	yz_str_tex = new Texture(str_yz_png);
	zx_str_tex = new Texture(str_zx_png);

	model_matrix.set_to_identity();
	view_matrix.set_to_identity();
	projection_matrix.set_to_identity();
	modelview_matrix.set_to_identity();
	modelview_matrix_inv.set_to_identity();

	VR_Draw::initialized = true;

	return 0;
}

void VR_Draw::uninit()
{
	if (controller_tex) {
		delete controller_tex;
		controller_tex = NULL;
	}
	if (controller_model[VR_SIDE_LEFT]) {
		delete controller_model[VR_SIDE_LEFT];
		controller_model[VR_SIDE_LEFT] = NULL;
	}
	if (controller_model[VR_SIDE_RIGHT]) {
		delete controller_model[VR_SIDE_RIGHT];
		controller_model[VR_SIDE_RIGHT] = NULL;
	}
	if (vr_cursor_tex) {
		delete vr_cursor_tex;
		vr_cursor_tex = NULL;
	}
	if (vr_cursor_model) {
		delete vr_cursor_model;
		vr_cursor_model = NULL;
	}
	if (cursor_tex) {
		delete cursor_tex;
		cursor_tex = NULL;
	}
    if (mouse_cursor_tex) {
        delete mouse_cursor_tex;
        mouse_cursor_tex = NULL;
    }

	if (nav_grabair_tex) {
		delete nav_grabair_tex;
		nav_grabair_tex = NULL;
	}
	if (nav_joystick_tex) {
		delete nav_joystick_tex;
		nav_joystick_tex = NULL;
	}
	if (nav_teleport_tex) {
		delete nav_teleport_tex;
		nav_teleport_tex = NULL;
	}
	if (nav_locktrans_tex) {
		delete nav_locktrans_tex;
		nav_locktrans_tex = NULL;
	}
	if (nav_locktransup_tex) {
		delete nav_locktransup_tex;
		nav_locktransup_tex = NULL;
	}
	if (nav_lockrot_tex) {
		delete nav_lockrot_tex;
		nav_lockrot_tex = NULL;
	}
	if (nav_lockrotup_tex) {
		delete nav_lockrotup_tex;
		nav_lockrotup_tex = NULL;
	}
	if (nav_lockscale_tex) {
		delete nav_lockscale_tex;
		nav_lockscale_tex = NULL;
	}
	if (nav_lockscalereal_tex) {
		delete nav_lockscalereal_tex;
		nav_lockscalereal_tex = NULL;
	}
	if (ctrl_tex) {
		delete ctrl_tex;
		ctrl_tex = NULL;
	}
	if (shift_tex) {
		delete shift_tex;
		shift_tex = NULL;
	}
	if (alt_tex) {
		delete alt_tex;
		alt_tex = NULL;
	}
	if (select_tex) {
		delete select_tex;
		select_tex = NULL;
	}
	if (select_raycast_tex) {
		delete select_raycast_tex;
		select_raycast_tex = NULL;
	}
	if (select_proximity_tex) {
		delete select_proximity_tex;
		select_proximity_tex = NULL;
	}
	if (cursor_teleport_tex) {
		delete cursor_teleport_tex;
		cursor_teleport_tex = NULL;
	}
	if (cursor_worldorigin_tex) {
		delete cursor_worldorigin_tex;
		cursor_worldorigin_tex = NULL;
	}
	if (cursor_objorigin_tex) {
		delete cursor_objorigin_tex;
		cursor_objorigin_tex = NULL;
	}
	if (transform_tex) {
		delete transform_tex;
		transform_tex = NULL;
	}
	if (move_tex) {
		delete move_tex;
		move_tex = NULL;
	}
	if (rotate_tex) {
		delete rotate_tex;
		rotate_tex = NULL;
	}
	if (scale_tex) {
		delete scale_tex;
		scale_tex = NULL;
	}
	if (annotate_tex) {
		delete annotate_tex;
		annotate_tex = NULL;
	}
	if (measure_tex) {
		delete measure_tex;
		measure_tex = NULL;
	}
	if (mesh_tex) {
		delete mesh_tex;
		mesh_tex = NULL;
	}
	if (mesh_plane_tex) {
		delete mesh_plane_tex;
		mesh_plane_tex = NULL;
	}
	if (mesh_cube_tex) {
		delete mesh_cube_tex;
		mesh_cube_tex = NULL;
	}
	if (mesh_circle_tex) {
		delete mesh_circle_tex;
		mesh_circle_tex = NULL;
	}
	if (mesh_cylinder_tex) {
		delete mesh_cylinder_tex;
		mesh_cylinder_tex = NULL;
	}
	if (mesh_cone_tex) {
		delete mesh_cone_tex;
		mesh_cone_tex = NULL;
	}
	if (mesh_grid_tex) {
		delete mesh_grid_tex;
		mesh_grid_tex = NULL;
	}
	if (mesh_monkey_tex) {
		delete mesh_monkey_tex;
		mesh_monkey_tex = NULL;
	}
	if (mesh_uvsphere_tex) {
		delete mesh_uvsphere_tex;
		mesh_uvsphere_tex = NULL;
	}
	if (mesh_icosphere_tex) {
		delete mesh_icosphere_tex;
		mesh_icosphere_tex = NULL;
	}
	if (extrude_tex) {
		delete extrude_tex;
		extrude_tex = NULL;
	}
	if (extrude_individual_tex) {
		delete extrude_individual_tex;
		extrude_individual_tex = NULL;
	}
	if (extrude_normals_tex) {
		delete extrude_normals_tex;
		extrude_normals_tex = NULL;
	}
	if (insetfaces_tex) {
		delete insetfaces_tex;
		insetfaces_tex = NULL;
	}
	if (bevel_tex) {
		delete bevel_tex;
		bevel_tex = NULL;
	}
	if (loopcut_tex) {
		delete loopcut_tex;
		loopcut_tex = NULL;
	}
	if (knife_tex) {
		delete knife_tex;
		knife_tex = NULL;
	}
	if (sculpt_tex) {
		delete sculpt_tex;
		sculpt_tex = NULL;
	}
	if (sculpt_draw_tex) {
		delete sculpt_draw_tex;
		sculpt_draw_tex = NULL;
	}
	if (sculpt_clay_tex) {
		delete sculpt_clay_tex;
		sculpt_clay_tex = NULL;
	}
	if (sculpt_claystrips_tex) {
		delete sculpt_claystrips_tex;
		sculpt_claystrips_tex = NULL;
	}
	if (sculpt_layer_tex) {
		delete sculpt_layer_tex;
		sculpt_layer_tex = NULL;
	}
	if (sculpt_inflate_tex) {
		delete sculpt_inflate_tex;
		sculpt_inflate_tex = NULL;
	}
	if (sculpt_blob_tex) {
		delete sculpt_blob_tex;
		sculpt_blob_tex = NULL;
	}
	if (sculpt_crease_tex) {
		delete sculpt_crease_tex;
		sculpt_crease_tex = NULL;
	}
	if (sculpt_smooth_tex) {
		delete sculpt_smooth_tex;
		sculpt_smooth_tex = NULL;
	}
	if (sculpt_flatten_tex) {
		delete sculpt_flatten_tex;
		sculpt_flatten_tex = NULL;
	}
	if (sculpt_fill_tex) {
		delete sculpt_fill_tex;
		sculpt_fill_tex = NULL;
	}
	if (sculpt_scrape_tex) {
		delete sculpt_scrape_tex;
		sculpt_scrape_tex = NULL;
	}
	if (sculpt_pinch_tex) {
		delete sculpt_pinch_tex;
		sculpt_pinch_tex = NULL;
	}
	if (sculpt_grab_tex) {
		delete sculpt_grab_tex;
		sculpt_grab_tex = NULL;
	}
	if (sculpt_snakehook_tex) {
		delete sculpt_snakehook_tex;
		sculpt_snakehook_tex = NULL;
	}
	if (sculpt_thumb_tex) {
		delete sculpt_thumb_tex;
		sculpt_thumb_tex = NULL;
	}
	if (sculpt_nudge_tex) {
		delete sculpt_nudge_tex;
		sculpt_nudge_tex = NULL;
	}
	if (sculpt_rotate_tex) {
		delete sculpt_rotate_tex;
		sculpt_rotate_tex = NULL;
	}
	if (sculpt_mask_tex) {
		delete sculpt_mask_tex;
		sculpt_mask_tex = NULL;
	}
	if (sculpt_simplify_tex) {
		delete sculpt_simplify_tex;
		sculpt_simplify_tex = NULL;
	}
	if (animation_tex) {
		delete animation_tex;
		animation_tex = NULL;
	}
	if (delete_tex) {
		delete delete_tex;
		delete_tex = NULL;
	}
	if (duplicate_tex) {
		delete duplicate_tex;
		duplicate_tex = NULL;
	}
	if (join_tex) {
		delete join_tex;
		join_tex = NULL;
	}
	if (separate_tex) {
		delete separate_tex;
		separate_tex = NULL;
	}
	if (undo_tex) {
		delete undo_tex;
		undo_tex = NULL;
	}
	if (redo_tex) {
		delete redo_tex;
		redo_tex = NULL;
	}
	if (manip_global_tex) {
		delete manip_global_tex;
		manip_global_tex = NULL;
	}
	if (manip_local_tex) {
		delete manip_local_tex;
		manip_local_tex = NULL;
	}
	if (manip_normal_tex) {
		delete manip_normal_tex;
		manip_normal_tex = NULL;
	}
	if (manip_plus_tex) {
		delete manip_plus_tex;
		manip_plus_tex = NULL;
	}
	if (manip_minus_tex) {
		delete manip_minus_tex;
		manip_minus_tex = NULL;
	}
	if (objectmode_tex) {
		delete objectmode_tex;
		objectmode_tex = NULL;
	}
	if (editmode_tex) {
		delete editmode_tex;
		editmode_tex = NULL;
	}
	if (object_tex) {
		delete object_tex;
		object_tex = NULL;
	}
	if (vertex_tex) {
		delete vertex_tex;
		vertex_tex = NULL;
	}
	if (edge_tex) {
		delete edge_tex;
		edge_tex = NULL;
	}
	if (face_tex) {
		delete face_tex;
		face_tex = NULL;
	}
	if (toolsettings_tex) {
		delete toolsettings_tex;
		toolsettings_tex = NULL;
	}
	if (box_empty_tex) {
		delete box_empty_tex;
		box_empty_tex = NULL;
	}
	if (box_filled_tex) {
		delete box_filled_tex;
		box_filled_tex = NULL;
	}
	if (plus_tex) {
		delete plus_tex;
		plus_tex = NULL;
	}
	if (minus_tex) {
		delete minus_tex;
		minus_tex = NULL;
	}
	if (reset_tex) {
		delete reset_tex;
		reset_tex = NULL;
	}

	if (background_menu_tex) {
		delete background_menu_tex;
		background_menu_tex = NULL;
	}
	if (colorwheel_menu_tex) {
		delete colorwheel_menu_tex;
		colorwheel_menu_tex = NULL;
	}

	if (ascii_tex) {
		delete ascii_tex;
		ascii_tex = NULL;
	}
	if (on_str_tex) {
		delete on_str_tex;
		on_str_tex = NULL;
	}
	if (off_str_tex) {
		delete off_str_tex;
		off_str_tex = NULL;
	}
	if (x_str_tex) {
		delete x_str_tex;
		x_str_tex = NULL;
	}
	if (y_str_tex) {
		delete y_str_tex;
		y_str_tex = NULL;
	}
	if (z_str_tex) {
		delete z_str_tex;
		z_str_tex = NULL;
	}
	if (xy_str_tex) {
		delete xy_str_tex;
		xy_str_tex = NULL;
	}
	if (yz_str_tex) {
		delete yz_str_tex;
		yz_str_tex = NULL;
	}
	if (zx_str_tex) {
		delete zx_str_tex;
		zx_str_tex = NULL;
	}
}

int VR_Draw::create_controller_models(VR_Device_Type type)
{
	/* Delete previous controller models (if any). */
	if (controller_tex) {
		delete controller_tex;
		controller_tex = NULL;
	}
	if (controller_model[VR_SIDE_LEFT]) {
		delete controller_model[VR_SIDE_LEFT];
		controller_model[VR_SIDE_LEFT] = NULL;
	}
	if (controller_model[VR_SIDE_RIGHT]) {
		delete controller_model[VR_SIDE_RIGHT];
		controller_model[VR_SIDE_RIGHT] = NULL;
	}

	/* Create new controller models based on UI type. */
	if (type == VR_DEVICE_TYPE_OCULUS) {
		controller_model[VR_SIDE_LEFT] = Model::create(ui_oculus_controller_left_obj_verts, ui_oculus_controller_left_obj_nrmls, ui_oculus_controller_left_obj_uvs, ui_oculus_controller_left_obj_numverts);
		controller_model[VR_SIDE_RIGHT] = Model::create(ui_oculus_controller_right_obj_verts, ui_oculus_controller_right_obj_nrmls, ui_oculus_controller_right_obj_uvs, ui_oculus_controller_right_obj_numverts);
		controller_tex = new Texture(ui_oculus_controller_png);
	}
	else if ((type == VR_DEVICE_TYPE_VIVE) || (type == VR_DEVICE_TYPE_PIMAX)) {
		controller_model[VR_SIDE_LEFT] = Model::create(ui_vive_controller_obj_verts, ui_vive_controller_obj_nrmls, ui_vive_controller_obj_uvs, ui_vive_controller_obj_numverts);
		controller_model[VR_SIDE_RIGHT] = Model::create(ui_vive_controller_obj_verts, ui_vive_controller_obj_nrmls, ui_vive_controller_obj_uvs, ui_vive_controller_obj_numverts);
		controller_tex = new Texture(ui_vive_controller_png);
	}
	else if (type == VR_DEVICE_TYPE_WINDOWSMR) {
		controller_model[VR_SIDE_LEFT] = Model::create(ui_windowsmr_controller_left_obj_verts, ui_windowsmr_controller_left_obj_nrmls, ui_windowsmr_controller_left_obj_uvs, ui_windowsmr_controller_left_obj_numverts);
		controller_model[VR_SIDE_RIGHT] = Model::create(ui_windowsmr_controller_right_obj_verts, ui_windowsmr_controller_right_obj_nrmls, ui_windowsmr_controller_right_obj_uvs, ui_windowsmr_controller_right_obj_numverts);
		controller_tex = new Texture(ui_windowsmr_controller_png);
	}
	else if (type == VR_DEVICE_TYPE_INDEX) {
		controller_model[VR_SIDE_LEFT] = Model::create(ui_index_controller_left_obj_verts, ui_index_controller_left_obj_nrmls, ui_index_controller_left_obj_uvs, ui_index_controller_left_obj_numverts);
		controller_model[VR_SIDE_RIGHT] = Model::create(ui_index_controller_right_obj_verts, ui_index_controller_right_obj_nrmls, ui_index_controller_right_obj_uvs, ui_index_controller_right_obj_numverts);
		controller_tex = new Texture(ui_index_controller_png);
	}
	else if (type == VR_DEVICE_TYPE_MAGICLEAP) {
		controller_model[VR_SIDE_LEFT] = Model::create(ui_magicleap_controller_obj_verts, ui_magicleap_controller_obj_nrmls, ui_magicleap_controller_obj_uvs, ui_magicleap_controller_obj_numverts);
		controller_model[VR_SIDE_RIGHT] = Model::create(ui_magicleap_controller_obj_verts, ui_magicleap_controller_obj_nrmls, ui_magicleap_controller_obj_uvs, ui_magicleap_controller_obj_numverts);
		/* TODO_XR: Fix the texture... */
		controller_tex = new Texture(ui_magicleap_controller_png);
	}
	else {
		/* Fove or unsupported type */
		return -1;
	}

	if (controller_model[VR_SIDE_LEFT] && controller_model[VR_SIDE_RIGHT]) {
		controller_model[VR_SIDE_LEFT]->texture = controller_tex;
		controller_model[VR_SIDE_RIGHT]->texture = controller_tex;
	}
	else {
		/* Allocation failure? */
		return -1;
	}

	return 0;
}

const Mat44f& VR_Draw::get_model_matrix()
{
	return VR_Draw::model_matrix;
}

const Mat44f& VR_Draw::get_view_matrix()
{
	return VR_Draw::view_matrix;
}

const Mat44f& VR_Draw::get_projection_matrix()
{
	return VR_Draw::projection_matrix;
}

const float* VR_Draw::get_color()
{
	return VR_Draw::color_vector;
}

void VR_Draw::update_model_matrix(const float _model[4][4])
{
	std::memcpy(model_matrix.m, _model, sizeof(float) * 4 * 4);
}

void VR_Draw::update_view_matrix(const float _view[4][4])
{
	std::memcpy(view_matrix.m, _view, sizeof(float) * 4 * 4);
}

void VR_Draw::update_projection_matrix(const float _projection[4][4])
{
	projection_matrix = _projection;
}

void VR_Draw::update_modelview_matrix(const Mat44f* _model, const Mat44f* _view)
{
	if (_model && _view) {
		VR_Draw::modelview_matrix = (*_model) * (*_view);
		//VR_Draw::model_matrix = *_model;
		//VR_Draw::view_matrix = *_view;
	}
	else if (_model) {
		VR_Draw::modelview_matrix = (*_model) * VR_Draw::view_matrix;
		//VR_Draw::model_matrix = *_model;
	}
	else if (_view) {
		VR_Draw::modelview_matrix = VR_Draw::model_matrix * (*_view);
		//VR_Draw::view_matrix = *_view;
	}
	else {
		return;
	}
	VR_Draw::modelview_matrix_inv = VR_Draw::modelview_matrix.inverse();
}

void VR_Draw::set_color(const float color[4])
{
	memcpy(VR_Draw::color_vector, color, sizeof(float) * 4);
}

void VR_Draw::set_color(const float& r, const float& g, const float& b, const float& a)
{
	VR_Draw::color_vector[0] = r;
	VR_Draw::color_vector[1] = g;
	VR_Draw::color_vector[2] = b;
	VR_Draw::color_vector[3] = a;
}

void VR_Draw::set_blend(bool on_off)
{
	if (on_off) {
		glEnable(GL_BLEND);
	}
	else {
		glDisable(GL_BLEND);
	}
}

void VR_Draw::set_depth_test(bool on_off, bool write_depth)
{
	if (on_off) { /* testing depth */
		if (write_depth) { /* testing depth and writing to depth buffer */
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS); /* default value */
			glDepthMask(GL_TRUE); /* If flag is GL_FALSE, depth buffer writing is disabled */
		}
		else { /* testing depth but NOT writing to depth buffer */
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS); /* default value */
			glDepthMask(GL_FALSE); /* If flag is GL_FALSE, depth buffer writing is disabled */
		}
	}
	else { /* not testing depth (always pass) */
		if (write_depth) { /* not testing depth (always pass) but still writing the new depth */
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);
			glDepthMask(GL_TRUE); /* If flag is GL_FALSE, depth buffer writing is disabled */
		}
		else { /* not testing depth (always pass), but not updating the depth buffer either */
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);
			glDepthMask(GL_FALSE); /* If flag is GL_FALSE, depth buffer writing is disabled */
		}
	}
}

static void Util_Image_decodePNGRGBA_readData(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
	uchar** b = (uchar**)png_get_io_ptr(png_ptr);
	std::memcpy(data, *b, length);
	*b += length;
}

static bool decodePNGRGBA(const uchar* png_data, uchar*& img, uint& w, uint& h)
{
	png_structp png_ptr;
	png_infop info_ptr;
	uint sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, 0, 0);
		return false;
	}

	png_set_read_fn(png_ptr, &png_data, (png_rw_ptr)Util_Image_decodePNGRGBA_readData);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

	w = width;
	h = height;

	img = (uchar*)malloc(sizeof(uchar)*w*h * 4);

	png_set_sig_bytes(png_ptr, sig_read);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

	for (uint y = 0; y < height; y++) {
		png_read_row(png_ptr, img + (y*w * 4), 0);
	}

	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	return true;
}

static bool createTextureFromPNG(const uchar* data, uint& texture_id, uint& w, uint& h)
{
	if (!data)
		return false;

	uchar* imgbuf;
	if (!decodePNGRGBA(data, imgbuf, w, h)) {
		return false;
	}

	/* Now turn it into an OpenGL texture. */
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)imgbuf);

	glBindTexture(GL_TEXTURE_2D, 0);

	free(imgbuf);

	return true;
}

/***************************************************************************************************
 * \class                                  VR_Draw::Shader
 ***************************************************************************************************
 * OpenGL shader implementation.
 **************************************************************************************************/
VR_Draw::Shader::Shader()
	: program(0), vertex_shader(0), fragment_shader(0),
	position_location(0), normal_location(0), uv_location(0), modelview_location(0),
	projection_location(0), normal_matrix_location(0), color_location(0), sampler_location(0)
{
	//
}

VR_Draw::Shader::~Shader()
{
	this->release();
}

int VR_Draw::Shader::create(const char* vss, const char* fss, bool tex)
{
	this->release();

	this->program = glCreateProgram();
	this->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	this->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertex_shader, 1, &vss, 0);
	glShaderSource(fragment_shader, 1, &fss, 0);

	GLint ret;

	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		return -1;
	}
	glAttachShader(program, vertex_shader);

	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		return -1;
	}
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &ret);
	if (!ret) {
		return -1;
	}

	this->position_location = glGetAttribLocation(program, "position");
	if (tex) {
		this->normal_location = glGetAttribLocation(program, "normal");
		this->uv_location = glGetAttribLocation(program, "uv");
		this->sampler_location = glGetUniformLocation(program, "tex");
		glUniform1i(this->sampler_location, 0);
		this->normal_matrix_location = glGetUniformLocation(program, "normal_matrix");
	}
	this->modelview_location = glGetUniformLocation(program, "modelview");
	this->projection_location = glGetUniformLocation(program, "projection");
	this->color_location = glGetUniformLocation(program, "color");

	return 0;
}

int VR_Draw::Shader::release()
{
	if (this->vertex_shader) {
		glDeleteShader(this->vertex_shader);
		this->vertex_shader = 0;
	}
	if (this->fragment_shader) {
		glDeleteShader(this->fragment_shader);
		this->fragment_shader = 0;
	}
	if (this->program) {
		glDeleteProgram(this->program);
		this->program = 0;
	}
	return 0;
}

GLuint VR_Draw::Shader::color_shader()
{
	if (!shader_col.program) {
		shader_col.create(shader_col_vsource, shader_col_fsource, false);
	}
	return shader_col.program;
}

GLuint VR_Draw::Shader::texture_shader()
{
	if (!shader_tex.program) {
		shader_tex.create(shader_tex_vsource, shader_tex_fsource, true);
	}
	return shader_tex.program;
}

VR_Draw::Shader VR_Draw::Shader::shader_col;

const char* const VR_Draw::Shader::shader_col_vsource(STRING(#version 120\n
	attribute vec3 position;
	varying vec4 front_color;
	uniform mat4 modelview;
	uniform mat4 projection;
void main()
{
	gl_Position = projection * modelview * vec4(position, 1.0);
}
));

const char* const VR_Draw::Shader::shader_col_fsource(STRING(#version 120\n
	uniform vec4 color;
void main()
{
	gl_FragColor = color;
}
));

VR_Draw::Shader VR_Draw::Shader::shader_tex;

const char* const VR_Draw::Shader::shader_tex_vsource(STRING(#version 120\n
	attribute vec3 position;
	attribute vec3 normal;
	attribute vec2 uv;
	varying vec3 normal_transformed;
	varying vec2 texcoord;
	uniform mat4 modelview;
	uniform mat4 projection;
	uniform mat4 normal_matrix; /* normal_matrix = transpose(inverse(modelview)) */
void main()
{
	gl_Position = projection * modelview * vec4(position, 1.0);
	texcoord = uv;
	normal_transformed = normalize(normal_matrix * vec4(normal, 0.0)).xyz; /* transformed to eye-space */
}
));

const char* const VR_Draw::Shader::shader_tex_fsource(STRING(#version 120\n
	varying vec3 normal_transformed;
	varying vec2 texcoord;
	uniform sampler2D tex;
	uniform vec4 color;
void main()
{
	vec4 normal_to_viewangle = vec4(clamp(dot(normal_transformed, vec3(0, 0, 1)), 0.1, 1.0));
	normal_to_viewangle.a = 1.0;
	vec4 texture = texture2D(tex, texcoord);
	gl_FragColor = (texture * color) * normal_to_viewangle;
}
));

VR_Draw::Texture::Texture(const uchar* png_blob)
	: png_blob(png_blob)
	, implementation(0)
{
	//
}

VR_Draw::Texture::Texture(const Texture& cpy)
{
	this->png_blob = cpy.png_blob;
	this->implementation = cpy.implementation;
}

VR_Draw::Texture::~Texture()
{
	if (this->implementation) {
		delete this->implementation;
	}
}

VR_Draw::Texture& VR_Draw::Texture::operator= (const Texture& o)
{
	this->png_blob = o.png_blob;
	this->implementation = o.implementation;

	return *this;
}

void VR_Draw::Texture::bind()
{
	if (!this->implementation) {
		if (!this->create_implementation()) {
			return;
		}
	}

	this->implementation->bind();
}

void VR_Draw::Texture::unbind()
{
	if (this->implementation) {
		this->implementation->unbind();
	}
}

uint VR_Draw::Texture::width()
{
	if (!this->implementation) {
		this->create_implementation();
	}
	return this->implementation->width;
}

uint VR_Draw::Texture::height()
{
	if (!this->implementation) {
		this->create_implementation();
	}
	return this->implementation->height;
}

bool VR_Draw::Texture::create_implementation()
{
	/* Generate from PNG blob. */
	if (this->png_blob) {
		this->implementation = new VR_Draw::Texture::TextureImplementation(this->png_blob);
		return true;
	}

	return true;
}

VR_Draw::Texture::TextureImplementation::TextureImplementation(const uchar* png_blob)
{
	/* Save previous OpenGL state. */
	GLboolean texture_enabled = glIsEnabled(GL_TEXTURE_2D);
	if (!texture_enabled)
		glEnable(GL_TEXTURE_2D);
	GLint bound_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound_texture);

	createTextureFromPNG(png_blob, this->texture_id, this->width, this->height);
	this->depth = 4;

	/* Revert to previous OpenGL state. */
	if (!texture_enabled)
		glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, bound_texture);
}

VR_Draw::Texture::TextureImplementation::~TextureImplementation()
{
	if (this->texture_id) {
		glDeleteTextures(1, &this->texture_id);
	}
}

void VR_Draw::Texture::TextureImplementation::bind()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, this->texture_id);
}

void VR_Draw::Texture::TextureImplementation::unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

VR_Draw::Model::Model()
	: verts(0)
	, nrmls(0)
	, uvs(0)
	, vertex_array(0)
	, num_verts(0)
{
	this->texture = 0;
}

VR_Draw::Model::~Model()
{
	/* Release the model (if any). */
	if (this->vertex_array) {
		glDeleteVertexArrays(1, &this->vertex_array);
	}
	if (this->verts) {
		glDeleteBuffers(1, &this->verts);
	}
	if (this->nrmls) {
		glDeleteBuffers(1, &this->nrmls);
	}
	if (this->uvs) {
		glDeleteBuffers(1, &this->uvs);
	}
}

VR_Draw::Model* VR_Draw::Model::create(float* verts, float* nrmls, float* uvs, uint num_verts)
{
	/* Try to load buffers into OpenGL. */
	Model* m = new Model();

	m->num_verts = num_verts;

	GLint prior_vertex_array_binding;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prior_vertex_array_binding);
	GLint prior_array_buffer;
	glGetIntegerv(GL_ARRAY_BUFFER, &prior_array_buffer);

	glGenBuffers(1, &m->verts);
	glBindBuffer(GL_ARRAY_BUFFER, m->verts);
	glBufferData(GL_ARRAY_BUFFER, 3 * num_verts * sizeof(float), verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m->nrmls);
	glBindBuffer(GL_ARRAY_BUFFER, m->nrmls);
	glBufferData(GL_ARRAY_BUFFER, 3 * num_verts * sizeof(float), nrmls, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m->uvs);
	glBindBuffer(GL_ARRAY_BUFFER, m->uvs);
	glBufferData(GL_ARRAY_BUFFER, 2 * num_verts * sizeof(float), uvs, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Create vertex array */
	glGenVertexArrays(1, &m->vertex_array);

	/* Set up attribute buffers */
	glBindVertexArray(m->vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, m->verts);
	glVertexAttribPointer(VR_Draw::Shader::shader_tex.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, m->nrmls);
	glVertexAttribPointer(VR_Draw::Shader::shader_tex.normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, m->uvs);
	glVertexAttribPointer(VR_Draw::Shader::shader_tex.uv_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, prior_array_buffer);
	glBindVertexArray(prior_vertex_array_binding);

	return m;
}

int VR_Draw::Model::render()
{
	/* Save previous OpenGL state */
	GLint prior_program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prior_program);
	GLboolean prior_backface_culling = glIsEnabled(GL_CULL_FACE);
	GLboolean prior_blend_enabled = glIsEnabled(GL_BLEND);
	GLboolean prior_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean prior_texture_enabled = glIsEnabled(GL_TEXTURE_2D);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

	glUseProgram(VR_Draw::Shader::texture_shader());
	//GLint prior_vertex_array_binding;
	//glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prior_vertex_array_binding);
	GLint prior_array_buffer;
	glGetIntegerv(GL_ARRAY_BUFFER, &prior_array_buffer);
	GLint prior_texture_binding_2d;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prior_texture_binding_2d);
	GLint prior_texture_unit;
	glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&prior_texture_unit);

	glActiveTexture(GL_TEXTURE0);

	if (this->texture) {
		this->texture->bind();
	}

	/* Load uniforms */
	glUniformMatrix4fv(VR_Draw::Shader::shader_tex.modelview_location, 1, false, (float*)VR_Draw::modelview_matrix.m);
	glUniformMatrix4fv(VR_Draw::Shader::shader_tex.projection_location, 1, false, (float*)VR_Draw::projection_matrix.m);
	glUniformMatrix4fv(VR_Draw::Shader::shader_tex.normal_matrix_location, 1, true, (float*)VR_Draw::modelview_matrix_inv.m);
	glUniform4fv(VR_Draw::Shader::shader_tex.color_location, 1, (float*)VR_Draw::color_vector);

	/* Load attribute buffers */
	//glBindVertexArray(this->vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, this->verts);
	glVertexAttribPointer(VR_Draw::Shader::shader_tex.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(VR_Draw::Shader::shader_tex.position_location);

	glBindBuffer(GL_ARRAY_BUFFER, this->nrmls);
	glVertexAttribPointer(VR_Draw::Shader::shader_tex.normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(VR_Draw::Shader::shader_tex.normal_location);

	glBindBuffer(GL_ARRAY_BUFFER, this->uvs);
	glVertexAttribPointer(VR_Draw::Shader::shader_tex.uv_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
	glEnableVertexAttribArray(VR_Draw::Shader::shader_tex.uv_location);

	glDrawArrays(GL_TRIANGLES, 0, this->num_verts);

	glDisableVertexAttribArray(VR_Draw::Shader::shader_tex.position_location);
	glDisableVertexAttribArray(VR_Draw::Shader::shader_tex.normal_location);
	glDisableVertexAttribArray(VR_Draw::Shader::shader_tex.uv_location);

	/* Restore previous OpenGL state */
	//glBindVertexArray(prior_vertex_array_binding);
	glBindBuffer(GL_ARRAY_BUFFER, prior_array_buffer);
	glBindTexture(GL_TEXTURE_2D, prior_texture_binding_2d);
	glActiveTexture(prior_texture_unit);

	glUseProgram(prior_program);
	prior_backface_culling ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	prior_blend_enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	prior_depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	prior_texture_enabled ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);

	return 0;
}

int VR_Draw::Model::render(const Mat44f& pos)
{
	VR_Draw::update_modelview_matrix(&pos, 0);
	int e = this->render();
	return e;
}

void VR_Draw::render_rect(float left, float right, float top, float bottom, float z, float u, float v, Texture *tex)
{
	/* Save previous OpenGL state */
	GLint prior_program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prior_program);
	GLboolean prior_backface_culling = glIsEnabled(GL_CULL_FACE);
	GLboolean prior_blend_enabled = glIsEnabled(GL_BLEND);
	GLboolean prior_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean prior_texture_enabled = glIsEnabled(GL_TEXTURE_2D);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (tex) {
		glUseProgram(VR_Draw::Shader::texture_shader());
	}
	else {
		glUseProgram(VR_Draw::Shader::color_shader());
	}
	GLint prior_vertex_array_binding;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prior_vertex_array_binding);
	GLint prior_array_buffer;
	glGetIntegerv(GL_ARRAY_BUFFER, &prior_array_buffer);
	GLint prior_texture_binding_2d;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prior_texture_binding_2d);
	GLint prior_texture_unit;
	glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&prior_texture_unit);

	if (tex) {
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		tex->bind();
	}

	/* Load uniforms */
	if (tex) {
		glUniformMatrix4fv(VR_Draw::Shader::shader_tex.modelview_location, 1, false, (float*)VR_Draw::modelview_matrix.m);
		glUniformMatrix4fv(VR_Draw::Shader::shader_tex.projection_location, 1, false, (float*)VR_Draw::projection_matrix.m);
		glUniformMatrix4fv(VR_Draw::Shader::shader_tex.normal_matrix_location, 1, true, (float*)VR_Draw::modelview_matrix_inv.m);
		glUniform4fv(VR_Draw::Shader::shader_tex.color_location, 1, (float*)VR_Draw::color_vector);
	}
	else {
		glUniformMatrix4fv(VR_Draw::Shader::shader_col.modelview_location, 1, false, (float*)VR_Draw::modelview_matrix.m);
		glUniformMatrix4fv(VR_Draw::Shader::shader_col.projection_location, 1, false, (float*)VR_Draw::projection_matrix.m);
		glUniform4fv(VR_Draw::Shader::shader_col.color_location, 1, (float*)VR_Draw::color_vector);
	}

	/* Create vertex array */
	static GLuint vertex_array;
	if (!vertex_array) {
		glGenVertexArrays(1, &vertex_array);
	}
	glBindVertexArray(vertex_array);

	/* Create vertex buffer */
	static GLfloat vertex_data[4][3];
	vertex_data[0][0] = left;  vertex_data[0][1] = bottom; vertex_data[0][2] = z;
	vertex_data[1][0] = right; vertex_data[1][1] = bottom; vertex_data[1][2] = z;
	vertex_data[2][0] = left;  vertex_data[2][1] = top;    vertex_data[2][2] = z;
	vertex_data[3][0] = right; vertex_data[3][1] = top;    vertex_data[3][2] = z;

	static GLuint verts = 0;
	if (!verts) {
		glGenBuffers(1, &verts);
	}
	glBindBuffer(GL_ARRAY_BUFFER, verts);
	glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), vertex_data, GL_STATIC_DRAW);
	if (tex) {
		glVertexAttribPointer(VR_Draw::Shader::shader_tex.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(VR_Draw::Shader::shader_tex.position_location);
	}
	else {
		glVertexAttribPointer(VR_Draw::Shader::shader_col.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(VR_Draw::Shader::shader_col.position_location);
	}

	static GLuint nrmls = 0;
	static GLuint uvs = 0;
	if (tex) {
		/* Create normal buffer */
		static const GLfloat normal_data[] = {
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f
		};
		if (!nrmls) {
			glGenBuffers(1, &nrmls);
			glBindBuffer(GL_ARRAY_BUFFER, nrmls);
			glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), normal_data, GL_STATIC_DRAW);
		}
		else {
			glBindBuffer(GL_ARRAY_BUFFER, nrmls);
		}
		glVertexAttribPointer(VR_Draw::Shader::shader_tex.normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(VR_Draw::Shader::shader_tex.normal_location);

		/* Create uv buffer */
		static GLfloat uv_data[4][2];
		uv_data[0][0] = 0.0f; uv_data[0][1] = v;
		uv_data[1][0] = u;    uv_data[1][1] = v;
		uv_data[2][0] = 0.0f; uv_data[2][1] = 0.0f;
		uv_data[3][0] = u;    uv_data[3][1] = 0.0f;

		if (!uvs) {
			glGenBuffers(1, &uvs);
		}
		glBindBuffer(GL_ARRAY_BUFFER, uvs);
		glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(float), uv_data, GL_STATIC_DRAW);
		glVertexAttribPointer(VR_Draw::Shader::shader_tex.uv_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
		glEnableVertexAttribArray(VR_Draw::Shader::shader_tex.uv_location);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (tex) {
		glDisableVertexAttribArray(VR_Draw::Shader::shader_tex.position_location);
		glDisableVertexAttribArray(VR_Draw::Shader::shader_tex.normal_location);
		glDisableVertexAttribArray(VR_Draw::Shader::shader_tex.uv_location);
	}
	else {
		glDisableVertexAttribArray(VR_Draw::Shader::shader_col.position_location);
	}

	/* Restore previous OpenGL state */
	glBindVertexArray(prior_vertex_array_binding);
	glBindBuffer(GL_ARRAY_BUFFER, prior_array_buffer);
	glBindTexture(GL_TEXTURE_2D, prior_texture_binding_2d);
	glActiveTexture(prior_texture_unit);

	glUseProgram(prior_program);
	prior_backface_culling ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	prior_blend_enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	prior_depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	prior_texture_enabled ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
}

void VR_Draw::render_frame(float left, float right, float top, float bottom, float b, float z)
{
	/* Save previous OpenGL state */
	GLint prior_program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prior_program);
	GLboolean prior_backface_culling = glIsEnabled(GL_CULL_FACE);
	GLboolean prior_blend_enabled = glIsEnabled(GL_BLEND);
	GLboolean prior_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean prior_texture_enabled = glIsEnabled(GL_TEXTURE_2D);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(VR_Draw::Shader::color_shader());

	GLint prior_vertex_array_binding;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prior_vertex_array_binding);
	GLint prior_array_buffer;
	glGetIntegerv(GL_ARRAY_BUFFER, &prior_array_buffer);

	/* Load uniforms */
	glUniformMatrix4fv(VR_Draw::Shader::shader_col.modelview_location, 1, false, (float*)VR_Draw::modelview_matrix.m);
	glUniformMatrix4fv(VR_Draw::Shader::shader_col.projection_location, 1, false, (float*)VR_Draw::projection_matrix.m);
	glUniform4fv(VR_Draw::Shader::shader_col.color_location, 1, (float*)VR_Draw::color_vector);

	/* Create vertex array */
	static GLuint vertex_array;
	if (!vertex_array) {
		glGenVertexArrays(1, &vertex_array);
	}
	glBindVertexArray(vertex_array);

	/* Create vertex buffer */
	static GLfloat vertex_data[10][3];
	vertex_data[0][0] = left - b;	vertex_data[0][1] = top + b;    vertex_data[0][2] = z;
	vertex_data[1][0] = left;	    vertex_data[1][1] = top;		vertex_data[1][2] = z;
	vertex_data[2][0] = right + b;	vertex_data[2][1] = top + b;	vertex_data[2][2] = z;
	vertex_data[2][0] = right + b;	vertex_data[2][1] = top + b;	vertex_data[2][2] = z;
	vertex_data[3][0] = right;		vertex_data[3][1] = top;        vertex_data[3][2] = z;
	vertex_data[4][0] = right + b;	vertex_data[4][1] = bottom - b;	vertex_data[4][2] = z;
	vertex_data[5][0] = right;		vertex_data[5][1] = bottom;		vertex_data[5][2] = z;
	vertex_data[6][0] = left - b;	vertex_data[6][1] = bottom - b; vertex_data[6][2] = z;
	vertex_data[7][0] = left;		vertex_data[7][1] = bottom;     vertex_data[7][2] = z;
	vertex_data[8][0] = left - b;	vertex_data[8][1] = top + b;    vertex_data[8][2] = z;
	vertex_data[9][0] = left;	    vertex_data[9][1] = top;		vertex_data[9][2] = z;

	static GLuint verts = 0;
	if (!verts) {
		glGenBuffers(1, &verts);
	}
	glBindBuffer(GL_ARRAY_BUFFER, verts);
	glBufferData(GL_ARRAY_BUFFER, 10 * 3 * sizeof(float), vertex_data, GL_STATIC_DRAW);
	glVertexAttribPointer(VR_Draw::Shader::shader_col.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(VR_Draw::Shader::shader_col.position_location);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);

	glDisableVertexAttribArray(VR_Draw::Shader::shader_col.position_location);

	/* Restore previous OpenGL state */
	glBindVertexArray(prior_vertex_array_binding);
	glBindBuffer(GL_ARRAY_BUFFER, prior_array_buffer);

	glUseProgram(prior_program);
	prior_backface_culling ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	prior_blend_enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	prior_depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	prior_texture_enabled ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
}

void VR_Draw::render_box(const Coord3Df& p0, const Coord3Df& p1, bool outline)
{
	/* Save previous OpenGL state */
	GLint prior_program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prior_program);
	GLboolean prior_backface_culling = glIsEnabled(GL_CULL_FACE);
	GLboolean prior_blend_enabled = glIsEnabled(GL_BLEND);
	GLboolean prior_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean prior_texture_enabled = glIsEnabled(GL_TEXTURE_2D);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	glUseProgram(VR_Draw::Shader::color_shader());

	GLint prior_vertex_array_binding;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prior_vertex_array_binding);
	GLint prior_array_buffer;
	glGetIntegerv(GL_ARRAY_BUFFER, &prior_array_buffer);

	/* Load uniforms */
	glUniformMatrix4fv(VR_Draw::Shader::shader_col.modelview_location, 1, false, (float*)VR_Draw::modelview_matrix.m);
	glUniformMatrix4fv(VR_Draw::Shader::shader_col.projection_location, 1, false, (float*)VR_Draw::projection_matrix.m);
	glUniform4fv(VR_Draw::Shader::shader_col.color_location, 1, (float*)VR_Draw::color_vector);

	/* Create vertex array */
	static GLuint vertex_array;
	if (!vertex_array) {
		glGenVertexArrays(1, &vertex_array);
	}
	glBindVertexArray(vertex_array);

	/* Create vertex buffer */
	static GLfloat vertex_data[14][3];
	vertex_data[0][0] = p0.x;		vertex_data[0][1] = p0.y;		vertex_data[0][2] = p0.z;
	vertex_data[1][0] = p0.x;		vertex_data[1][1] = p0.y;		vertex_data[1][2] = p1.z;
	vertex_data[2][0] = p0.x;		vertex_data[2][1] = p1.y;		vertex_data[2][2] = p0.z;
	vertex_data[3][0] = p0.x;		vertex_data[3][1] = p1.y;		vertex_data[3][2] = p1.z;
	vertex_data[4][0] = p1.x;		vertex_data[4][1] = p1.y;		vertex_data[4][2] = p1.z;
	vertex_data[5][0] = p0.x;		vertex_data[5][1] = p0.y;		vertex_data[5][2] = p1.z;
	vertex_data[6][0] = p1.x;		vertex_data[6][1] = p0.y;		vertex_data[6][2] = p1.z;
	vertex_data[7][0] = p0.x;		vertex_data[7][1] = p0.y;		vertex_data[7][2] = p0.z;
	vertex_data[8][0] = p1.x;		vertex_data[8][1] = p0.y;		vertex_data[8][2] = p0.z;
	vertex_data[9][0] = p0.x;		vertex_data[9][1] = p1.y;		vertex_data[9][2] = p0.z;
	vertex_data[10][0] = p1.x;		vertex_data[10][1] = p1.y;		vertex_data[10][2] = p0.z;
	vertex_data[11][0] = p1.x;		vertex_data[11][1] = p1.y;		vertex_data[11][2] = p1.z;
	vertex_data[12][0] = p1.x;		vertex_data[12][1] = p0.y;		vertex_data[12][2] = p0.z;
	vertex_data[13][0] = p1.x;		vertex_data[13][1] = p0.y;		vertex_data[13][2] = p1.z;

	static GLuint verts = 0;
	if (!verts) {
		glGenBuffers(1, &verts);
	}
	glBindBuffer(GL_ARRAY_BUFFER, verts);
	glBufferData(GL_ARRAY_BUFFER, 14 * 3 * sizeof(float), vertex_data, GL_STATIC_DRAW);
	glVertexAttribPointer(VR_Draw::Shader::shader_col.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(VR_Draw::Shader::shader_col.position_location);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);

	/* Draw the cube outline */
	if (outline) {
		GLfloat line_width;
		glGetFloatv(GL_LINE_WIDTH, &line_width);
		glLineWidth(2.0f);

		VR_Draw::set_color(0.0f, 0.0f, 0.0f, 0.4f);
		static GLfloat line_vertex_data[16][3];
		line_vertex_data[0][0] = p0.x;	line_vertex_data[0][1] = p0.y;		line_vertex_data[0][2] = p0.z;
		line_vertex_data[1][0] = p0.x;	line_vertex_data[1][1] = p1.y;		line_vertex_data[1][2] = p0.z;
		line_vertex_data[2][0] = p1.x;	line_vertex_data[2][1] = p1.y;		line_vertex_data[2][2] = p0.z;
		line_vertex_data[3][0] = p1.x;	line_vertex_data[3][1] = p0.y;		line_vertex_data[3][2] = p0.z;
		line_vertex_data[4][0] = p0.x;	line_vertex_data[4][1] = p0.y;		line_vertex_data[4][2] = p0.z;
		line_vertex_data[5][0] = p0.x;	line_vertex_data[5][1] = p0.y;		line_vertex_data[5][2] = p1.z;
		line_vertex_data[6][0] = p0.x;	line_vertex_data[6][1] = p1.y;		line_vertex_data[6][2] = p1.z;
		line_vertex_data[7][0] = p0.x;	line_vertex_data[7][1] = p1.y;		line_vertex_data[7][2] = p0.z;
		line_vertex_data[8][0] = p0.x;	line_vertex_data[8][1] = p1.y;		line_vertex_data[8][2] = p1.z;
		line_vertex_data[9][0] = p1.x;	line_vertex_data[9][1] = p1.y;		line_vertex_data[9][2] = p1.z;
		line_vertex_data[10][0] = p1.x;	line_vertex_data[10][1] = p1.y;		line_vertex_data[10][2] = p0.z;
		line_vertex_data[11][0] = p1.x;	line_vertex_data[11][1] = p1.y;		line_vertex_data[11][2] = p1.z;
		line_vertex_data[12][0] = p1.x;	line_vertex_data[12][1] = p0.y;		line_vertex_data[12][2] = p1.z;
		line_vertex_data[13][0] = p1.x;	line_vertex_data[13][1] = p0.y;		line_vertex_data[13][2] = p0.z;
		line_vertex_data[14][0] = p1.x;	line_vertex_data[14][1] = p0.y;		line_vertex_data[14][2] = p1.z;
		line_vertex_data[15][0] = p0.x;	line_vertex_data[15][1] = p0.y;		line_vertex_data[15][2] = p1.z;

		static GLuint line_verts = 0;
		if (!line_verts) {
			glGenBuffers(1, &line_verts);
		}
		glBindBuffer(GL_ARRAY_BUFFER, line_verts);
		glBufferData(GL_ARRAY_BUFFER, 16 * 3 * sizeof(float), line_vertex_data, GL_STATIC_DRAW);
		glVertexAttribPointer(VR_Draw::Shader::shader_col.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

		glDrawArrays(GL_LINE_STRIP, 0, 16);

		VR_Draw::set_color(1.0f, 1.0f, 1.0f, 0.7f);
		glLineStipple(1, 0xF0F0);
		glEnable(GL_LINE_STIPPLE);

		glDrawArrays(GL_LINE_STRIP, 0, 16);

		glDisable(GL_LINE_STIPPLE);
		if (line_width != 2.0f)
			glLineWidth(line_width);
	}

	glDisableVertexAttribArray(VR_Draw::Shader::shader_col.position_location);

	/* Restore previous OpenGL state */
	glBindVertexArray(prior_vertex_array_binding);
	glBindBuffer(GL_ARRAY_BUFFER, prior_array_buffer);

	glUseProgram(prior_program);
	prior_backface_culling ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	prior_blend_enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	prior_depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	prior_texture_enabled ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
}

void VR_Draw::render_ball(float r, bool golf)
{
	/* Save previous OpenGL state */
	GLint prior_program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prior_program);
	GLboolean prior_backface_culling = glIsEnabled(GL_CULL_FACE);
	GLboolean prior_blend_enabled = glIsEnabled(GL_BLEND);
	GLboolean prior_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean prior_texture_enabled = glIsEnabled(GL_TEXTURE_2D);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	glUseProgram(VR_Draw::Shader::color_shader());

	GLint prior_vertex_array_binding;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prior_vertex_array_binding);
	GLint prior_array_buffer;
	glGetIntegerv(GL_ARRAY_BUFFER, &prior_array_buffer);

	/* Load uniforms */
	glUniformMatrix4fv(VR_Draw::Shader::shader_col.modelview_location, 1, false, (float*)VR_Draw::modelview_matrix.m);
	glUniformMatrix4fv(VR_Draw::Shader::shader_col.projection_location, 1, false, (float*)VR_Draw::projection_matrix.m);
	glUniform4fv(VR_Draw::Shader::shader_col.color_location, 1, (float*)VR_Draw::color_vector);

	/* Create vertex array */
	static GLuint vertex_array;
	if (!vertex_array) {
		glGenVertexArrays(1, &vertex_array);
	}
	glBindVertexArray(vertex_array);

	/* Create vertex buffer */
	const int res = 16; /* Sphere resolution, hardcoded for now. */
	const int num_verts = res * res * 6;
	static GLfloat vertex_data[num_verts][3];

	float n1, n2, n3, n4, x1, y1, x2, y2, z1, z2, r1, r2;
	int i = 0;
	for (int x = 0; x < res; ++x) {
		for (int y = -res / 2; y < res / 2; ++y) {
			n1 = (x / (float)res) * 2 * PI;
			n2 = ((x + 1) / (float)res) * 2 * PI;
			n3 = (y / (float)res) * PI;
			n4 = ((y + 1) / (float)res) * PI;

			x1 = sin(n1);
			y1 = cos(n1);
			x2 = sin(n2);
			y2 = cos(n2);
			z1 = r * sin(n3);
			z2 = r * sin(n4);

			r1 = r * cos(n3);
			r2 = r * cos(n4);

			vertex_data[i][0] = r1 * x1; vertex_data[i][1] = r1 * y1; vertex_data[i][2] = z1; ++i;
			vertex_data[i][0] = r1 * x2; vertex_data[i][1] = r1 * y2; vertex_data[i][2] = z1; ++i;
			vertex_data[i][0] = r2 * x2; vertex_data[i][1] = r2 * y2; vertex_data[i][2] = z2; ++i;

			vertex_data[i][0] = r1 * x1; vertex_data[i][1] = r1 * y1; vertex_data[i][2] = z1; ++i;
			vertex_data[i][0] = r2 * x2; vertex_data[i][1] = r2 * y2; vertex_data[i][2] = z2; ++i;
			vertex_data[i][0] = r2 * x1; vertex_data[i][1] = r2 * y1; vertex_data[i][2] = z2; ++i;
		}
	}

	static GLuint verts = 0;
	if (!verts) {
		glGenBuffers(1, &verts);
	}
	glBindBuffer(GL_ARRAY_BUFFER, verts);
	glBufferData(GL_ARRAY_BUFFER, num_verts * 3 * sizeof(float), vertex_data, GL_STATIC_DRAW);
	glVertexAttribPointer(VR_Draw::Shader::shader_col.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(VR_Draw::Shader::shader_col.position_location);

	if (golf) {
		glDrawArrays(GL_TRIANGLE_STRIP, 0, num_verts);
	}
	else {
		glDrawArrays(GL_TRIANGLES, 0, num_verts);
	}

	glDisableVertexAttribArray(VR_Draw::Shader::shader_col.position_location);

	/* Restore previous OpenGL state */
	glBindVertexArray(prior_vertex_array_binding);
	glBindBuffer(GL_ARRAY_BUFFER, prior_array_buffer);

	glUseProgram(prior_program);
	prior_backface_culling ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	prior_blend_enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	prior_depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	prior_texture_enabled ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
}

void VR_Draw::render_arrow(const Coord3Df& from, const Coord3Df& to, float width)
{
	/* Save previous OpenGL state */
	GLint prior_program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prior_program);
	GLboolean prior_backface_culling = glIsEnabled(GL_CULL_FACE);
	GLboolean prior_blend_enabled = glIsEnabled(GL_BLEND);
	GLboolean prior_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean prior_texture_enabled = glIsEnabled(GL_TEXTURE_2D);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	glUseProgram(VR_Draw::Shader::color_shader());

	GLint prior_vertex_array_binding;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prior_vertex_array_binding);
	GLint prior_array_buffer;
	glGetIntegerv(GL_ARRAY_BUFFER, &prior_array_buffer);

	/* Load uniforms */
	glUniformMatrix4fv(VR_Draw::Shader::shader_col.modelview_location, 1, false, (float*)VR_Draw::modelview_matrix.m);
	glUniformMatrix4fv(VR_Draw::Shader::shader_col.projection_location, 1, false, (float*)VR_Draw::projection_matrix.m);
	glUniform4fv(VR_Draw::Shader::shader_col.color_location, 1, (float*)VR_Draw::color_vector);

	/* Create vertex array */
	static GLuint vertex_array;
	if (!vertex_array) {
		glGenVertexArrays(1, &vertex_array);
	}
	glBindVertexArray(vertex_array);

	/* Create vertex buffer */
	static Coord3Df vd;
	vd.x = to.x - from.x; vd.y = to.y - from.y; vd.z = to.z - from.z;
	static Coord3Df vn;
	vn = vd.normalize() * width;

	static GLfloat vertex_data[4][3];
	vertex_data[0][0] = vd.x + from.x;	vertex_data[0][1] = vd.y + from.y;	vertex_data[0][2] = to.z;
	vertex_data[1][0] = vn.y + from.y;	vertex_data[1][1] = -vn.x + from.y;	vertex_data[1][2] = from.z;
	vertex_data[2][0] = -vn.y + from.x;	vertex_data[2][1] = vn.x + from.y;  vertex_data[2][2] = from.z;
	vertex_data[3][0] = -vn.x + from.x;	vertex_data[3][1] = -vn.y + from.y; vertex_data[3][2] = from.z;

	static GLuint verts = 0;
	if (!verts) {
		glGenBuffers(1, &verts);
	}
	glBindBuffer(GL_ARRAY_BUFFER, verts);
	glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), vertex_data, GL_STATIC_DRAW);
	glVertexAttribPointer(VR_Draw::Shader::shader_col.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(VR_Draw::Shader::shader_col.position_location);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(VR_Draw::Shader::shader_col.position_location);

	/* Restore previous OpenGL state */
	glBindVertexArray(prior_vertex_array_binding);
	glBindBuffer(GL_ARRAY_BUFFER, prior_array_buffer);

	glUseProgram(prior_program);
	prior_backface_culling ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	prior_blend_enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	prior_depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	prior_texture_enabled ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
}

void VR_Draw::render_string(const char* str, float w, float h, VR_HAlign h_align, VR_VAlign v_align, float x_offset, float y_offset, float z_offset)
{
	/* 1: If not done yet: allocate image texture. */
	if (!ascii_tex) { /* Should have been created in VR_Draw::init() */
		ascii_tex = new Texture(ascii_png);
	}

	/* 2: Save previous OpenGL state. */
	GLint prior_program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prior_program);
	GLboolean prior_backface_culling = glIsEnabled(GL_CULL_FACE);
	GLboolean prior_blend_enabled = glIsEnabled(GL_BLEND);
	GLboolean prior_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean prior_texture_enabled = glIsEnabled(GL_TEXTURE_2D);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

	glUseProgram(VR_Draw::Shader::texture_shader());
	GLint prior_vertex_array_binding;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prior_vertex_array_binding);
	GLint prior_array_buffer;
	glGetIntegerv(GL_ARRAY_BUFFER, &prior_array_buffer);
	GLint prior_texture_binding_2d;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prior_texture_binding_2d);
	GLint prior_texture_unit;
	glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&prior_texture_unit);

	glActiveTexture(GL_TEXTURE0);

	/* Bind the texture (and create the texture implementation if necessary). */
	ascii_tex->bind();

	/* 3: Walk over all characters and render them. */
	int i;
	float full_height = h;
	float full_width = 0.0f;
	float line_width = 0.0f;
	i = 0;
	while (str[i]) {
		int index = int(str[i]);
		if (index == '\n') {
			full_height += h * 1.2f;
			if (full_width < line_width) {
				full_width = line_width;
			}
			line_width = 0.0f;
		}
		else if (index == '\t') {
			line_width += w * 4.0f;
		}
		else if (index >= 32 && index <= 126) { /* valid ASCII character */
			line_width += w;
		}
		i++;
	}
	if (full_width < line_width) {
		full_width = line_width;
	}

	/* top-left corner */
	if (h_align == VR_HALIGN_RIGHT) {
		x_offset -= full_width;
	}
	else if (h_align == VR_HALIGN_CENTER) {
		x_offset -= full_width / 2.0f;
	}
	if (v_align == VR_VALIGN_BOTTOM) {
		y_offset += full_height;
	}
	else if (v_align == VR_HALIGN_RIGHT) {
		y_offset += full_height / 2.0f;
	}

	float x = x_offset;
	float y = y_offset;

	/* Load uniforms */
	glUniformMatrix4fv(VR_Draw::Shader::shader_tex.modelview_location, 1, false, (float*)VR_Draw::modelview_matrix.m);
	glUniformMatrix4fv(VR_Draw::Shader::shader_tex.projection_location, 1, false, (float*)VR_Draw::projection_matrix.m);
	glUniformMatrix4fv(VR_Draw::Shader::shader_tex.normal_matrix_location, 1, true, (float*)VR_Draw::modelview_matrix_inv.m);
	glUniform4fv(VR_Draw::Shader::shader_tex.color_location, 1, (float*)VR_Draw::color_vector);

	/* Create vertex array */
	static GLuint vertex_array;
	if (!vertex_array) {
		glGenVertexArrays(1, &vertex_array);
	}
	glBindVertexArray(vertex_array);

	/* Create normal buffer */
	static const GLfloat normal_data[] = {
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};
	static GLuint nrmls = 0;
	if (!nrmls) {
		glGenBuffers(1, &nrmls);
		glBindBuffer(GL_ARRAY_BUFFER, nrmls);
		glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), normal_data, GL_STATIC_DRAW);
	}
	else {
		glBindBuffer(GL_ARRAY_BUFFER, nrmls);
	}
	glVertexAttribPointer(VR_Draw::Shader::shader_tex.normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(VR_Draw::Shader::shader_tex.normal_location);

	i = 0;
	while (str[i]) {
		int index = int(str[i]);
		if (index == '\n') {
			y -= h * 1.2f;
			x = x_offset;
			++i;
			continue;
		}
		if (index == '\t') {
			x += w * 4.0f;
			++i;
			continue;
		}
		index -= 32; /* based of first printable ascii character */
		if (index < 0 || index>94) { /* invalid character: skip */
			++i;
			continue;
		}
		int col = index % 14; /* row in 14x7 grid */
		int row = (index - col) / 14; /* col in 14x7 grid */

		/* Create vertex buffer */
		static GLfloat vertex_data[4][3];
		vertex_data[0][0] = x;     vertex_data[0][1] = y - h; vertex_data[0][2] = z_offset; /* bottom-left */
		vertex_data[1][0] = x + w; vertex_data[1][1] = y - h; vertex_data[1][2] = z_offset; /* bottom-right */
		vertex_data[2][0] = x;     vertex_data[2][1] = y;     vertex_data[2][2] = z_offset; /* top-left */
		vertex_data[3][0] = x + w; vertex_data[3][1] = y;     vertex_data[3][2] = z_offset; /* top-right */

		static GLuint verts = 0;
		if (!verts) {
			glGenBuffers(1, &verts);
		}
		glBindBuffer(GL_ARRAY_BUFFER, verts);
		glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), vertex_data, GL_STATIC_DRAW); //GL_DYNAMIC_DRAW);
		glVertexAttribPointer(VR_Draw::Shader::shader_tex.position_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(VR_Draw::Shader::shader_tex.position_location);

		/* Create uv buffers */
		static GLfloat uv_data[4][2];
		uv_data[0][0] = float(col + 0) / 14.0f; uv_data[0][1] = float(row + 1) / 7.0f;
		uv_data[1][0] = float(col + 1) / 14.0f; uv_data[1][1] = float(row + 1) / 7.0f;
		uv_data[2][0] = float(col + 0) / 14.0f, uv_data[2][1] = float(row + 0) / 7.0f;
		uv_data[3][0] = float(col + 1) / 14.0f; uv_data[3][1] = float(row + 0) / 7.0f;

		static GLuint uvs = 0;
		if (!uvs) {
			glGenBuffers(1, &uvs);
		}
		glBindBuffer(GL_ARRAY_BUFFER, uvs);
		glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(float), uv_data, GL_STATIC_DRAW);
		glVertexAttribPointer(VR_Draw::Shader::shader_tex.uv_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
		glEnableVertexAttribArray(VR_Draw::Shader::shader_tex.uv_location);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		++i;
		x += w;
	}

	glDisableVertexAttribArray(VR_Draw::Shader::shader_tex.position_location);
	glDisableVertexAttribArray(VR_Draw::Shader::shader_tex.normal_location);
	glDisableVertexAttribArray(VR_Draw::Shader::shader_tex.uv_location);

	/* 4: Restore previous OpenGL state */
	glBindVertexArray(prior_vertex_array_binding);
	glBindBuffer(GL_ARRAY_BUFFER, prior_array_buffer);
	glBindTexture(GL_TEXTURE_2D, prior_texture_binding_2d);
	glActiveTexture(prior_texture_unit);

	glUseProgram(prior_program);
	prior_backface_culling ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	prior_blend_enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	prior_depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	prior_texture_enabled ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
}
