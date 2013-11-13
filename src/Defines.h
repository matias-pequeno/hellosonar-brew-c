 

#ifdef _DEBUG
#define DEBUG_HTTP
#define DEBUG_STACK
//#define TURN_NETWORK_OFF
#define DEBUG_ISHELL_ERRORS
#endif


#ifdef DEBUG_ISHELL_ERRORS
#define	CHECK_RESUL(X,Y) (Y!=SUCCESS) ? DBGPRINTF("ISHELL ERROR in %s code: %d",X,Y) : 0
#else
#define	CHECK_RESUL(X,Y)
#endif

// GENERAL CONFIG

#define SONARID_MAXCHARS	40


#define WANTED_FPS			20

#define OBJARRAYSIZE		60
#define STRINGBUFFERSIZE	128


#define MAXCONTACTS			100
#define WEBSTRINGBUFFERSIZE	30 * 1024

#define MAXCALLLOG			20

#define MAXWIDGETS			20

#define MAX_ITEMS_CONTACT_DETAIL		15
#define MAX_ITEMS_PP_CONTACT_DETAIL		6

// FILE NAMES

#define FILE_FIELD_SEPARATOR	'|'
#define FILE_ROW_SEPARATOR		'\n'	

#define FILE_SETTINGS			"settings"
#define FILE_SETTINGS_LENGTH	512

#define FILE_CONTACT			"contact"
#define FILE_CONTACT_LENGTH		1024

#define FILE_CALLS				"calls"
#define FILE_CALLS_LENGTH		1024

#define FILE_WIDGETS			"widgets"
#define FILE_WIDGETS_LENGTH		1024

// FONT

#define FONT_NUMBER			9

typedef enum	_Font_Types	{GOTHAM_14_BLACK, 
							 GOTHAM_14_WHITE,
							 GOTHAM_18_WHITE,
							 GOTHAM_18_GRAY,
							 GOTHAM_18_BLACK,
							 GOTHAM_44_WHITE,
							 HELVETICA_11_GRAY,
							 GOTHAM_14_B_WHITE,
							 GOTHAM_14_B_BLACK} Font_Types;

typedef enum	_Obj_Types	{OBJ_STRING = 1, 
							 OBJ_STRING_NODEALLOC,
							 OBJ_STRING_AREA_NODEALLOC,
							 OBJ_IMAGE,
							 OBJ_IMAGE_NODEALLOC,
							 OBJ_PRIMITIVE_FILLRECT} ObjTypes;

typedef enum _Anim_Types	{ANIM_FROM_TOP,
							 ANIM_FROM_BOTTOM,
							 ANIM_FROM_LEFT,
							 ANIM_FROM_RIGHT,
							 ANIM_NONE} AnimTypes;

#define DONTCHANGE_VALUE	9999

// STATES

typedef enum	_States	{STATE_LOADING, 
						 STATE_MAINMENU,
						 STATE_CALLS,
						 STATE_ADRESSBOOK,
						 STATE_MESSAGES,
						 STATE_WIDGETS,
						 STATE_CALLS_MIN,
						 STATE_ADRESSBOOK_MIN,
						 STATE_MESSAGES_MIN,
						 STATE_WIDGETS_MIN,
						 STATE_CALLING,
						 STATE_CONTACTINFO,
						 STATE_OPTIONSMENU,
						 STATE_DEVICEOPTIONS,
						 STATE_POWEROPTIONS,
						 STATE_DEBUGSCR} States;

// SOFTKEYS

typedef enum	_SoftKeys{SOFTKEY_NONE, 
						  SOFTKEY_ACCEPT,
					 	  SOFTKEY_CANCEL,
						  SOFTKEY_BACK,
						  SOFTKEY_MAINMENU} SoftKeys;


//	THEMES

#define THEME_GREEN				MAKE_RGB(77,173,96)
#define THEME_RED				MAKE_RGB(221,24,4)
#define THEME_ORANGE			MAKE_RGB(255,102,00)
#define THEME_BLUE				MAKE_RGB(2,185,227)


// SEARCH BOX

#define SEARCH_STRING_MAX_CHARS		35
#define INPUTBOX_KEY_PRESS_MS		1500

// MAIN_MENU

#define MAINMENU_MAXICON	4
#define MAINMENU_ICONSPEED	10
#define MAINMENU_TEXTSPEED	25
#define MAINMENU_IDLETIME	3000


// MESSAGES

#define MESSAGE_BACKGROUNDSPEED		45



// MACROS
#define STR_STARTSWITH(X,Y) ((STRNCMP(X, Y, STRLEN(Y)))==0)


//#define CONN_URL						"http://staging.hellosonar.com"
#define CONN_REFRESH_INTERVAL_SECS			10
#define CONN_REFRESH_DELAY_BEFORE_START		20
#define CONN_MAX_STATES						10

// REQUESTS

typedef enum _Conn_Reqs	   {CONN_REQ_IDLE=1,
							CONN_REQ_CALL_RETRIEVE_ALL,
							CONN_REQ_CALL_RETRIEVE_TS,
							CONN_REQ_CALL_ACTIVITY,
							CONN_REQ_CONTACT_RETRIEVE_ALL,
							CONN_REQ_CONTACT_RETRIEVE_TS,
							CONN_REQ_CONTACT_RETRIEVE_DETAIL,
							CONN_REQ_CONTACT_REORDER_ALL,
							CONN_REQ_CONTACT_CHECK_CHANGES,
							CONN_REQ_CONTACT_UPDATE_PENDING,
							CONN_REQ_CONTACT_CHECK_PHOTOS,
							CONN_REQ_CONTACT_RETRIEVE_PHOTO,
							CONN_REQ_WIDGET_RETRIEVE_ALL,
							CONN_REQ_WIDGET_RETRIEVE_TS,
							CONN_REQ_WIDGET_RETRIEVE_DETAIL,
							CONN_REQ_WIDGET_REORDER_ALL,
							CONN_REQ_WIDGET_UPDATE_PENDING,
							CONN_REQ_WIDGET_CHECK_PHOTOS,
							CONN_REQ_WIDGET_RETRIEVE_PHOTO,
							CONN_REQ_SETTINGS_CHECK_CHANGES}Conn_Reqs;


// ADRESS BOOK


#define AB_NO_CONTACTS_PAGE					8
#define AB_NO_WIDGETS_PAGE					5


// CONTACTS

#define CONTACT_MAX_LBL_STRING				3

// FONT


#define FONT_USE_CLIP						4
#define FONT_ALIGN_TOP						0
#define FONT_ALIGN_LEFT						0
#define FONT_ALIGN_CENTER					1
#define FONT_ALIGN_RIGHT					2
#define FONT_ALIGN_BOTTOM					8
#define FONT_ALIGN_CURVED					16
#define FONT_ALIGN_VCENTER					32

// MD5

/* F, G and H are basic MD5 functions: selection, majority, parity */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z))) 

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }

// PRIVATE DEFINES

#define ID_MENUS_LINE_SEPARATOR_00			10
#define ID_MENUS_LINE_SEPARATOR_01			11
#define ID_MENUS_LINE_SEPARATOR_02			12
#define ID_MENUS_LINE_SEPARATOR_03			13
#define ID_MENUS_LINE_SEPARATOR_04			14
#define ID_MENUS_LINE_SEPARATOR_05			15
#define ID_MENUS_LINE_SEPARATOR_06			16
#define ID_MENUS_LINE_SEPARATOR_07			17
#define ID_MENUS_LINE_SEPARATOR_08			18
#define ID_MENUS_LINE_SEPARATOR_09			19

#define ID_MENUS_ICON_FAVORITE_00			100
#define ID_MENUS_ICON_FAVORITE_01			101
#define ID_MENUS_ICON_FAVORITE_02			102
#define ID_MENUS_ICON_FAVORITE_03			103
#define ID_MENUS_ICON_FAVORITE_04			104
#define ID_MENUS_ICON_FAVORITE_05			105
#define ID_MENUS_ICON_FAVORITE_06			106
#define ID_MENUS_ICON_FAVORITE_07			107
#define ID_MENUS_ICON_FAVORITE_08			108
#define ID_MENUS_ICON_FAVORITE_09			109

#define ID_MENUS_ICON_NOPHOTO_00			110
#define ID_MENUS_ICON_NOPHOTO_01			111
#define ID_MENUS_ICON_NOPHOTO_02			112
#define ID_MENUS_ICON_NOPHOTO_03			113
#define ID_MENUS_ICON_NOPHOTO_04			114
#define ID_MENUS_ICON_NOPHOTO_05			115
#define ID_MENUS_ICON_NOPHOTO_06			116
#define ID_MENUS_ICON_NOPHOTO_07			117
#define ID_MENUS_ICON_NOPHOTO_08			118
#define ID_MENUS_ICON_NOPHOTO_09			119

#define ID_MENUS_PHOTO_FRAME_00				120
#define ID_MENUS_PHOTO_FRAME_01				121
#define ID_MENUS_PHOTO_FRAME_02				122
#define ID_MENUS_PHOTO_FRAME_03				123
#define ID_MENUS_PHOTO_FRAME_04				124
#define ID_MENUS_PHOTO_FRAME_05				125
#define ID_MENUS_PHOTO_FRAME_06				126
#define ID_MENUS_PHOTO_FRAME_07				127
#define ID_MENUS_PHOTO_FRAME_08				128
#define ID_MENUS_PHOTO_FRAME_09				129

#define ID_MENUS_ITEM_NAME_00				210
#define ID_MENUS_ITEM_NAME_01				211
#define ID_MENUS_ITEM_NAME_02				212
#define ID_MENUS_ITEM_NAME_03				213
#define ID_MENUS_ITEM_NAME_04				214
#define ID_MENUS_ITEM_NAME_05				215
#define ID_MENUS_ITEM_NAME_06				216
#define ID_MENUS_ITEM_NAME_07				217
#define ID_MENUS_ITEM_NAME_08				218
#define ID_MENUS_ITEM_NAME_09				219

#define ID_MENUS_ITEM_DESCRIPTION_00		220
#define ID_MENUS_ITEM_DESCRIPTION_01		221
#define ID_MENUS_ITEM_DESCRIPTION_02		222
#define ID_MENUS_ITEM_DESCRIPTION_03		223
#define ID_MENUS_ITEM_DESCRIPTION_04		224
#define ID_MENUS_ITEM_DESCRIPTION_05		225
#define ID_MENUS_ITEM_DESCRIPTION_06		226
#define ID_MENUS_ITEM_DESCRIPTION_07		227
#define ID_MENUS_ITEM_DESCRIPTION_08		228
#define ID_MENUS_ITEM_DESCRIPTION_09		229

#define ID_MENUS_ITEM_DESCRIPTION_2_00		230
#define ID_MENUS_ITEM_DESCRIPTION_2_01		231
#define ID_MENUS_ITEM_DESCRIPTION_2_02		232
#define ID_MENUS_ITEM_DESCRIPTION_2_03		233
#define ID_MENUS_ITEM_DESCRIPTION_2_04		234
#define ID_MENUS_ITEM_DESCRIPTION_2_05		235
#define ID_MENUS_ITEM_DESCRIPTION_2_06		236
#define ID_MENUS_ITEM_DESCRIPTION_2_07		237
#define ID_MENUS_ITEM_DESCRIPTION_2_08		238
#define ID_MENUS_ITEM_DESCRIPTION_2_09		239

#define IDB_SELECTION_BOX_BASE				501
#define IDB_SELECTION_BOX_BASE_WHITE		502
#define IDB_SELECTION_BOX_BACKGROUND		503

#define ID_CD_CONTACT_DISPNAME				600
#define ID_CD_CONTACT_STATUS				601
#define ID_CD_CONTACT_FAVORITE				602
#define ID_CD_CONTACT_PHOTO 				603
#define ID_CD_SELECTION_BOX					604
#define ID_CD_CONTACT_FRAME 				605

#define ID_OPT_LINE_SEPARATOR_00			700
#define ID_OPT_LINE_SEPARATOR_01			701
#define ID_OPT_LINE_SEPARATOR_02			702
#define ID_OPT_LINE_SEPARATOR_03			703

