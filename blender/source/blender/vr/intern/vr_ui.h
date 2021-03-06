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

/** \file blender/vr/intern/vr_ui.h
*   \ingroup vr
*/

#ifndef __VR_UI_H__
#define __VR_UI_H__

#define VR_UI_DEFAULTDRAGTHRESDIST    0.012f	/* Default distance threshold (meters) to detect "dragging". */
#define VR_UI_DEFAULTDRAGTHRESROT     8.0f		/* Default rotation threshold (deg) to detect "dragging". */
#define VR_UI_DEFAULTDRAGTHRESTIME    150		/* Default time threshold (ms) to distinguish between "clicking" and "dragging". */

#define VR_UI_DEFAULTWORKSPACESIZE    0.450f	/* Size of the estimated default workspace in meters. */
#define VR_UI_DEFAULTWORKSPACEDIST    0.550f	/* Distance of default workspace center from the HMD in meters. */
#define VR_UI_DEFAULTWORKSPACEHEIGHT -0.350f	/* Height of default workspace center (relative to HMD) in meters. */

#define VR_UI_MAXUPDATEINTERVAL      (1000/1)	/* Maximum interval in ms at which the UI should perform updates on Blender. (1Hz) */
#if 0
#define VR_UI_MINUPDATEINTERVAL      (1000/60)	/* Minimum interval in ms at which the UI should perform updates on Blender. (60Hz) */
#else
#define VR_UI_MINUPDATEINTERVAL      (1000/120)	/* Minimum interval in ms at which the UI should perform updates on Blender. (120Hz) */
#endif
#define VR_UI_OPTIMIZEPERFORMANCEMELTCPU 0		/* Whether to override the update interval limits and update as fast as possible. */

#define VR_UI_MINNAVIGATIONSCALE	  0.001f	/* Minimum navigation scale (Real to Blender) permitted. */
#define VR_UI_MAXNAVIGATIONSCALE	  1000.0f	/* Maximum navigation scale (Real to Blender) permitted. */

#include "DNA_userdef_types.h"

#include <string>
#include <vector>

struct bGPDspoint;
class VR_Widget; 

/* User interation master controller/translator class. */
class VR_UI
{
	// ============================================================================================== //
	// ===========================    USER INTERFACE TRACKING UTILITIES    ========================== //
	// ---------------------------------------------------------------------------------------------- //
public:
	/* Enum defining error codes. Null indicates successful operation. */
	typedef enum Error
	{
		ERROR_NONE				/* Operation performed successfully. */
		,
		ERROR_NOTINITIALIZED	/* The module was not correctly initialized. */
		,
		ERROR_INVALIDPARAMETER	/* One or more of the provided parameters were invalid. */
		,
		ERROR_INTERNALFAILURE	/* A failure has occurred during execution. */
		,
		ERROR_NOTAVAILABLE		/* The requested functionality is not available in this implementation. */
	} Error;

	/* Possible states of a button in interaction, for building a state machine.
	 * Used to distinguish dragging, clicking, etc. */
	typedef enum ButtonState
	{
		BUTTONSTATE_IDLE		/* No button in interaction (initial state). */
		,
		BUTTONSTATE_DOWN		/* Button is pressed, but no action was triggered yet. */
		,
		BUTTONSTATE_RELEASE		/* Button was recently released (no action triggered yet).  */
		,
		BUTTONSTATE_DRAG		/* Button in holding/dragging action.  */
		,
		BUTTONSTATE_DRAGRELEASE /* Button was released from hold (hold hasn't finished yet though). */
	} ButtonState;

	/* Possible states of the "Ctrl"-key in interaction. */
	typedef enum CtrlState
	{
		CTRLSTATE_OFF = 0	/* "Ctrl"-key not active. */
		,
		CTRLSTATE_ON = 1	/* "Ctrl"-key active. */
		,
		CTRLSTATES = 2	/* Number of distinct "Ctrl"-key states. */
	} CtrlState;

	/* Possible states of the "Shift"-key in interaction. */
	typedef enum ShiftState
	{
		SHIFTSTATE_OFF	= 0	/* "Shift"-key not active. */
		,
		SHIFTSTATE_ON	= 1	/* "Shift"-key active. */
		,
		SHIFTSTATES		= 2	/* Number of distinct "Shift"-key states. */
	} ShiftState;

	/* Possible states of the "Alt"-key in interaction. */
	typedef enum AltState
	{
		ALTSTATE_OFF	= 0	/* "Alt"-key not active. */
		,
		ALTSTATE_ON		= 1	/* "Alt"-key active. */
		,
		ALTSTATES		= 2	/* Number of distinct "Alt"-key states. */
	} AltState;

	/* Possible modes of navigation. */
	typedef enum NavMode
	{
		NAVMODE_NONE		/* No navigation / disable locomotion. */
		,
		NAVMODE_GRABAIR	/* Grabbing-the-air navigation (default). */
		,
		NAVMODE_JOYSTICK	/* Joystick-style navigation (alyways keeping z-up). */
		,
		NAVMODE_TELEPORT	/* Teleport navigation. */
	} NavMode;

	/* Possible navigation locks. */
	typedef enum NavLock
	{
		NAVLOCK_NONE = 0	/* No locks. */
		,
		NAVLOCK_TRANS = 1	/* Translation lock. */
		,
		NAVLOCK_TRANS_UP = 2	/* Up translation lock. */
		,
		NAVLOCK_ROT = 3	/* Rotation lock. */
		,
		NAVLOCK_ROT_UP = 4	/* Up-direction rotation lock. */
		,
		NAVLOCK_SCALE = 5	/* Scale lock. */
		,
		NAVLOCK_SCALE_REAL = 6	/* 1:1 Blender/Real scale lock. */
		,
		NAVLOCKS = 7	/* Number of distinct navigation locks. */
	} NavLock;

	/* Selection mode (raycast or proximity). */
	enum SelectionMode {
		SELECTIONMODE_RAYCAST = 0	/* The default raycast / rectangle selection method. */
		,
		SELECTIONMODE_PROXIMITY = 1 /* The proximity / volume selection method. */
		,
		SELECTIONMODES = 2 /* Number of existing selection modes. */
	};

	/* Possible transformation spaces. */
	typedef enum TransformSpace
	{
		TRANSFORMSPACE_GLOBAL = 0	/* Global space. */
		,
		TRANSFORMSPACE_LOCAL = 1	/* Local space. */
		,
		TRANSFORMSPACE_NORMAL = 2	/* Normal space. */
		,
		TRANSFORMSPACES = 3	/* Number of distinct transformation spaces. */
	} TransformSpace;

	/* Possible constraint modes. */
	typedef enum ConstraintMode
	{
		CONSTRAINTMODE_NONE = 0	/* No constraints. */
		,
		CONSTRAINTMODE_TRANS_X = 1	/* X translation. */
		,
		CONSTRAINTMODE_TRANS_Y = 2	/* Y translation. */
		,
		CONSTRAINTMODE_TRANS_Z = 3	/* Z translation. */
		,
		CONSTRAINTMODE_TRANS_XY = 4	/* XY translation. */
		,
		CONSTRAINTMODE_TRANS_YZ = 5	/* YZ translation. */
		,
		CONSTRAINTMODE_TRANS_ZX = 6	/* ZX translation. */
		,
		CONSTRAINTMODE_ROT_X = 7	/* X rotation. */
		,
		CONSTRAINTMODE_ROT_Y = 8	/* Y rotation. */
		,
		CONSTRAINTMODE_ROT_Z = 9	/* Z rotation. */
		,
		CONSTRAINTMODE_SCALE_X = 10	/* X scaling. */
		,
		CONSTRAINTMODE_SCALE_Y = 11	/* Y scaling. */
		,
		CONSTRAINTMODE_SCALE_Z = 12	/* Z scaling. */
		,
		CONSTRAINTMODE_SCALE_XY = 13	/* XY scaling. */
		,
		CONSTRAINTMODE_SCALE_YZ = 14	/* YZ scaling. */
		,
		CONSTRAINTMODE_SCALE_ZX = 15	/* ZX scaling. */
		,
		CONSTRAINTMODES = 16	/* Number of distinct constraint modes. */
	} ConstraintMode;

	/* Possible snapping modes. */
	typedef enum SnapMode
	{
		SNAPMODE_NONE = 0	/* No snapping. */
		,
		SNAPMODE_TRANSLATION = 1	/* Translation snapping to Blender scene units. */
		,
		SNAPMODE_ROTATION = 2	/* Rotation snapping to Blender scene units. */
		,
		SNAPMODE_SCALE = 3	/* Scale snapping to Blender scene units. */
		,
		SNAPMODE_POINTS = 4	/* Snapping to Polygon vertices and NURBS CVs. */
		,
		SNAPMODE_CURVES = 5	/* Snapping to NURBS curves. */
		,
		SNAPMODE_MESH = 6	/* Snapping to Polygon meshes (faces). */
		,
		SNAPMODES = 7	/* Number of distinct snapping modes. */
	} SnapMode;

	/* Transformation matrix extended to allow lazy evaluation. */
	typedef struct LMatrix
	{
		Mat44f	mat;		/* Transformation matrix. */
		bool	mat_curr;	/* Whether the transformation matrix is up-to-date. */
		Mat44f	inv;		/* Inverse of the matrix. */
		bool    inv_curr;   /* Whether the matrix inverse is up-to-date. */
		LMatrix() : mat_curr(false), inv_curr(false) {};
	} LMatrix;

	/* Transformation matrix extended to allow lazy evaluation,
	 * for two spaces (Real and Blender). */
	typedef struct LMatrix2
	{
		LMatrix position[VR_SPACES];	/* Cursor transformation in both spaces (Real and Blender). */
		LMatrix2();	/* Initializing constructor (null-init). */
		LMatrix2(const LMatrix2& cpy);	/* Copy constructor. */
		LMatrix2& operator = (const LMatrix2 &o);	/* Copy operator. */
		void set(const float m[4][4], VR_Space s = VR_SPACE_REAL);	/* Assign a new matrix. */
		const Mat44f& get(VR_Space s = VR_SPACE_REAL, bool inverse = false);	/* Retrieve the matrix in a given space. */
	} LMatrix2;

	/* Struct for holding information regarding a 3D cursor. */
	typedef struct Cursor
	{
		LMatrix2	position;			/* Current (most recent) registered position. */
		bool		active;				/* Whether this cursor is active (i.e. used, not necessarily clicking anything). */
		bool        visible;			/* Whether this cursor is visible and needs to be rendered. */
		ui64		last_upd;			/* Timestamp of the last successful positional update. */
		LMatrix2    last_position;		/* Last registered position (prior to the current one). */
		ui64        last_buttons;		/* Currently depressed buttons associated with this cursor (flagword). */
		bool        trigger;			/* Whether the Trigger button is currently pressed on this cursor. */
		bool		grip;				/* Whether the Grip buton is currently pressed on this cursor. */
		CtrlState	ctrl;				/* Whether the CTRL key is currently pressed on this cursor (0 or 1). */
		ShiftState  shift;				/* Whether the SHIFT key is currently pressed on this cursor (0 or 1). */
		AltState    alt;				/* Whether the ALT key is currently pressed on this cursor (0 or 1). */
		ButtonState	interaction_state;	/* The state of the button state machine. */
		ui64        interaction_button;	/* The button that caused the current interaction (or 0 if idle). */
		LMatrix2    interaction_position;/* The position of the cursor when the button was pressed. */
		ui64		interaction_time;	/* Timestamp of when the interaction was started. */
		CtrlState	interaction_ctrl;	/* Whether the CTRL key was pressed when the interaction was started (0 or 1). */
		ShiftState  interaction_shift;	/* Whether the SHIFT key was pressed when the interaction was started (0 or 1). */
		AltState    interaction_alt;	/* Whether the ALT key was pressed when the interaction was started (0 or 1). */
		VR_Widget*	interaction_widget;	/* Currently active widget (or 0 if none is mapped). */
		Coord3Df    offset_pos;			/* Relative positional offset between actualy controller position and virtual cursor position (global). */
		Mat44f	    offset_rot;			/* Relative rotational offset between actual controller rotation and virtual cursor rotation (local). */

		/* Enum of the states of bimanual interation (which hand started the interaction). */
		typedef enum Bimanual {
			BIMANUAL_OFF	/* Currently no bi-manual operation. */
			,
			BIMANUAL_FIRST	/* Currently in bi-manual operation as first hand. */
			,
			BIMANUAL_SECOND	/* Currently in bi-manual operation as second hand. */
		} Bimanual;
		Bimanual bimanual;	/* Current state in bi-manual interaction. */

		VR_Side side;		/* Hand side (if two controllers are available). */
		Cursor* other_hand; /* Access the other hand's cursor (if any). */
		Cursor();			/* Constructor (null-init). */

		Mat44f	reference;  /* Reference coordinate system for transformations. */
		void*	target_obj;	/* Target object of the hand (if any/context dependent). */

	} Cursor;

// ============================================================================================== //
// ==============================    STATIC GLOBAL UI MONITOR    ================================ //
// ---------------------------------------------------------------------------------------------- //
//private:
public:
	static VR_UI *ui; /* Currently used UI instance (singleton). */
	static VR_Device_Type ui_type;  /* The type of the currently used UI. */
protected:
	static Mat44f		navigation_matrix;	/* Navigation matrix. */
	static Mat44f		navigation_inverse;	/* Navigation matrix inverse. */
	static float		navigation_scale;	/* Scale factor for the Blender origin (origin_m * scale = blender_space_units). */

	static bool hmd_position_current[VR_SPACES][2]; /* Whether the HMD transformation is current for [VR_SPACE][inverse] */
	static bool eye_position_current[VR_SPACES][VR_SIDES][2]; /* Whether the eye transformation is current for [VR_SPACE][VR_SIDE][inverse] */
	static float		eye_baseline;		/* Distance between the eyes in real-world meters. */
	static VR_Side		eye_dominance;		/* Which of the eyes is dominant. */

	static bool controller_position_current[VR_SPACES][VR_MAX_CONTROLLERS][2]; /* Whether the controller transformation is current for [VR_SPACE][VR_SIDE][inverse] */

	static Cursor		cursor[VR_MAX_CONTROLLERS];	/* Cursors for both hands plus auxilliary controllers if available. */
	static CtrlState	ctrl_key;			/* Whether the UI is in "ctrl-key" mode (0 or 1) */
	static ShiftState	shift_key;			/* Whether the UI is in "shift-key" mode (0 or 1) */
	static AltState     alt_key;			/* Whether the UI is in "alt-key" mode (0 or 1) */
	static VR_Side      hand_dominance;		/* Which hand is the main (dominant) hand ("handedness"). */
	
	static bool			updating;			/* Whether the UI is currently in its update() process. */
public:
	static ui64			fps_render;			/* Current framerate of(real) rendering(excluding empty re - posts). */
public:
	static const Mat44f& navigation_matrix_get();  /* Navigation matrix. */
	static const Mat44f& navigation_inverse_get(); /* Navigation matrix inverse. */
	static const float& navigation_scale_get();   /* Scale factor for the Blender origin (origin_m * scale = blender_space_units). */
	static void			navigation_set(const Mat44f& m);	/* Set navigation matrix. */
	static void			navigation_apply_transformation(const Mat44f& m, VR_Space space = VR_SPACE_BLENDER, bool inverse = false); /* Apply navigational transformation, relative to current navigation. */
	static void			navigation_reset(); /* Reset navigation matrix to identity. */

	static void			navigation_fit_scene();	/* Reset navigation to fit the whole scene into the work area. */
	static void			navigation_fit_selection(VR_Direction look_from_direction = VR_DIRECTION_NONE);	/* Reset navigation to fit the current selection into the work area. */

	static bool			is_zaxis_up(); /* Whether the z-axis is the up direction in Blender. (If not, the y-axis is assumed to be the up direction). */
	static void			navigation_orient_up(Coord3Df* pivot = 0); /* Rotate the origin so than the Blender up-axis is facing (real) upwards again. */

	static NavMode navigation_mode;	/* Current mode of navigation (locomotion). */

	static const Mat44f& hmd_position_get(VR_Space space, bool inverse = false);	/* Get the current transformation matrix of the HMD. */
	static const Mat44f& eye_position_get(VR_Space space, VR_Side side, bool inverse = false);	/* Get the transformation matrix of the eye (or physical camera in space). */
	static float        eye_baseline_get();	/* Get the distance between the eyes in meters. */
	static VR_Side      eye_dominance_get();	/* Get which of the eyes is dominant. */
	static void			eye_baseline_set(float baseline); /* Set the distance between the eyes in real-world meters. */
	static void			eye_dominance_set(VR_Side side);	/* Get which of the eyes is dominant. */

	static VR_Side		hand_dominance_get();	/* Get dominant hand side (right-handed or left-handed). */
	static void			hand_dominance_set(VR_Side side);	/* Set dominant hand side (right-handed or left-handed). */

	static const Mat44f& controller_position_get(VR_Space space, VR_Side side, bool inverse = false); /* Get the transformation matrix of the controller. */

	static const Mat44f& cursor_position_get(VR_Space space, VR_Side side, bool inverse = false);	/* Get the 3D cursor position of either hand. */
	static const Mat44f& cursor_interaction_position_get(VR_Space space, VR_Side side, bool inverse = false);	/* Get the 3D cursor position at the time of interaction start. */
	static ui64			cursor_buttons_get(VR_Side side);	/* Get the cursors current button state. */
	static bool			cursor_trigger_get(VR_Side side);	/* Get whether the trigger button is currently pressed on the controller. */
	static bool			cursor_grip_get(VR_Side side);	/* Get whether the grip button is currently pressed on the controller. */
	static bool			cursor_active_get(VR_Side side);		/* Get whether the 3D cursor is currently active (in use, but not necessarily doing anything right now). */
	static bool			cursor_visible_get(VR_Side side);	/* Get whether the 3D cursor is set to be visible (tracked, but not necessarily on screen). */
	static void			cursor_position_set(VR_Space space, VR_Side side, const Mat44f& m); /* Set the 3D cursors current transformation. */
	static void			cursor_active_set(VR_Side side, bool b);	/* Set whether the 3D cursor is active. */
	static void			cursor_visible_set(VR_Side side, bool b); /* Set whether the 3D cursor should be rendered. */
	static bool			cursor_offset_enabled;	/* Whether the controller offset is currently being enabled. */
	static bool			cursor_offset_update;	/* Whether the controller offset is currently being updated. */
	static void			cursor_offset_set(VR_Side side, const Mat44f& rot, const Coord3Df& pos);	/* Set the current cursor offset for target cursor. */

	static bool			mouse_cursor_enabled;	/* Whether to enable (render) the mouse cursor. */
	static Mat44f		viewport_projection[VR_SIDES];	/* Projection matrices for the VR viewports. */
	static rcti			viewport_bounds;	/* Viewport (window) bounds for the VR viewports. */

	static CtrlState	ctrl_key_get();		/* Get Whether the CTRL key is currently held down (on either controller). */
	static ShiftState	shift_key_get();	/* Get whether the SHIFT key is currently held down (on either controller). */
	static AltState		alt_key_get();		/* Get whether the ALT key is currently held down (on either controller). */
	static void			ctrl_key_set(CtrlState state);	/* Manually set the VR_UI ctrl key state. */
	static void			shift_key_set(ShiftState state);	/* Manually set the VR_UI shift key state. */
	static void			alt_key_set(AltState state);	/* Manually set the VR_UI alt key state; */

	static VR_Widget*	get_current_tool(VR_Side side);	/* Get the currently active tool for the controller. */
	static VR_UI::Error set_current_tool(const VR_Widget* tool, VR_Side side);	/* Set the currently active tool for the controller. */

	static Mat44f		convert_space(const Mat44f& m, VR_Space m_space, VR_Space target_space); /* Convert a matrix into target space. */
	static Coord3Df		convert_space(const Coord3Df& v, VR_Space v_space, VR_Space target_space); /* Convert vector/position into target space. */
	static int			get_screen_coordinates(const Coord3Df& c, float& x, float&, VR_Side side = VR_SIDE_DOMINANT);	/* Get 2D screen coordinates (-1 ~ 1) of a 3D point. */
	static int			get_pixel_coordinates(const Coord3Df& c, int& x, int& y, VR_Side side = VR_SIDE_DOMINANT);		/* Get 2D pixel coordinates of a 3D point. */
	
	static float		scene_unit_scale(VR_Space space);	/* Get the length (scale) of Blender scene units in respective space. */

	static SelectionMode selection_mode;	/* The current selection mode. */
	static float        selection_tolerance;	/* The current selection tolerance (meters). */

	static float		drag_threshold_distance;	/* Distance threshold (meters) to detect "dragging" (if the cursor moves farther than this with a button held down, it's dragging). */
	static float		drag_threshold_rotation;	/* Rotation threshold (deg) to detect "dragging" (if the cursor rotates more than this with a button held down, it's dragging). */
	static uint			drag_threshold_time;		/* Time threshold (ms) to distinguish between "clicking" and "dragging". */

	static int			undo_count;	/* Number of pending VR_UI undo operations to be executed post-scene render. */
	static int			redo_count; /* Number of pending VR_UI redo operations to be executed post-scene render. */
	static bool			editmode_exit;	/* Whether to exit edit mode (executed post-scene render). */

	static bool pie_menu_active[VR_SIDES];	/* Whether a VR pie menu is active for the controller. */
	static const VR_Widget *pie_menu[VR_SIDES];	/* The current VR pie menu for the controller (if any). */

// ============================================================================================== //
// ===========================    DYNAMIC UI OBJECT IMPLEMENTATION    =========================== //
// ---------------------------------------------------------------------------------------------- //
protected:
	VR_UI();            /* Constructor (hidden - use static init method). */
	VR_UI(VR_UI& cpy);	/* Copy constructor (hidden - do not copy objects). */
	virtual ~VR_UI();	/* Destructor (hidden - use static uninit method). */
public:
	static VR_UI*	i();	/* Access currently used instance of UI. */
	static VR_Device_Type type();	/* Get the type of the currently used UI. */

	static bool		is_available(VR_Device_Type type);	/* Test whether a certain UI type is available. */
	static Error	set_ui(VR_Device_Type type);	/* Set desired UI implementation. */
	static Error	shutdown();					/* Shutdown UI module. */

	static Error	update_tracking();			/* Update HMD and cursor positions, button state, etc. (pre-render). */
	static Error	execute_operations();		/* Execute internal UI operations and possibly execute users actions in Blender. (pre-render). */
	static Error	update_cursor(Cursor& c);	/* Cursor interaction update. */
	static Error	update_menus();				/* Update any VR floating menus. */
	static Error	execute_post_render_operations();	/* Execute special UI operations (i.e. undo/redo) that need to be called after the scene is rendered. */

	static Error	pre_render(VR_Side side);	/* Render UI elements, called prior to rendering the scene. */
	static Error	post_render(VR_Side side);	/* Render UI elements, called after rendering the scene. */

	static Error	render_controller(VR_Side controller_side);	/* Helper function to render the controller. */
	static Error	render_widget_icons(VR_Side controller_side, const Mat44f& t_controller);	/* Helper function to render the widget icons on the controller. */
	
	static Error	execute_widget_renders(VR_Side side);	/* Helper function to execute widget render functions. */
	static Error	render_menus(const Mat44f *_model, const Mat44f *_view);	/* Helper function to render VR floating menus. */
};

#endif /* __VR_UI_H__ */
