#pragma once

#include <stdio.h>
#include <string.h>

//
// Keyboard definition. Everything below = 0x100 matches
// a mode 1 keyboard scan code.
//

enum EKeyCodes
{
	KEY_PAUSE				= 0xc5,	// DIK_PAUSE
	KEY_RIGHTARROW			= 0xcd,	// DIK_RIGHT
	KEY_LEFTARROW			= 0xcb,	// DIK_LEFT
	KEY_UPARROW 			= 0xc8,	// DIK_UP
	KEY_DOWNARROW			= 0xd0,	// DIK_DOWN
	KEY_ESCAPE				= 0x01,	// DIK_ESCAPE
	KEY_ENTER				= 0x1c,	// DIK_RETURN
	KEY_SPACE				= 0x39,	// DIK_SPACE
	KEY_TAB 				= 0x0f,	// DIK_TAB
	KEY_F1					= 0x3b,	// DIK_F1
	KEY_F2					= 0x3c,	// DIK_F2
	KEY_F3					= 0x3d,	// DIK_F3
	KEY_F4					= 0x3e,	// DIK_F4
	KEY_F5					= 0x3f,	// DIK_F5
	KEY_F6					= 0x40,	// DIK_F6
	KEY_F7					= 0x41,	// DIK_F7
	KEY_F8					= 0x42,	// DIK_F8
	KEY_F9					= 0x43,	// DIK_F9
	KEY_F10 				= 0x44,	// DIK_F10
	KEY_F11 				= 0x57,	// DIK_F11
	KEY_F12 				= 0x58,	// DIK_F12
	KEY_GRAVE				= 0x29,	// DIK_GRAVE

	KEY_BACKSPACE			= 0x0e,	// DIK_BACK

	KEY_EQUALS				= 0x0d,	// DIK_EQUALS
	KEY_MINUS				= 0x0c,	// DIK_MINUS

	KEY_LSHIFT				= 0x2A,	// DIK_LSHIFT
	KEY_LCTRL				= 0x1d,	// DIK_LCONTROL
	KEY_LALT				= 0x38,	// DIK_LMENU

	KEY_RSHIFT				= 0x36,
	KEY_RCTRL				= 0x9d,
	KEY_RALT				= 0xb8,

	KEY_INS 				= 0xd2,	// DIK_INSERT
	KEY_DEL 				= 0xd3,	// DIK_DELETE
	KEY_END 				= 0xcf,	// DIK_END
	KEY_HOME				= 0xc7,	// DIK_HOME
	KEY_PGUP				= 0xc9,	// DIK_PRIOR
	KEY_PGDN				= 0xd1,	// DIK_NEXT

	KEY_VOLUMEDOWN			= 0xAE, // DIK_VOLUMEDOWN
	KEY_VOLUMEUP			= 0xB0, // DIK_VOLUMEUP

	KEY_FIRSTMOUSEBUTTON	= 0x100,
	KEY_MOUSE1				= 0x100,
	KEY_MOUSE2				= 0x101,
	KEY_MOUSE3				= 0x102,
	KEY_MOUSE4				= 0x103,
	KEY_MOUSE5				= 0x104,
	KEY_MOUSE6				= 0x105,
	KEY_MOUSE7				= 0x106,
	KEY_MOUSE8				= 0x107,

	KEY_FIRSTJOYBUTTON		= 0x108,
	KEY_JOY1				= KEY_FIRSTJOYBUTTON+0,
	KEY_JOY2,
	KEY_JOY3,
	KEY_JOY4,
	KEY_JOY5,
	KEY_JOY6,
	KEY_JOY7,
	KEY_JOY8,
	KEY_JOY9,
	KEY_JOY10,
	KEY_JOY11,
	KEY_JOY12,
	KEY_JOY13,
	KEY_JOY14,
	KEY_JOY15,
	KEY_LASTJOYBUTTON		= 0x187,
	KEY_JOYPOV1_UP			= 0x188,
	KEY_JOYPOV1_RIGHT		= 0x189,
	KEY_JOYPOV1_DOWN		= 0x18a,
	KEY_JOYPOV1_LEFT		= 0x18b,
	KEY_JOYPOV2_UP			= 0x18c,
	KEY_JOYPOV3_UP			= 0x190,
	KEY_JOYPOV4_UP			= 0x194,

	KEY_MWHEELUP			= 0x198,
	KEY_MWHEELDOWN			= 0x199,
	KEY_MWHEELRIGHT			= 0x19A,
	KEY_MWHEELLEFT			= 0x19B,

	KEY_JOYAXIS1PLUS		= 0x19C,
	KEY_JOYAXIS1MINUS		= 0x19D,
	KEY_JOYAXIS2PLUS		= 0x19E,
	KEY_JOYAXIS2MINUS		= 0x19F,
	KEY_JOYAXIS3PLUS		= 0x1A0,
	KEY_JOYAXIS3MINUS		= 0x1A1,
	KEY_JOYAXIS4PLUS		= 0x1A2,
	KEY_JOYAXIS4MINUS		= 0x1A3,
	KEY_JOYAXIS5PLUS		= 0x1A4,
	KEY_JOYAXIS5MINUS		= 0x1A5,
	KEY_JOYAXIS6PLUS		= 0x1A6,
	KEY_JOYAXIS6MINUS		= 0x1A7,
	KEY_JOYAXIS7PLUS		= 0x1A8,
	KEY_JOYAXIS7MINUS		= 0x1A9,
	KEY_JOYAXIS8PLUS		= 0x1AA,
	KEY_JOYAXIS8MINUS		= 0x1AB,

	KEY_PAD_LTHUMB_RIGHT	= 0x1AC,
	KEY_PAD_LTHUMB_LEFT		= 0x1AD,
	KEY_PAD_LTHUMB_DOWN		= 0x1AE,
	KEY_PAD_LTHUMB_UP		= 0x1AF,

	KEY_PAD_RTHUMB_RIGHT	= 0x1B0,
	KEY_PAD_RTHUMB_LEFT		= 0x1B1,
	KEY_PAD_RTHUMB_DOWN		= 0x1B2,
	KEY_PAD_RTHUMB_UP		= 0x1B3,

	KEY_PAD_DPAD_UP			= 0x1B4,
	KEY_PAD_DPAD_DOWN		= 0x1B5,
	KEY_PAD_DPAD_LEFT		= 0x1B6,
	KEY_PAD_DPAD_RIGHT		= 0x1B7,
	KEY_PAD_START			= 0x1B8,
	KEY_PAD_BACK			= 0x1B9,
	KEY_PAD_LTHUMB			= 0x1BA,
	KEY_PAD_RTHUMB			= 0x1BB,
	KEY_PAD_LSHOULDER		= 0x1BC,
	KEY_PAD_RSHOULDER		= 0x1BD,
	KEY_PAD_LTRIGGER		= 0x1BE,
	KEY_PAD_RTRIGGER		= 0x1BF,
	KEY_PAD_A				= 0x1C0,
	KEY_PAD_B				= 0x1C1,
	KEY_PAD_X				= 0x1C2,
	KEY_PAD_Y				= 0x1C3,

	NUM_KEYS				= 0x1C4,

	NUM_JOYAXISBUTTONS		= 8,
};

