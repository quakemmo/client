/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
/*
=======================================================================

XQ LOGIN MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_FRAME		"menu/art/cut_frame"
#define ART_ACCEPT0		"menu/art/accept_0"
#define ART_ACCEPT1		"menu/art/accept_1"	
#define ART_BACK0		"menu/art/back_0"
#define ART_BACK1		"menu/art/back_1"	

#define ID_CDKEY		10
#define ID_ACCEPT		11
#define ID_BACK			12
#define ID_CHAR			13


typedef struct {
	menuframework_s	menu;

	menutext_s		banner;
	menubitmap_s	frame;

	menutext_s		c[8];

	menubitmap_s	accept;
	menubitmap_s	back;
} xqcharselMenuInfo_t;

static xqcharselMenuInfo_t	xqcharselMenuInfo;
int go_straight_to_charcreation = 0;


/*
===============
UI_xqcharselMenu_Event
===============
*/
static void UI_XQCharselMenu_Event( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
		case ID_CHAR:
		case ID_CHAR+1:
		case ID_CHAR+2:
		case ID_CHAR+3:
		case ID_CHAR+4:
		case ID_CHAR+5:
		case ID_CHAR+6:
		case ID_CHAR+7:
		case ID_CHAR+8:;
			char *charname = (char *)((menucommon_s*)ptr)->name;
			if (xq_seq(charname, "Empty slot")) {
				UI_XQCharCreatorMenu("");
			} else {
				trap_Cvar_Set( "cl_charname", charname);
				trap_Cvar_Set("xq_charSelOn", "0");
				trap_Cmd_ExecuteText( EXEC_APPEND, "reconnect\n" );
			}
			break;

		case ID_ACCEPT:
			UI_PopMenu();
			break;

		case ID_BACK:
			UI_PopMenu();
			break;
	}
}

typedef struct {
	char name[100];
	char zone[100];
	char class[100];
	int level;
	char description[1001];
} character_t;
static void describe(character_t *c) {
	snprintf(c->description, 1000,
		"%s - %s(%i) - %s",
		c->name, c->class, c->level, c->zone
	);
	c->description[0] = toupper(c->description[0]);
}
xq_model_t xq_playable_models[XQ_RACES][2];


/*
===============
UI_XQCharselMenu_Init
===============
*/
static void UI_XQCharselMenu_Init( char *ss ) {
	trap_Cvar_Set("cl_charname", "");
	char *s = ss+2;
	char toons[10000] = {0};
	char models[10000] = {0};

	char *ptr = strchr(s, '!');
	if (ptr) {
		Q_strncpyz(models, ptr+1, sizeof(models));
		*ptr = 0;
	}
	Q_strncpyz(toons, s, sizeof(toons));


	UI_XQCharsel_Cache();

	int num = 0;

	character_t chars[8] = {0};

	memset( &xqcharselMenuInfo, 0, sizeof(xqcharselMenuInfo) );
	xqcharselMenuInfo.menu.wrapAround = qtrue;
	xqcharselMenuInfo.menu.fullscreen = qtrue;

	xqcharselMenuInfo.banner.generic.type				= MTYPE_BTEXT;
	xqcharselMenuInfo.banner.generic.x					= 320;
	xqcharselMenuInfo.banner.generic.y					= 10;
	xqcharselMenuInfo.banner.string						= "CHARACTER SELECTION";
	xqcharselMenuInfo.banner.color						= color_white;
	xqcharselMenuInfo.banner.style						= UI_CENTER;

	xqcharselMenuInfo.frame.generic.type				= MTYPE_BITMAP;
	xqcharselMenuInfo.frame.generic.name				= ART_FRAME;
	xqcharselMenuInfo.frame.generic.flags				= QMF_INACTIVE;
	xqcharselMenuInfo.frame.generic.x					= 142;
	xqcharselMenuInfo.frame.generic.y					= 118;
	xqcharselMenuInfo.frame.width  						= 359;
	xqcharselMenuInfo.frame.height  					= 256;

	char *tok1 = strtok(toons, "|");
	while (tok1 != NULL) {
		int ret = sscanf(tok1, "%s %s %i %s",
			chars[num].name, chars[num].class, &chars[num].level, chars[num].zone);
		if (strlen(chars[num].name) > 0 && ret == 4) {

			describe(&chars[num]);

			xqcharselMenuInfo.c[num].generic.type		= MTYPE_PTEXT;
			xqcharselMenuInfo.c[num].generic.name		= chars[num].name;
			xqcharselMenuInfo.c[num].generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
			xqcharselMenuInfo.c[num].generic.x			= 320;
			xqcharselMenuInfo.c[num].generic.y			= 150 + num * 30;
			xqcharselMenuInfo.c[num].generic.id			= ID_CHAR + num;
			xqcharselMenuInfo.c[num].generic.callback	= UI_XQCharselMenu_Event;
			xqcharselMenuInfo.c[num].string				= chars[num].description;
			xqcharselMenuInfo.c[num].color				= color_yellow;
			xqcharselMenuInfo.c[num].style				= UI_CENTER;

			num++;
			if (num == 7) break;
		}
		tok1 = strtok(NULL, "|");
	}

//Com_Printf("Done chars. models: [%s]\n", models);

	for (int i = num;  i < 8;  i++) {
		xqcharselMenuInfo.c[i].generic.type                 = MTYPE_PTEXT;
		xqcharselMenuInfo.c[i].generic.name					= "Empty slot";
		xqcharselMenuInfo.c[i].generic.flags                = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
		xqcharselMenuInfo.c[i].generic.x                    = 320;
		xqcharselMenuInfo.c[i].generic.y                    = 150 + i * 30;
		xqcharselMenuInfo.c[i].generic.id                   = ID_CHAR + i;
		xqcharselMenuInfo.c[i].generic.callback             = UI_XQCharselMenu_Event;
		xqcharselMenuInfo.c[i].string                       = "Empty slot";
		xqcharselMenuInfo.c[i].color                        = color_orange;
		xqcharselMenuInfo.c[i].style                        = UI_CENTER;
	}

	xqcharselMenuInfo.accept.generic.type				= MTYPE_BITMAP;
	xqcharselMenuInfo.accept.generic.name				= ART_ACCEPT0;
	xqcharselMenuInfo.accept.generic.flags				= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	xqcharselMenuInfo.accept.generic.id					= ID_ACCEPT;
	xqcharselMenuInfo.accept.generic.callback			= UI_XQCharselMenu_Event;
	xqcharselMenuInfo.accept.generic.x					= 640;
	xqcharselMenuInfo.accept.generic.y					= 480-64;
	xqcharselMenuInfo.accept.width						= 128;
	xqcharselMenuInfo.accept.height						= 64;
	xqcharselMenuInfo.accept.focuspic					= ART_ACCEPT1;

	xqcharselMenuInfo.back.generic.type					= MTYPE_BITMAP;
	xqcharselMenuInfo.back.generic.name					= ART_BACK0;
	xqcharselMenuInfo.back.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	xqcharselMenuInfo.back.generic.id					= ID_BACK;
	xqcharselMenuInfo.back.generic.callback				= UI_XQCharselMenu_Event;
	xqcharselMenuInfo.back.generic.x					= 0;
	xqcharselMenuInfo.back.generic.y					= 480-64;
	xqcharselMenuInfo.back.width						= 128;
	xqcharselMenuInfo.back.height						= 64;
	xqcharselMenuInfo.back.focuspic						= ART_BACK1;

	Menu_AddItem( &xqcharselMenuInfo.menu, &xqcharselMenuInfo.banner );
	for (int i = 0;  i < 8;  i++) {
		Menu_AddItem( &xqcharselMenuInfo.menu, &xqcharselMenuInfo.c[i] );
	}
	Menu_AddItem( &xqcharselMenuInfo.menu, &xqcharselMenuInfo.accept );
	if( uis.menusp ) {
		Menu_AddItem( &xqcharselMenuInfo.menu, &xqcharselMenuInfo.back );
	}

	// set the global xq_playable_models array with the playable model path, gender and race info
	// that is used by the char creator menu
	memset(xq_playable_models, 0, sizeof(xq_playable_models));
	if (strlen(models)) {
		tok1 = strtok(models, "^");
		while (tok1 != NULL) {
			int race, gender, facenum;
			char path[XQ_MODEL_MAX_PATH] = {0};
			int ret = sscanf(tok1, "%i.%i.%i.%s", &race, &gender, &facenum, path);
			if (strlen(path) > 0 && strlen(path) <= XQ_MODEL_MAX_PATH && ret == 4) {
				Q_strncpyz(xq_playable_models[race][gender].path, path, sizeof(xq_playable_models[race][gender].path));
				xq_playable_models[race][gender].numfaces = facenum;
//Com_Printf("Copying: [%s]\n", path);
			}
			tok1 = strtok(NULL, "^");
		}
	}

}

/*
=================
UI_XQCharsel_Cache
=================
*/
void UI_XQCharsel_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_ACCEPT0 );
	trap_R_RegisterShaderNoMip( ART_ACCEPT1 );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	trap_R_RegisterShaderNoMip( ART_FRAME );
}


/*
===============
UI_XQCharselMenu
===============
*/
void UI_XQCharselMenu( char *s ) {
	// remove trailing \n
	if (strlen(s) > 0) {
		if (s[strlen(s)-1] == '\n') {
			s[strlen(s)-1] = 0;
		}
	}
	if (s[1] == '0') {
		go_straight_to_charcreation = 0;
	} else if (s[1] == '1') {
		go_straight_to_charcreation = 1;
		s[1] = '0';
	}
	UI_XQCharselMenu_Init(s);
	UI_PushMenu(&xqcharselMenuInfo.menu);

	if (go_straight_to_charcreation) {
		go_straight_to_charcreation = 0;
		UI_XQCharCreatorMenu("Name unavailable");
	}
}
