/*=============================================================================
FILE: HelloSonar.c
=============================================================================*/


/*-----------------------------------------------------------------------------
Includes and Variable Definitions
-----------------------------------------------------------------------------*/
#include "AEEModGen.h"          // Module interface definitions.
#include "AEEAppGen.h"          // Applet interface definitions.
#include "AEEShell.h"           // Shell interface definitions.  
#include "AEEStdLib.h"
#include "AEEAddrBook.h"
#include "AEETAPI.h" 
#include "AEEWeb.h"
#include "AEEfile.h"

#ifdef BREW_MP
#include "..\HelloSonar\HelloSonar.bid"
#include "..\HelloSonar\HelloSonar_res.h"
#else //BREW_MP
#include "..\HelloSonar.bid"
#include "..\HelloSonar_res.h"
#endif //BREW_MP

#include "Defines.h"

/* typedef a 32 bit type */
typedef unsigned long int UINT4;

/* Data structure for MD5 (Message Digest) computation */
typedef struct {
  UINT4 i[2];                   /* number of _bits_ handled mod 2^64 */
  UINT4 buf[4];                                    /* scratch buffer */
  unsigned char in[64];                              /* input buffer */
  unsigned char digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;

static unsigned char PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

typedef struct _Font
{
	int font_count;
	int *font_x;
	int *font_y;
	int *font_w;
	int *font_h;
	
	IImage *pFont;

}Font;

typedef struct _MoveableObj
{
	IImage *pImage;
	AEERect	*pClip;
	char	*pStr;

	int id;
	int x;
	int y;
	int destX;
	int destY;
	int speed;
	int width;
	int height;
	RGBVAL color;
	int align;
	Font_Types fontId;

	boolean visible;
	ObjTypes type;

}MoveableObj;

typedef struct _LblString
{
	char *pLabel;
	char *pString;
}LblString;

typedef struct _Contact
{
	char *pFirstName;
	char *pLastName;
	char *pDisplayName;
	char *pStatus;
	char *pCompany;
	char *pImageURL;

	char *pSearchName;

	IImage *pImage;

	LblString *pTelephone;
	LblString *pAddress;
	LblString *pEmail;

	int id;

	boolean favorite;
	boolean needUpdate;

}Contact;

enum callTypes {CALL_SENT=1,CALL_RECEIVED,CALL_VOICEMAIL,CALL_MISSED};

typedef struct _Call
{
	char type;

	Contact *pContact;

	LblString telephone;

	char *pFormatedDate;

	char *pHour;

}Call;


typedef struct _Widget
{
	int id;

	char	*pName;
	char	*pDescription;
	char	*pCategory;

	char	*pImageURL;
	IImage	*pImage;
	char	*pSearchName;

	boolean needUpdate;

}Widget;

typedef struct _ListNode
{
	void *pNode;
	struct ListNode *pPrev;
	struct ListNode *pNext;
}ListNode;

typedef  enum _ListTypes {LIST_NONE=1,LIST_CONTACTS,LIST_CALLS,LIST_MESSAGES,LIST_WIDGETS} ListTypes;
typedef struct _List
{
	ListNode *pFirstElement;
	ListNode *pLastElement;

	int cantNode;

}List;

typedef struct _InputBox
{
	boolean enable;
	char	*string;
	uint32	mLastKeyPress;
	char	lastChar;
	int		charPos;
}InputBox;

typedef struct _Menu
{
	int curPosition;
	int	firstItemPosition;

	int itemsPerPage;
	int maxItemsPerPage;
	void **filteredMenuList;
	int filteredMenuItems;

	ListNode	*pFirstItemPointer;
}Menu;

typedef enum _Button_Types	{BUTTON_NONE = 1, 
							 BUTTON_CALL,
							 BUTTON_EMAIL,
							 BUTTON_ADDRESS,
							 BUTTON_SEND_MSG,
							 BUTTON_EDIT,
							 BUTTON_DELETE} Button_Types;


typedef struct _Button
{
	boolean enabled;
	Button_Types type;

	char	*str1;
	char	*str2;
}Button;

typedef  enum _MenuDetailedTypes {MENU_CONTACT=1,MENU_CNCT_CALL,MENU_CALL} MenuDetailedTypes;
typedef struct _MenuDetailed
{
	MenuDetailedTypes subState;
	int menuPos;
	int menuFirstItemPos;

	Button *buttonArray;

	int buttonNumber;

	int offset;
}MenuDetailed;

/*-----------------------------------------------------------------------------
Applet Structure - Definition of the Applet Structure that's passed to Brew MP 
API functions. All variables in here are referenced via the applet structure 
pointer "pMe->", and will be able to be referenced as static.
-----------------------------------------------------------------------------*/
typedef struct _HelloSonar 
{
    AEEApplet		 applet;    // First element of this structure must be AEEApplet.
    IDisplay		*piDisplay; // Copy of IDisplay Interface pointer for easy access.
    IShell			*piShell;   // Copy of IShell Interface pointer for easy access.
    AEEDeviceInfo	deviceInfo; // Copy of device info for easy access.

	IWeb			*pIWeb;   // pointer to web object 
	IWebResp		*pIWebResponse;  // pointer to response 
	WebRespInfo		*pWebRespInfo;  // pointer to info about web response 
	ISource			*pISource;   // pointer to response stream object 
	AEECallback		WebCBStruct;  // structure for Iweb callback function 
	IFileMgr		*fmgr;

	ITAPI *pITAPI;

	char	*pWebURL;
	char	*pWebStringBuffer;
	char	*pSonarIdEncoded;
	char	*pSonarId;
	char	*pBaseURL;

	int webStringBufferLenght;
	Conn_Reqs serviceCalled;

	int nWidth;    // Stores the device screen width 
	int nHeight;   // Stores the device screen height 

	States nState;
	States nOldState;

	Conn_Reqs nConnState[CONN_MAX_STATES];

	int nLeftSoftKey;
	int nRightSoftKey;

	int nMainMenuCursorPos;

	uint32   mLastUpdate_Call;
	uint32   mLastUpdate_Contact;
	uint32   mLastUpdate_Widget;

	uint32	 mOldTime;
	uint32	 mLastKeyPressTime;

	MoveableObj objArray[OBJARRAYSIZE];
	int nObjArrayItems;

	List contactsList;
	List callsList;
	List widgetsList;

	char	*pCallingName;

	Font *pFont;

	ListNode *pNodeImgRequest;

	IImage *pImageCntNoPhoto;
	IImage *pImageCntPhotoFrame;
	IImage *pImageIconFavorite;
	IImage *pImageIconFavoriteSel;

	IImage *pCurrentBackground;

	IImage *pSearchIcon;

	InputBox *searchBox;

	int	inputBoxNumber;
	int currentInputBox;

	ListTypes	currentList;

	RGBVAL currentThemeColor;

	Menu *menu;

	Contact *pCurrentContact;

	MenuDetailed menuDetailed;

	ListNode	*pLastNode;

	boolean bDrawHour;

} HelloSonar;


/*-----------------------------------------------------------------------------
Function Prototypes
-----------------------------------------------------------------------------*/
static  boolean HelloSonar_HandleEvent(HelloSonar *pMe, AEEEvent eCode, uint16 wParam, uint32 dwParam);
boolean HelloSonar_InitAppData(HelloSonar *pMe);
void    HelloSonar_FreeAppData(HelloSonar *pMe);
static void HelloSonar_DrawScreen(HelloSonar *pMe);
static void drawHour(HelloSonar	*pMe);
static void DrawMainMenu(HelloSonar *pMe);
static void DrawAdressBook(HelloSonar *pMe);
static void DrawCall(HelloSonar *pMe);
static void DrawCalling(HelloSonar *pMe);
static void DrawWidgets(HelloSonar *pMe);
static void DrawMessages(HelloSonar *pMe);
static void drawDebugScreen(HelloSonar *pMe);

static boolean HandleKeyPress(HelloSonar *pMe, uint16 wParam);

void SetTimer(HelloSonar * pMe);


static void MakeCall(HelloSonar *pMe, Contact *contact, char *telephone);


static void paintSoftKeys(HelloSonar *pMe);
static boolean handleSoftKeys(HelloSonar *pMe, uint16 wParam);

static void setSoftKeys(HelloSonar *pMe, int left, int right);
static void setNextState(HelloSonar *pMe, States newState);

static void LoadWidgetData(HelloSonar *pMe);
static void FillWidgetData(HelloSonar *pMe);

static void LoadContactData(HelloSonar *pMe);

static void FillContactData(HelloSonar *pMe);

static void FillCalls(HelloSonar *pMe);

static void FillWidgets(HelloSonar *pMe);

static void initializeMenu(HelloSonar *pMe, ListTypes listType);
static void resetDeviceInfo(HelloSonar *pMe);
static void LoadMainMenu(HelloSonar *pMe);
static void LoadCalls(HelloSonar *pMe, boolean extended);
static void LoadAddressBook(HelloSonar *pMe, boolean extended);
static void LoadMessage(HelloSonar *pMe, boolean extended);
static void LoadCalling(HelloSonar *pMe);

static short processResourceId(short resId);
static void AddImage2Array(HelloSonar *pMe, short imageId, int x, int y);
static void AddString2Array(HelloSonar *pMe, Font_Types fontId, short stringId, int x, int y);
static void AddPrimitive2Array(HelloSonar *pMe, int id, int type, int x, int y, int width, int height, RGBVAL color);

static void AddImage2ArrayAnim(HelloSonar *pMe, short imageId, int x, int y, AnimTypes anim);
static void AddString2ArrayAnim(HelloSonar *pMe, int fontId, short stringId, int x, int y, AnimTypes anim);
static void AddPrimitive2ArrayAnim(HelloSonar *pMe, int id, int type, int x, int y, int width, int height, RGBVAL color, AnimTypes anim);

static int getObjArrayPos(HelloSonar *pMe, int Id);
static void ObjArraySetClip(HelloSonar *pMe, int Id, int x, int y, int width, int height);
static void ObjArraySetPosition(HelloSonar *pMe, int Id, int x, int y);
static void ObjArraySetDestination(HelloSonar *pMe, int Id, int destX, int destY, int speed);
static void ObjArraySetFont(HelloSonar *pMe, int Id, Font_Types fontId);
static void ObjArraySetVisible(HelloSonar *pMe, int Id, boolean visible);

static void UpdateAnims(HelloSonar *pMe);
static void ClearObjArray(HelloSonar *pMe);

static void UpdateLogic(HelloSonar *pMe);

void ReadFromWebCB(HelloSonar *pMe);
static uint32 getUTCFromSonarResponse(HelloSonar *pMe, char *buf, int byteCount);
static void ProcessXmlLine(HelloSonar *pMe, char *buf, int byteCount);
static ListNode * returnNewNode(HelloSonar *pMe, int listType);
static void AddContact2Array(HelloSonar *pMe, int contactId);
static void AddFullContact2Array(HelloSonar *pMe, Contact *newContact);
static void RemoveWidgetFromArray(HelloSonar *pMe, int widgetId);
static void RemoveCall(HelloSonar *pMe, ListNode *nodeCall);
static void RemoveContact(HelloSonar *pMe, int contactId);
static void ReorderWidgets(HelloSonar *pMe, char *line);
static void ReorderContacts(HelloSonar *pMe, char *line);
static void ProcessContactInfo(HelloSonar *pMe, char *line);
static void ProcessContactChanges(HelloSonar *pMe, char *line);
static void getHourFromTimeStamp(HelloSonar *pMe, char *line, char *output);
static void ProcessWidgetInfo(HelloSonar *pMe, char *line);
static void AddCall2Array(HelloSonar *pMe, Call *newCall, boolean removeOlder);
static void swapNodes(ListNode *a, ListNode *b);

static void RequestReorderedWidgets(HelloSonar *pMe);
static void RequestReorderedContacts(HelloSonar *pMe);

static void AddWidget2Array(HelloSonar *pMe, int widgetId);

static void ProcessCallInfo(HelloSonar *pMe, char *line);

static int returnTagPosition (char *buffer, char *tag);

static int returnTagText(char *buffer, char *tag, char *text);

static void UpdateContactInfo(HelloSonar *pMe, Contact *tmpContact);
static void UpdateWidgetInfo(HelloSonar *pMe, Widget  *tmpWidget);
static void cleanConnectionStates(HelloSonar *pMe);
static void UpdateConnectionState(HelloSonar *pMe);

static void GoToURL(HelloSonar *pMe, char *url);

static void CheckForUpdates(HelloSonar *pMe, Conn_Reqs serviceReq);

static void LoadWidgets(HelloSonar *pMe, boolean extended);

static void FillAddressBook(HelloSonar *pMe);

static void ChangeMenuPosition(HelloSonar *pMe, int offset); 
static void RequestImages(HelloSonar *pMe, ListTypes listType);
static ListNode * ReturnContactNode (HelloSonar *pMe, int contactId);
static ListNode * ReturnWidgetNode(HelloSonar *pMe, int widgetId);
static void getSonarId(HelloSonar *pMe);
static void putConnectionState(HelloSonar *pMe, int newConnState);
static int getActualConnectionState(HelloSonar *pMe);
static void removeActualConnectionState(HelloSonar *pMe);
static void appendBuffer2File(HelloSonar *pMe, IFile *pFile, char *buffer);
static void addField2Buffer(HelloSonar *pMe, char *buffer, char*data);
static int processContactData(HelloSonar *pMe, char *data);
static void loadContactDataFromDevice(HelloSonar *pMe);
static void saveContactData(HelloSonar *pMe);
static void saveSettings(HelloSonar *pMe);
static void loadSettings(HelloSonar *pMe);
static int extractNextField(HelloSonar *pMe, char *data, char **dest);
static void writeDataToFile(HelloSonar *pMe, char *data, char *fileName);
static void readDataFromFile(HelloSonar *pMe, char *data, char	*fileName);
static void encodeBase64 (char *input, char *output);
static void processImg(HelloSonar *pMe, char *data, int lenght);
static void loadFont(HelloSonar *pMe, Font_Types fontId);
static void drawString(HelloSonar *pMe, Font_Types fontId, char *string, int x, int y, int align);
static int getStringWidht(HelloSonar *pMe, Font_Types fontId, char *string);
static void drawStringInArea(HelloSonar *pMe, Font_Types fontId, char *string, int x, int y, int width, int height, int align);
static void checkUserSettings(HelloSonar *pMe);
static void ProcessSettingsChanges(HelloSonar *pMe, char *buf, int lenght);
static void changeWallpaper(HelloSonar *pMe, char *name);
static void CheckCallUpdates(HelloSonar *pMe);
static boolean handleSearchBox(HelloSonar *pMe, uint16 wParam);
static void searchBox_Init(HelloSonar *pMe, int number);
static void drawSearchBox(HelloSonar *pMe);
static void searchBox_AddCurrentChar(HelloSonar *pMe);
static void fillFilterList(HelloSonar *pMe, ListTypes typeList);
static boolean handleKey_MainMenu(HelloSonar* pMe, uint16 wParam);
static boolean handleKey_MenuOption(HelloSonar* pMe, uint16 wParam);
static boolean handleKey_MenuDetailed(HelloSonar* pMe, uint16 wParam);
static boolean handleKey_Options(HelloSonar* pMe, uint16 wParam);
static boolean handleKey_DebugScreen(HelloSonar* pMe, uint16 wParam);
static boolean handleKey_Menu(HelloSonar* pMe, uint16 wParam);
static void updateActiveMenu(HelloSonar *pMe, boolean filterChanged);
static void updateMenuDetailedObjects(HelloSonar *pMe);
static void updateMenuObjects(HelloSonar *pMe);
static void loadOptions(HelloSonar *pMe, States state);
static void updateOptionsObjects(HelloSonar *pMe);
static void loadDebugScreen(HelloSonar *pMe);
static void loadOptionsMenu(HelloSonar *pMe);
static void fillMenuOptions(HelloSonar *pMe);
static void updateMenuOption(HelloSonar* pMe);
static void loadContactInfoScreen(HelloSonar *pMe);
static void fillContactInfoScreen(HelloSonar *pMe);
static void MD5Init (MD5_CTX *mdContext);
static void MD5Update (MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen);
static void Transform (UINT4 *buf, UINT4 *in);
static void MD5Final (MD5_CTX *mdContext);

/*-----------------------------------------------------------------------------
Function Definitions
-----------------------------------------------------------------------------*/

const int TickTime = 1000 / WANTED_FPS;


/*=============================================================================
FUNCTION: AEEClsCreateInstance

DESCRIPTION:
    This function is invoked while the app is being loaded. All modules must 
    provide this function. Ensure to retain the same name and parameters for 
    this function. In here, the module must verify the ClassID and then invoke 
    the AEEApplet_New() function that has been provided in AEEAppGen.c. 

    After invoking AEEApplet_New(), this function can do app-specific 
    initialization. In this example, a generic structure is provided so that 
    app developers need not change the app-specific initialization section every 
    time, except for a call to IDisplay_InitAppData(). This is done as follows:
    InitAppData() is called to initialize the AppletData instance. It is the app 
    developer's responsibility to fill-in the app data initialization code of 
    InitAppData(). The app developer is also responsible to release memory 
    allocated for data contained in AppletData. This is done in 
    IDisplay_FreeAppData().

PROTOTYPE:
    int AEEClsCreateInstance(AEECLSID ClsId, IShell * piShell, IModule * piModule,
                            void ** ppObj)

PARAMETERS:
    clsID: [in]: Specifies the ClassID of the applet which is being loaded.

    piShell: [in]: Contains pointer to the IShell object. 

    piModule: [in]: Contains pointer to the IModule object to the current 
     module to which this app belongs.

    ppObj: [out]: On return, *ppObj must point to a valid IApplet structure. 
     Allocation of memory for this structure and initializing the base data 
     members is done by AEEApplet_New().

DEPENDENCIES:
    None

RETURN VALUE:
    AEE_SUCCESS: If this app needs to be loaded and if AEEApplet_New() 
     invocation was successful.
   
   AEE_EFAILED: If this app does not need to be loaded or if errors occurred in 
    AEEApplet_New(). If this function returns FALSE, this app will not load.

SIDE EFFECTS:
    None
=============================================================================*/
int AEEClsCreateInstance(AEECLSID ClsId, IShell *piShell, IModule *piModule, 
						 void ** ppObj)
{
    *ppObj = NULL;

    // Confirm this applet is the one intended to be created (classID matches):
    if( AEECLSID_HELLOSONAR == ClsId ) {
        // Create the applet and make room for the applet structure.
        // NOTE: FreeAppData is called after EVT_APP_STOP is sent to HandleEvent.
	    if( TRUE == AEEApplet_New(sizeof(HelloSonar),
                        ClsId,
                        piShell,
                        piModule,
                        (IApplet**)ppObj,
                        (AEEHANDLER)HelloSonar_HandleEvent,
                        (PFNFREEAPPDATA)HelloSonar_FreeAppData) ) {
                     		
            // Initialize applet data. This is called before EVT_APP_START is
            // sent to the HandleEvent function.
		    if(TRUE == HelloSonar_InitAppData((HelloSonar*)*ppObj)) {
			    return(AEE_SUCCESS); // Data initialized successfully.
		    }
		    else {
                // Release the applet. This will free the memory allocated for
                // the applet when AEEApplet_New was called.
                //AEEApplet_Release((IApplet*)*ppObj);
                return (EFAILED);
            }
        } // End AEEApplet_New
    }
    return(EFAILED);
}


/*=============================================================================
FUNCTION: HelloSonar_InitAppData

DESCRIPTION:
    This function is called when the application is starting up, so the 
	initialization and resource allocation code is executed here.

PROTOTYPE:
    boolean HelloSonar_InitAppData(HelloSonar * pMe)

PARAMETERS:
    pMe: Pointer to the AEEApplet structure. This structure contains information 
     specific to this applet. It was initialized during AEEClsCreateInstance().
  
DEPENDENCIES:
    None

RETURN VALUE:
    TRUE if there were no failures.

SIDE EFFECTS:
    None
=============================================================================*/
boolean HelloSonar_InitAppData(HelloSonar *pMe)
{
	int resul;
	int i;
	int len;

    // Save local copy for easy access:
    pMe->piDisplay = pMe->applet.m_pIDisplay;
    pMe->piShell   = pMe->applet.m_pIShell;

    // Get the device information for this handset.
    // Reference all the data by looking at the pMe->deviceInfo structure.
    // Check the API reference guide for all the handy device info you can get.
    pMe->deviceInfo.wStructSize = sizeof(AEEDeviceInfo);

    ISHELL_GetDeviceInfo(pMe->applet.m_pIShell,&pMe->deviceInfo);



	resul =  ISHELL_CreateInstance(pMe->piShell, AEECLSID_TAPI,(void **)&pMe->pITAPI);

	CHECK_RESUL("ISHELL_GetDeviceInfo",resul);
    
    // Insert your code here for initializing or allocating resources...
	pMe->nWidth  = pMe->deviceInfo.cxScreen;  // Cache the width of the device screen 
	pMe->nHeight = pMe->deviceInfo.cyScreen;  // Cache the height of the device screen 

	len	= STRLEN("http://staging.hellosonar.com") + 1;
	pMe->pBaseURL	 = MALLOC(sizeof(char)*len);
	STRCPY (pMe->pBaseURL, "http://staging.hellosonar.com");

	pMe->pWebStringBuffer = MALLOC(sizeof(char)*WEBSTRINGBUFFERSIZE);

	resul = ISHELL_CreateInstance(pMe->piShell, AEECLSID_WEB, (void **)&pMe->pIWeb);

	CHECK_RESUL("ISHELL_CreateInstance AEECLSID_WEB",resul);

	if (resul!= SUCCESS)
	{ 
		pMe->pIWeb = NULL; 
		return (FALSE);  
	} 
	else
	{ 
		CALLBACK_Init(&pMe->WebCBStruct, ReadFromWebCB, pMe);  // out, in, in 
	}

	ISHELL_CreateInstance(pMe->piShell, AEECLSID_FILEMGR, (void **)&pMe->fmgr);

	CHECK_RESUL("ISHELL_CreateInstance AEECLSID_FILEMGR",resul);

	if (resul!= SUCCESS)
	{ 
		pMe->fmgr = NULL; 
		return (FALSE);  
	} 

	loadSettings(pMe);

	pMe->pWebURL	= MALLOC(sizeof(char)*255);	

	pMe->contactsList.cantNode	= 0;
	pMe->widgetsList.cantNode	= 0;
	pMe->callsList.cantNode		= 0;

	for (i=0; i<CONN_MAX_STATES; i++)
		pMe->nConnState[i]	 = CONN_REQ_IDLE;

#ifndef TURN_NETWORK_OFF
	putConnectionState(pMe, CONN_REQ_CONTACT_RETRIEVE_ALL);
	putConnectionState(pMe, CONN_REQ_CALL_RETRIEVE_ALL);
	putConnectionState(pMe, CONN_REQ_WIDGET_RETRIEVE_ALL);
#endif

	pMe->pFont = MALLOC(sizeof(Font)*FONT_NUMBER);

	for (i=0; i<FONT_NUMBER; i++)
		loadFont(pMe, i);

	pMe->mLastUpdate_Call = GETUTCSECONDS() + CONN_REFRESH_DELAY_BEFORE_START;
	pMe->mLastUpdate_Contact = GETUTCSECONDS() + CONN_REFRESH_DELAY_BEFORE_START;
	pMe->mLastUpdate_Widget = GETUTCSECONDS() + CONN_REFRESH_DELAY_BEFORE_START;

	pMe->serviceCalled = CONN_REQ_IDLE;

	pMe->pImageCntNoPhoto		= ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, IDB_AB_NOPHOTO_5015);
	pMe->pImageCntPhotoFrame	= ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, IDB_AB_PHOTO_FRAME_5024);
	pMe->pImageIconFavorite		= ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, IDB_AB_FAVORITE_5014);
	pMe->pImageIconFavoriteSel	= ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, IDB_AB_FAVORITE_SEL_5025);

	pMe->pSearchIcon = ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, IDB_SEARCH_ICON_5018);

    return (TRUE);// No failures up to this point, so return success.
}


/*=============================================================================
FUNCTION: HelloSonar_FreeAppData

DESCRIPTION:
    This function is called when the application is exiting, so free the
	allocated resources, etc.

PROTOTYPE:
    void HelloSonar_HandleEvent(HelloSonar * pMe)

PARAMETERS:
    pMe: Pointer to the AEEApplet structure. This structure contains information 
     specific to this applet. It was initialized during AEEClsCreateInstance().
  
DEPENDENCIES:
    None

RETURN VALUE:
    None

SIDE EFFECTS:
    None
=============================================================================*/
void HelloSonar_FreeAppData(HelloSonar *pMe)
{
#ifdef DEBUG_STACK
	DBGPRINTF("HelloSonar_FreeAppData");
#endif
	
	CALLBACK_Cancel(&pMe->WebCBStruct); 
 
	if (pMe->pIWebResponse) 
	{ 
		IWEBRESP_Release(pMe->pIWebResponse); 
		pMe->pIWebResponse = NULL;
	} 
 
	if (pMe->pIWeb)  
	{ 
		IWEB_Release(pMe->pIWeb); 
	} 

	ClearObjArray(pMe);

	FREEIF(pMe->pWebURL);
}


/*=============================================================================
FUNCTION: HelloSonar_HandleEvent

DESCRIPTION:
    This is the EventHandler for this app. All events to this app are handled 
    in this function. All APPs must supply an Event Handler.

PROTOTYPE:
    static boolean HelloSonar_HandleEvent(IApplet * pMe, AEEEvent eCode, 
                                      uint16 wParam, uint32 dwParam)

PARAMETERS:
    pMe: Pointer to the AEEApplet structure. This structure contains information 
     specific to this applet. It was initialized during AEEClsCreateInstance().
  
    eCode: Specifies the Event sent to this applet

    wParam, dwParam: Event specific data.

DEPENDENCIES:
    None

RETURN VALUE:
    TRUE: If the app has processed the event.

    FALSE: If the app did not process the event.

SIDE EFFECTS:
    None
=============================================================================*/
static boolean HelloSonar_HandleEvent(HelloSonar *pMe, 
								AEEEvent eCode, uint16 wParam, uint32 dwParam)
{  
    switch (eCode) {
        // Event to inform app to start, so start-up code is here:
        case EVT_APP_START:
#ifdef DEBUG
			DBGPRINTF("EVT_APP_START");
#endif
			setNextState(pMe, STATE_MAINMENU);
			SetTimer(pMe);
            HelloSonar_DrawScreen(pMe); // Draw text on display screen.
            return(TRUE);         

        // Event to inform app to exit, so shut-down code is here:
        case EVT_APP_STOP:
      	    return(TRUE);

        // Event to inform app to suspend, so suspend code is here:
        case EVT_APP_SUSPEND:
      	    return(TRUE);

        // Event to inform app to resume, so resume code is here:
        case EVT_APP_RESUME:
            HelloSonar_DrawScreen(pMe); // Redraw text on display screen.
      	    return(TRUE);

        // An SMS message has arrived for this app. 
        // The Message is in the dwParam above as (char *).
        // sender simply uses this format "//BREW:ClassId:Message", 
        // example //BREW:0x00000001:Hello World
        case EVT_APP_MESSAGE:
      	    return(TRUE);

        // A key was pressed:
        case EVT_KEY:
      	    return HandleKeyPress(pMe, wParam);

        // Clamshell has opened/closed
        // wParam = TRUE if open, FALSE if closed
        case EVT_FLIP:
            return(TRUE);
      
	    // Clamtype device is closed and reexposed when opened, and LCD 
        // is blocked, or keys are locked by software. 
        // wParam = TRUE if keygaurd is on
        case EVT_KEYGUARD:
            return (TRUE);

        // If event wasn't handled here, then break out:
        default:
            break;
    }
    return (FALSE); // Event wasn't handled.
}


/*=============================================================================
FUNCTION: HelloSonar_DrawScreen

DESCRIPTION:
	Draw text in the middle of the display screen.

PROTOTYPE:
    static void HelloSonar_DrawScreen(HelloSonar * pMe)

PARAMETERS:
    pMe: Pointer to the AEEApplet structure. This structure contains information 
     specific to this applet. It was initialized during AEEClsCreateInstance().

DEPENDENCIES:
    None

RETURN VALUE:
    None

SIDE EFFECTS:
    None
=============================================================================*/
static void HelloSonar_DrawScreen(HelloSonar *pMe)
{
	int i;
	AEERect rectAux;

    IDISPLAY_ClearScreen(pMe->piDisplay);
	IIMAGE_Draw(pMe->pCurrentBackground, 0,0);

	if (pMe->bDrawHour == TRUE)
		drawHour(pMe);

	for (i=0; i<pMe->nObjArrayItems; i++)
	{
		if (pMe->objArray[i].visible==TRUE)
		{
			switch (pMe->objArray[i].type)
			{
			case OBJ_IMAGE:
			case OBJ_IMAGE_NODEALLOC:
				if (pMe->objArray[i].pImage != NULL)
					IIMAGE_Draw(pMe->objArray[i].pImage, pMe->objArray[i].x, pMe->objArray[i].y);
				break;
			case OBJ_PRIMITIVE_FILLRECT:
				rectAux.x = pMe->objArray[i].x;
				rectAux.y = pMe->objArray[i].y;
				rectAux.dx = pMe->objArray[i].width;
				rectAux.dy = pMe->objArray[i].height;

				IDISPLAY_FillRect(pMe->piDisplay, &rectAux, pMe->objArray[i].color);
				break;
			case OBJ_STRING:
			case OBJ_STRING_NODEALLOC:
				if (pMe->objArray[i].pStr != NULL)
				{
					drawString(pMe, pMe->objArray[i].fontId,  pMe->objArray[i].pStr,  pMe->objArray[i].x,  pMe->objArray[i].y, pMe->objArray[i].align);
				}
				break;
			case OBJ_STRING_AREA_NODEALLOC:
				if (pMe->objArray[i].pStr != NULL)
				{
					drawStringInArea(pMe, pMe->objArray[i].fontId,  pMe->objArray[i].pStr,  pMe->objArray[i].x,  pMe->objArray[i].y, 
										pMe->objArray[i].width, pMe->objArray[i].height, pMe->objArray[i].align);
				}
				break;
			}
		}
	}

	switch (pMe->nState) 
	{
        case STATE_MAINMENU:
			DrawMainMenu(pMe);
			break;
 /*       case STATE_ADRESSBOOK:
			DrawAdressBook(pMe);
			break;
		case STATE_CALLS:
			DrawCall(pMe);
			break;
		case STATE_CALLING:
			DrawCalling(pMe);
			break;
		case STATE_WIDGETS:
			DrawWidgets(pMe);
			break; 
		case STATE_MESSAGES:
			IDISPLAY_DrawText(pMe->piDisplay, AEE_FONT_BOLD, L"NO MESSAGES",
								-1, pMe->nWidth/2, pMe->nHeight/2, NULL,
								IDF_TEXT_TRANSPARENT | IDF_ALIGN_MIDDLE | IDF_ALIGN_CENTER); 
			break;*/
		case STATE_DEBUGSCR:
			drawDebugScreen(pMe);
			break;
	}

	paintSoftKeys(pMe);

	if (pMe->inputBoxNumber == 1 && pMe->searchBox[0].enable == TRUE)
	{
		drawSearchBox(pMe);

		if ((pMe->searchBox[0].charPos>0) && (pMe->menu->filteredMenuList[0] == NULL))
		{
			IDISPLAY_DrawText(pMe->piDisplay, AEE_FONT_BOLD, L"NO RESULTS",
								-1, pMe->nWidth/2, pMe->nHeight/2, NULL,
								IDF_TEXT_TRANSPARENT | IDF_ALIGN_MIDDLE | IDF_ALIGN_CENTER); 
		}
	}

    IDISPLAY_Update (pMe->piDisplay);
}

static void drawHour(HelloSonar	*pMe)
{
	int nTimeValues;
	JulianType date;
	IImage * piImage;

    AECHAR szBuf2[128];
	AECHAR szBuf3[128];

	char	buffer[128];

	int hour;

	nTimeValues=GETTIMESECONDS();
	GETJULIANDATE(nTimeValues,&date);

	hour = date.wHour;

	if (hour>12)
	{
		hour -= 12;
		piImage = ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, IDB_MAINMENU_PM_5020);
	}
	else
	{
		piImage = ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, IDB_MAINMENU_AM_5019);
	}

	SPRINTF(buffer, "%.2d:%.2d",hour,date.wMinute);

	drawString(pMe, GOTHAM_44_WHITE, buffer, pMe->nWidth/2 - 17, 55, FONT_ALIGN_CENTER);

	IIMAGE_Draw(piImage, (pMe->nWidth/2)  + (getStringWidht(pMe, GOTHAM_44_WHITE, buffer))/2 - 12 ,  67);
	IIMAGE_Release(piImage);

	ISHELL_LoadResString(pMe->piShell, HELLOSONAR_RES_FILE, (short)(
								   IDS_STRING_DAYS_1002+date.wWeekDay), szBuf2, 128);
	ISHELL_LoadResString(pMe->piShell, HELLOSONAR_RES_FILE,(short)(
								   IDS_STRING_MONTHS_SHORT_1009+date.wMonth-1), szBuf3, 128);
  
	SPRINTF(buffer, "%S, %d %S",szBuf2, date.wDay,szBuf3);

	drawString(pMe, GOTHAM_18_WHITE, buffer, pMe->nWidth/2, 93, FONT_ALIGN_CENTER);
}

static void DrawMainMenu(HelloSonar *pMe)
{
	IImage * piImage;  

	int aux;

	aux = getObjArrayPos(pMe, IDB_RIGHT_ARROW_5011);

	ObjArraySetPosition(pMe, IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos, DONTCHANGE_VALUE,pMe->objArray[aux].y+1);

	piImage = ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, (short)(IDB_ICON_5002+pMe->nMainMenuCursorPos+MAINMENU_MAXICON));
	
	aux = getObjArrayPos(pMe, IDB_ICON_5002+pMe->nMainMenuCursorPos);

	IIMAGE_Draw(piImage, pMe->objArray[aux].x,  pMe->objArray[aux].y);
	IIMAGE_Release(piImage);
}

static void drawDebugScreen(HelloSonar *pMe)
{
	int i;
	int y = 70;
	AEERect rectAux;

	drawString (pMe, GOTHAM_18_BLACK, "Sonar Id", 50, 50, 0);

	drawString (pMe, GOTHAM_18_BLACK, "Server URL", 50, 130, 0);

	for (i=0; i<(pMe->inputBoxNumber+2); i++)
	{
		if (i>=pMe->inputBoxNumber)
			y -= 40;

		rectAux.x  = 0;
		rectAux.y  = y;
		rectAux.dx = pMe->nWidth;
		rectAux.dy = 33;

		if (i==pMe->currentInputBox)
			IDISPLAY_FillRect(pMe->piDisplay, &rectAux, pMe->currentThemeColor );
		else
			IDISPLAY_FillRect(pMe->piDisplay, &rectAux, MAKE_RGB(204,204,204));

		if (i<pMe->inputBoxNumber)
		{
			if (pMe->searchBox[i].charPos!=0 || pMe->searchBox[i].lastChar != ' ')
			{
				if ( pMe->searchBox[i].lastChar!= ' ' && pMe->searchBox[i].lastChar >= 32 )
				{
					if (pMe->searchBox[i].charPos>0 && pMe->searchBox[i].string[pMe->searchBox[i].charPos-1]!= ' ')
						pMe->searchBox[i].string[pMe->searchBox[i].charPos]	= pMe->searchBox[i].lastChar + 32;
					else
						pMe->searchBox[i].string[pMe->searchBox[i].charPos]	= pMe->searchBox[i].lastChar;
				}
				pMe->searchBox[i].string[pMe->searchBox[i].charPos+1] = '\0';

				drawString(pMe, GOTHAM_14_BLACK, pMe->searchBox[i].string, 5, y + 12, 0);
			}
		}
		else if (i==pMe->inputBoxNumber)
		{
			drawString(pMe, GOTHAM_14_BLACK, "RESET INFO", pMe->nWidth/2, y + 12, FONT_ALIGN_CENTER);
		}
		else if (i==(pMe->inputBoxNumber+1))
		{
			drawString(pMe, GOTHAM_14_BLACK, "FORCE RESYNC", pMe->nWidth/2, y + 12, FONT_ALIGN_CENTER);
		}

		y += 80;
	}
}
/*
static void DrawAdressBook(HelloSonar *pMe)
{
	AECHAR szBuf[64] = {0};

	IAddrBook*		pIAddrBook = NULL;

    ISHELL_LoadResString(pMe->piShell, HELLOSONAR_RES_FILE, IDS_STRING_ICON_1022, szBuf, sizeof(szBuf));

    IDisplay_SetColor(pMe->piDisplay, CLR_USER_TEXT, RGB_BLACK);

    IDisplay_DrawText(pMe->piDisplay, // Display instance.
                        AEE_FONT_BOLD, // Use Bold font.
                        szBuf,         // String buffer containing text.
                        -1,     // Automatically compute string length.
                        0,      // x-coordinate ignored since IDF_ALIGN_CENTER.
                        5,      // y-coordinate ignored since IDF_ALIGN_MIDDLE.
                        NULL,   // No clipping.
                        IDF_ALIGN_CENTER | // Center-align horizontally.
						IDF_TEXT_TRANSPARENT); // Middle-align vertically.


	/*

	if( SUCCESS != (ISHELL_CreateInstance(pMe->piShell, AEECLSID_ADDRBOOK, (void**)&pIAddrBook)) )
	{
		return;
	}

/*
	// SIR_KU : This code is to retrieve al fields in the adressbook

	if(AEE_SUCCESS == IADDRBOOK_EnumFieldsInfoInit(pIAddrBook, AEE_ADDR_CAT_PERSONAL)) // initialize the enumeration operation 
	{ 
		AEEAddrFieldInfo afi; // stores enumerated field info 
 
		while(IADDRBOOK_EnumNextFieldsInfo(pIAddrBook, &afi)) // enumerate all supported fields 
		{ 
			// perform some action...
			DBGPRINTF("%d",	  afi.fieldID );

			  
		} 
	} */
/*
	if(AEE_SUCCESS == IADDRBOOK_EnumRecInit(pIAddrBook,AEE_ADDR_CAT_NONE,AEE_ADDRFIELD_NONE,NULL,0))
	{

		IAddrRec *	pIAddrRec;
			while((pIAddrRec = IADDRBOOK_EnumNextRec(pIAddrBook)) != NULL)
			{
				AEEAddrField * pIAddrField = IADDRREC_GetField(pIAddrRec, AEE_ADDRFIELD_NAME);

				DBGPRINTF("%s",    pIAddrField->pBuffer );

			}

	}
}
*/

static boolean handleSearchBox(HelloSonar *pMe, uint16 wParam)
{
	int i = pMe->currentInputBox;

	if (i>=pMe->inputBoxNumber)
		return FALSE;

	switch (wParam) 
	{
	case AVK_2:
		switch(pMe->searchBox[i].lastChar)
		{
		case 'A':
			pMe->searchBox[i].lastChar = 'B';
			break;
		case 'B':
			pMe->searchBox[i].lastChar = 'C';
			break;
		case 'C':
			pMe->searchBox[i].lastChar = 'A';
			break;
		default:
			if (pMe->searchBox[i].lastChar != ' ')
				searchBox_AddCurrentChar(pMe);
			pMe->searchBox[i].lastChar = 'A';
			break;
		}
		break;
	case AVK_3:
		switch(pMe->searchBox[i].lastChar)
		{
		case 'D':
			pMe->searchBox[i].lastChar = 'E';
			break;
		case 'E':
			pMe->searchBox[i].lastChar = 'F';
			break;
		case 'F':
			pMe->searchBox[i].lastChar = 'D';
			break;
		default:
			if (pMe->searchBox[i].lastChar != ' ')
				searchBox_AddCurrentChar(pMe);
			pMe->searchBox[i].lastChar = 'D';
			break;
		}
		break;
	case AVK_4:
		switch(pMe->searchBox[i].lastChar)
		{
		case 'G':
			pMe->searchBox[i].lastChar = 'H';
			break;
		case 'H':
			pMe->searchBox[i].lastChar = 'I';
			break;
		case 'I':
			pMe->searchBox[i].lastChar = 'G';
			break;
		default:
			if (pMe->searchBox[i].lastChar != ' ')
				searchBox_AddCurrentChar(pMe);
			pMe->searchBox[i].lastChar = 'G';
			break;
		}
		break;
	case AVK_5:
		switch(pMe->searchBox[i].lastChar)
		{
		case 'J':
			pMe->searchBox[i].lastChar = 'K';
			break;
		case 'K':
			pMe->searchBox[i].lastChar = 'L';
			break;
		case 'L':
			pMe->searchBox[i].lastChar = 'J';
			break;
		default:
			if (pMe->searchBox[i].lastChar != ' ')
				searchBox_AddCurrentChar(pMe);
			pMe->searchBox[i].lastChar = 'J';
			break;
		}
		break;
	case AVK_6:
		switch(pMe->searchBox[i].lastChar)
		{
		case 'M':
			pMe->searchBox[i].lastChar = 'N';
			break;
		case 'N':
			pMe->searchBox[i].lastChar = 'O';
			break;
		case 'O':
			pMe->searchBox[i].lastChar = 'M';
			break;
		default:
			if (pMe->searchBox[i].lastChar != ' ')
				searchBox_AddCurrentChar(pMe);
			pMe->searchBox[i].lastChar = 'M';
			break;
		}
		break;
	case AVK_7:
		switch(pMe->searchBox[i].lastChar)
		{
		case 'P':
			pMe->searchBox[i].lastChar = 'Q';
			break;
		case 'Q':
			pMe->searchBox[i].lastChar = 'R';
			break;
		case 'R':
			pMe->searchBox[i].lastChar = 'S';
			break;
		case 'S':
			pMe->searchBox[i].lastChar = 'P';
			break;
		default:
			if (pMe->searchBox[i].lastChar != ' ')
				searchBox_AddCurrentChar(pMe);
			pMe->searchBox[i].lastChar = 'P';
			break;
		}
		break;
	case AVK_8:
		switch(pMe->searchBox[i].lastChar)
		{
		case 'T':
			pMe->searchBox[i].lastChar = 'U';
			break;
		case 'U':
			pMe->searchBox[i].lastChar = 'V';
			break;
		case 'V':
			pMe->searchBox[i].lastChar = 'T';
			break;
		default:
			if (pMe->searchBox[i].lastChar != ' ')
				searchBox_AddCurrentChar(pMe);
			pMe->searchBox[i].lastChar = 'T';
			break;
		}
		break;
	case AVK_9:
		switch(pMe->searchBox[i].lastChar)
		{
		case 'W':
			pMe->searchBox[i].lastChar = 'X';
			break;
		case 'X':
			pMe->searchBox[i].lastChar = 'Y';
			break;
		case 'Y':
			pMe->searchBox[i].lastChar = 'Z';
			break;
		case 'Z':
			pMe->searchBox[i].lastChar = 'W';
			break;
		default:
			if (pMe->searchBox[i].lastChar != ' ')
				searchBox_AddCurrentChar(pMe);
			pMe->searchBox[i].lastChar = 'W';
			break;
		}
		break;
	case AVK_0:
		searchBox_AddCurrentChar(pMe);
		pMe->searchBox[i].lastChar = ' ';
		searchBox_AddCurrentChar(pMe);
		pMe->searchBox[i].lastChar = 0;
		break;
	case AVK_STAR:
		if (pMe->searchBox[i].lastChar != ' ')
			searchBox_AddCurrentChar(pMe);
		if (pMe->searchBox[i].charPos>0)
		{
			pMe->searchBox[i].charPos --;
			pMe->searchBox[i].string[pMe->searchBox[i].charPos] = '\0';

			if (pMe->searchBox[i].charPos == 0)
				pMe->menu->itemsPerPage ++;
		}
		pMe->searchBox[i].lastChar = 0;

		if (pMe->nState != STATE_DEBUGSCR)
			updateActiveMenu(pMe, TRUE);
		break;
	default:
		return FALSE;
	}

	pMe->searchBox[i].mLastKeyPress = GETTIMEMS();
	return TRUE;
}

static void searchBox_Init(HelloSonar *pMe, int number)
{
	int i;
	if(pMe->inputBoxNumber > 0)
	{
		for (i=0; i< pMe->inputBoxNumber; i++)
			FREEIF(pMe->searchBox[i].string);

		FREE(pMe->searchBox);
	}

	pMe->inputBoxNumber				= number;
	pMe->searchBox					= MALLOC(sizeof(InputBox)* number);

	for (i=0; i< pMe->inputBoxNumber; i++)
	{
		pMe->searchBox[i].enable		= TRUE;
		pMe->searchBox[i].lastChar		= ' ';
		pMe->searchBox[i].mLastKeyPress	= 0;
		pMe->searchBox[i].string		= MALLOC(sizeof(char) * SEARCH_STRING_MAX_CHARS);
		pMe->searchBox[i].string[0]		= '\0';
		pMe->searchBox[i].charPos		= 0;
	}
}

static void drawSearchBox(HelloSonar *pMe)
{
	AEERect rectAux;

	if (pMe->searchBox[0].charPos==0 && pMe->searchBox[0].lastChar == ' ')
		return;

	rectAux.x  = 0;
	rectAux.y  = pMe->nHeight - 33;
	rectAux.dx = pMe->nWidth;
	rectAux.dy = 33;

	IDISPLAY_FillRect(pMe->piDisplay, &rectAux, MAKE_RGB(204,204,204));

	rectAux.x  = 0;
	rectAux.y  = pMe->nHeight - 33;
	rectAux.dx = pMe->nWidth;
	rectAux.dy = 1;

	IDISPLAY_FillRect(pMe->piDisplay, &rectAux, MAKE_RGB(153,153,153));

	IIMAGE_Draw(pMe->pSearchIcon, 14, pMe->nHeight-27);

	if ( pMe->searchBox[0].lastChar!= ' ' && pMe->searchBox[0].lastChar >= 32 )
	{
		if (pMe->searchBox[0].charPos>0 && pMe->searchBox[0].string[pMe->searchBox[0].charPos-1]!= ' ')
			pMe->searchBox[0].string[pMe->searchBox[0].charPos]	= pMe->searchBox[0].lastChar + 32;
		else
			pMe->searchBox[0].string[pMe->searchBox[0].charPos]	= pMe->searchBox[0].lastChar;
	}
	pMe->searchBox[0].string[pMe->searchBox[0].charPos+1] = '\0';


	drawString (pMe, GOTHAM_18_GRAY, pMe->searchBox[0].string, 50, 292, 0);

}

static void searchBox_AddCurrentChar(HelloSonar *pMe)
{
	int i = pMe->currentInputBox;

	if (i>= pMe->inputBoxNumber)
		return;

	if ((pMe->searchBox[i].charPos==0 && pMe->searchBox[i].lastChar == ' ')  || pMe->searchBox[i].lastChar < 32)
		return;

	if (pMe->nState != STATE_DEBUGSCR && pMe->searchBox[i].charPos==0)
		pMe->menu->itemsPerPage --;

	if (pMe->searchBox[i].charPos>0 && pMe->searchBox[i].string[pMe->searchBox[i].charPos-1]!= ' ' && pMe->searchBox[i].lastChar!=' ')
		pMe->searchBox[i].string[pMe->searchBox[i].charPos]	= pMe->searchBox[i].lastChar + 32;
	else
		pMe->searchBox[i].string[pMe->searchBox[i].charPos]	= pMe->searchBox[i].lastChar;

	if (pMe->searchBox[i].charPos + 2 < SEARCH_STRING_MAX_CHARS)
	{
		pMe->searchBox[i].charPos ++;
		pMe->searchBox[i].string[pMe->searchBox[i].charPos]	= '\0';
	}

	pMe->searchBox[i].lastChar = 0;

	if (pMe->nState != STATE_DEBUGSCR)
		updateActiveMenu(pMe, TRUE);
}

static void updateActiveMenu(HelloSonar *pMe, boolean filterChanged)
{
	if (filterChanged == TRUE)
		fillFilterList(pMe, pMe->currentList);

	switch (pMe->nState)
	{
	case STATE_CALLS:
	case STATE_CALLS_MIN:
		FillCalls(pMe);
		break;
	case STATE_ADRESSBOOK:
	case STATE_ADRESSBOOK_MIN:
		FillAddressBook(pMe);
		break;
	case STATE_MESSAGES:
	case STATE_MESSAGES_MIN:
		break;
	case STATE_WIDGETS:
	case STATE_WIDGETS_MIN:
		FillWidgets(pMe);
		break;
	}
}

static boolean HandleKeyPress(HelloSonar *pMe, uint16 wParam)
{
	boolean resul = FALSE;

	if (handleSoftKeys(pMe, wParam)!= FALSE)
		return TRUE;

	if (pMe->searchBox != NULL && pMe->searchBox[pMe->currentInputBox].enable == TRUE)
		if (handleSearchBox(pMe, wParam)!= FALSE)
			return TRUE;

	pMe->mLastKeyPressTime = GETUPTIMEMS();

	switch (pMe->nState)
	{
	case STATE_MAINMENU:
		resul = handleKey_MainMenu(pMe, wParam);
		break;
	case STATE_CALLS:
	case STATE_ADRESSBOOK:
	case STATE_MESSAGES:
	case STATE_WIDGETS:
	case STATE_CALLS_MIN:
	case STATE_ADRESSBOOK_MIN:
	case STATE_MESSAGES_MIN:
	case STATE_WIDGETS_MIN:
		resul = handleKey_Menu(pMe, wParam);
		break;
	case STATE_CONTACTINFO:
		resul = handleKey_MenuDetailed(pMe, wParam);
		break;
	case STATE_OPTIONSMENU:
		resul = handleKey_MenuOption(pMe, wParam);
		break;
	case STATE_DEBUGSCR:
		resul = handleKey_DebugScreen(pMe, wParam);
		break;	
	case STATE_DEVICEOPTIONS:
	case STATE_POWEROPTIONS:
		resul = handleKey_Options(pMe, wParam);
		break;
	}

	return resul;
}

static boolean handleKey_MenuOption(HelloSonar* pMe, uint16 wParam)
{
	switch (wParam) 
	{
	case AVK_UP: 
		if (pMe->menuDetailed.menuPos > 0)
			pMe->menuDetailed.menuPos--;
		break;
	case AVK_DOWN: 
		if (pMe->menuDetailed.menuPos < pMe->menuDetailed.buttonNumber-1)
			pMe->menuDetailed.menuPos++;
		break;
	case AVK_SELECT:
		switch(pMe->menuDetailed.subState)
		{
		case MENU_CONTACT:
			if (pMe->menuDetailed.menuPos == 0)
				pMe->menuDetailed.subState = MENU_CNCT_CALL;
			break;
		case MENU_CNCT_CALL:
			MakeCall(pMe, pMe->pCurrentContact, pMe->menuDetailed.buttonArray[pMe->menuDetailed.menuPos].str2);
			break;
		case MENU_CALL:
			if (pMe->menuDetailed.menuPos == 0)
				pMe->menuDetailed.subState = MENU_CNCT_CALL;
			else if (pMe->menuDetailed.menuPos == 2)
			{
				pMe->menuDetailed.menuPos = 0;
				pMe->nState = pMe->nOldState;
				setNextState(pMe, STATE_CONTACTINFO);
				return TRUE;
			}
			break;
		default:
			return FALSE;
		}
		fillMenuOptions(pMe);
		break;
	default:
		return FALSE;
	}

	updateMenuOption(pMe);

	return TRUE;
}

static boolean handleKey_MenuDetailed(HelloSonar* pMe, uint16 wParam)
{
	int i;

	switch (wParam) 
	{
	case AVK_UP: 
		if(pMe->menuDetailed.buttonArray[pMe->menuDetailed.menuPos+pMe->menuDetailed.menuFirstItemPos].type == BUTTON_NONE)
			pMe->menuDetailed.menuPos -= 2;

		if (pMe->menuDetailed.menuPos > 0)
		{
			pMe->menuDetailed.menuPos--;
			if(pMe->menuDetailed.buttonArray[pMe->menuDetailed.menuPos+pMe->menuDetailed.menuFirstItemPos].type == BUTTON_NONE)
			{
				while (pMe->menuDetailed.buttonArray[pMe->menuDetailed.menuPos+pMe->menuDetailed.menuFirstItemPos].type != BUTTON_ADDRESS)
				{
					if (pMe->menuDetailed.menuPos > 0) 
						pMe->menuDetailed.menuPos--;
					else
						pMe->menuDetailed.menuFirstItemPos--;
				}
				pMe->menuDetailed.menuPos += 2;
			}
		}
		else if (pMe->menuDetailed.menuFirstItemPos > 0)
		{
			pMe->menuDetailed.menuFirstItemPos--;
			if(pMe->menuDetailed.buttonArray[pMe->menuDetailed.menuFirstItemPos].type == BUTTON_NONE)
			{
				pMe->menuDetailed.menuFirstItemPos -= 2;
				pMe->menuDetailed.menuPos += 2;
			}
		}
		break;
	case AVK_DOWN: 
		if (pMe->menuDetailed.menuPos < MAX_ITEMS_PP_CONTACT_DETAIL-1 && pMe->menuDetailed.menuPos < pMe->menuDetailed.buttonNumber-1) 
		{
			pMe->menuDetailed.menuPos++;
			if(pMe->menuDetailed.buttonArray[pMe->menuDetailed.menuPos+pMe->menuDetailed.menuFirstItemPos].type == BUTTON_ADDRESS)
			{
				if (pMe->menuDetailed.menuPos + 2 < MAX_ITEMS_PP_CONTACT_DETAIL-1)
					pMe->menuDetailed.menuPos += 2;
				else
				{
					for(i=0; i<2; i++)
					{
						if (pMe->menuDetailed.menuPos < MAX_ITEMS_PP_CONTACT_DETAIL-1) 
							pMe->menuDetailed.menuPos++;
						else
							pMe->menuDetailed.menuFirstItemPos++;
					}
				}
			}				
		}
		else if (pMe->menuDetailed.menuPos + pMe->menuDetailed.menuFirstItemPos < pMe->menuDetailed.buttonNumber -1)
		{
			 pMe->menuDetailed.menuFirstItemPos++;
			 if(pMe->menuDetailed.buttonArray[pMe->menuDetailed.menuPos+pMe->menuDetailed.menuFirstItemPos].type == BUTTON_ADDRESS)
				pMe->menuDetailed.menuFirstItemPos += 2;
		}
		while (pMe->menuDetailed.buttonArray[pMe->menuDetailed.menuFirstItemPos].type == BUTTON_NONE)
		{
			pMe->menuDetailed.menuFirstItemPos++;
			pMe->menuDetailed.menuPos--;
		}
		break;
	case AVK_SELECT:
		if(pMe->menuDetailed.buttonArray[pMe->menuDetailed.menuPos+pMe->menuDetailed.menuFirstItemPos].type == BUTTON_CALL)
			MakeCall(pMe, pMe->pCurrentContact, pMe->menuDetailed.buttonArray[pMe->menuDetailed.menuPos+pMe->menuDetailed.menuFirstItemPos].str2);
		break;
	default:
		return FALSE;
	}

	fillContactInfoScreen (pMe);

	return TRUE;
}

static boolean handleKey_Options(HelloSonar* pMe, uint16 wParam)
{
	switch (wParam) 
	{
	case AVK_UP: 
		if (pMe->nMainMenuCursorPos>0)
		{
			pMe->nMainMenuCursorPos--;
			updateOptionsObjects(pMe);
		}
		break;
	case AVK_DOWN:
		if (pMe->nMainMenuCursorPos<3)
		{
			pMe->nMainMenuCursorPos++;
			updateOptionsObjects(pMe);
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

static boolean handleKey_DebugScreen(HelloSonar* pMe, uint16 wParam)
{
	int len;
	switch (wParam) 
	{
	case AVK_UP: 
		if (pMe->currentInputBox>0)
		{
			pMe->currentInputBox--;
		}
		break;
	case AVK_DOWN:
		if (pMe->currentInputBox<(pMe->inputBoxNumber -1 + 2))
		{
			pMe->currentInputBox++;
		}
		break;
	case AVK_SELECT:
		if  (pMe->currentInputBox==pMe->inputBoxNumber)
			resetDeviceInfo(pMe);
		else if (pMe->currentInputBox==(pMe->inputBoxNumber+1))
		{
			if (pMe->contactsList.cantNode==0)
				putConnectionState(pMe, CONN_REQ_CONTACT_RETRIEVE_ALL);

			if (pMe->callsList.cantNode==0)
				putConnectionState(pMe, CONN_REQ_CALL_RETRIEVE_ALL);

			if (pMe->widgetsList.cantNode==0)
				putConnectionState(pMe, CONN_REQ_WIDGET_RETRIEVE_ALL);
		}
		break;
	case AVK_SOFT2:
		if (STRCMP(pMe->pSonarId, pMe->searchBox[0].string) != 0)
		{
			FREE(pMe->pSonarId);
			len = STRLEN (pMe->searchBox[0].string) + 1;
			pMe->pSonarId = MALLOC(sizeof(char) * len);
			STRCPY(pMe->pSonarId, pMe->searchBox[0].string);

			getSonarId(pMe);

			saveSettings(pMe);

			resetDeviceInfo(pMe);

			putConnectionState(pMe, CONN_REQ_CONTACT_RETRIEVE_ALL);
			putConnectionState(pMe, CONN_REQ_CALL_RETRIEVE_ALL);
			putConnectionState(pMe, CONN_REQ_WIDGET_RETRIEVE_ALL);
		}
		if (STRCMP(pMe->pBaseURL, pMe->searchBox[1].string) != 0)
		{
			FREE(pMe->pBaseURL);
			len = STRLEN (pMe->searchBox[1].string) + 1;
			pMe->pBaseURL = MALLOC(sizeof(char) * len);
			STRCPY(pMe->pBaseURL, pMe->searchBox[1].string);
		}
		setNextState(pMe, STATE_MAINMENU);
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

static boolean handleKey_Menu(HelloSonar* pMe, uint16 wParam)
{
	List *listAux;
	ListNode *nodeAux;
	int i;

	switch (pMe->currentList)
	{
	case LIST_CALLS:
		listAux = &(pMe->callsList);
		break;
	case LIST_CONTACTS:
		listAux = &(pMe->contactsList);
		break;
	case LIST_MESSAGES:
	//	listAux = &(pMe->);
		break;
	case LIST_WIDGETS:
		listAux = &(pMe->widgetsList);
		break;
	}


	if (pMe->currentList == LIST_NONE )
	{
		if( wParam == AVK_UP )
			setNextState(pMe, STATE_MAINMENU);
		else 
			return FALSE;

		return TRUE;
	}

	if( listAux->cantNode==0)
	{
		if( wParam == AVK_UP )
			setNextState(pMe, STATE_MAINMENU);
		else 
			return FALSE;


		return TRUE;
	}

	switch (wParam) 
	{
	case AVK_UP: 
		if (pMe->menu->curPosition>0)
		{
			pMe->menu->curPosition--;
			updateMenuObjects(pMe);
		}
		else if (pMe->menu->firstItemPosition>0)
		{
			ChangeMenuPosition(pMe, -1);
			updateActiveMenu(pMe, FALSE);
		}
		else if (pMe->nState >= STATE_CALLS_MIN && pMe->nState <= STATE_WIDGETS_MIN)
		{
			setNextState(pMe, STATE_MAINMENU);
		}
		else if (pMe->menu->firstItemPosition == 0 )
		{
			setNextState(pMe, STATE_MAINMENU);
		}
		break;
	case AVK_DOWN:
		if(pMe->searchBox[0].charPos>0) 
		{	
			if ((pMe->menu->curPosition<(pMe->menu->itemsPerPage-1)) && (pMe->menu->curPosition<pMe->menu->filteredMenuItems-1))
			{
				pMe->menu->curPosition++;
				updateMenuObjects(pMe);
			}
			else if (pMe->menu->firstItemPosition+(pMe->menu->itemsPerPage)<pMe->menu->filteredMenuItems)
			{
				ChangeMenuPosition(pMe, 1);
				updateActiveMenu(pMe, FALSE);
			}
		}
		else 
		{
			if (pMe->menu->curPosition<(pMe->menu->itemsPerPage-1))
			{
				pMe->menu->curPosition++;
				updateMenuObjects(pMe);
			}
			else if (pMe->menu->firstItemPosition+(pMe->menu->itemsPerPage)<listAux->cantNode)
			{
				ChangeMenuPosition(pMe, 1);
				updateActiveMenu(pMe, FALSE);
			}
		}

		if (pMe->menu->itemsPerPage < pMe->menu->maxItemsPerPage && pMe->menu->curPosition==(pMe->menu->itemsPerPage-1))
		{
			pMe->menu->itemsPerPage ++;
			if (pMe->menu->itemsPerPage == pMe->menu->maxItemsPerPage && pMe->nState >= STATE_CALLS_MIN)
			{
				pMe->nState -= 4;
				pMe->bDrawHour = FALSE;
			}
			updateActiveMenu(pMe, FALSE);
		}
		break;
	case AVK_SELECT:
		switch (pMe->nState)
		{
		case STATE_CALLS:
		case STATE_CALLS_MIN:
			if (pMe->searchBox[0].charPos > 0)
			{
				pMe->pLastNode = ((ListNode *) pMe->menu->filteredMenuList[pMe->menu->firstItemPosition + pMe->menu->curPosition]);
			}
			else
			{
				if (pMe->callsList.cantNode==0)
					return FALSE;

				nodeAux = pMe->menu->pFirstItemPointer;
				for (i=0; i<pMe->menu->curPosition; i++)
			 		nodeAux = (ListNode *) nodeAux->pNext;
			
				pMe->pLastNode = nodeAux;
			}

			pMe->pCurrentContact = ((Call *) pMe->pLastNode->pNode)->pContact;

			setNextState(pMe, STATE_CONTACTINFO);
			break;
		case STATE_ADRESSBOOK:
		case STATE_ADRESSBOOK_MIN:
			if (pMe->searchBox[0].charPos > 0)
			{
				pMe->pLastNode = ((ListNode *) pMe->menu->filteredMenuList[pMe->menu->firstItemPosition + pMe->menu->curPosition]);
			}
			else
			{
				if (pMe->contactsList.cantNode==0)
					return FALSE;

				nodeAux = pMe->menu->pFirstItemPointer;
				for (i=0; i<pMe->menu->curPosition; i++)
			 		nodeAux = (ListNode *) nodeAux->pNext;
			
				pMe->pLastNode = nodeAux;
			}

			pMe->pCurrentContact = ((Contact *) pMe->pLastNode->pNode);

			setNextState(pMe, STATE_CONTACTINFO);
			break;
		case STATE_MESSAGES:
			return FALSE;
			break;
		case STATE_WIDGETS:
		case STATE_WIDGETS_MIN:
			return FALSE;
			break;
		default:
			return FALSE;
			break;
		}
		break;
	case AVK_SOFT2:
		switch (pMe->nState)
		{
		case STATE_CALLS:
		case STATE_CALLS_MIN:
			if (pMe->searchBox[0].charPos > 0)
			{
				pMe->pLastNode = ((ListNode *) pMe->menu->filteredMenuList[pMe->menu->firstItemPosition + pMe->menu->curPosition]);
			}
			else
			{
				nodeAux = pMe->menu->pFirstItemPointer;
				for (i=0; i<pMe->menu->curPosition; i++)
			 		nodeAux = (ListNode *) nodeAux->pNext;
			
				pMe->pLastNode = nodeAux;
			}

			pMe->pCurrentContact = ((Call *) pMe->pLastNode->pNode)->pContact;

			setNextState(pMe, STATE_OPTIONSMENU);
			break;
		case STATE_ADRESSBOOK:
		case STATE_ADRESSBOOK_MIN:
			if (pMe->searchBox[0].charPos > 0)
			{
				pMe->pLastNode = ((ListNode *) pMe->menu->filteredMenuList[pMe->menu->firstItemPosition + pMe->menu->curPosition]);
			}
			else
			{
				nodeAux = pMe->menu->pFirstItemPointer;
				for (i=0; i<pMe->menu->curPosition; i++)
			 		nodeAux = (ListNode *) nodeAux->pNext;
			
				pMe->pLastNode = nodeAux;
			}

			pMe->pCurrentContact = ((Contact *) pMe->pLastNode->pNode);

			setNextState(pMe, STATE_OPTIONSMENU);
			break;
		case STATE_MESSAGES:
			return FALSE;
			break;
		case STATE_WIDGETS:
		case STATE_WIDGETS_MIN:
			return FALSE;
			break;
		default:
			return FALSE;
			break;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

static void updateMenuDetailedObjects(HelloSonar *pMe)
{
	int aux;
	int aux2;
	int i;
	int iOffset = 0;

	aux = getObjArrayPos(pMe, IDB_SELECTION_BOX_BASE);
	if (pMe->menuDetailed.buttonArray[pMe->menuDetailed.menuPos+pMe->menuDetailed.menuFirstItemPos].type != BUTTON_NONE)
	{
		pMe->objArray[aux].height = 34;
		ObjArraySetDestination(pMe, IDB_SELECTION_BOX_BASE, 0, 122 + (33*pMe->menuDetailed.menuPos), MESSAGE_BACKGROUNDSPEED);
		ObjArraySetDestination(pMe, ID_CD_SELECTION_BOX, 0, 122 + (33*pMe->menuDetailed.menuPos), MESSAGE_BACKGROUNDSPEED);

		aux2 = getObjArrayPos(pMe, IDB_SELECTION_SMALL_BOX_5022);
	}
	else
	{
		pMe->objArray[aux].height = 99;
		ObjArraySetDestination(pMe, IDB_SELECTION_BOX_BASE, 0, 122 + (33*(pMe->menuDetailed.menuPos-2)), MESSAGE_BACKGROUNDSPEED);
		ObjArraySetDestination(pMe, ID_CD_SELECTION_BOX, 0, 122 + (33*(pMe->menuDetailed.menuPos-2)), MESSAGE_BACKGROUNDSPEED);
		aux2 = getObjArrayPos(pMe, IDB_SELECTION_BIG_BOX_5023);
		iOffset = 2;
	}

	aux	= getObjArrayPos(pMe, ID_CD_SELECTION_BOX);
	pMe->objArray[aux].pImage = pMe->objArray[aux2].pImage;

	for (i=0; i < MAX_ITEMS_PP_CONTACT_DETAIL; i++)
	{
		if (i == (pMe->menuDetailed.menuPos-iOffset))
		{
			aux = getObjArrayPos (pMe, ID_MENUS_ITEM_DESCRIPTION_00+i);
			if (pMe->objArray[aux].pStr != NULL)
				pMe->objArray[aux].fontId = GOTHAM_14_WHITE;
			else
				ObjArraySetFont(pMe, ID_MENUS_ITEM_NAME_00+i, GOTHAM_18_WHITE);
		}
		else
		{
			aux = getObjArrayPos (pMe, ID_MENUS_ITEM_DESCRIPTION_00+i);
			if (pMe->objArray[aux].pStr != NULL)
				pMe->objArray[aux].fontId = GOTHAM_14_BLACK;
			else
				ObjArraySetFont(pMe, ID_MENUS_ITEM_NAME_00+i, GOTHAM_18_BLACK);
		}
	}
}

static void updateMenuObjects(HelloSonar *pMe)
{
	int i;
	int y;
	int aux;

	switch(pMe->currentList)
	{
	case LIST_CALLS:
	case LIST_CONTACTS:
		y = 31 + ((AB_NO_CONTACTS_PAGE - pMe->menu->itemsPerPage) * 31);

		ObjArraySetDestination(pMe, IDB_UPPER_SHADOW_5021, 0, y-43, MESSAGE_BACKGROUNDSPEED);

		ObjArraySetDestination(pMe, IDB_SELECTION_BOX_5013, 0, y + (32*pMe->menu->curPosition), MESSAGE_BACKGROUNDSPEED);
		ObjArraySetDestination(pMe, IDB_SELECTION_BOX_BASE, 0, y + (32*pMe->menu->curPosition), MESSAGE_BACKGROUNDSPEED);

		ObjArraySetDestination(pMe, IDB_UPPER_BAR_5017, DONTCHANGE_VALUE, y-30, MESSAGE_BACKGROUNDSPEED);
		ObjArraySetDestination(pMe, IDB_SELECTION_BOX_BACKGROUND, DONTCHANGE_VALUE, y-1, MESSAGE_BACKGROUNDSPEED);
		ObjArraySetDestination(pMe, IDS_STRING_ICON_1021, DONTCHANGE_VALUE, y-22, MESSAGE_BACKGROUNDSPEED);
		ObjArraySetDestination(pMe, IDS_STRING_ICON_1022, DONTCHANGE_VALUE, y-22, MESSAGE_BACKGROUNDSPEED);
		ObjArraySetDestination(pMe, IDS_STRING_ICON_1023, DONTCHANGE_VALUE, y-22, MESSAGE_BACKGROUNDSPEED);
		ObjArraySetDestination(pMe, IDS_STRING_ICON_1024, DONTCHANGE_VALUE, y-22, MESSAGE_BACKGROUNDSPEED);

		for (i=0; i<AB_NO_CONTACTS_PAGE; i++)
		{
			if (i<pMe->menu->itemsPerPage)
			{
				ObjArraySetDestination(pMe, ID_MENUS_ITEM_NAME_00+i, DONTCHANGE_VALUE, y + 7, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetDestination(pMe, ID_MENUS_ICON_FAVORITE_00+i, DONTCHANGE_VALUE, y + 10, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetDestination(pMe, ID_MENUS_LINE_SEPARATOR_00+i, DONTCHANGE_VALUE, y, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetDestination(pMe, ID_MENUS_ICON_NOPHOTO_00+i, DONTCHANGE_VALUE, y + 9, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetDestination(pMe, ID_MENUS_PHOTO_FRAME_00+i, DONTCHANGE_VALUE, y + 4, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetDestination(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i, DONTCHANGE_VALUE, y + 37, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetDestination(pMe, ID_MENUS_ITEM_DESCRIPTION_2_00+i, DONTCHANGE_VALUE, y + 37, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetVisible(pMe, ID_MENUS_LINE_SEPARATOR_00+i,TRUE);

				if (i == pMe->menu->curPosition)
				{
					ObjArraySetFont(pMe, ID_MENUS_ITEM_NAME_00+i, GOTHAM_18_WHITE);
					ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i,TRUE);
					ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_2_00+i,TRUE);
					ObjArraySetVisible(pMe, ID_MENUS_ICON_NOPHOTO_00+i,TRUE);
					ObjArraySetVisible(pMe, ID_MENUS_PHOTO_FRAME_00+i,TRUE);
					y += 60;
					if (pMe->currentList == LIST_CONTACTS)
					{
						aux = getObjArrayPos(pMe, ID_MENUS_ICON_FAVORITE_00+i);
						pMe->objArray[aux].pImage = pMe->pImageIconFavoriteSel;
					}	
				}
				else
				{
					ObjArraySetFont(pMe, ID_MENUS_ITEM_NAME_00+i, GOTHAM_18_BLACK);
					ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i,FALSE);
					ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_2_00+i,FALSE);
					ObjArraySetVisible(pMe, ID_MENUS_ICON_NOPHOTO_00+i,FALSE);
					ObjArraySetVisible(pMe, ID_MENUS_PHOTO_FRAME_00+i,FALSE);
					y +=32;
					if (pMe->currentList == LIST_CONTACTS)
					{
						aux = getObjArrayPos(pMe, ID_MENUS_ICON_FAVORITE_00+i);
						pMe->objArray[aux].pImage = pMe->pImageIconFavorite;
					}
				}
			}
			else
			{
				ObjArraySetVisible(pMe, ID_MENUS_ITEM_NAME_00+i,FALSE);
				ObjArraySetVisible(pMe, ID_MENUS_ICON_FAVORITE_00+i,FALSE);
				ObjArraySetVisible(pMe, ID_MENUS_ICON_NOPHOTO_00+i,FALSE);
				ObjArraySetVisible(pMe, ID_MENUS_PHOTO_FRAME_00+i,FALSE);
				ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i,FALSE);
				ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_2_00+i,FALSE);
				ObjArraySetVisible(pMe, ID_MENUS_LINE_SEPARATOR_00+i,FALSE);
			}
		}
		break;
	case LIST_WIDGETS:
		y = 31 + ((AB_NO_WIDGETS_PAGE - pMe->menu->itemsPerPage) * 60);

		ObjArraySetDestination(pMe, IDB_UPPER_SHADOW_5021, 0, y-43, MESSAGE_BACKGROUNDSPEED);

		ObjArraySetDestination(pMe, IDB_SELECTION_BOX_5013, 0, y + (60*pMe->menu->curPosition), MESSAGE_BACKGROUNDSPEED);
		ObjArraySetDestination(pMe, IDB_SELECTION_BOX_BASE, 0, y + (60*pMe->menu->curPosition), MESSAGE_BACKGROUNDSPEED);

		ObjArraySetDestination(pMe, IDB_UPPER_BAR_5017, DONTCHANGE_VALUE, y-30, MESSAGE_BACKGROUNDSPEED);
		ObjArraySetDestination(pMe, IDB_SELECTION_BOX_BACKGROUND, DONTCHANGE_VALUE, y-1, MESSAGE_BACKGROUNDSPEED);

		ObjArraySetDestination(pMe, IDS_STRING_ICON_1024, DONTCHANGE_VALUE, y-22, MESSAGE_BACKGROUNDSPEED);


		for (i=0; i<AB_NO_CONTACTS_PAGE; i++)
		{
			if (i<pMe->menu->itemsPerPage)
			{
				ObjArraySetDestination(pMe, ID_MENUS_ITEM_NAME_00+i, DONTCHANGE_VALUE, y + 7, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetDestination(pMe, ID_MENUS_ICON_FAVORITE_00+i, DONTCHANGE_VALUE, y + 10, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetDestination(pMe, ID_MENUS_LINE_SEPARATOR_00+i, DONTCHANGE_VALUE, y, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetDestination(pMe, ID_MENUS_ICON_NOPHOTO_00+i, DONTCHANGE_VALUE, y + 9, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetDestination(pMe, ID_MENUS_PHOTO_FRAME_00+i, DONTCHANGE_VALUE, y + 4, MESSAGE_BACKGROUNDSPEED);
				ObjArraySetDestination(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i, DONTCHANGE_VALUE, y + 32, MESSAGE_BACKGROUNDSPEED);

				if (i == pMe->menu->curPosition)
					ObjArraySetFont(pMe, ID_MENUS_ITEM_NAME_00+i, GOTHAM_18_WHITE);
				else
					ObjArraySetFont(pMe, ID_MENUS_ITEM_NAME_00+i, GOTHAM_18_BLACK);

				ObjArraySetVisible(pMe, ID_MENUS_ITEM_NAME_00+i,TRUE);
				ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i,TRUE);
				ObjArraySetVisible(pMe, ID_MENUS_ICON_NOPHOTO_00+i,TRUE);
				ObjArraySetVisible(pMe, ID_MENUS_PHOTO_FRAME_00+i,TRUE);
				ObjArraySetVisible(pMe, ID_MENUS_LINE_SEPARATOR_00+i,TRUE);
			}
			else
			{
				ObjArraySetVisible(pMe, ID_MENUS_ITEM_NAME_00+i,FALSE);
				ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i,FALSE);
				ObjArraySetVisible(pMe, ID_MENUS_ICON_NOPHOTO_00+i,FALSE);
				ObjArraySetVisible(pMe, ID_MENUS_PHOTO_FRAME_00+i,FALSE);
				ObjArraySetVisible(pMe, ID_MENUS_LINE_SEPARATOR_00+i,FALSE);
			}
			y += 60;
		}
		break;
	}

}

static boolean handleKey_MainMenu(HelloSonar* pMe, uint16 wParam)
{
	int x = 9;
	int i;

	switch (wParam) 
	{
	case AVK_LEFT: 
		if ( pMe->nMainMenuCursorPos >0)
		{
			ObjArraySetVisible(pMe, IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,FALSE);
			pMe->nMainMenuCursorPos --;
			ObjArraySetVisible(pMe, IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,TRUE);
			ObjArraySetPosition(pMe,IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,0,pMe->nHeight);
			ObjArraySetDestination(pMe,IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,(pMe->nWidth/2),pMe->nHeight,MAINMENU_TEXTSPEED);
		}
		else
		{
			ObjArraySetVisible(pMe, IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,FALSE);
			pMe->nMainMenuCursorPos = MAINMENU_MAXICON-1;
			ObjArraySetVisible(pMe, IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,TRUE);
			ObjArraySetPosition(pMe,IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,0,pMe->nHeight);
			ObjArraySetDestination(pMe,IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,(pMe->nWidth/2),pMe->nHeight,MAINMENU_TEXTSPEED);
		}
		break;
	case AVK_RIGHT: 		
		if ( pMe->nMainMenuCursorPos < (MAINMENU_MAXICON-1))
		{
			ObjArraySetVisible(pMe, IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,FALSE);
			pMe->nMainMenuCursorPos ++;
			ObjArraySetVisible(pMe, IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,TRUE);
			ObjArraySetPosition(pMe,IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,pMe->nWidth,pMe->nHeight);
			ObjArraySetDestination(pMe,IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,(pMe->nWidth/2), pMe->nHeight,MAINMENU_TEXTSPEED);
		}
		else
		{
			ObjArraySetVisible(pMe, IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,FALSE);
			pMe->nMainMenuCursorPos = 0;
			ObjArraySetVisible(pMe, IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,TRUE);
			ObjArraySetPosition(pMe,IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,pMe->nWidth,pMe->nHeight);
			ObjArraySetDestination(pMe,IDS_STRING_ICON_1021+pMe->nMainMenuCursorPos,(pMe->nWidth/2), pMe->nHeight,MAINMENU_TEXTSPEED);
		}

		break;
	case AVK_DOWN:
		setNextState(pMe, pMe->nMainMenuCursorPos+STATE_CALLS_MIN);
		return TRUE;
	case AVK_SELECT:
		setNextState(pMe, pMe->nMainMenuCursorPos+STATE_CALLS);
		return TRUE;
	case AVK_POUND:
		setNextState(pMe, STATE_DEBUGSCR);
		return TRUE;
	case AVK_SOFT2:
		setNextState(pMe, STATE_DEVICEOPTIONS);
		return TRUE;
	case AVK_SOFT1:
		setNextState(pMe, STATE_POWEROPTIONS);
		return TRUE;
	default:
		return FALSE;
	}

	ObjArraySetDestination(pMe, IDB_LEFT_ARROW_5010, DONTCHANGE_VALUE, pMe->nHeight-23, MAINMENU_ICONSPEED);
	ObjArraySetDestination(pMe, IDB_RIGHT_ARROW_5011, DONTCHANGE_VALUE, pMe->nHeight-23, MAINMENU_ICONSPEED);
	ObjArraySetDestination(pMe, IDB_LOWER_BAR_5016, DONTCHANGE_VALUE, pMe->nHeight-30, MAINMENU_ICONSPEED);

	for (i = 0; i < MAINMENU_MAXICON; i++)
	{
		ObjArraySetDestination(pMe, IDB_ICON_5002+i, x, pMe->nHeight-90, MAINMENU_ICONSPEED);
		x += 58;
	}

	return TRUE;
}

void timer_tick(void *data)
{
	HelloSonar * pMe = (HelloSonar*) (data);
	/*int TimeElapsed = GETUPTIMEMS() - pMe->mOldTime;*/
	uint32 currentUTCSecs = GETUTCSECONDS();
	pMe->mOldTime = GETUPTIMEMS();

	UpdateLogic(pMe);
	HelloSonar_DrawScreen(pMe);

	if (getActualConnectionState(pMe)!=CONN_REQ_IDLE)
	{
		UpdateConnectionState(pMe);
	}
	else
	{
		if ((pMe->mLastUpdate_Contact + CONN_REFRESH_INTERVAL_SECS) < currentUTCSecs)
		{
			putConnectionState(pMe, CONN_REQ_CONTACT_CHECK_CHANGES);
			putConnectionState(pMe, CONN_REQ_SETTINGS_CHECK_CHANGES);
		}
		else if ((pMe->mLastUpdate_Widget + CONN_REFRESH_INTERVAL_SECS) < currentUTCSecs)
		{
			putConnectionState(pMe, CONN_REQ_WIDGET_RETRIEVE_TS);	
		}
	}

	if (pMe->searchBox != NULL && pMe->searchBox[pMe->currentInputBox].enable == TRUE)
	{
		if (pMe->searchBox[pMe->currentInputBox].mLastKeyPress!=0
			&& (pMe->searchBox[pMe->currentInputBox].mLastKeyPress + INPUTBOX_KEY_PRESS_MS < GETTIMEMS()))
		{
			searchBox_AddCurrentChar(pMe);
			pMe->searchBox[pMe->currentInputBox].lastChar		= ' ';
			pMe->searchBox[pMe->currentInputBox].mLastKeyPress	= 0;
		}
	}

	SetTimer(pMe);
}

void SetTimer(HelloSonar *pMe)
{
  /*int result = */ ISHELL_SetTimer(pMe->piShell, TickTime, timer_tick, (void *) pMe);
}


static void LoadContactData(HelloSonar *pMe)
{
	char *url;

#ifdef DEBUG_STACK
	DBGPRINTF("LoadContactData");
#endif

	url = MALLOC(sizeof(char)*100);

	loadContactDataFromDevice(pMe);

	if (pMe->contactsList.cantNode==0)
	{
		SPRINTF(url, "/services/contactslist/RetrieveAll?sonarID=%s",pMe->pSonarIdEncoded);
		pMe->serviceCalled = CONN_REQ_CONTACT_RETRIEVE_ALL;
		GoToURL(pMe, url); 
	}
	else
	{
		putConnectionState(pMe, CONN_REQ_CONTACT_CHECK_PHOTOS);
	}

	FREE(url);
}

static void RequestReorderedWidgets(HelloSonar *pMe)
{
	char *url;

#ifdef DEBUG_STACK
	DBGPRINTF("RequestReorderedWidgets");
#endif

	url = MALLOC(sizeof(char)*100);

	SPRINTF(url, "/services/widgetslist/RetrieveAll?sonarID=%s",pMe->pSonarIdEncoded);
	pMe->serviceCalled = CONN_REQ_WIDGET_REORDER_ALL;
	GoToURL(pMe, url); 

	FREE(url);
}

static void RequestReorderedContacts(HelloSonar *pMe)
{
	char *url;

#ifdef DEBUG_STACK
	DBGPRINTF("RequestReorderedContacts");
#endif

	url = MALLOC(sizeof(char)*100);

	SPRINTF(url, "/services/contactslist/RetrieveAll?sonarID=%s",pMe->pSonarIdEncoded);
	pMe->serviceCalled = CONN_REQ_CONTACT_REORDER_ALL;
	GoToURL(pMe, url); 

	FREE(url);

}

static void initializeMenu(HelloSonar *pMe, ListTypes listType)
{
	List		*listAux;
	ListNode	*nodeAux;

	if(pMe->menu == NULL);
		pMe->menu = MALLOC(sizeof(Menu));

	pMe->menu->curPosition				= 0;
	pMe->menu->filteredMenuItems		= 0;
	FREEIF(pMe->menu->filteredMenuList);
	pMe->menu->filteredMenuList			= MALLOC(sizeof(void*)*MAXCONTACTS);
	pMe->menu->firstItemPosition		= 0;

	switch(listType)
	{
	case LIST_CALLS:
		pMe->menu->itemsPerPage			= AB_NO_CONTACTS_PAGE;
		pMe->menu->maxItemsPerPage		= AB_NO_CONTACTS_PAGE;
		listAux							= &(pMe->callsList);
		pMe->currentList				= LIST_CALLS;
		break;
	case LIST_CONTACTS:
		pMe->menu->itemsPerPage			= AB_NO_CONTACTS_PAGE;
		pMe->menu->maxItemsPerPage		= AB_NO_CONTACTS_PAGE;
		listAux							= &(pMe->contactsList);
		pMe->currentList				= LIST_CONTACTS;
		break;
	case LIST_MESSAGES:
		pMe->menu->itemsPerPage			= AB_NO_CONTACTS_PAGE;
		pMe->menu->maxItemsPerPage		= AB_NO_CONTACTS_PAGE;
		pMe->menu->pFirstItemPointer	= NULL;
		pMe->currentList				= LIST_MESSAGES;
		break;
	case LIST_WIDGETS:
		pMe->menu->itemsPerPage			= AB_NO_WIDGETS_PAGE;
		pMe->menu->maxItemsPerPage		= AB_NO_WIDGETS_PAGE;
		listAux							= &(pMe->widgetsList);
		pMe->currentList				= LIST_WIDGETS;
		break;
	}

	if (pMe->pLastNode == NULL)
		pMe->menu->pFirstItemPointer	=  listAux->pFirstElement;
	else
	{
		nodeAux = listAux->pFirstElement;
		while(pMe->pLastNode != nodeAux)
		{
			pMe->menu->firstItemPosition++;
			nodeAux = ((ListNode *) nodeAux->pNext);
		}

		pMe->menu->pFirstItemPointer	= (ListNode *) nodeAux;
	}
}

static void LoadWidgetData(HelloSonar *pMe)
{
	char *url;

#ifdef DEBUG_STACK
	DBGPRINTF("LoadWidgetData");
#endif

	url = MALLOC(sizeof(char)*100);

	if (pMe->widgetsList.cantNode==0)
	{
		SPRINTF(url, "/services/widgetslist/RetrieveAll?sonarID=%s",pMe->pSonarIdEncoded);
		pMe->serviceCalled = CONN_REQ_WIDGET_RETRIEVE_ALL;
		GoToURL(pMe, url); 
	}
	else
	{
		/* here we'll check only for updates since last run */
	}

	FREE(url);
}

static void LoadCallData(HelloSonar *pMe)
{
	char *url;

#ifdef DEBUG_STACK
	DBGPRINTF("LoadContactData");
#endif

	url = MALLOC(sizeof(char)*100);

	if (pMe->callsList.cantNode==0)
	{
		SPRINTF(url, "/services/calllog/RetrieveAll?sonarID=%s&maxCount=20&order=desc",pMe->pSonarIdEncoded);
		pMe->serviceCalled = CONN_REQ_CALL_RETRIEVE_ALL;
		GoToURL(pMe, url); 
	}
	else
	{
		/* here we'll check only for updates since last run */
	}

	FREE(url);
}

static void FillWidgetData(HelloSonar *pMe)
{
	char * aux;
	char * aux2;

	int lenAux;
	int lenAux2;
	int widgetsRequested = 0;

	ListNode * nodeAux;

#ifdef DEBUG_STACK
	DBGPRINTF("FillWidgetData");
#endif

	aux = MALLOC(255*sizeof(char));
	aux2 = MALLOC(20*sizeof(char));

	SPRINTF(aux, "/services/widget/Get?sonarID=%s", pMe->pSonarIdEncoded);


	lenAux = STRLEN(aux);
	
	nodeAux = pMe->widgetsList.pFirstElement;

	while (nodeAux != NULL)
	{
		if (((Widget*) nodeAux->pNode)->needUpdate == TRUE)
		{
			lenAux2 = SPRINTF(aux2,"&widgetID=%d",((Widget*) nodeAux->pNode)->id);
			STRNCPY(&(aux[lenAux]), aux2, lenAux2);
			lenAux = STRLEN(aux);
			widgetsRequested ++;

			if (widgetsRequested > 10)  //URL size limit is 255 chars, so we split the call in a maximum of 10 contacts each
			{
				putConnectionState(pMe, CONN_REQ_WIDGET_UPDATE_PENDING);
				break;
			}
		}

		nodeAux =  (ListNode*) nodeAux->pNext;

	}

	if (widgetsRequested != 0)
	{
		aux[lenAux] = '\0';
		pMe->serviceCalled = CONN_REQ_WIDGET_RETRIEVE_DETAIL;
		GoToURL(pMe, aux);
	}

	FREE(aux);
	FREE(aux2);

}

static void FillContactData(HelloSonar *pMe)
{
	char * aux;
	char * aux2;

	int lenAux;
	int lenAux2;
	int contactsRequested = 0;

	ListNode * nodeAux;

#ifdef DEBUG_STACK
	DBGPRINTF("FillContactData");
#endif

	aux = MALLOC(255*sizeof(char));
	aux2 = MALLOC(20*sizeof(char));

	SPRINTF(aux, "/services/contact/Get?sonarID=%s", pMe->pSonarIdEncoded);

	lenAux = STRLEN(aux);
	
	nodeAux = pMe->contactsList.pFirstElement;

	while (nodeAux != NULL)
	{
		if (((Contact*) nodeAux->pNode)->needUpdate == TRUE)
		{
			lenAux2 = SPRINTF(aux2,"&contactID=%d",((Contact*) nodeAux->pNode)->id);
			STRNCPY(&(aux[lenAux]), aux2, lenAux2);
			lenAux = STRLEN(aux);
			contactsRequested ++;

			if (contactsRequested > 10)  //URL size limit is 255 chars, so we split the call in a maximum of 10 contacts each
			{
				putConnectionState(pMe, CONN_REQ_CONTACT_UPDATE_PENDING);
				break;
			}
		}

		nodeAux =  (ListNode*) nodeAux->pNext;
	}

	if (contactsRequested != 0)
	{
		aux[lenAux] = '\0';
		pMe->serviceCalled = CONN_REQ_CONTACT_RETRIEVE_DETAIL;
		GoToURL(pMe, aux);
	}

	FREE(aux);
	FREE(aux2);
}



static void setNextState(HelloSonar *pMe, States newState)
{
#ifdef DEBUG_STACK
	DBGPRINTF("setNextState");
#endif
	pMe->nOldState	= pMe->nState;
	pMe->nState		= newState;
	ClearObjArray(pMe);

	pMe->bDrawHour	= FALSE;

	pMe->currentList = LIST_NONE;
	if (pMe->searchBox != NULL)
		pMe->searchBox[0].enable = FALSE;

	switch (newState)
	{
	case STATE_MAINMENU:
		LoadMainMenu(pMe);
		setSoftKeys(pMe, SOFTKEY_NONE,SOFTKEY_NONE);
		break;
	case STATE_CALLS_MIN:
		LoadCalls(pMe, FALSE);
		setSoftKeys(pMe, SOFTKEY_MAINMENU,SOFTKEY_NONE);
		break;
	case STATE_CALLS:
		LoadCalls(pMe, TRUE);
		setSoftKeys(pMe, SOFTKEY_MAINMENU,SOFTKEY_NONE);
		break;
    case STATE_ADRESSBOOK_MIN:
		LoadAddressBook(pMe, FALSE);
		setSoftKeys(pMe, SOFTKEY_MAINMENU,SOFTKEY_NONE);
		break;
    case STATE_ADRESSBOOK:
		LoadAddressBook(pMe, TRUE);
		setSoftKeys(pMe, SOFTKEY_MAINMENU,SOFTKEY_NONE);
		break;
    case STATE_MESSAGES:
		LoadMessage(pMe, TRUE);
		setSoftKeys(pMe, SOFTKEY_MAINMENU,SOFTKEY_NONE);
		break;
	case STATE_MESSAGES_MIN:
		LoadMessage(pMe, FALSE);
		setSoftKeys(pMe, SOFTKEY_MAINMENU,SOFTKEY_NONE);
		break;
	case STATE_WIDGETS_MIN:
		LoadWidgets(pMe, FALSE);
		setSoftKeys(pMe, SOFTKEY_MAINMENU,SOFTKEY_NONE);
		break;
	case STATE_WIDGETS:
		LoadWidgets(pMe, TRUE);
		setSoftKeys(pMe, SOFTKEY_MAINMENU,SOFTKEY_NONE);
		break;
	case STATE_CALLING:
		LoadCalling(pMe);
		setSoftKeys(pMe, SOFTKEY_MAINMENU,SOFTKEY_NONE);
		break;
	case STATE_CONTACTINFO:
		loadContactInfoScreen(pMe);
		setSoftKeys(pMe, SOFTKEY_BACK, SOFTKEY_NONE);
		break;
	case STATE_OPTIONSMENU:
		loadOptionsMenu(pMe);
		setSoftKeys(pMe, SOFTKEY_BACK, SOFTKEY_NONE);
		break;
	case STATE_DEBUGSCR:
		loadDebugScreen(pMe);
		setSoftKeys(pMe, SOFTKEY_CANCEL, SOFTKEY_ACCEPT);
		break;
	case STATE_DEVICEOPTIONS:
	case STATE_POWEROPTIONS:
		loadOptions(pMe, pMe->nState);
		setSoftKeys(pMe, SOFTKEY_BACK, SOFTKEY_BACK);
		break;
	}
}

static void setSoftKeys(HelloSonar *pMe, int left, int right)
{
	pMe->nLeftSoftKey	= left;
	pMe->nRightSoftKey	= right;
}

static void paintSoftKeys(HelloSonar *pMe)
{
	AECHAR szBuf[64];

	if (pMe->nLeftSoftKey==SOFTKEY_CANCEL)
	{
		ISHELL_LoadResString(pMe->piShell, HELLOSONAR_RES_FILE, (short)(IDS_STRING_SK_NONE_1025+pMe->nLeftSoftKey), szBuf, sizeof(szBuf));

		IDISPLAY_SetColor(pMe->piDisplay, CLR_USER_TEXT, RGB_BLACK);

		IDISPLAY_DrawText(pMe->piDisplay, // Display instance.
                        AEE_FONT_BOLD, // Use Bold font.
                        szBuf,         // String buffer containing text.
                        -1,     // Automatically compute string length.
                        10,      // x-coordinate ignored since IDF_ALIGN_CENTER.
                        pMe->nHeight-20,      // y-coordinate ignored since IDF_ALIGN_MIDDLE.
                        NULL,   // No clipping.
						IDF_TEXT_TRANSPARENT); // Middle-align vertically.
	}
	if (pMe->nRightSoftKey==SOFTKEY_ACCEPT)
	{
		ISHELL_LoadResString(pMe->piShell, HELLOSONAR_RES_FILE, (short)(IDS_STRING_SK_NONE_1025+pMe->nRightSoftKey), szBuf, sizeof(szBuf));

		IDISPLAY_SetColor(pMe->piDisplay, CLR_USER_TEXT, RGB_BLACK);

		IDISPLAY_DrawText(pMe->piDisplay, // Display instance.
                        AEE_FONT_BOLD, // Use Bold font.
                        szBuf,         // String buffer containing text.
                        -1,     // Automatically compute string length.
						pMe->nWidth-10,      // x-coordinate ignored since IDF_ALIGN_CENTER.
                        pMe->nHeight-20,      // y-coordinate ignored since IDF_ALIGN_MIDDLE.
                        NULL,   // No clipping. 
						IDF_ALIGN_RIGHT |
						IDF_TEXT_TRANSPARENT); // Middle-align vertically.
	}


}
static boolean handleSoftKeys(HelloSonar *pMe, uint16 wParam)
{
	int softKeyToProcess;

	if (wParam==AVK_SOFT1)
		softKeyToProcess = pMe->nLeftSoftKey;
	else
		if (wParam==AVK_SOFT2)
			softKeyToProcess = pMe->nRightSoftKey;
		else
			return FALSE;

	switch (softKeyToProcess)
	{
	case SOFTKEY_BACK:
		setNextState(pMe, pMe->nOldState);
		break;
	case SOFTKEY_CANCEL:
	case SOFTKEY_MAINMENU:
		setNextState(pMe, STATE_MAINMENU);
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

static void MakeCall(HelloSonar *pMe, Contact *contactAux, char *telephone)
{
	char *aux;
	char *aux2;

	JulianType date;
	uint32   time;

	Call *tmpCall;
	int lenAux;
	int i = 0;

#ifdef DEBUG_STACK
	DBGPRINTF("MakeCall");
#endif

	aux = MALLOC(sizeof(char)*200);
	aux2 = MALLOC(sizeof(char)*200);

	if (contactAux != NULL)
		pMe->pCallingName = contactAux->pDisplayName;
	else
		pMe->pCallingName = telephone;


	DBGPRINTF("Calling to: %s", telephone);

	time = GETUPTIMEMS();
	GETJULIANDATE(time,&date);

	SPRINTF(aux2, "/services/calllog/CallActivity?sonarID=%s", pMe->pSonarIdEncoded);

	SPRINTF(aux,"%s&callTime=%d-%.2d-%.2dT%.2d%%3a%.2d%%3a%.2dZ", aux2, date.wYear,date.wMonth,date.wDay,date.wHour,date.wMinute, date.wSecond);
	SPRINTF(aux2, "&phoneNumber=%s&callType=sent&duration=1&contactID=%d",  telephone, contactAux->id);
	
	STRNCPY(&(aux[STRLEN(aux)]), aux2, STRLEN(aux2));

	GoToURL(pMe, aux);


	tmpCall = MALLOC(sizeof(Call));

	tmpCall->pContact = contactAux;

	lenAux = STRLEN(telephone) + 1;
	tmpCall->telephone.pString = MALLOC (sizeof(char) * lenAux);
	STRCPY(tmpCall->telephone.pString, telephone);
	tmpCall->type = CALL_SENT;

	tmpCall->pFormatedDate	= MALLOC (sizeof(char) * 64);
	SPRINTF(tmpCall->pFormatedDate, "%d-%.2d-%.2dT%.2d:%.2d:%.2dZ",date.wYear,date.wMonth,date.wDay,date.wHour,date.wMinute, date.wSecond); 
		
	tmpCall->pHour			= MALLOC (sizeof(char) * 6);
	SPRINTF(tmpCall->pFormatedDate, "%.2d:%.2d",date.wHour,date.wMinute);

	AddCall2Array(pMe, tmpCall, TRUE);

    ITAPI_MakeVoiceCall(pMe->pITAPI,telephone, 0);


	FREEIF(aux);
	FREEIF(aux2);

	//setNextState(pMe, STATE_CALLING);
}

static void resetDeviceInfo(HelloSonar *pMe)
{
	ListNode	*nodeAux;
	ListNode	*nodeAux2;
	Contact		*contactAux;
	Widget		*widgetAux;

	cleanConnectionStates(pMe);

	nodeAux = pMe->contactsList.pFirstElement;

	while (nodeAux != NULL)
	{
		contactAux	=  ((Contact *) nodeAux->pNode);
		nodeAux		= ((ListNode *) nodeAux->pNext);

		RemoveContact(pMe, contactAux->id);
	}

	pMe->contactsList.cantNode			= 0;
	pMe->contactsList.pFirstElement		= NULL;
	pMe->contactsList.pLastElement		= NULL;


	nodeAux = pMe->callsList.pFirstElement;

	while (nodeAux != NULL)
	{
		nodeAux2		= ((ListNode *) nodeAux->pNext);
		RemoveCall(pMe, nodeAux);
		nodeAux			= nodeAux2;
	}

	pMe->callsList.cantNode			= 0;
	pMe->callsList.pFirstElement	= NULL;
	pMe->callsList.pLastElement		= NULL;

	
	nodeAux = pMe->widgetsList.pFirstElement;

	while (nodeAux != NULL)
	{
		widgetAux	=  ((Widget *) nodeAux->pNode);
		nodeAux		= ((ListNode *) nodeAux->pNext);

		RemoveWidgetFromArray(pMe, widgetAux->id);
	}

	pMe->widgetsList.cantNode			= 0;
	pMe->widgetsList.pFirstElement		= NULL;
	pMe->widgetsList.pLastElement		= NULL;

	IFILEMGR_Remove(pMe->fmgr, FILE_CONTACT); 
	IFILEMGR_Remove(pMe->fmgr, FILE_CALLS); 
	IFILEMGR_Remove(pMe->fmgr, FILE_WIDGETS); 
}

static void LoadMainMenu(HelloSonar *pMe)
{
	int x = 9;
	int i;
	int aux;

#ifdef DEBUG_STACK
	DBGPRINTF("LoadMainMenu");
#endif

	AddImage2Array(pMe,IDB_LOWER_BAR_5016,0,pMe->nHeight-30);
	AddImage2Array(pMe,IDB_LEFT_ARROW_5010, 9,pMe->nHeight-23);
	AddImage2Array(pMe,IDB_RIGHT_ARROW_5011,pMe->nWidth-25,pMe->nHeight-23);


	for (i = 0; i < MAINMENU_MAXICON; i++)
	{
		AddImage2Array(pMe,IDB_ICON_5002+i,x,pMe->nHeight-90);
		ObjArraySetDestination(pMe, IDB_ICON_5002+i, x, pMe->nHeight-90, MAINMENU_ICONSPEED);
		x += 58;
		AddString2Array(pMe,GOTHAM_14_B_WHITE, IDS_STRING_ICON_1021+i, 0, pMe->nHeight-22);

		aux = getObjArrayPos(pMe, IDS_STRING_ICON_1021+i);

		pMe->objArray[aux].align = FONT_ALIGN_CENTER;

		ObjArraySetVisible(pMe, IDS_STRING_ICON_1021+i,FALSE);
		ObjArraySetClip(pMe, IDS_STRING_ICON_1021+i, 30, pMe->nHeight-30, pMe->nWidth-60, 30);
	}
	

	ObjArraySetPosition(pMe,IDS_STRING_ICON_1021,(pMe->nWidth/2),pMe->nHeight-22);
	ObjArraySetVisible(pMe, IDS_STRING_ICON_1021,TRUE);

	pMe->nMainMenuCursorPos  = 0;
	pMe->bDrawHour			 = TRUE;
	pMe->pLastNode = NULL;
}

static void LoadCalls(HelloSonar *pMe, boolean extended)
{
	int i;
	int aux;

#ifdef DEBUG_STACK
	DBGPRINTF("LoadCalls");
#endif

	AddImage2ArrayAnim(pMe,IDB_UPPER_SHADOW_5021,0,-13,ANIM_FROM_BOTTOM);
	AddImage2ArrayAnim(pMe,IDB_UPPER_BAR_5017,0,0,ANIM_FROM_BOTTOM);
	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BACKGROUND, OBJ_PRIMITIVE_FILLRECT, 0, 30, pMe->nWidth, pMe->nHeight-30, MAKE_RGB(255,255,255),ANIM_FROM_BOTTOM);

	AddString2ArrayAnim(pMe, GOTHAM_14_B_BLACK, IDS_STRING_ICON_1021, pMe->nWidth/2, 8,ANIM_FROM_BOTTOM);
	aux = getObjArrayPos(pMe, IDS_STRING_ICON_1021);
	pMe->objArray[aux].align = FONT_ALIGN_CENTER;

	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BASE, OBJ_PRIMITIVE_FILLRECT, 0, 31, pMe->nWidth, 60, pMe->currentThemeColor,ANIM_FROM_BOTTOM);
	AddImage2ArrayAnim(pMe,IDB_SELECTION_BOX_5013,0,31,ANIM_FROM_BOTTOM);

	initializeMenu(pMe, LIST_CALLS);

	for (i=0; i<AB_NO_CONTACTS_PAGE; i++)
	{
		AddString2ArrayAnim(pMe, GOTHAM_18_BLACK, ID_MENUS_ITEM_NAME_00+i, 14, 38+(21*i),ANIM_FROM_BOTTOM);
		AddImage2ArrayAnim(pMe,ID_MENUS_PHOTO_FRAME_00+i, pMe->nWidth-50,35+(32*i),ANIM_FROM_BOTTOM);
		AddImage2ArrayAnim(pMe,ID_MENUS_ICON_NOPHOTO_00+i, pMe->nWidth-44,40+(32*i),ANIM_FROM_BOTTOM);
		AddString2ArrayAnim(pMe, HELVETICA_11_GRAY, ID_MENUS_ITEM_DESCRIPTION_00+i, 20, 65+(32*i),ANIM_FROM_BOTTOM);
		AddString2ArrayAnim(pMe, HELVETICA_11_GRAY, ID_MENUS_ITEM_DESCRIPTION_2_00+i, pMe->nWidth - 58, 65+(32*i),ANIM_FROM_BOTTOM);
		aux = getObjArrayPos(pMe, ID_MENUS_ITEM_DESCRIPTION_2_00+i);
		pMe->objArray[aux].align = FONT_ALIGN_RIGHT;

		AddPrimitive2ArrayAnim(pMe, ID_MENUS_LINE_SEPARATOR_00+i, OBJ_PRIMITIVE_FILLRECT, 0, 31+(32*i), pMe->nWidth, 1, MAKE_RGB(230,230,230),ANIM_FROM_BOTTOM);
	}

	searchBox_Init(pMe, 1);

	if (extended == FALSE)
	{
		pMe->menu->itemsPerPage -= 5;
		pMe->bDrawHour			 = TRUE;
		AddString2ArrayAnim(pMe, GOTHAM_18_GRAY, IDS_STRING_CALL_NORECENT_1045, pMe->nWidth/2, 203,ANIM_FROM_BOTTOM);
	}
	else
		AddString2ArrayAnim(pMe, GOTHAM_18_GRAY, IDS_STRING_CALL_NORECENT_1045, pMe->nWidth/2, 53,ANIM_FROM_BOTTOM);

	aux = getObjArrayPos(pMe, IDS_STRING_CALL_NORECENT_1045);
	pMe->objArray[aux].align = FONT_ALIGN_CENTER;

	FillCalls(pMe);
}

static void FillCalls(HelloSonar *pMe)
{
	int aux;
	int i;
	int j = 0;

	ListNode	*nodeAux;
	Call	*callAux;
	Contact		*contactAux;

#ifdef DEBUG_STACK
	DBGPRINTF("FillCalls");
#endif

	if (pMe->searchBox[0].charPos>0)
	{
		j = pMe->menu->firstItemPosition;
		nodeAux = pMe->menu->filteredMenuList[j];
	}
	else
	{
		nodeAux = pMe->menu->pFirstItemPointer;
	}


	for (i=0; i<AB_NO_CONTACTS_PAGE; i++)
	{
		if ((nodeAux != NULL)&&(i<pMe->callsList.cantNode))
		{
			ObjArraySetVisible(pMe,  ID_MENUS_ITEM_NAME_00+i, TRUE);

			aux = getObjArrayPos(pMe, ID_MENUS_ITEM_NAME_00+i);


			callAux = ((Call*) nodeAux->pNode);
			contactAux = callAux->pContact;


			if (contactAux != NULL)
			{
				pMe->objArray[aux].pStr		= contactAux->pDisplayName;
				pMe->objArray[aux].type		= OBJ_STRING_NODEALLOC;
			}

			aux = getObjArrayPos(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i);
			pMe->objArray[aux].pStr		= callAux->telephone.pString;
			pMe->objArray[aux].type		= OBJ_STRING_NODEALLOC;

			aux = getObjArrayPos(pMe, ID_MENUS_ITEM_DESCRIPTION_2_00+i);
			pMe->objArray[aux].pStr		= callAux->pHour;
			pMe->objArray[aux].type		= OBJ_STRING_NODEALLOC;

			aux = getObjArrayPos(pMe, ID_MENUS_ICON_NOPHOTO_00+i);
			pMe->objArray[aux].type = OBJ_IMAGE_NODEALLOC;
			if (contactAux != NULL && contactAux->pImage != NULL)
				pMe->objArray[aux].pImage = contactAux->pImage;
			else
				pMe->objArray[aux].pImage =  pMe->pImageCntNoPhoto;

			aux = getObjArrayPos(pMe, ID_MENUS_PHOTO_FRAME_00+i);
			pMe->objArray[aux].type		= OBJ_IMAGE_NODEALLOC;
			pMe->objArray[aux].pImage	= pMe->pImageCntPhotoFrame;

			if (pMe->searchBox[0].charPos>0)
			{
				j++;
				nodeAux = pMe->menu->filteredMenuList[j];
			}
			else
				nodeAux =  (ListNode*) nodeAux->pNext;
		}	
		else
		{
			ObjArraySetVisible(pMe,  ID_MENUS_ITEM_NAME_00+i, FALSE);
			ObjArraySetVisible(pMe,  ID_MENUS_ITEM_DESCRIPTION_00+i, FALSE);
			ObjArraySetVisible(pMe,  ID_MENUS_ICON_NOPHOTO_00+i, FALSE);
			ObjArraySetVisible(pMe,  ID_MENUS_PHOTO_FRAME_00+i, FALSE);
		}
	}

	updateMenuObjects(pMe);

	if (pMe->callsList.cantNode == 0)
		ObjArraySetVisible(pMe,  IDS_STRING_CALL_NORECENT_1045, TRUE);
	else
		ObjArraySetVisible(pMe,  IDS_STRING_CALL_NORECENT_1045, FALSE);


	if ((pMe->menu->filteredMenuItems == 0 && pMe->searchBox[0].charPos>0) || pMe->callsList.cantNode == 0)
	{
		ObjArraySetVisible(pMe,  ID_MENUS_ICON_NOPHOTO_00, FALSE);
		ObjArraySetVisible(pMe,  ID_MENUS_PHOTO_FRAME_00, FALSE);
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_5013, FALSE);
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_BASE, FALSE);
		ObjArraySetVisible(pMe,  ID_MENUS_ITEM_DESCRIPTION_00, FALSE);
		ObjArraySetVisible(pMe,  ID_MENUS_ITEM_DESCRIPTION_2_00, FALSE);
	}
	else
	{
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_5013, TRUE);
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_BASE, TRUE);
	}
}

static void LoadMessage(HelloSonar *pMe, boolean extended)
{
	int aux;
	int y = 31;
	int i;
#ifdef DEBUG_STACK
	DBGPRINTF("LoadMessage");
#endif
 
	if (extended == FALSE)
	{
		y += (5 * 31);
		pMe->bDrawHour = TRUE;
	}

	AddImage2ArrayAnim(pMe,IDB_UPPER_SHADOW_5021,0,y-43,ANIM_FROM_BOTTOM);
	AddImage2ArrayAnim(pMe,IDB_UPPER_BAR_5017,0,y-30,ANIM_FROM_BOTTOM);
	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BACKGROUND, OBJ_PRIMITIVE_FILLRECT, 0, y-1, pMe->nWidth, pMe->nHeight-30, MAKE_RGB(255,255,255),ANIM_FROM_BOTTOM);

	AddString2ArrayAnim(pMe, GOTHAM_14_B_BLACK, IDS_STRING_ICON_1023, pMe->nWidth/2, y-22,ANIM_FROM_BOTTOM);
	aux = getObjArrayPos(pMe, IDS_STRING_ICON_1023);

	pMe->objArray[aux].align = FONT_ALIGN_CENTER;

	if (extended == TRUE)
	{
		AddString2ArrayAnim(pMe, GOTHAM_18_GRAY, IDS_STRING_MSG_NORECENT_1047, pMe->nWidth/2, 53,ANIM_FROM_BOTTOM);
		aux = 0;
	}
	else
	{
		AddString2ArrayAnim(pMe, GOTHAM_18_GRAY, IDS_STRING_MSG_NORECENT_1047, pMe->nWidth/2, 215,ANIM_FROM_BOTTOM);
		aux = 5; 
	}

	for (i=aux; i<AB_NO_CONTACTS_PAGE-1; i++)
	{
		AddPrimitive2ArrayAnim(pMe, ID_MENUS_LINE_SEPARATOR_00+i, OBJ_PRIMITIVE_FILLRECT, 0,  95+(32*i), pMe->nWidth, 1, MAKE_RGB(230,230,230), ANIM_FROM_BOTTOM);
	}

	aux = getObjArrayPos(pMe, IDS_STRING_MSG_NORECENT_1047);
	pMe->objArray[aux].align = FONT_ALIGN_CENTER;

}

static void LoadWidgets(HelloSonar *pMe, boolean extended)
{
	int i;
	int aux;

#ifdef DEBUG_STACK
	DBGPRINTF("LoadWidgets");
#endif

	AddImage2ArrayAnim(pMe,IDB_UPPER_SHADOW_5021,0,-13,ANIM_FROM_BOTTOM);
	AddImage2ArrayAnim(pMe,IDB_UPPER_BAR_5017,0,0,ANIM_FROM_BOTTOM);
	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BACKGROUND, OBJ_PRIMITIVE_FILLRECT, 0, 30, pMe->nWidth, pMe->nHeight-30, MAKE_RGB(255,255,255),ANIM_FROM_BOTTOM);
	AddString2ArrayAnim(pMe, GOTHAM_14_B_BLACK, IDS_STRING_ICON_1024, pMe->nWidth/2, 8,ANIM_FROM_BOTTOM);
	aux = getObjArrayPos(pMe, IDS_STRING_ICON_1024);
	pMe->objArray[aux].align = FONT_ALIGN_CENTER;
	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BASE, OBJ_PRIMITIVE_FILLRECT, 0, 31, pMe->nWidth, 60, pMe->currentThemeColor,ANIM_FROM_BOTTOM);
	AddImage2ArrayAnim(pMe,IDB_SELECTION_BOX_5013,0,31,ANIM_FROM_BOTTOM);

	initializeMenu(pMe, LIST_WIDGETS);

	for (i=0; i<AB_NO_WIDGETS_PAGE; i++)
	{
		AddString2ArrayAnim(pMe, GOTHAM_18_BLACK, ID_MENUS_ITEM_NAME_00+i, 55, 55+(60*i),ANIM_FROM_BOTTOM);
		AddString2ArrayAnim(pMe, HELVETICA_11_GRAY, ID_MENUS_ITEM_DESCRIPTION_00+i, 55, 36+(60*i),ANIM_FROM_BOTTOM);
		AddImage2ArrayAnim(pMe,ID_MENUS_PHOTO_FRAME_00+i, 2,33,ANIM_FROM_BOTTOM);
		AddImage2ArrayAnim(pMe,ID_MENUS_ICON_NOPHOTO_00+i, 8,40+(60*i),ANIM_FROM_BOTTOM);
		AddPrimitive2ArrayAnim(pMe, ID_MENUS_LINE_SEPARATOR_00+i, OBJ_PRIMITIVE_FILLRECT, 0, 90+(60*i), pMe->nWidth, 1, MAKE_RGB(230,230,230),ANIM_FROM_BOTTOM);
	}

	searchBox_Init(pMe, 1);

	if (extended == FALSE)
	{
		pMe->menu->itemsPerPage -= 3;
		pMe->bDrawHour			 = TRUE;
	}

	FillWidgets(pMe);
}

static void LoadCalling(HelloSonar *pMe)
{
#ifdef DEBUG_STACK
	DBGPRINTF("LoadCalling");
#endif

	AddImage2Array(pMe,IDB_UPPER_BAR_5017,0,pMe->nHeight);
	ObjArraySetDestination(pMe, IDB_UPPER_BAR_5017, 0, 0, MESSAGE_BACKGROUNDSPEED);

	AddString2Array(pMe, GOTHAM_14_BLACK, IDS_STRING_ICON_1022, 100, 15);
}

static void loadOptions(HelloSonar *pMe, States state)
{
	int i;
	int x = 9;
	int auxId;
	int y = pMe->nHeight - (32 *4);

/*
	for (i = 0; i < MAINMENU_MAXICON; i++)
	{
		AddImage2Array(pMe,IDB_ICON_5002+i,x,pMe->nHeight-60);
		x += 58;
	}

	AddImage2ArrayAnim(pMe,IDB_DARK_BACKGROUND_5026,0,0,ANIM_FROM_LEFT);
	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BASE_WHITE, OBJ_PRIMITIVE_FILLRECT, 0, y-1, pMe->nWidth, 34, MAKE_RGB(255,255,255), ANIM_FROM_LEFT);
	AddImage2ArrayAnim(pMe,IDB_SELECTION_BOX_5013,0,y-1, ANIM_FROM_LEFT);

	if (state == STATE_POWEROPTIONS)
		auxId = IDS_STRING_PO_SWITCH_1036;
	else
		auxId = IDS_STRING_DO_TIMEALARM_1040;

	for (i = 0; i < 4; i++)
	{
		AddString2ArrayAnim(pMe, GOTHAM_18_WHITE, auxId+i, 14, y+4, ANIM_FROM_LEFT);
		AddImage2ArrayAnim(pMe,ID_OPT_LINE_SEPARATOR_00+i,0,y, ANIM_FROM_LEFT);

		y += 33;
	}
*/

	
	for (i = 0; i < MAINMENU_MAXICON; i++)
	{
		AddImage2Array(pMe,IDB_ICON_5002+i,x,pMe->nHeight-60);
		x += 58;
	}

	AddImage2ArrayAnim(pMe,IDB_DARK_BACKGROUND_5026,0,0,ANIM_NONE);
	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BASE_WHITE, OBJ_PRIMITIVE_FILLRECT, 0, y-1, pMe->nWidth, 34, MAKE_RGB(255,255,255), ANIM_NONE);
	AddImage2ArrayAnim(pMe,IDB_SELECTION_BOX_5013,0,y-1, ANIM_NONE);

	if (state == STATE_POWEROPTIONS)
		auxId = IDS_STRING_PO_SWITCH_1036;
	else
		auxId = IDS_STRING_DO_TIMEALARM_1040;

	for (i = 0; i < 4; i++)
	{
		AddString2ArrayAnim(pMe, GOTHAM_18_WHITE, auxId+i, 14, y+4, ANIM_NONE);
		AddImage2ArrayAnim(pMe,ID_OPT_LINE_SEPARATOR_00+i,0,y, ANIM_NONE);

		y += 33;
	}

	pMe->bDrawHour			= TRUE;
	pMe->nMainMenuCursorPos = 0;

	updateOptionsObjects(pMe);
}

static void updateOptionsObjects(HelloSonar *pMe)
{
	int i;
	int auxId;

	//ObjArraySetDestination(pMe, IDB_SELECTION_BOX_BASE_WHITE, 0, 187 + (32 * pMe->nMainMenuCursorPos), MESSAGE_BACKGROUNDSPEED);
	//ObjArraySetDestination(pMe, IDB_SELECTION_BOX_5013, 0, 187 + (32 * pMe->nMainMenuCursorPos), MESSAGE_BACKGROUNDSPEED);

	ObjArraySetDestination(pMe, IDB_SELECTION_BOX_BASE_WHITE, 0, pMe->nHeight - (32 *4) + (32 * pMe->nMainMenuCursorPos), MESSAGE_BACKGROUNDSPEED);
	ObjArraySetDestination(pMe, IDB_SELECTION_BOX_5013, 0, pMe->nHeight - (32 *4) + (32 * pMe->nMainMenuCursorPos), MESSAGE_BACKGROUNDSPEED);

	if (pMe->nState == STATE_POWEROPTIONS)
		auxId = IDS_STRING_PO_SWITCH_1036;
	else
		auxId = IDS_STRING_DO_TIMEALARM_1040;

	for (i = 0; i < 4; i++)
	{
		if (i == pMe->nMainMenuCursorPos)
		{
			ObjArraySetFont(pMe, auxId+i, GOTHAM_18_BLACK);
		}
		else
		{
			ObjArraySetFont(pMe, auxId+i, GOTHAM_18_WHITE);
		}
	}

}

static void loadDebugScreen(HelloSonar *pMe)
{
	int aux;

	AddImage2Array(pMe,IDB_UPPER_BAR_5017,0,pMe->nHeight);
	ObjArraySetDestination(pMe, IDB_UPPER_BAR_5017, 0, 0, MESSAGE_BACKGROUNDSPEED);

	AddPrimitive2Array(pMe, IDB_SELECTION_BOX_BACKGROUND, OBJ_PRIMITIVE_FILLRECT, 0, pMe->nHeight, pMe->nWidth, pMe->nHeight-30, MAKE_RGB(255,255,255));
	ObjArraySetDestination(pMe, IDB_SELECTION_BOX_BACKGROUND, 0, 30, MESSAGE_BACKGROUNDSPEED);

	AddString2Array(pMe, GOTHAM_14_B_BLACK, IDS_STRING_DEBUG_TITLE_1035, pMe->nWidth/2, 8);
	aux = getObjArrayPos(pMe, IDS_STRING_DEBUG_TITLE_1035);
	pMe->objArray[aux].align = FONT_ALIGN_CENTER;

	searchBox_Init (pMe, 2);

	STRCPY(pMe->searchBox[0].string, pMe->pSonarId);
	pMe->searchBox[0].charPos = STRLEN(pMe->searchBox[0].string);

	STRCPY(pMe->searchBox[1].string, pMe->pBaseURL);
	pMe->searchBox[1].charPos = STRLEN(pMe->searchBox[1].string);
}

static void loadOptionsMenu(HelloSonar *pMe)
{
	int aux;
	int i;

	Contact	*contactAux = pMe->pCurrentContact;

	FREEIF(pMe->menuDetailed.buttonArray)
	pMe->menuDetailed.buttonArray = MALLOC(sizeof(Button) * CONTACT_MAX_LBL_STRING);

	AddImage2ArrayAnim(pMe,IDB_UPPER_BAR_5017,0,0,ANIM_FROM_BOTTOM);
	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BACKGROUND, OBJ_PRIMITIVE_FILLRECT,0,30, pMe->nWidth, pMe->nHeight-30, MAKE_RGB(255,255,255),ANIM_FROM_BOTTOM);
	AddString2ArrayAnim(pMe, GOTHAM_14_B_BLACK, IDS_STRING_ICON_1022, pMe->nWidth/2, 8,ANIM_FROM_BOTTOM);
	aux = getObjArrayPos(pMe, IDS_STRING_ICON_1022);
	pMe->objArray[aux].align = FONT_ALIGN_CENTER;

	AddString2ArrayAnim(pMe, GOTHAM_18_BLACK, ID_CD_CONTACT_DISPNAME, 89, 37,ANIM_FROM_BOTTOM);
	aux = getObjArrayPos(pMe, ID_CD_CONTACT_DISPNAME);

	if (contactAux->pDisplayName != NULL)
	{
		pMe->objArray[aux].pStr		= contactAux->pDisplayName;
		pMe->objArray[aux].type		= OBJ_STRING_NODEALLOC;
	}

	AddString2ArrayAnim(pMe, HELVETICA_11_GRAY, ID_CD_CONTACT_STATUS,58,60,ANIM_FROM_BOTTOM);

	aux = getObjArrayPos(pMe, ID_CD_CONTACT_STATUS);

	if (contactAux->pStatus != NULL)
	{
		pMe->objArray[aux].pStr		= contactAux->pStatus;
		pMe->objArray[aux].type		= OBJ_STRING_AREA_NODEALLOC;
		pMe->objArray[aux].width	= pMe->nWidth - 50 - 12;
		pMe->objArray[aux].height 	= 92;
	}

	if (contactAux->favorite == TRUE)
	{
		AddImage2ArrayAnim(pMe,ID_CD_CONTACT_FAVORITE, 58,42,ANIM_FROM_BOTTOM);
		aux = getObjArrayPos(pMe, ID_CD_CONTACT_FAVORITE);
		pMe->objArray[aux].pImage = pMe->pImageIconFavorite;
	}

	AddImage2ArrayAnim(pMe,ID_CD_CONTACT_PHOTO, 8, 38,ANIM_FROM_BOTTOM);

	aux = getObjArrayPos(pMe, ID_CD_CONTACT_PHOTO);
	pMe->objArray[aux].type = OBJ_IMAGE_NODEALLOC;
	if (contactAux->pImage != NULL)
		pMe->objArray[aux].pImage = contactAux->pImage;
	else
		pMe->objArray[aux].pImage =  pMe->pImageCntNoPhoto;

	pMe->menuDetailed.menuPos			= 0;
	pMe->menuDetailed.menuFirstItemPos	= 0;

	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BASE, OBJ_PRIMITIVE_FILLRECT, 0, 155, pMe->nWidth, 34, pMe->currentThemeColor,ANIM_FROM_BOTTOM);
	AddImage2ArrayAnim(pMe,IDB_SELECTION_SMALL_BOX_5022,0, 155,ANIM_FROM_BOTTOM);

	AddString2Array(pMe, GOTHAM_14_BLACK, IDS_STRING_BUTTON_CALL_1033, 0, pMe->nHeight);
	ObjArraySetVisible(pMe, IDS_STRING_BUTTON_CALL_1033,FALSE);
	AddString2Array(pMe, GOTHAM_14_BLACK, IDS_STRING_BUTTON_SEND_MSG_1030, 0, pMe->nHeight);
	ObjArraySetVisible(pMe, IDS_STRING_BUTTON_SEND_MSG_1030,FALSE);
	AddString2Array(pMe, GOTHAM_14_BLACK, IDS_STRING_BUTTON_ADD_FAV_1034, 0, pMe->nHeight);
	ObjArraySetVisible(pMe, IDS_STRING_BUTTON_ADD_FAV_1034,FALSE);
	AddString2Array(pMe, GOTHAM_14_BLACK, IDS_STRING_BUTTON_EDIT_1031, 0, pMe->nHeight);
	ObjArraySetVisible(pMe, IDS_STRING_BUTTON_EDIT_1031,FALSE);
	AddString2Array(pMe, GOTHAM_14_BLACK, IDS_STRING_BUTTON_DELETE_1032, 0, pMe->nHeight);
	ObjArraySetVisible(pMe, IDS_STRING_BUTTON_DELETE_1032,FALSE);
	AddString2Array(pMe, GOTHAM_14_BLACK, IDS_STRING_BUTTON_VIEW_CNT_1044, 0, pMe->nHeight);
	ObjArraySetVisible(pMe, IDS_STRING_BUTTON_VIEW_CNT_1044,FALSE);

	for (i=0; i<5; i++)
	{
		AddPrimitive2ArrayAnim(pMe, ID_MENUS_LINE_SEPARATOR_00+i, OBJ_PRIMITIVE_FILLRECT, 0, 155+(33*i), pMe->nWidth, 1, MAKE_RGB(230,230,230),ANIM_FROM_BOTTOM);
		AddString2ArrayAnim(pMe, GOTHAM_18_BLACK, ID_MENUS_ITEM_NAME_00+i, 14, 160+(33*i),ANIM_FROM_BOTTOM);
		AddString2ArrayAnim(pMe, HELVETICA_11_GRAY, ID_MENUS_ITEM_DESCRIPTION_00+i, pMe->nWidth - 12, 164+(33*i),ANIM_FROM_BOTTOM);
		
		aux = getObjArrayPos(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i);
		pMe->objArray[aux].align = FONT_ALIGN_RIGHT;
	}

	i = 0;

	while (i < CONTACT_MAX_LBL_STRING)
	{
		if(contactAux->pTelephone[i].pLabel!= NULL)
		{
			pMe->menuDetailed.buttonArray[i].str1 = contactAux->pTelephone[i].pLabel;
			pMe->menuDetailed.buttonArray[i].str2 = contactAux->pTelephone[i].pString;
			pMe->menuDetailed.buttonArray[i].type	= BUTTON_CALL;

			i++;
		}
		else
			break;
	}

	pMe->menuDetailed.offset		= 5 - i;
	if (pMe->nOldState == STATE_CALLS || pMe->nOldState == STATE_CALLS_MIN)
		pMe->menuDetailed.subState		= MENU_CALL;
	else
		pMe->menuDetailed.subState		= MENU_CONTACT;

	fillMenuOptions(pMe);
}

static void fillMenuOptions(HelloSonar *pMe)
{
	int idAux;
	int aux;
	int i = 0;
	int aux2;

	switch(pMe->menuDetailed.subState)
	{
	case MENU_CONTACT:
		for (i=0; i<5; i++)
		{
			switch(i)
			{
			case 0:
				idAux = IDS_STRING_BUTTON_CALL_1033;
				break;
			case 1:
				idAux = IDS_STRING_BUTTON_SEND_MSG_1030;
				break;
			case 2:
				idAux = IDS_STRING_BUTTON_ADD_FAV_1034;
				break;
			case 3:
				idAux = IDS_STRING_BUTTON_EDIT_1031;
				break;	
			case 4:
				idAux = IDS_STRING_BUTTON_DELETE_1032;
				break;
			}

			aux		= getObjArrayPos(pMe, ID_MENUS_ITEM_NAME_00+i);
			aux2	= getObjArrayPos(pMe, idAux);

			pMe->objArray[aux].pStr	= pMe->objArray[aux2].pStr;
			ObjArraySetVisible(pMe, ID_MENUS_ITEM_NAME_00+i, TRUE);
			ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i,FALSE);
		}
		pMe->menuDetailed.buttonNumber  = 5;
		break;
	case MENU_CNCT_CALL :
		pMe->menuDetailed.buttonNumber = 0;
		while(i<5)
		{
			if ((i-pMe->menuDetailed.offset)<CONTACT_MAX_LBL_STRING 
				&& pMe->menuDetailed.buttonArray[i-pMe->menuDetailed.offset].str1 != NULL 
				&& i >= pMe->menuDetailed.offset)
			{
				aux	= getObjArrayPos(pMe, ID_MENUS_ITEM_NAME_00+i);
				pMe->objArray[aux].pStr	= pMe->menuDetailed.buttonArray[i-pMe->menuDetailed.offset].str1;
				ObjArraySetVisible(pMe, ID_MENUS_ITEM_NAME_00+i, TRUE);

				aux	= getObjArrayPos(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i);
				pMe->objArray[aux].pStr	= pMe->menuDetailed.buttonArray[i-pMe->menuDetailed.offset].str2;
				ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i,TRUE);

				pMe->menuDetailed.buttonNumber++;
			}
			else
			{
				ObjArraySetVisible(pMe, ID_MENUS_ITEM_NAME_00+i, FALSE);
				ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i,FALSE);
			}

			i++;
		}
		break;
	case MENU_CALL:
		for (i=2; i<5; i++)
		{
			switch(i)
			{
			case 2:
				idAux = IDS_STRING_BUTTON_CALL_1033;
				break;
			case 3:
				idAux = IDS_STRING_BUTTON_SEND_MSG_1030;
				break;
			case 4:
				idAux = IDS_STRING_BUTTON_VIEW_CNT_1044;
				break;
			}

			aux		= getObjArrayPos(pMe, ID_MENUS_ITEM_NAME_00+i);
			aux2	= getObjArrayPos(pMe, idAux);

			pMe->objArray[aux].pStr	= pMe->objArray[aux2].pStr;
			ObjArraySetVisible(pMe, ID_MENUS_ITEM_NAME_00+i, TRUE);
			ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i,FALSE);
		}

		ObjArraySetVisible(pMe, ID_MENUS_ITEM_NAME_00, FALSE);
		ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00,FALSE);
		ObjArraySetVisible(pMe, ID_MENUS_ITEM_NAME_01, FALSE);
		ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_01,FALSE);
		ObjArraySetVisible(pMe, ID_MENUS_LINE_SEPARATOR_00,FALSE);
		ObjArraySetVisible(pMe, ID_MENUS_LINE_SEPARATOR_01, FALSE);

		pMe->menuDetailed.buttonNumber  = 3;
		break;
	}

	updateMenuOption(pMe);
}

static void updateMenuOption(HelloSonar* pMe)
{
	int menuPosaux;
	int i;

	switch(pMe->menuDetailed.subState)
	{
	case MENU_CNCT_CALL:
		menuPosaux = pMe->menuDetailed.menuPos + pMe->menuDetailed.offset;
		break;
	case MENU_CONTACT:
		menuPosaux = pMe->menuDetailed.menuPos;
		break;
	case MENU_CALL:
		menuPosaux = pMe->menuDetailed.menuPos + 2;
		break;
	}

	ObjArraySetDestination(pMe, IDB_SELECTION_BOX_BASE, 0, 155 + (33*menuPosaux), MESSAGE_BACKGROUNDSPEED);
	ObjArraySetDestination(pMe, IDB_SELECTION_SMALL_BOX_5022, 0, 155 + (33*menuPosaux), MESSAGE_BACKGROUNDSPEED);

	for (i=0; i < MAX_ITEMS_PP_CONTACT_DETAIL; i++)
	{
		if (i == menuPosaux)
			ObjArraySetFont(pMe, ID_MENUS_ITEM_NAME_00+i, GOTHAM_18_WHITE);
		else
			ObjArraySetFont(pMe, ID_MENUS_ITEM_NAME_00+i, GOTHAM_18_BLACK);
	}
}

static void loadContactInfoScreen(HelloSonar *pMe)
{
	int aux;
	int i;
	int buttonCount = 0;

	Contact	*contactAux = pMe->pCurrentContact;

	FREEIF(pMe->menuDetailed.buttonArray)

	pMe->menuDetailed.buttonArray = MALLOC(sizeof(Button) * MAX_ITEMS_CONTACT_DETAIL);


	AddImage2ArrayAnim(pMe,IDB_UPPER_BAR_5017,0,0,ANIM_FROM_RIGHT);
	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BACKGROUND, OBJ_PRIMITIVE_FILLRECT, 0, 30,pMe->nWidth,pMe->nHeight, MAKE_RGB(255,255,255),ANIM_FROM_RIGHT);
	AddString2ArrayAnim(pMe, GOTHAM_14_B_BLACK, IDS_STRING_ICON_1022, pMe->nWidth/2, 8,ANIM_FROM_RIGHT);
	aux = getObjArrayPos(pMe, IDS_STRING_ICON_1022);
	pMe->objArray[aux].align = FONT_ALIGN_CENTER;
	AddString2ArrayAnim(pMe, GOTHAM_18_BLACK, ID_CD_CONTACT_DISPNAME, 89, 37, ANIM_FROM_RIGHT);

	aux = getObjArrayPos(pMe, ID_CD_CONTACT_DISPNAME);
	if (contactAux->pDisplayName != NULL)
	{
		pMe->objArray[aux].pStr		= contactAux->pDisplayName;
		pMe->objArray[aux].type		= OBJ_STRING_NODEALLOC;
	}

	AddString2ArrayAnim(pMe, HELVETICA_11_GRAY, ID_CD_CONTACT_STATUS, 58, 60,ANIM_FROM_RIGHT);
	aux = getObjArrayPos(pMe, ID_CD_CONTACT_STATUS);

	if (contactAux->pStatus != NULL)
	{
		pMe->objArray[aux].pStr		= contactAux->pStatus;
		pMe->objArray[aux].type		= OBJ_STRING_AREA_NODEALLOC;
		pMe->objArray[aux].width	= pMe->nWidth - 50 - 12;
		pMe->objArray[aux].height 	= 70;
	}

	if (contactAux->favorite == TRUE)
	{
		AddImage2ArrayAnim(pMe,ID_CD_CONTACT_FAVORITE, 58,42,ANIM_FROM_RIGHT);
		aux = getObjArrayPos(pMe, ID_CD_CONTACT_FAVORITE);
		pMe->objArray[aux].pImage = pMe->pImageIconFavorite;
	}

	AddImage2ArrayAnim(pMe,ID_CD_CONTACT_FRAME, 2,33,ANIM_FROM_RIGHT);
	AddImage2ArrayAnim(pMe,ID_CD_CONTACT_PHOTO, 8,38,ANIM_FROM_RIGHT);

	aux = getObjArrayPos(pMe, ID_CD_CONTACT_PHOTO);
	pMe->objArray[aux].type = OBJ_IMAGE_NODEALLOC;
	if (contactAux->pImage != NULL)
		pMe->objArray[aux].pImage = contactAux->pImage;
	else
		pMe->objArray[aux].pImage =  pMe->pImageCntNoPhoto;

	pMe->menuDetailed.menuPos			= 0;
	pMe->menuDetailed.menuFirstItemPos	= 0;

	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BASE, OBJ_PRIMITIVE_FILLRECT, 0, 122, pMe->nWidth, 34, pMe->currentThemeColor, ANIM_FROM_RIGHT);
	AddImage2ArrayAnim(pMe,ID_CD_SELECTION_BOX,0,122,ANIM_FROM_RIGHT);

	AddImage2Array(pMe,IDB_SELECTION_SMALL_BOX_5022,0,pMe->nHeight);
	ObjArraySetVisible(pMe,IDB_SELECTION_SMALL_BOX_5022,FALSE);
	AddImage2Array(pMe,IDB_SELECTION_BIG_BOX_5023,0,pMe->nHeight);
	ObjArraySetVisible(pMe,IDB_SELECTION_BIG_BOX_5023,FALSE);

	i = 0;

	while (i < CONTACT_MAX_LBL_STRING && buttonCount < (MAX_ITEMS_CONTACT_DETAIL-3))
	{
		if(contactAux->pTelephone[i].pLabel!= NULL)
		{
			pMe->menuDetailed.buttonArray[buttonCount].str1 = contactAux->pTelephone[i].pLabel;
			pMe->menuDetailed.buttonArray[buttonCount].str2 = contactAux->pTelephone[i].pString;
			pMe->menuDetailed.buttonArray[buttonCount].type	= BUTTON_CALL;

			buttonCount ++;
		}
		else
			break;

		i++;
	}

	i = 0;

	while (i < CONTACT_MAX_LBL_STRING && buttonCount < (MAX_ITEMS_CONTACT_DETAIL-3))
	{
		if(contactAux->pEmail[i].pLabel!= NULL)
		{
			pMe->menuDetailed.buttonArray[buttonCount].str1 = contactAux->pEmail[i].pLabel;
			pMe->menuDetailed.buttonArray[buttonCount].str2 = contactAux->pEmail[i].pString;
			pMe->menuDetailed.buttonArray[buttonCount].type	= BUTTON_EMAIL;

			buttonCount ++;
		}
		else
			break;

		i++;
	}

	i = 0;

	while (i < CONTACT_MAX_LBL_STRING && buttonCount < (MAX_ITEMS_CONTACT_DETAIL-3))
	{
		if(contactAux->pAddress[i].pLabel!= NULL)
		{
			pMe->menuDetailed.buttonArray[buttonCount].str1 = contactAux->pAddress[i].pLabel;
			pMe->menuDetailed.buttonArray[buttonCount].str2 = contactAux->pAddress[i].pString;
			pMe->menuDetailed.buttonArray[buttonCount].type	= BUTTON_ADDRESS;

			buttonCount ++;

			pMe->menuDetailed.buttonArray[buttonCount].type	= BUTTON_NONE;
			buttonCount ++;
			pMe->menuDetailed.buttonArray[buttonCount].type	= BUTTON_NONE;
			buttonCount ++;

		}
		else
			break;

		i++;
	}

	
	pMe->menuDetailed.buttonArray[buttonCount].type	= BUTTON_SEND_MSG;
	buttonCount ++;

	pMe->menuDetailed.buttonArray[buttonCount].type	= BUTTON_EDIT;
	buttonCount ++;

	pMe->menuDetailed.buttonArray[buttonCount].type	= BUTTON_DELETE;


	pMe->menuDetailed.buttonNumber	= buttonCount + 1;


	AddString2Array(pMe, GOTHAM_14_BLACK, IDS_STRING_BUTTON_SEND_MSG_1030, 0, pMe->nHeight);
	ObjArraySetVisible(pMe, IDS_STRING_BUTTON_SEND_MSG_1030,FALSE);
	AddString2Array(pMe, GOTHAM_14_BLACK, IDS_STRING_BUTTON_EDIT_1031, 0, pMe->nHeight);
	ObjArraySetVisible(pMe, IDS_STRING_BUTTON_EDIT_1031,FALSE);
	AddString2Array(pMe, GOTHAM_14_BLACK, IDS_STRING_BUTTON_DELETE_1032, 0, pMe->nHeight);
	ObjArraySetVisible(pMe, IDS_STRING_BUTTON_DELETE_1032,FALSE);

	for (i=0; i<MAX_ITEMS_PP_CONTACT_DETAIL; i++)
	{
		AddPrimitive2ArrayAnim(pMe, ID_MENUS_LINE_SEPARATOR_00+i, OBJ_PRIMITIVE_FILLRECT, 0, 122+(33*i), pMe->nWidth, 1, MAKE_RGB(230,230,230),ANIM_FROM_RIGHT);
		AddString2ArrayAnim(pMe, HELVETICA_11_GRAY, ID_MENUS_ITEM_NAME_00+i, 58, 130+(33*i),ANIM_FROM_RIGHT);
		AddString2ArrayAnim(pMe, GOTHAM_14_BLACK, ID_MENUS_ITEM_DESCRIPTION_00+i, 74, 130+(33*i),ANIM_FROM_RIGHT);
	}

	fillContactInfoScreen(pMe);
}

static void fillContactInfoScreen(HelloSonar *pMe)
{
	int i = 0;
	int aux;
	int aux2;

#ifdef DEBUG_STACK
	DBGPRINTF("FillContactInfoScreen");
#endif

	while ( i < MAX_ITEMS_PP_CONTACT_DETAIL && (i+pMe->menuDetailed.menuFirstItemPos<pMe->menuDetailed.buttonNumber))
	{
		aux = getObjArrayPos(pMe, ID_MENUS_ITEM_NAME_00+i);

		if (pMe->menuDetailed.buttonArray[i+pMe->menuDetailed.menuFirstItemPos].type == BUTTON_SEND_MSG)
		{
			aux2 = getObjArrayPos(pMe, IDS_STRING_BUTTON_SEND_MSG_1030);
			pMe->objArray[aux].pStr     = pMe->objArray[aux2].pStr;
			pMe->objArray[aux].align	= FONT_ALIGN_LEFT;
			pMe->objArray[aux].destX 	= 14;
			pMe->objArray[aux].fontId	= GOTHAM_18_BLACK;
		}
		else if (pMe->menuDetailed.buttonArray[i+pMe->menuDetailed.menuFirstItemPos].type == BUTTON_EDIT)
		{
			aux2 = getObjArrayPos(pMe, IDS_STRING_BUTTON_EDIT_1031);
			pMe->objArray[aux].pStr     = pMe->objArray[aux2].pStr;
			pMe->objArray[aux].align	= FONT_ALIGN_LEFT;
			pMe->objArray[aux].destX 	= 14;
			pMe->objArray[aux].fontId	= GOTHAM_18_BLACK;
		}
		else if (pMe->menuDetailed.buttonArray[i+pMe->menuDetailed.menuFirstItemPos].type == BUTTON_DELETE)
		{
			aux2 = getObjArrayPos(pMe, IDS_STRING_BUTTON_DELETE_1032);
			pMe->objArray[aux].pStr     = pMe->objArray[aux2].pStr;
			pMe->objArray[aux].align	= FONT_ALIGN_LEFT;
			pMe->objArray[aux].destX 	= 14;
			pMe->objArray[aux].fontId	= GOTHAM_18_BLACK;
		}
		else
		{
			pMe->objArray[aux].pStr     = pMe->menuDetailed.buttonArray[i+pMe->menuDetailed.menuFirstItemPos].str1;
			pMe->objArray[aux].align	= FONT_ALIGN_RIGHT;
			pMe->objArray[aux].destX 	= 58;
			pMe->objArray[aux].fontId	= HELVETICA_11_GRAY;
		}

		pMe->objArray[aux].type		= OBJ_STRING_NODEALLOC;

		aux = getObjArrayPos(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i);
		pMe->objArray[aux].pStr     = pMe->menuDetailed.buttonArray[i+pMe->menuDetailed.menuFirstItemPos].str2;

		if (pMe->menuDetailed.buttonArray[i+pMe->menuDetailed.menuFirstItemPos].type == BUTTON_ADDRESS)
		{
			pMe->objArray[aux].type		= OBJ_STRING_AREA_NODEALLOC;
			pMe->objArray[aux].width	= 168;
			pMe->objArray[aux].height	= 99;
		}
		else
			pMe->objArray[aux].type		= OBJ_STRING_NODEALLOC;


		if (pMe->menuDetailed.buttonArray[i+pMe->menuDetailed.menuFirstItemPos].type == BUTTON_NONE)
			ObjArraySetVisible(pMe, ID_MENUS_LINE_SEPARATOR_00+i, FALSE);
		else
			ObjArraySetVisible(pMe, ID_MENUS_LINE_SEPARATOR_00+i, TRUE);


		i++;
	}

	updateMenuDetailedObjects(pMe);
}

static void LoadAddressBook(HelloSonar *pMe, boolean extended)
{
	int i;
	int aux;

#ifdef DEBUG_STACK
	DBGPRINTF("LoadAddressBook");
#endif

	AddImage2ArrayAnim(pMe, IDB_UPPER_SHADOW_5021, 0, -13, ANIM_FROM_BOTTOM);
	AddImage2ArrayAnim(pMe, IDB_UPPER_BAR_5017, 0, 0, ANIM_FROM_BOTTOM);
	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BACKGROUND, OBJ_PRIMITIVE_FILLRECT, 0, 30, pMe->nWidth, pMe->nHeight-30, MAKE_RGB(255,255,255), ANIM_FROM_BOTTOM);
	AddString2ArrayAnim(pMe, GOTHAM_14_B_BLACK, IDS_STRING_ICON_1022, pMe->nWidth/2, 8, ANIM_FROM_BOTTOM);

	aux = getObjArrayPos(pMe, IDS_STRING_ICON_1022);
	pMe->objArray[aux].align = FONT_ALIGN_CENTER;

	AddPrimitive2ArrayAnim(pMe, IDB_SELECTION_BOX_BASE, OBJ_PRIMITIVE_FILLRECT, 0, 31, pMe->nWidth, 60, pMe->currentThemeColor, ANIM_FROM_BOTTOM);
	AddImage2ArrayAnim(pMe,IDB_SELECTION_BOX_5013,0,31, ANIM_FROM_BOTTOM);

	initializeMenu(pMe, LIST_CONTACTS);

	for (i=0; i<AB_NO_CONTACTS_PAGE; i++)
	{
		AddString2ArrayAnim(pMe, GOTHAM_18_BLACK, ID_MENUS_ITEM_NAME_00+i, 14, 38 +(32*i), ANIM_FROM_BOTTOM);
		AddImage2ArrayAnim(pMe,ID_MENUS_ICON_FAVORITE_00+i, 14, 44 +(32*i), ANIM_FROM_BOTTOM);
		AddImage2ArrayAnim(pMe,ID_MENUS_PHOTO_FRAME_00+i, pMe->nWidth-50, 35 +(32*i), ANIM_FROM_BOTTOM);
		AddImage2ArrayAnim(pMe,ID_MENUS_ICON_NOPHOTO_00+i, pMe->nWidth-44, 40 +(32*i), ANIM_FROM_BOTTOM);
		AddString2ArrayAnim(pMe, HELVETICA_11_GRAY, ID_MENUS_ITEM_DESCRIPTION_00+i, 20, 65 +(32*i), ANIM_FROM_BOTTOM);
		aux = getObjArrayPos(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i);
		pMe->objArray[aux].type		= OBJ_STRING_AREA_NODEALLOC;
		pMe->objArray[aux].width	= 180;
		pMe->objArray[aux].height 	= 15;
		AddPrimitive2ArrayAnim(pMe, ID_MENUS_LINE_SEPARATOR_00+i, OBJ_PRIMITIVE_FILLRECT, 0,  31+(32*i), pMe->nWidth, 1, MAKE_RGB(230,230,230), ANIM_FROM_BOTTOM);
	}

	searchBox_Init(pMe, 1);

	if (extended == FALSE)
	{
		pMe->menu->itemsPerPage -= 5;
		pMe->bDrawHour			 = TRUE;
	}

	FillAddressBook(pMe);
}


static void FillAddressBook(HelloSonar *pMe)
{
	int i;
	int aux;
	int j = 0;

	ListNode	*nodeAux;
	Contact		*contactAux;

#ifdef DEBUG_STACK
	DBGPRINTF("FillAddressBook");
#endif

	if (pMe->searchBox[0].charPos>0)
	{
		j = pMe->menu->firstItemPosition;
		nodeAux = pMe->menu->filteredMenuList[j];
	}
	else
	{
		nodeAux = pMe->menu->pFirstItemPointer;
	}


	for (i=0; i<AB_NO_CONTACTS_PAGE; i++)
	{
		if ((nodeAux != NULL)&&(i<pMe->contactsList.cantNode))
		{
			ObjArraySetVisible(pMe,  ID_MENUS_ITEM_NAME_00+i, TRUE);

			aux = getObjArrayPos(pMe, ID_MENUS_ITEM_NAME_00+i);

			contactAux = (Contact*) nodeAux->pNode;


			pMe->objArray[aux].pStr		= contactAux->pDisplayName;
			pMe->objArray[aux].type		= OBJ_STRING_NODEALLOC;

			ObjArraySetVisible(pMe,  ID_MENUS_ICON_NOPHOTO_00+i, TRUE);

			if (contactAux->pStatus != NULL)
			{			
				aux = getObjArrayPos(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i);
				pMe->objArray[aux].pStr		= contactAux->pStatus;

				ObjArraySetVisible(pMe,  ID_MENUS_ITEM_DESCRIPTION_00+i, TRUE);
			}
			else
			{
				ObjArraySetVisible(pMe,  ID_MENUS_ITEM_DESCRIPTION_00+i, FALSE);
			}

			if (contactAux->favorite == TRUE)
			{
				ObjArraySetVisible(pMe,  ID_MENUS_ICON_FAVORITE_00+i, TRUE);
				ObjArraySetPosition(pMe, ID_MENUS_ITEM_NAME_00+i, 42,DONTCHANGE_VALUE);
			}
			else
			{
				ObjArraySetVisible(pMe,  ID_MENUS_ICON_FAVORITE_00+i, FALSE);
				ObjArraySetPosition(pMe, ID_MENUS_ITEM_NAME_00+i, 14,DONTCHANGE_VALUE);
			}

			aux = getObjArrayPos(pMe, ID_MENUS_ICON_NOPHOTO_00+i);
			pMe->objArray[aux].type = OBJ_IMAGE_NODEALLOC;
			if (contactAux->pImage != NULL)
				pMe->objArray[aux].pImage = contactAux->pImage;
			else
				pMe->objArray[aux].pImage =  pMe->pImageCntNoPhoto;

			aux = getObjArrayPos(pMe, ID_MENUS_PHOTO_FRAME_00+i);
			pMe->objArray[aux].type = OBJ_IMAGE_NODEALLOC;
			pMe->objArray[aux].pImage =  pMe->pImageCntPhotoFrame;

			if (pMe->searchBox[0].charPos>0)
			{
				j++;
				nodeAux = pMe->menu->filteredMenuList[j];
			}
			else
				nodeAux =  (ListNode*) nodeAux->pNext;
		}	
		else
		{
			ObjArraySetVisible(pMe,  ID_MENUS_ITEM_NAME_00+i, FALSE);
			ObjArraySetVisible(pMe,  ID_MENUS_ITEM_DESCRIPTION_00+i, FALSE);
			ObjArraySetVisible(pMe,  ID_MENUS_ICON_FAVORITE_00+i, FALSE);
			ObjArraySetVisible(pMe,  ID_MENUS_ICON_NOPHOTO_00+i, FALSE);
			ObjArraySetVisible(pMe,  ID_MENUS_PHOTO_FRAME_00+i, FALSE);
		}

	}

	updateMenuObjects(pMe);
	
	if ((pMe->menu->filteredMenuItems == 0 && pMe->searchBox[0].charPos>0) || pMe->contactsList.cantNode == 0)
	{
		ObjArraySetVisible(pMe,  ID_MENUS_ICON_NOPHOTO_00, FALSE);
		ObjArraySetVisible(pMe,  ID_MENUS_PHOTO_FRAME_00, FALSE);
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_5013, FALSE);
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_BASE, FALSE);
	}
	else
	{
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_5013, TRUE);
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_BASE, TRUE);
	}
}


static void FillWidgets(HelloSonar *pMe)
{
	int aux;
	int i;
	int j = 0;

	ListNode	*nodeAux;
	Widget *widgetAux;

#ifdef DEBUG_STACK
	DBGPRINTF("FillWidgets");
#endif

	if (pMe->searchBox[0].charPos>0)
	{
		j = pMe->menu->firstItemPosition;
		nodeAux = pMe->menu->filteredMenuList[j];
	}
	else
	{
		nodeAux = pMe->menu->pFirstItemPointer;
	}

	for (i=0; i<AB_NO_WIDGETS_PAGE; i++)
	{
		if ((nodeAux != NULL)&&(i<pMe->widgetsList.cantNode))
		{
			widgetAux = (Widget*) nodeAux->pNode;

			aux = getObjArrayPos(pMe, ID_MENUS_ITEM_NAME_00+i);
			pMe->objArray[aux].pStr		= widgetAux->pDescription;
			pMe->objArray[aux].type		= OBJ_STRING_NODEALLOC;
			ObjArraySetVisible(pMe, ID_MENUS_ITEM_NAME_00+i,TRUE);

			aux = getObjArrayPos(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i);
			pMe->objArray[aux].pStr		= widgetAux->pName;
			pMe->objArray[aux].type		= OBJ_STRING_NODEALLOC;
			ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i,TRUE);

			aux = getObjArrayPos(pMe, ID_MENUS_ICON_NOPHOTO_00+i);
			pMe->objArray[aux].pImage  = widgetAux->pImage;
			pMe->objArray[aux].type		= OBJ_IMAGE_NODEALLOC;

			aux = getObjArrayPos(pMe, ID_MENUS_PHOTO_FRAME_00+i);
			pMe->objArray[aux].pImage  = pMe->pImageCntPhotoFrame;

			if (pMe->searchBox[0].charPos>0)
			{
				j++;
				nodeAux = pMe->menu->filteredMenuList[j];
			}
			else
				nodeAux =  (ListNode*) nodeAux->pNext;
		}	
		else
		{
			ObjArraySetVisible(pMe, ID_MENUS_ITEM_NAME_00+i,FALSE);
			ObjArraySetVisible(pMe, ID_MENUS_ITEM_DESCRIPTION_00+i,FALSE);
			ObjArraySetVisible(pMe, ID_MENUS_ICON_NOPHOTO_00+i,FALSE);
			ObjArraySetVisible(pMe, ID_MENUS_PHOTO_FRAME_00+i,FALSE);
		}

	}

	if ((pMe->menu->filteredMenuItems == 0 && pMe->searchBox[0].charPos>0) || pMe->widgetsList.cantNode == 0)
	{
		ObjArraySetVisible(pMe,  ID_MENUS_ICON_NOPHOTO_00, FALSE);
		ObjArraySetVisible(pMe,  ID_MENUS_PHOTO_FRAME_00, FALSE);
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_5013, FALSE);
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_BASE, FALSE);
		ObjArraySetVisible(pMe,  ID_MENUS_ITEM_DESCRIPTION_00, FALSE);
		ObjArraySetVisible(pMe,  ID_MENUS_ITEM_DESCRIPTION_2_00, FALSE);
	}
	else
	{
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_5013, TRUE);
		ObjArraySetVisible(pMe,  IDB_SELECTION_BOX_BASE, TRUE);
	}

	updateMenuObjects(pMe);
}

static void ChangeMenuPosition(HelloSonar *pMe, int offset)
{
	int i;

	if (offset>0)
	{
		for(i=0; i<offset;i++)
		{
			if (pMe->menu->pFirstItemPointer!=NULL)
			{
				pMe->menu->pFirstItemPointer = (ListNode *) pMe->menu->pFirstItemPointer->pNext;
				pMe->menu->firstItemPosition ++;
			}
		}
	}
	else
	{
		for(i=0; i>offset;i--)
		{
			if (pMe->menu->pFirstItemPointer!=NULL)
			{
				pMe->menu->pFirstItemPointer = (ListNode *) pMe->menu->pFirstItemPointer->pPrev;
				pMe->menu->firstItemPosition --;
			}
		}
	}

}
 
void ReadFromWebCB(HelloSonar *pMe) 
{ 
	char buf[1024];  
	int byteCount; 

#ifdef DEBUG_STACK
	DBGPRINTF("ReadFromWebCB");
#endif 

	pMe->pWebRespInfo = IWEBRESP_GetInfo(pMe->pIWebResponse); 

 
	if (!WEB_ERROR_SUCCEEDED(pMe->pWebRespInfo->nCode)) 
	{ 
#ifdef DEBUG_HTTP
		DBGPRINTF("WEB_ERROR received");
#endif
		pMe->webStringBufferLenght = 0;
		pMe->serviceCalled = CONN_REQ_IDLE;
		return; 
	} 
 
	pMe->pISource = pMe->pWebRespInfo->pisMessage; 
	if (pMe->pISource == NULL) 
	{ 
#ifdef DEBUG_HTTP
		DBGPRINTF("pMe->pWebRespInfo->pisMessage = NULL");
#endif
		pMe->webStringBufferLenght = 0;
		pMe->serviceCalled = CONN_REQ_IDLE;
		return; 
	}	 

	byteCount = ISOURCE_Read(pMe->pISource, buf, sizeof(buf)); 
 
	switch (byteCount) 
	{
	case ISOURCE_WAIT:
		ISOURCE_Readable(pMe->pISource,&pMe->WebCBStruct); 
		break; 
 
	case ISOURCE_ERROR: 
#ifdef DEBUG_HTTP
		DBGPRINTF("ISOURCE_ERROR received");
#endif
		break; 
 
	case ISOURCE_END: 
#ifdef DEBUG_HTTP
		DBGPRINTF("ISOURCE_END received with lenght: %d", pMe->webStringBufferLenght);
#endif
		pMe->pWebStringBuffer[pMe->webStringBufferLenght] = '\0';
		ProcessXmlLine(pMe, pMe->pWebStringBuffer,pMe->webStringBufferLenght);
		pMe->webStringBufferLenght = 0;
		pMe->serviceCalled = CONN_REQ_IDLE;
		break; 
 
	default:   // data read; copy from chunk buffer 
		if (pMe->webStringBufferLenght +byteCount < WEBSTRINGBUFFERSIZE)
		{
			MEMCPY(pMe->pWebStringBuffer+pMe->webStringBufferLenght, buf, byteCount); 
			pMe->webStringBufferLenght += byteCount; 
		}
#ifdef DEBUG_HTTP
		else
		{
			DBGPRINTF("WEBSTRINGBUFFER EXCEED");
		}
#endif


		ISOURCE_Readable(pMe->pISource,&pMe->WebCBStruct); 
		break; 
   } 
}

static uint32 getUTCFromSonarResponse(HelloSonar *pMe, char *buf, int byteCount)
{
	uint32 utcTime = 0;
	int i = 0;
	char aux[5];
	int resul;

	JulianType date;
	
	while (i< byteCount)
	{
		if (STR_STARTSWITH(&(buf[i]), "nextSinceTime="))
		{
			i += STRLEN("nextSinceTime=") + 1;

			STRNCPY(aux, &(buf[i]), 4);
			aux[4]	= '\0'; 
			resul	= ATOI(aux);

			date.wYear = resul;
			i += 5;

			STRNCPY(aux, &(buf[i]), 2);
			aux[2]	= '\0'; 
			resul	= ATOI(aux);

			date.wMonth = resul;
			i += 3;

			STRNCPY(aux, &(buf[i]), 2);
			aux[2]	= '\0'; 
			resul	= ATOI(aux);

			date.wDay  = resul;
			i += 3;

			STRNCPY(aux, &(buf[i]), 2);
			aux[2]	= '\0'; 
			resul	= ATOI(aux);

			date.wHour  = resul;
			i += 3;

			STRNCPY(aux, &(buf[i]), 2);
			aux[2]	= '\0'; 
			resul	= ATOI(aux);

			date.wMinute  = resul;
			i += 3;

			STRNCPY(aux, &(buf[i]), 2);
			aux[2]	= '\0'; 
			resul	= ATOI(aux);

			date.wSecond  = resul;

			utcTime = JULIANTOSECONDS(&date);
			break;
		}
		i++;
	}

	return utcTime;
}

static void ProcessXmlLine(HelloSonar *pMe, char *buf, int byteCount)
{
	int i = 0;
	int j = 0;
	char *aux;

#ifdef DEBUG_STACK
	DBGPRINTF("ProcessXmlLine");
#endif

	if (pMe->serviceCalled==CONN_REQ_IDLE)
		return;
	
	if (pMe->serviceCalled == CONN_REQ_CONTACT_RETRIEVE_ALL
		|| pMe->serviceCalled == CONN_REQ_CONTACT_RETRIEVE_DETAIL)
	{

		aux = MALLOC (sizeof(char)*1024*3);

		while (i < byteCount)
		{ 

			if (STR_STARTSWITH(&(buf[i]), "<contact isFavorite="))
			{
				j = returnTagPosition(&(buf[i]), "</contact>");
				STRNCPY(aux, &(buf[i]), (j));
				aux[j] = '\0';

				ProcessContactInfo(pMe, aux);

				i += j + STRLEN("</contact>");

			}
			else if (STR_STARTSWITH(&(buf[i]), "<contactID>"))
			{	
				i += 11;

				j = returnTagPosition(&(buf[i]), "</contactID>");

				STRNCPY(aux, &(buf[i]), (j));
				aux[j] = '\0';
				
				AddContact2Array (pMe, ATOI(aux));

				i += j + STRLEN("</contactID>");
			}

			i++;
		}

#ifdef DEBUG_HTTP
		DBGPRINTF("Number of contacts in list: %d",   pMe->contactsList.cantNode);
#endif
		if (pMe->nState == STATE_ADRESSBOOK)
			FillContactData(pMe);

		saveContactData(pMe);
	}
	else if (pMe->serviceCalled == CONN_REQ_CONTACT_REORDER_ALL)
	{
		aux = MALLOC (sizeof(char)*1024*5);

		while (i < byteCount)
		{ 
			if (STR_STARTSWITH(&(buf[i]), "<contacts>"))
			{
				j = returnTagPosition(&(buf[i]), "</contacts>");
				STRNCPY(aux, &(buf[i]), (j));
				aux[j] = '\0';

				ReorderContacts(pMe, aux);

				i += j + STRLEN("</contacts>");

			}
			i++;

		}
		saveContactData(pMe);
	}
	else if (pMe->serviceCalled == CONN_REQ_CONTACT_RETRIEVE_TS)
	{
		while (i < byteCount)
		{ 
			if (STR_STARTSWITH(&(buf[i]), "<sonarResponse"))
			{
				i += STRLEN("<sonarResponse");

				pMe->mLastUpdate_Contact = getUTCFromSonarResponse(pMe, &(buf[i]), byteCount);
			}
			else if (STR_STARTSWITH(&(buf[i]), "<contacts"))
			{
				j = returnTagPosition(&(buf[i]), "</contact>");

				aux = MALLOC (sizeof(char)*(j+1));

				STRNCPY(aux, &(buf[i]), j);
				aux[j] = '\0';

				ProcessContactChanges(pMe, aux);

				i += j + STRLEN("</contact>");

			}

			i++;
		}

		saveContactData(pMe);
	}
	else if (pMe->serviceCalled == CONN_REQ_CALL_RETRIEVE_ALL)
	{

		aux = MALLOC (sizeof(char)*1024);

		while (i < byteCount)
		{ 

			if (STR_STARTSWITH(&(buf[i]), "<callLogEvent type="))
			{
				i += STRLEN("<callLogEvent type=") +1;

				j = returnTagPosition(&(buf[i]), "</callLogEvent>");
				STRNCPY(aux, &(buf[i]), (j));
				aux[j] = '\0';

				ProcessCallInfo(pMe, aux);

				i += j + STRLEN("</contact>");

			}

			i++;
		}

	}
	else if (pMe->serviceCalled == CONN_REQ_WIDGET_RETRIEVE_ALL)
	{

		aux = MALLOC (sizeof(char)*1024);

		while (i < byteCount)
		{ 

			if (STR_STARTSWITH(&(buf[i]), "<widgetID>"))
			{	
				i += STRLEN ("<widgetID>");

				j = returnTagPosition(&(buf[i]), "</widgetID>");

				STRNCPY(aux, &(buf[i]), (j));
				aux[j] = '\0';
				
				AddWidget2Array (pMe, ATOI(aux));

				i += j + STRLEN("</widgetID>");
			}
			i++;
		}

	}
	else if (pMe->serviceCalled == CONN_REQ_WIDGET_RETRIEVE_DETAIL)
	{

		aux = MALLOC (sizeof(char)*1024*3);

		while (i < byteCount)
		{ 

			if (STR_STARTSWITH(&(buf[i]), "<widget>"))
			{
				j = returnTagPosition(&(buf[i]), "</widget>");
				STRNCPY(aux, &(buf[i]), (j));
				aux[j] = '\0';

				ProcessWidgetInfo(pMe, aux);

				i += j + STRLEN("</widget>");

			}
			i++;
		}

		if (getActualConnectionState(pMe) == CONN_REQ_WIDGET_UPDATE_PENDING)	
		{
			removeActualConnectionState(pMe);
			putConnectionState(pMe, CONN_REQ_WIDGET_RETRIEVE_DETAIL);
		}
	}
	else if (pMe->serviceCalled == CONN_REQ_WIDGET_RETRIEVE_TS)
	{
		aux = MALLOC (sizeof(char)*1024);

		while (i < byteCount)
		{
			if (STR_STARTSWITH(&(buf[i]), "<sonarResponse"))
			{
				i += STRLEN("<sonarResponse");

				pMe->mLastUpdate_Widget = getUTCFromSonarResponse(pMe, &(buf[i]), byteCount);
			}
			else if STR_STARTSWITH(&(buf[i]), "widgets reordered=")
			{
				i += STRLEN("widgets reordered=") +1;
				if STR_STARTSWITH(&(buf[i]), "true")
				{
					putConnectionState(pMe, CONN_REQ_WIDGET_REORDER_ALL);
					i += STRLEN("true");
				}
			}
			else if (STR_STARTSWITH(&(buf[i]), "<installed>"))
			{
				i += STRLEN ("<installed>");
				while ( i < byteCount)
				{
					if (STR_STARTSWITH(&(buf[i]), "<widgetID>"))
					{	
						i += STRLEN ("<widgetID>");

						j = returnTagPosition(&(buf[i]), "</widgetID>");

						STRNCPY(aux, &(buf[i]), (j));
						aux[j] = '\0';
						
						AddWidget2Array(pMe, ATOI(aux));

						i += j + STRLEN("</widgetID>");
					}
					else if (STR_STARTSWITH(&(buf[i]), "</installed>"))
					{
						i += STRLEN ("</installed>");
						break;
					}
					
					i++;
				}
			}
			if (STR_STARTSWITH(&(buf[i]), "<uninstalled>"))
			{
				i += STRLEN ("<uninstalled>");
				while ( i < byteCount)
				{
					if (STR_STARTSWITH(&(buf[i]), "<widgetID>"))
					{	
						i += STRLEN ("<widgetID>");

						j = returnTagPosition(&(buf[i]), "</widgetID>");

						STRNCPY(aux, &(buf[i]), (j));
						aux[j] = '\0';
						
						RemoveWidgetFromArray(pMe, ATOI(aux));

						i += j + STRLEN("</widgetID>");
					}
					else if (STR_STARTSWITH(&(buf[i]), "</uninstalled>"))
					{
						i += STRLEN ("</uninstalled>");
						break;
					}
					
					i++;
				}
			}
			i++;
		}

		if (pMe->nState == STATE_WIDGETS)
			FillWidgets(pMe);
	}
	else if (pMe->serviceCalled == CONN_REQ_WIDGET_REORDER_ALL)
	{
		aux = MALLOC (sizeof(char)*1024*5);

		while (i < byteCount)
		{ 
			if (STR_STARTSWITH(&(buf[i]), "<widgets>"))
			{
				j = returnTagPosition(&(buf[i]), "</contacts>");
				STRNCPY(aux, &(buf[i]), (j));
				aux[j] = '\0';

				ReorderWidgets(pMe, aux);

				i += j + STRLEN("</widgets>");

			}
			i++;

		}

		if (pMe->nState == STATE_WIDGETS)
			FillWidgets(pMe);
	}
	else if (pMe->serviceCalled == CONN_REQ_CONTACT_RETRIEVE_PHOTO)
	{
		processImg(pMe, buf, byteCount);
		putConnectionState(pMe, CONN_REQ_CONTACT_CHECK_PHOTOS);
	}
	else if (pMe->serviceCalled == CONN_REQ_WIDGET_RETRIEVE_PHOTO)
	{
		processImg(pMe, buf, byteCount);
		putConnectionState(pMe, CONN_REQ_WIDGET_CHECK_PHOTOS);
	}
	else if (pMe->serviceCalled == CONN_REQ_SETTINGS_CHECK_CHANGES)
	{

		aux = MALLOC (sizeof(char)*1024*3);

		while (i < byteCount)
		{ 

			if (STR_STARTSWITH(&(buf[i]), "<settings>"))
			{
				j = returnTagPosition(&(buf[i]), "</settings>");
				STRNCPY(aux, &(buf[i]), j);
				aux[j] = '\0';

				ProcessSettingsChanges(pMe, aux, j);

				i += j + STRLEN("</settings>");

			}
			i++;
		}
	}	
	
	updateActiveMenu(pMe, FALSE);

	FREEIF(aux);

#ifdef DEBUG_HTTP
	DBGPRINTF("XML process finished");	
#endif

}

static void ProcessSettingsChanges(HelloSonar *pMe, char *buf, int lenght)
{
	int i = 0;
	char parameter [256];
	int j;


	while (i<lenght)
	{
		if (STR_STARTSWITH(&(buf[i]), "<setting name="))
		{
			i += STRLEN("<setting name=") +1;

			if (STR_STARTSWITH(&(buf[i]), "theme"))
			{
				i += STRLEN("theme") + 2;

				j = returnTagPosition(&(buf[i]), "</setting>");
				STRNCPY(parameter, &(buf[i]), j);
				parameter[j] = '\0';

				changeWallpaper(pMe, parameter);

				i += j + STRLEN("</setting>");
			}
		}

		i++;
	}


}

static void changeWallpaper(HelloSonar *pMe, char *name)
{
	int aux;
	RGBVAL oldColor = pMe->currentThemeColor;

	if (pMe->pCurrentBackground != NULL)
		IIMAGE_Release(pMe->pCurrentBackground);

	if (STR_STARTSWITH(name, "O"))
	{
		pMe->pCurrentBackground = ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, IDB_WALLPAPER_ORANGE_5103);
		pMe->currentThemeColor	= THEME_ORANGE;
	}
	else if (STR_STARTSWITH(name, "R"))
	{
		pMe->pCurrentBackground = ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, IDB_WALLPAPER_RED_5101);
		pMe->currentThemeColor	= THEME_RED;
	}
	else if (STR_STARTSWITH(name, "G"))
	{
		pMe->pCurrentBackground = ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, IDB_WALLPAPER_GREEN_5100);
		pMe->currentThemeColor	= THEME_GREEN;
	}
	else if (STR_STARTSWITH(name, "B"))
	{
		pMe->pCurrentBackground = ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, IDB_WALLPAPER_BLUE_5102);
		pMe->currentThemeColor	= THEME_BLUE;
	}

	if (oldColor != pMe->currentThemeColor)
	{
		aux = getObjArrayPos(pMe, IDB_SELECTION_BOX_BASE);

		if (aux != -1)
			pMe->objArray[aux].color = pMe->currentThemeColor;
		if (oldColor != NULL)
			saveSettings(pMe);
	}
}

static void processImg(HelloSonar *pMe, char *data, int lenght)
{
	AEECLSID cls;
	IMemAStream *pMemStream;
	IImage	*pIImage;
	
	if (pMe->serviceCalled == CONN_REQ_CONTACT_RETRIEVE_PHOTO)
		cls = ISHELL_GetHandler(pMe->piShell, HTYPE_VIEWER, "image/jpg");  
	else
		cls = ISHELL_GetHandler(pMe->piShell, HTYPE_VIEWER, "image/png");  
 
	if (cls) 
	{ 
		ISHELL_CreateInstance(pMe->piShell, cls, (void **)(&pIImage)); 
	}  
	
	if (lenght)
	{
		ISHELL_CreateInstance(pMe->piShell, AEECLSID_MEMASTREAM, (void **)(&pMemStream));

		if (pMemStream)
		{  
			//Create MEMSTREAM and use it as stream for the in-memory bitmap. 
			IMEMASTREAM_Set(pMemStream, data, lenght, 0, 0);
			IIMAGE_SetStream(pIImage, (IAStream*)pMemStream);
		}
	} 

	if (pMe->serviceCalled == CONN_REQ_CONTACT_RETRIEVE_PHOTO)
		((Contact *) (pMe->pNodeImgRequest->pNode))->pImage = pIImage;
	else
		((Widget *) (pMe->pNodeImgRequest->pNode))->pImage = pIImage;

}

static ListNode * returnNewNode(HelloSonar *pMe, int listType)
{
	List *listAux;

	switch (listType)
	{
	case LIST_CALLS:
		listAux = &(pMe->callsList);
		break;
	case LIST_CONTACTS:
		listAux = &(pMe->contactsList);
		break;
	case LIST_WIDGETS:
		listAux = &(pMe->widgetsList);
		break;
	default:
		return NULL;
	}

	if (listAux->cantNode!=0)
	{
		listAux->pLastElement->pNext										= MALLOC(sizeof(ListNode));
		((ListNode *) ((ListNode *) listAux->pLastElement->pNext)->pPrev)	=  listAux->pLastElement;
		listAux->pLastElement												= (ListNode*) listAux->pLastElement->pNext;
	}
	else
	{
		listAux->pFirstElement	= MALLOC(sizeof(ListNode));
		listAux->pLastElement	= listAux->pFirstElement;
	}

	listAux->cantNode ++;
	listAux->pLastElement->pNext = 0;

	return listAux->pLastElement;
}

static void AddContact2Array(HelloSonar *pMe, int contactId)
{
	Contact *newContact;
	ListNode *newNode;
	
	if (pMe->contactsList.cantNode >= MAXCONTACTS)
		return;

	putConnectionState(pMe, CONN_REQ_CONTACT_RETRIEVE_DETAIL); 

	newNode = returnNewNode(pMe, LIST_CONTACTS);

	if (newNode!=NULL)
	{
		newContact				= MALLOC(sizeof(Contact));
		newContact->id			= contactId;
		newContact->needUpdate	= TRUE;

		newNode->pNode = newContact;
	}	
}

static void AddFullContact2Array(HelloSonar *pMe, Contact *newContact)
{
	ListNode *newNode;
	
	if (pMe->contactsList.cantNode >= MAXCONTACTS)
		return;

	newNode = returnNewNode(pMe, LIST_CONTACTS);

	if (newNode!=NULL)
	{
		newContact->needUpdate	= FALSE;
		newNode->pNode = newContact;
	}	
}

static void AddCall2Array(HelloSonar *pMe, Call *newCall, boolean removeOlder)
{
	ListNode *newNode;
	
	if (pMe->callsList.cantNode >= MAXCALLLOG)
	{
		if (removeOlder == TRUE)
		{
			newNode = MALLOC(sizeof(ListNode));
			newNode->pPrev = NULL;
			((ListNode *) newNode->pNext) = (ListNode *) pMe->callsList.pFirstElement;

			if (pMe->callsList.cantNode!=0)
			{
				RemoveCall(pMe, pMe->callsList.pLastElement);
				((ListNode *) ((ListNode *) pMe->callsList.pFirstElement)->pPrev) = newNode;
			}
			else
			{
				pMe->callsList.pLastElement = newNode;
				pMe->callsList.cantNode = 1;
			}
			pMe->callsList.pFirstElement = newNode;				
		}	
		else
			return;
	}
	else
		newNode = returnNewNode(pMe, LIST_CALLS);


	if (newNode!=NULL)
		newNode->pNode = newCall;
}

static void AddWidget2Array(HelloSonar *pMe, int widgetId)
{
	Widget *newWidget;
	ListNode *newNode;
	
	if (pMe->widgetsList.cantNode >= MAXWIDGETS)
		return;

	putConnectionState(pMe, CONN_REQ_WIDGET_RETRIEVE_DETAIL); 

	newNode = returnNewNode(pMe, LIST_WIDGETS);

	if (newNode!=NULL)
	{
		newWidget				= MALLOC(sizeof(Widget));
		newWidget->id			= widgetId;
		newWidget->needUpdate	= TRUE;

		newNode->pNode			= newWidget;
	}
	
}

static void RemoveWidgetFromArray(HelloSonar *pMe, int widgetId)
{
	ListNode *nodeAux;
	ListNode *prevNodeAux;
	ListNode *nextNodeAux;

	Widget *widgetAux;
	int i = 0;

	nodeAux = ReturnWidgetNode (pMe, widgetId);

	if (nodeAux != NULL)
	{
		prevNodeAux = (ListNode *) nodeAux->pPrev;
		nextNodeAux = (ListNode *) nodeAux->pNext;


		if (prevNodeAux != NULL)
			((ListNode *) prevNodeAux->pNext) = (ListNode *) nextNodeAux;

		if (nextNodeAux != NULL)
			((ListNode *) nextNodeAux->pPrev) = (ListNode *) prevNodeAux;

	
		widgetAux = (Widget *) nodeAux->pNode;

		FREEIF(widgetAux->pCategory);
		FREEIF(widgetAux->pDescription);
		if (widgetAux->pImage != NULL)
		{
			IIMAGE_Release (widgetAux->pImage);
		}
		FREEIF(widgetAux->pImageURL);
		FREEIF(widgetAux->pName);
		FREEIF(widgetAux->pSearchName);

		FREE(widgetAux);
		FREE(nodeAux);
	}
}

static void RemoveCall(HelloSonar *pMe, ListNode *nodeCall)
{
	ListNode *prevNodeAux;
	ListNode *nextNodeAux;
	Call	*callAux;

	if (nodeCall != NULL)
	{
		prevNodeAux = (ListNode *) nodeCall->pPrev;
		nextNodeAux = (ListNode *) nodeCall->pNext;

		callAux = nodeCall->pNode;

		if (prevNodeAux != NULL)
			((ListNode *) prevNodeAux->pNext) = (ListNode *) nextNodeAux;

		if (nextNodeAux != NULL)
			((ListNode *) nextNodeAux->pPrev) = (ListNode *) prevNodeAux;


		FREEIF(callAux->pFormatedDate);
		FREEIF(callAux->pHour);
		FREEIF(callAux->telephone.pLabel);
		FREEIF(callAux->telephone.pString);

		FREE(callAux);
		FREE(nodeCall);
	}
}

static void RemoveContact(HelloSonar *pMe, int contactId)
{
	ListNode *nodeAux;
	ListNode *prevNodeAux;
	ListNode *nextNodeAux;

	Contact *contactAux;
	int i = 0;

	nodeAux = ReturnContactNode (pMe, contactId);

	if (nodeAux != NULL)
	{
		prevNodeAux = (ListNode *) nodeAux->pPrev;
		nextNodeAux = (ListNode *) nodeAux->pNext;


		if (prevNodeAux != NULL)
			((ListNode *) prevNodeAux->pNext) = (ListNode *) nextNodeAux;

		if (nextNodeAux != NULL)
			((ListNode *) nextNodeAux->pPrev) = (ListNode *) prevNodeAux;

	
		contactAux	= (Contact *) nodeAux->pNode;

		FREEIF(contactAux->pDisplayName);
		FREEIF(contactAux->pStatus);
		FREEIF(contactAux->pFirstName);
		FREEIF(contactAux->pLastName);
		FREEIF(contactAux->pSearchName);
		FREEIF(contactAux->pCompany);
		FREEIF(contactAux->pImageURL);

		if (contactAux->pImage != NULL)
			IIMAGE_Release(contactAux->pImage);

		if (contactAux->pTelephone!=NULL)
		{
			for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
			{
				FREEIF(contactAux->pTelephone[i].pLabel);
				FREEIF(contactAux->pTelephone[i].pString);
			}
			FREE(contactAux->pTelephone);
		}

		if (contactAux->pAddress!=NULL)
		{
			for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
			{
				FREEIF(contactAux->pAddress[i].pLabel);
				FREEIF(contactAux->pAddress[i].pString);
			}
			FREE(contactAux->pAddress);
		}

		if (contactAux->pEmail!=NULL)
		{
			for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
			{
				FREEIF(contactAux->pEmail[i].pLabel);
				FREEIF(contactAux->pEmail[i].pString);
			}
			FREE(contactAux->pEmail);
		}

		FREE(contactAux);
		FREE(nodeAux);
	}
	
}

static void ReorderWidgets(HelloSonar *pMe, char *line)
{
	int i = 0;
	int len;

	ListNode *nodeIterator;
	ListNode *nodeAux;

	char *aux;

	len = STRLEN(line);
	aux = MALLOC(sizeof(char)*10);
	nodeIterator = pMe->widgetsList.pFirstElement;


	while (i < len)
	{
		if STR_STARTSWITH(&(line[i]), "<widgetID>")
		{	
			i += returnTagText(&(line[i]), "widgetID", aux);
			nodeAux = ReturnWidgetNode(pMe, ATOI(aux));

			if (nodeAux != NULL)
			{
				swapNodes(nodeAux,nodeIterator);

				nodeIterator = (ListNode*) nodeIterator->pNext;
			}
		}
		i++;
	}


	FREE(aux);
}

static void ReorderContacts(HelloSonar *pMe, char *line)
{
	int i = 0;
	int len;

	ListNode *nodeIterator;
	ListNode *nodeAux;

	char *aux;

	len = STRLEN(line);
	aux = MALLOC(sizeof(char)*10);
	nodeIterator = pMe->contactsList.pFirstElement;


	while (i < len)
	{
		if STR_STARTSWITH(&(line[i]), "<contactID>")
		{	
			i += returnTagText(&(line[i]), "contactID", aux);
			nodeAux = ReturnContactNode(pMe, ATOI(aux));

			if (nodeAux != NULL)
			{
				swapNodes(nodeAux,nodeIterator);

				nodeIterator = (ListNode*) nodeIterator->pNext;
			}
		}
		i++;
	}


	FREE(aux);
}

static void swapNodes(ListNode *a, ListNode *b)
{
	void *aux;

	aux		 = a->pNode;
	a->pNode = b->pNode;
	b->pNode = aux;
}


static void ProcessContactInfo(HelloSonar *pMe, char *line)
{

	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;


	char aux[1024];

	int len =  STRLEN(line);

	Contact  *tmpContact;

	tmpContact = MALLOC(sizeof(Contact));

	tmpContact->pEmail		= MALLOC(sizeof(LblString)*CONTACT_MAX_LBL_STRING);
	tmpContact->pAddress	= MALLOC(sizeof(LblString)*CONTACT_MAX_LBL_STRING);
	tmpContact->pTelephone	= MALLOC(sizeof(LblString)*CONTACT_MAX_LBL_STRING);


	while (i < len)
	{
		if STR_STARTSWITH(&(line[i]), "<contact isFavorite=")
		{	
			i += STRLEN("<contact isFavorite=") + 1;
			if (STRNCMP(&(line[i]), "true", 4)==0)
				tmpContact->favorite = TRUE;
			else
				tmpContact->favorite = FALSE;

			i+=6;
		}
		else if STR_STARTSWITH(&(line[i]), "<contactID>")
		{	
			i += returnTagText(&(line[i]), "contactID", aux);
			tmpContact->id = ATOI(aux);
		}
		else if STR_STARTSWITH(&(line[i]), "<firstName>")
		{
			i += returnTagText(&(line[i]), "firstName", aux);
			tmpContact->pFirstName = MALLOC(sizeof(char)*(STRLEN(aux)+1));
			STRCPY(tmpContact->pFirstName, aux);
		}
		else if STR_STARTSWITH(&(line[i]), "<lastName>")
		{
			i += returnTagText(&(line[i]), "lastName", aux);
			tmpContact->pLastName = MALLOC(sizeof(char)*(STRLEN(aux)+1));
			STRCPY(tmpContact->pLastName, aux);
		}
		else if STR_STARTSWITH(&(line[i]), "<displayName>")
		{
			i += returnTagText(&(line[i]), "displayName", aux);
			tmpContact->pDisplayName = MALLOC(sizeof(char)*(STRLEN(aux)+1));
			STRCPY(tmpContact->pDisplayName, aux);
		}
		else if STR_STARTSWITH(&(line[i]), "<images>")
		{
			j = i +  returnTagPosition(&(line[i]), "</images>");

			while (i < j)
			{
				if STR_STARTSWITH(&(line[i]), "thumbnail_36x36")
				{
					i += STRLEN("thumbnail_36x36") + 2;

					k =  returnTagPosition(&(line[i]), "</image>");

					tmpContact->pImageURL = MALLOC(sizeof(char)*(k+1));

					STRNCPY(tmpContact->pImageURL, &(line[i]), k);

					i += k +  STRLEN("</image>");
				}
				i++;
			}
		}
		else if STR_STARTSWITH(&(line[i]), "<company>")
		{
			i += returnTagText(&(line[i]), "company", aux);
			tmpContact->pCompany = MALLOC(sizeof(char)*(STRLEN(aux)+1));
			STRCPY(tmpContact->pCompany, aux);
		}
		else if STR_STARTSWITH(&(line[i]), "<status>")
		{
			i += returnTagText(&(line[i]), "status", aux);
			tmpContact->pStatus = MALLOC(sizeof(char)*(STRLEN(aux)+1));
			STRCPY(tmpContact->pStatus, aux);
		}
		else if STR_STARTSWITH(&(line[i]), "<emailAddresses>")
		{
			j = i +  returnTagPosition(&(line[i]), "</emailAddresses>");
			l = 0;

			while (i < j)
			{
				if STR_STARTSWITH(&(line[i]), "category")
				{
					i += STRLEN("category=") + 1;
					k = i;
					while (line[k] != '\"')
						k++;

					tmpContact->pEmail[l].pLabel =  MALLOC(sizeof(char)*(k-i+1));
					STRNCPY(tmpContact->pEmail[l].pLabel, &(line[i]), k-i);
					tmpContact->pEmail[l].pLabel[k-i] = '\0';

					i = k;
					while (line[i] != '>')
						i++;	

					i++;

					k = returnTagPosition(&(line[i]), "</emailAddress>");

					tmpContact->pEmail[l].pString =  MALLOC(sizeof(char)*(k+1));
					STRNCPY(tmpContact->pEmail[l].pString,  &(line[i]), k);
					tmpContact->pEmail[l].pString[k] = '\0';

					i += k + STRLEN("</emailAddress>");

					l++;

					if (l>=CONTACT_MAX_LBL_STRING)
						break;
				}
				i++;
			}

			i += STRLEN("</emailAddresses>");
		}
		else if STR_STARTSWITH(&(line[i]), "<addresses>")
		{
			j = i +  returnTagPosition(&(line[i]), "</addresses>");
			l = 0;

			while (i < j)
			{
				if STR_STARTSWITH(&(line[i]), "category")
				{
					i += STRLEN("category=") + 1;
					k = i;
					while (line[k] != '\"')
						k++;


					tmpContact->pAddress[l].pLabel =  MALLOC(sizeof(char)*(k-i+1));
					STRNCPY(tmpContact->pAddress[l].pLabel, &(line[i]), k-i);
					tmpContact->pAddress[l].pLabel[k-i] = '\0';

					i = k;
					while (line[i] != '>')
						i++;	

					i++;

					k = returnTagPosition(&(line[i]), "</address>");

					tmpContact->pAddress[l].pString =  MALLOC(sizeof(char)*(k+1));
					STRNCPY(tmpContact->pAddress[l].pString,  &(line[i]), k);
					tmpContact->pAddress[l].pString[k] = '\0';

					i += k + STRLEN("</address>");

					l++;

					if (l>=CONTACT_MAX_LBL_STRING)
						break;
				}
				i++;
			}

			i += STRLEN("</addresses>");
		}
		else if STR_STARTSWITH(&(line[i]), "<phoneNumbers>")
		{
			j = i +  returnTagPosition(&(line[i]), "</phoneNumbers>");
			l = 0;

			while (i < j)
			{
				if STR_STARTSWITH(&(line[i]), "category")
				{
					i += STRLEN("category=") + 1;
					k = i;
					while (line[k] != '\"')
						k++;

					tmpContact->pTelephone[l].pLabel =  MALLOC(sizeof(char)*(k-i+1));
					STRNCPY(tmpContact->pTelephone[l].pLabel, &(line[i]), k-i);
					tmpContact->pTelephone[l].pLabel[k-i] = '\0';

					i = k;
					while (line[i] != '>')
						i++;	

					i++;

					k = returnTagPosition(&(line[i]), "</phoneNumber>");

					tmpContact->pTelephone[l].pString =  MALLOC(sizeof(char)*(k+1));
					STRNCPY(tmpContact->pTelephone[l].pString, &(line[i]), k);
					tmpContact->pTelephone[l].pString[k] = '\0';

					i += k + STRLEN("</phoneNumber>");

					l++;

					if (l>=CONTACT_MAX_LBL_STRING)
						break;
				}
				i++;
			}

			i += STRLEN("</phoneNumbers>");
		}

		i++;

	}

	UpdateContactInfo (pMe, tmpContact);

	FREE(tmpContact);

}

static void ProcessCallInfo(HelloSonar *pMe, char *line)
{
	int len;
	int lenAux;
	int i =0;
	Call *tmpCall;
	char *aux;
	ListNode * nodeAux;

	aux = MALLOC (sizeof(char)*256);
	len = STRLEN(line);

	tmpCall = MALLOC(sizeof(Call));

	if STR_STARTSWITH(line, "sent")
		tmpCall->type = CALL_SENT;
	else if STR_STARTSWITH(line, "received")
		tmpCall->type = CALL_RECEIVED;
	else if STR_STARTSWITH(line, "voicemail")
		tmpCall->type = CALL_VOICEMAIL;
	else
		tmpCall->type = CALL_MISSED;


	while (i<len)
	{
		if STR_STARTSWITH(&(line[i]), "<contactID>")
		{
			i += returnTagText(&(line[i]), "contactID", aux);
			nodeAux = ReturnContactNode (pMe, ATOI(aux));
			if (nodeAux != NULL)
				tmpCall->pContact = (Contact *) nodeAux->pNode;
			else
				tmpCall->pContact = NULL;

		}
		else if STR_STARTSWITH(&(line[i]), "<phoneNumber>")
		{
			i += returnTagText(&(line[i]), "phoneNumber", aux);
			lenAux = STRLEN(aux) + 1;
			tmpCall->telephone.pString = MALLOC (sizeof(char) * lenAux);

			STRCPY(tmpCall->telephone.pString, aux);
		}
		else if STR_STARTSWITH(&(line[i]), "<timestemap>")
		{
			i += returnTagText(&(line[i]), "timestemap", aux);
			lenAux = STRLEN(aux) + 1;
			tmpCall->pFormatedDate	= MALLOC (sizeof(char) * lenAux);
			STRCPY(tmpCall->pFormatedDate, aux);
			tmpCall->pHour			= MALLOC (sizeof(char) * 6);

			getHourFromTimeStamp(pMe, tmpCall->pFormatedDate, tmpCall->pHour);
		}
	
		i++;
	}

	AddCall2Array(pMe, tmpCall, FALSE);

}

static void getHourFromTimeStamp(HelloSonar *pMe, char *line, char *output)
{
	int i = 0;
	int len = STRLEN(line);

	while(i<len)
	{
		if (line[i] == 'T')
		{
			STRNCPY(output, &(line[i+1]), 5);

			output[5] = '\0';
		}

		i++;
	}
}

static void ProcessWidgetInfo(HelloSonar *pMe, char *line)
{
	int i = 0;

	char aux[1024];

	int len =  STRLEN(line);

	Widget  * tmpWidget;

	tmpWidget = MALLOC(sizeof(Widget));

	while (i < len)
	{
		if STR_STARTSWITH(&(line[i]), "<widgetID>")
		{	
			i += returnTagText(&(line[i]), "widgetID", aux);
			tmpWidget->id = ATOI(aux);
		}
		else if STR_STARTSWITH(&(line[i]), "<name>")
		{
			i += returnTagText(&(line[i]), "name", aux);
			tmpWidget->pName = MALLOC(sizeof(char)*(STRLEN(aux)+1));
			STRCPY(tmpWidget->pName, aux);
		}
		else if STR_STARTSWITH(&(line[i]), "<description>")
		{
			i += returnTagText(&(line[i]), "description", aux);
			tmpWidget->pDescription = MALLOC(sizeof(char)*(STRLEN(aux)+1));
			STRCPY(tmpWidget->pDescription, aux);
		}
		else if STR_STARTSWITH(&(line[i]), "<category>")
		{
			i += returnTagText(&(line[i]), "category", aux);
			tmpWidget->pCategory = MALLOC(sizeof(char)*(STRLEN(aux)+1));
			STRCPY(tmpWidget->pCategory, aux); 
		}
		else if STR_STARTSWITH(&(line[i]), "<icon36x36>")
		{
			i += returnTagText(&(line[i]), "icon36x36", aux);
			tmpWidget->pImageURL = MALLOC(sizeof(char)*(STRLEN(aux)+1));
			STRCPY(tmpWidget->pImageURL, aux); 
		}

		i++;
	}

	UpdateWidgetInfo (pMe, tmpWidget);

	FREE(tmpWidget);
}

static void ProcessContactChanges(HelloSonar *pMe, char *line)
{
	int len;
	int i = 0;
	int j = 0;
	int k = 0;

	ListNode * nodeAux;
	char aux[1024];

	len  =  STRLEN(line);

	while (i < len)
	{
		if STR_STARTSWITH(&(line[i]), "favoritesReordered=")
		{
			i += STRLEN("favoritesReordered=") +1;
			if STR_STARTSWITH(&(line[i]), "true")
			{
				putConnectionState(pMe, CONN_REQ_CONTACT_REORDER_ALL);
				i += STRLEN("true");
			}
		}
		else if STR_STARTSWITH(&(line[i]), "<added>")
		{	
			i += STRLEN("<added>");
			j = i + returnTagPosition(&(line[i]), "</added>");

			while (i < j)
			{
				if (STR_STARTSWITH(&(line[i]), "<contactID>"))
				{	
					i += 11;

					k = returnTagPosition(&(line[i]), "</contactID>");
					STRNCPY(aux, &(line[i]), k);
					aux[k] = '\0';

					AddContact2Array (pMe, ATOI(aux));

					i += k + STRLEN("</contactID>");
				}
				i++;
			}

			i += STRLEN("</added>"); 
		}
		else if STR_STARTSWITH(&(line[i]), "<changed>")
		{	
			i += STRLEN("<changed>");
			j = i + returnTagPosition(&(line[i]), "</changed>");

			while (i < j)
			{
				if (STR_STARTSWITH(&(line[i]), "<contactID>"))
				{	
					i += 11;

					k = returnTagPosition(&(line[i]), "</contactID>");
					STRNCPY(aux, &(line[i]), k);
					aux[k] = '\0';

					nodeAux = ReturnContactNode (pMe, ATOI(aux));

					if (nodeAux != NULL)
						((Contact *) nodeAux->pNode)->needUpdate = TRUE;

					putConnectionState(pMe, CONN_REQ_CONTACT_RETRIEVE_DETAIL);

					i += k + STRLEN("</contactID>");
				}
				i++;
			}



			i += STRLEN("</changed>"); 
		}
		else if STR_STARTSWITH(&(line[i]), "<deleted>")
		{	
			i += STRLEN("<deleted>");
			j = i + returnTagPosition(&(line[i]), "</deleted>");

			while (i < j)
			{
				if (STR_STARTSWITH(&(line[i]), "<contactID>"))
				{	
					i += 11;
					k = returnTagPosition(&(line[i]), "</contactID>");
					STRNCPY(aux, &(line[i]), k);
					aux[k] = '\0';

					RemoveContact(pMe, ATOI(aux));

					i += k + STRLEN("</contactID>");
				}
				i++;
			}

			i += STRLEN("</deleted>"); 
		}
		else if STR_STARTSWITH(&(line[i]), "<statusUpdate>")
		{	
			i += STRLEN("<changed>");
			j = i + returnTagPosition(&(line[i]), "</statusUpdate>");

			while (i < j)
			{
				if (STR_STARTSWITH(&(line[i]), "<status contactID="))
				{	
					i +=  STRLEN("<status contactID=") +1;  //For the " char

					k = i;

					while (line[k]!='\"')
						k++;

					STRNCPY(aux, &(line[i]), k-i);
					aux[k-i] = '\0';

					i = k + 2; //for the "> char

					k = 0;

					nodeAux = ReturnContactNode (pMe, ATOI(aux));

					if (nodeAux != NULL)
					{
						FREEIF(((Contact *) nodeAux->pNode)->pStatus);
		
						k = returnTagPosition(&(line[i]), "</status>");

						if (k>0)
						{
							(((Contact *) nodeAux->pNode)->pStatus) = MALLOC(sizeof(char)*(k+1));
							STRNCPY((((Contact *) nodeAux->pNode)->pStatus), &(line[i]), k);
							aux[k] = '\0';
						}
						
					}

					i += k + STRLEN("</contactID>");
				}

				i++;

			}

			i += STRLEN("</statusUpdate>"); 
		}

		i++;
	}
}

static void UpdateContactInfo(HelloSonar *pMe, Contact  *tmpContact)
{
	int i;
	ListNode *nodeAux;
	Contact *contactAux;

	nodeAux = pMe->contactsList.pFirstElement;


	while (nodeAux != NULL)
	{
		if (tmpContact->id==((Contact*)nodeAux->pNode)->id)
		{
			contactAux = ((Contact*)nodeAux->pNode);

			contactAux->favorite		= tmpContact->favorite;
			FREEIF(contactAux->pFirstName)
			contactAux->pFirstName		= tmpContact->pFirstName;
			FREEIF(contactAux->pLastName)
			contactAux->pLastName		= tmpContact->pLastName;
			FREEIF(contactAux->pDisplayName)
			FREEIF(contactAux->pSearchName)
			contactAux->pDisplayName	= tmpContact->pDisplayName;
			contactAux->pSearchName		= MALLOC(sizeof(char)*(STRLEN(contactAux->pDisplayName)+1));
			STRCPY(contactAux->pSearchName, contactAux->pDisplayName);
			STRUPPER(contactAux->pSearchName);
			FREEIF(contactAux->pImageURL)
			contactAux->pImageURL		= tmpContact->pImageURL;

			if (contactAux->pImageURL != NULL)
				putConnectionState(pMe, CONN_REQ_CONTACT_CHECK_PHOTOS);

			FREEIF(contactAux->pStatus)
			contactAux->pStatus			= tmpContact->pStatus;
			FREEIF(contactAux->pTelephone)


			if (contactAux->pTelephone!=NULL)
			{
				for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
				{
					FREEIF(contactAux->pTelephone[i].pLabel);
					FREEIF(contactAux->pTelephone[i].pString);
				}

				FREE(contactAux->pTelephone);
			}

			if (contactAux->pAddress!=NULL)
			{
				for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
				{
					FREEIF(contactAux->pAddress[i].pLabel);
					FREEIF(contactAux->pAddress[i].pString);
				}

				FREE(contactAux->pAddress);
			}

			if (contactAux->pEmail!=NULL)
			{
				for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
				{
					FREEIF(contactAux->pEmail[i].pLabel);
					FREEIF(contactAux->pEmail[i].pString);
				}

				FREE(contactAux->pEmail);
			}

			contactAux->pTelephone	= tmpContact->pTelephone;
			contactAux->pEmail		= tmpContact->pEmail;
			contactAux->pAddress	= tmpContact->pAddress;
			contactAux->needUpdate	= FALSE;
			break;
		}

		nodeAux = (ListNode *) nodeAux->pNext;
	}

}

static void UpdateWidgetInfo(HelloSonar *pMe, Widget  *tmpWidget)
{
	ListNode * nodeAux;

	nodeAux = pMe->widgetsList.pFirstElement;


	while (nodeAux != NULL)
	{
		if (tmpWidget->id==((Widget*)nodeAux->pNode)->id)
		{
			FREEIF(((Widget*)nodeAux->pNode)->pName)
			((Widget*)nodeAux->pNode)->pName		= tmpWidget->pName;
			FREEIF(((Widget*)nodeAux->pNode)->pCategory)
			((Widget*)nodeAux->pNode)->pCategory		= tmpWidget->pCategory;
			FREEIF(((Widget*)nodeAux->pNode)->pDescription)
			((Widget*)nodeAux->pNode)->pDescription		= tmpWidget->pDescription;
			FREEIF(((Widget*)nodeAux->pNode)->pSearchName)
			((Widget*)nodeAux->pNode)->pSearchName		= MALLOC(sizeof(char)*(STRLEN(tmpWidget->pDescription)+1));
			STRCPY(((Widget*)nodeAux->pNode)->pSearchName, tmpWidget->pDescription);
			STRUPPER(((Widget*)nodeAux->pNode)->pSearchName);
			FREEIF(((Widget*)nodeAux->pNode)->pImageURL)
			((Widget*)nodeAux->pNode)->pImageURL		= tmpWidget->pImageURL;
			if (tmpWidget->pImageURL != NULL)
				putConnectionState(pMe, CONN_REQ_WIDGET_CHECK_PHOTOS);
			((Widget*)nodeAux->pNode)->needUpdate			= FALSE;
			break;
		}

		nodeAux = (ListNode *) nodeAux->pNext;
	}
}

static int returnTagText(char *buffer, char *tag, char *text)
{
	int i;
	int j;

	char * tagEnd;


	i = STRLEN(tag) + 2; // two more chars for < >


	tagEnd = MALLOC(sizeof(char) * (i+2)); // for / and \0

	SPRINTF(tagEnd, "</%s>",tag);

	tagEnd[i+1] = '\0';


	j =  returnTagPosition(&(buffer[i]), tagEnd);


	STRNCPY(text, &(buffer[i]), j);
			
	text[j] = '\0';

	i += j;

	FREE (tagEnd);

	return i;
}

static int returnTagPosition (char *buffer, char *tag)
{
	int i = 0;
	int len = STRLEN(buffer);

	while( !STR_STARTSWITH(&(buffer[i]), tag))
	{
		i ++;
		if (i>=len)
			return len-1;

	}


	return i;
}

static ListNode * ReturnContactNode(HelloSonar *pMe, int contactId)
{
	ListNode * nodeAux = pMe->contactsList.pFirstElement;


	while (nodeAux != NULL)
	{
		if (contactId==((Contact*)nodeAux->pNode)->id)
			break;

		nodeAux = (ListNode *) nodeAux->pNext;
	}

	return nodeAux;
}

static ListNode * ReturnWidgetNode(HelloSonar *pMe, int widgetId)
{
	ListNode * nodeAux = pMe->widgetsList.pFirstElement;


	while (nodeAux != NULL)
	{
		if (widgetId==((Widget*)nodeAux->pNode)->id)
			break;

		nodeAux = (ListNode *) nodeAux->pNext;
	}

	return nodeAux;
}

static short processResourceId(short resId)
{
	short aux;

	switch(resId)
	{
	case ID_CD_CONTACT_FRAME:
		aux = IDB_AB_PHOTO_FRAME_5024;
		break;
	case ID_OPT_LINE_SEPARATOR_00:
	case ID_OPT_LINE_SEPARATOR_01:
	case ID_OPT_LINE_SEPARATOR_02:
	case ID_OPT_LINE_SEPARATOR_03:
		aux = IDB_DARK_LINE_5027;
		break;
	default:
		if (resId <1000)
			aux = -1;
		else
			aux = resId;
		break;
	}

	return aux;

}

static void AddImage2Array(HelloSonar *pMe, short imageId, int x, int y)
{
	short auxId = processResourceId(imageId);

	if (auxId != -1)
	{
		pMe->objArray[pMe->nObjArrayItems].pImage	= ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, auxId);
		pMe->objArray[pMe->nObjArrayItems].type		= OBJ_IMAGE;
	}
	else
	{
		pMe->objArray[pMe->nObjArrayItems].pImage	= NULL;
		pMe->objArray[pMe->nObjArrayItems].type		= OBJ_IMAGE_NODEALLOC;
	}

	pMe->objArray[pMe->nObjArrayItems].pClip	= NULL;
	pMe->objArray[pMe->nObjArrayItems].id		= imageId;
	pMe->objArray[pMe->nObjArrayItems].x		= x;
	pMe->objArray[pMe->nObjArrayItems].y		= y;
	pMe->objArray[pMe->nObjArrayItems].destX	= x;
	pMe->objArray[pMe->nObjArrayItems].destY	= y;
	pMe->objArray[pMe->nObjArrayItems].speed	= 0;
	pMe->objArray[pMe->nObjArrayItems].visible	= TRUE;

	

	if (pMe->nObjArrayItems < OBJARRAYSIZE)
		pMe->nObjArrayItems ++;
#ifdef DEBUG
	else
		DBGPRINTF("Object Array BufferExceed");
#endif
}

static void AddPrimitive2Array(HelloSonar *pMe, int id, int type, int x, int y, int width, int height, RGBVAL color)
{

	pMe->objArray[pMe->nObjArrayItems].pClip	= NULL;
	pMe->objArray[pMe->nObjArrayItems].id		= id;
	pMe->objArray[pMe->nObjArrayItems].x		= x;
	pMe->objArray[pMe->nObjArrayItems].y		= y;
	pMe->objArray[pMe->nObjArrayItems].destX	= x;
	pMe->objArray[pMe->nObjArrayItems].destY	= y;
	pMe->objArray[pMe->nObjArrayItems].width	= width;
	pMe->objArray[pMe->nObjArrayItems].height	= height;
	pMe->objArray[pMe->nObjArrayItems].speed	= 0;
	pMe->objArray[pMe->nObjArrayItems].visible	= TRUE;

	pMe->objArray[pMe->nObjArrayItems].color	= color;

	pMe->objArray[pMe->nObjArrayItems].type		= type;

	if (pMe->nObjArrayItems < OBJARRAYSIZE)
		pMe->nObjArrayItems ++;
#ifdef DEBUG
	else
		DBGPRINTF("Object Array BufferExceed");
#endif
}

static void AddString2Array(HelloSonar *pMe, Font_Types fontId, short stringId, int x, int y)
{
	short auxId = processResourceId(stringId);
	int size;
	AECHAR	*auxWSTR;

	if (auxId != -1)
	{
		auxWSTR =  MALLOC(STRINGBUFFERSIZE * sizeof(AECHAR));

		ISHELL_LoadResString(pMe->piShell, HELLOSONAR_RES_FILE, auxId, auxWSTR, STRINGBUFFERSIZE * sizeof(AECHAR));

		size = sizeof(char) * (WSTRLEN(auxWSTR) +1);
		pMe->objArray[pMe->nObjArrayItems].pStr = MALLOC(size);
		WSTRTOSTR(auxWSTR, pMe->objArray[pMe->nObjArrayItems].pStr, size);

		FREE(auxWSTR);
		pMe->objArray[pMe->nObjArrayItems].type		= OBJ_STRING;
	}
	else
	{
		pMe->objArray[pMe->nObjArrayItems].type		= OBJ_STRING_NODEALLOC;
	}

	pMe->objArray[pMe->nObjArrayItems].pClip	= NULL;
	pMe->objArray[pMe->nObjArrayItems].id		= stringId;
	pMe->objArray[pMe->nObjArrayItems].x		= x;
	pMe->objArray[pMe->nObjArrayItems].y		= y;
	pMe->objArray[pMe->nObjArrayItems].destX	= x;
	pMe->objArray[pMe->nObjArrayItems].destY	= y;
	pMe->objArray[pMe->nObjArrayItems].speed	= 0;
	pMe->objArray[pMe->nObjArrayItems].visible	= TRUE;

	pMe->objArray[pMe->nObjArrayItems].fontId	= fontId;
	pMe->objArray[pMe->nObjArrayItems].align	= 0;
	pMe->objArray[pMe->nObjArrayItems].width	= 0;
	pMe->objArray[pMe->nObjArrayItems].height	= 0;


	if (pMe->nObjArrayItems < OBJARRAYSIZE)
		pMe->nObjArrayItems ++;
#ifdef DEBUG
	else
		DBGPRINTF("Object Array BufferExceed");
#endif
}

static void AddImage2ArrayAnim(HelloSonar *pMe, short imageId, int x, int y, AnimTypes anim)
{
	int auxX;
	int auxY;

	switch(anim)
	{
	case ANIM_FROM_BOTTOM:
		auxX = x;
		auxY = pMe->nHeight + y;
		break;
	case ANIM_FROM_TOP:
		auxX = x;
		auxY = 0 - pMe->nHeight + y;
		break;
	case ANIM_FROM_LEFT:
		auxX = 0 - pMe->nWidth + x;
		auxY = y;
		break;
	case ANIM_FROM_RIGHT:
		auxX = pMe->nWidth + x;
		auxY = y;
		break;
	case ANIM_NONE:
		auxX = x;
		auxY = y;
		break;
	}

	AddImage2Array(pMe, imageId, auxX, auxY);
	ObjArraySetDestination(pMe, imageId, x, y, MESSAGE_BACKGROUNDSPEED);
}

static void AddString2ArrayAnim(HelloSonar *pMe, int fontId, short stringId, int x, int y, AnimTypes anim) 
{
	int auxX;
	int auxY;

	switch(anim)
	{
	case ANIM_FROM_BOTTOM:
		auxX = x;
		auxY = pMe->nHeight + y;
		break;
	case ANIM_FROM_TOP:
		auxX = x;
		auxY = 0 - pMe->nHeight + y;
		break;
	case ANIM_FROM_LEFT:
		auxX = 0 - pMe->nWidth + x;
		auxY = y;
		break;
	case ANIM_FROM_RIGHT:
		auxX = pMe->nWidth + x;
		auxY = y;
		break;
	case ANIM_NONE:
		auxX = x;
		auxY = y;
	}

	AddString2Array(pMe, fontId, stringId, auxX, auxY);
	ObjArraySetDestination(pMe, stringId, x, y, MESSAGE_BACKGROUNDSPEED);
}

static void AddPrimitive2ArrayAnim(HelloSonar *pMe, int id, int type, int x, int y, int width, int height, RGBVAL color, AnimTypes anim)
{
	int auxX;
	int auxY;

	switch(anim)
	{
	case ANIM_FROM_BOTTOM:
		auxX = x;
		auxY = pMe->nHeight + y;
		break;
	case ANIM_FROM_TOP:
		auxX = x;
		auxY = 0 - pMe->nHeight + y;
		break;
	case ANIM_FROM_LEFT:
		auxX = 0 - pMe->nWidth + x;
		auxY = y;
		break;
	case ANIM_FROM_RIGHT:
		auxX = pMe->nWidth + x;
		auxY = y;
		break;
	case ANIM_NONE:
		auxX = x;
		auxY = y;
	}

	AddPrimitive2Array(pMe, id, type, auxX, auxY, width, height, color);
	ObjArraySetDestination(pMe, id, x, y, MESSAGE_BACKGROUNDSPEED);
}

static int getObjArrayPos(HelloSonar *pMe, int Id)
{
	int i;
	for (i=0; i < OBJARRAYSIZE; i++)
	{
		if (pMe->objArray[i].id	== Id)
			return i;
	}
	return -1;
}

static void ObjArraySetClip(HelloSonar *pMe, int Id, int x, int y, int width, int height)
{

	int i = getObjArrayPos(pMe, Id);

	if (i==-1)
		return;

	pMe->objArray[i].pClip		= (AEERect*)MALLOC(sizeof(AEERect));;

	pMe->objArray[i].pClip->x	= x;
	pMe->objArray[i].pClip->y	= y;
	pMe->objArray[i].pClip->dx	= width;
	pMe->objArray[i].pClip->dy	= height;
}

static void ObjArraySetPosition(HelloSonar *pMe, int Id, int x, int y)
{
	int i = getObjArrayPos(pMe, Id);

	if (i==-1)
		return;

	if (x!=DONTCHANGE_VALUE)
	{
		pMe->objArray[i].x		= x;
		pMe->objArray[i].destX	= x;
	}
	if(y!=DONTCHANGE_VALUE)
	{
		pMe->objArray[i].y		= y;
		pMe->objArray[i].destY	= y;
	}
}

static void ObjArraySetDestination(HelloSonar *pMe, int Id, int destX, int destY, int speed)
{
	int i = getObjArrayPos(pMe, Id);

	if (i==-1)
		return;
	
	if (destX!=DONTCHANGE_VALUE)
		pMe->objArray[i].destX	= destX;

	if(destY!=DONTCHANGE_VALUE)
		pMe->objArray[i].destY	= destY;

	pMe->objArray[i].speed	= speed;
}

static void ObjArraySetFont(HelloSonar *pMe, int Id, Font_Types fontId)
{
	int i = getObjArrayPos(pMe, Id);

	if (i==-1)
		return;

	pMe->objArray[i].fontId = fontId;
}

static void ObjArraySetVisible(HelloSonar *pMe, int Id, boolean visible)
{
	int i = getObjArrayPos(pMe, Id);

	if (i==-1)
		return;

	pMe->objArray[i].visible = visible;
}

static void UpdateAnims(HelloSonar * pMe)
{
	int i;

	for (i = 0; i < pMe->nObjArrayItems; i++)
	{
		if (pMe->objArray[i].x < pMe->objArray[i].destX)
		{
			if(pMe->objArray[i].x + pMe->objArray[i].speed < pMe->objArray[i].destX)
				pMe->objArray[i].x += pMe->objArray[i].speed;
			else
				pMe->objArray[i].x = pMe->objArray[i].destX;
		}
		else if (pMe->objArray[i].x > pMe->objArray[i].destX)
		{
			if(pMe->objArray[i].x - pMe->objArray[i].speed > pMe->objArray[i].destX)
				pMe->objArray[i].x -= pMe->objArray[i].speed;
			else
				pMe->objArray[i].x = pMe->objArray[i].destX;
		}	
		if (pMe->objArray[i].y < pMe->objArray[i].destY)
		{
			if(pMe->objArray[i].y + pMe->objArray[i].speed < pMe->objArray[i].destY)
				pMe->objArray[i].y += pMe->objArray[i].speed;
			else
				pMe->objArray[i].y = pMe->objArray[i].destY;
		}
		else if (pMe->objArray[i].y > pMe->objArray[i].destY)
		{
			if(pMe->objArray[i].y - pMe->objArray[i].speed > pMe->objArray[i].destY)
				pMe->objArray[i].y -= pMe->objArray[i].speed;
			else
				pMe->objArray[i].y = pMe->objArray[i].destY;
		}	
	}
}

static void ClearObjArray(HelloSonar *pMe)
{
	int i;

	for (i = 0; i < pMe->nObjArrayItems; i++)
	{
		switch(pMe->objArray[i].type)
		{
		case OBJ_IMAGE:
			if (pMe->objArray[i].pImage!=NULL)
				IIMAGE_Release(pMe->objArray[i].pImage);
		case OBJ_IMAGE_NODEALLOC:
			pMe->objArray[i].pImage = NULL;
			break;
		case OBJ_STRING:
			FREEIF(pMe->objArray[i].pStr);
			break;
		case OBJ_STRING_AREA_NODEALLOC:
		case OBJ_STRING_NODEALLOC:
			pMe->objArray[i].pStr = NULL;
			break;
		}
		FREEIF(pMe->objArray[i].pClip);
	}

	pMe->nObjArrayItems = 0;
}

static void UpdateLogic(HelloSonar *pMe)
{
	int x = 9;
	int i;

	UpdateAnims(pMe);

	switch (pMe->nState)
	{
	case STATE_MAINMENU:
		if (pMe->mLastKeyPressTime+MAINMENU_IDLETIME < GETUPTIMEMS())
		{
			ObjArraySetDestination(pMe, IDB_LEFT_ARROW_5010, DONTCHANGE_VALUE, pMe->nHeight, MAINMENU_ICONSPEED);
			ObjArraySetDestination(pMe, IDB_RIGHT_ARROW_5011, DONTCHANGE_VALUE, pMe->nHeight, MAINMENU_ICONSPEED);
			ObjArraySetDestination(pMe, IDB_LOWER_BAR_5016, DONTCHANGE_VALUE, pMe->nHeight, MAINMENU_ICONSPEED);
			

			for (i = 0; i < MAINMENU_MAXICON; i++)
			{
				ObjArraySetDestination(pMe, IDB_ICON_5002+i, x, pMe->nHeight-60, MAINMENU_ICONSPEED);
				x += 58;
			}
		}
		break;
	}
}

static void RequestImages(HelloSonar *pMe, ListTypes listType)
{
	ListNode	*nodeAux;
	Contact		*contactAux;
	Widget		*widgetAux;
	char imgUrl[255];

	if (listType == LIST_CONTACTS)
	{
		nodeAux = pMe->contactsList.pFirstElement;

		while (nodeAux != NULL)
		{
			contactAux = (Contact *) nodeAux->pNode;

			if (contactAux->pImageURL != NULL)
			{
				if (contactAux->pImage == NULL)
				{
					STRCPY(imgUrl, contactAux->pImageURL);
					pMe->serviceCalled = CONN_REQ_CONTACT_RETRIEVE_PHOTO;
					pMe->pNodeImgRequest = nodeAux;
					GoToURL(pMe, imgUrl);
					break;
				}
			}

			nodeAux = (ListNode *) nodeAux->pNext;
		}
	}
	else
	{
		nodeAux = pMe->widgetsList.pFirstElement;

		while (nodeAux != NULL)
		{
			widgetAux = (Widget *) nodeAux->pNode;

			if (widgetAux->pImageURL != NULL)
			{
				if (widgetAux->pImage == NULL)
				{
					STRCPY(imgUrl, widgetAux->pImageURL);
					pMe->serviceCalled = CONN_REQ_WIDGET_RETRIEVE_PHOTO;
					pMe->pNodeImgRequest = nodeAux;
					GoToURL(pMe, imgUrl);
					break;
				}
			}

			nodeAux = (ListNode *) nodeAux->pNext;
		}
	}

}

static void fillFilterList(HelloSonar *pMe, ListTypes typeList)
{
	int i = 0;
	char	searchStr[SEARCH_STRING_MAX_CHARS];

	ListNode	*nodeAux;

	char *pSearchName;

	if (pMe->searchBox[0].charPos>0)
	{

		switch (typeList)
		{
		case LIST_CALLS:
			nodeAux = pMe->callsList.pFirstElement;
			break;
		case LIST_CONTACTS:
			nodeAux	= pMe->contactsList.pFirstElement;
			break;
		case LIST_WIDGETS:
			nodeAux = pMe->widgetsList.pFirstElement;
			break;
		}

		STRCPY(searchStr, pMe->searchBox[0].string);
		STRUPPER(searchStr);

		while (nodeAux != NULL)
		{

			switch (typeList)
			{
			case LIST_CALLS:
				pSearchName = ((Call *) nodeAux->pNode)->pContact->pSearchName;
				break;
			case LIST_CONTACTS:
				pSearchName = ((Contact *) nodeAux->pNode)->pSearchName;
				break;
			case LIST_WIDGETS:
				pSearchName = ((Widget *) nodeAux->pNode)->pSearchName;
				break;
			}

			if (pSearchName != NULL)
			{
				if (STR_STARTSWITH(pSearchName, searchStr))
				{
					pMe->menu->filteredMenuList[i] = nodeAux;
					i++;

					if (i>=MAXCONTACTS)
						break;
				}
			}
			nodeAux = (ListNode *) nodeAux->pNext;
		}
	}

	pMe->menu->filteredMenuList[i]	= NULL;

	switch (typeList)
	{
	case LIST_CALLS:
		pMe->menu->pFirstItemPointer = pMe->callsList.pFirstElement;
		break;
	case LIST_CONTACTS:
		pMe->menu->pFirstItemPointer = pMe->contactsList.pFirstElement;
		break;
	case LIST_WIDGETS:
		pMe->menu->pFirstItemPointer = pMe->widgetsList.pFirstElement;
		break;
	}

	pMe->menu->firstItemPosition	= 0;
	pMe->menu->curPosition			= 0;
	pMe->menu->filteredMenuItems	= i;

	ObjArraySetDestination(pMe, IDB_SELECTION_BOX_5013, 0, 31 + (60*pMe->menu->curPosition), MESSAGE_BACKGROUNDSPEED);
	ObjArraySetDestination(pMe, IDB_SELECTION_BOX_BASE, 0, 31 + (60*pMe->menu->curPosition), MESSAGE_BACKGROUNDSPEED);
}

static void cleanConnectionStates(HelloSonar *pMe)
{
	int i;

	pMe->serviceCalled = CONN_REQ_IDLE;

	for (i=0;i<CONN_MAX_STATES;i++)
	{
		pMe->nConnState[i]	 = CONN_REQ_IDLE;
	}
}

static void UpdateConnectionState(HelloSonar *pMe)
{
	if (pMe->serviceCalled != CONN_REQ_IDLE)
		return;

	switch(getActualConnectionState(pMe))
	{
	case CONN_REQ_CONTACT_RETRIEVE_ALL:
		removeActualConnectionState(pMe);
		LoadContactData(pMe);
		break;

	case CONN_REQ_CONTACT_RETRIEVE_DETAIL:
		removeActualConnectionState(pMe);
		FillContactData(pMe);
		break;
	case CONN_REQ_CONTACT_CHECK_CHANGES:
		removeActualConnectionState(pMe);
		CheckForUpdates(pMe, CONN_REQ_CONTACT_RETRIEVE_TS);
		break;
	case CONN_REQ_CONTACT_UPDATE_PENDING:
		removeActualConnectionState(pMe);
		putConnectionState(pMe, CONN_REQ_CONTACT_RETRIEVE_DETAIL);
		break;
	case CONN_REQ_CALL_RETRIEVE_ALL:
		removeActualConnectionState(pMe);
		LoadCallData(pMe);
		break;
	case CONN_REQ_CALL_RETRIEVE_TS:
		removeActualConnectionState(pMe);
		CheckCallUpdates(pMe);
		break;
	case CONN_REQ_WIDGET_RETRIEVE_ALL:
		removeActualConnectionState(pMe);
		LoadWidgetData(pMe); 
		break;
	case CONN_REQ_WIDGET_RETRIEVE_DETAIL:
		removeActualConnectionState(pMe);
		FillWidgetData(pMe);
		break;
	case CONN_REQ_WIDGET_RETRIEVE_TS:
		removeActualConnectionState(pMe);
		CheckForUpdates(pMe, CONN_REQ_WIDGET_RETRIEVE_TS);
		break;
	case CONN_REQ_CONTACT_REORDER_ALL:
		removeActualConnectionState(pMe);
		RequestReorderedContacts(pMe);
		break;
	case CONN_REQ_WIDGET_REORDER_ALL:
		removeActualConnectionState(pMe);
		RequestReorderedWidgets(pMe);
		break;
	case CONN_REQ_CONTACT_CHECK_PHOTOS:
		removeActualConnectionState(pMe);
		RequestImages(pMe, LIST_CONTACTS);
		break;
	case CONN_REQ_WIDGET_CHECK_PHOTOS:
		removeActualConnectionState(pMe);
		RequestImages(pMe, LIST_WIDGETS);
		break;
	case CONN_REQ_SETTINGS_CHECK_CHANGES:
		removeActualConnectionState(pMe);
		checkUserSettings(pMe);
		break;
	}

}

static void checkUserSettings(HelloSonar *pMe)
{
	char * aux;

#ifdef DEBUG_STACK
	DBGPRINTF("checkUserSettings");
#endif

	aux = MALLOC(sizeof(char)*255);

	SPRINTF(aux,"/services/user/GetSettings?sonarID=%s",pMe->pSonarIdEncoded);
		
	pMe->serviceCalled = CONN_REQ_SETTINGS_CHECK_CHANGES;
	GoToURL(pMe,aux);

	FREE(aux);
}

static void GoToURL(HelloSonar *pMe, char *url)
{
	int i;

	pMe->webStringBufferLenght = 0;

#ifdef DEBUG_STACK
	DBGPRINTF("GoToURL");
#endif

#ifdef DEBUG_HTTP
	DBGPRINTF("URL: %s", url);
#endif


	if STR_STARTSWITH(url, "http://")
		SPRINTF(pMe->pWebURL, "%s", url);
	else
		SPRINTF(pMe->pWebURL, "%s%s", pMe->pBaseURL, url);

	i = STRLEN(pMe->pBaseURL) + STRLEN(url);

	pMe->pWebURL[i] = '\0';

	CALLBACK_Cancel(&pMe->WebCBStruct); 

	if (pMe->pIWebResponse) 
	{ 
		IWEBRESP_Release(pMe->pIWebResponse); 
		pMe->pIWebResponse = NULL;
	} 

	IWEB_GetResponse(pMe->pIWeb, (pMe->pIWeb, &pMe->pIWebResponse, &pMe->WebCBStruct, pMe->pWebURL,WEBOPT_END)); 

}

static void CheckCallUpdates(HelloSonar *pMe)
{
	JulianType date;
	char *dateString;
	char *aux;

#ifdef DEBUG_STACK
	DBGPRINTF("CheckCallUpdates");
#endif

	dateString = MALLOC(sizeof(char)*50);
	aux = MALLOC(sizeof(char)*255);

	GETJULIANDATE(pMe->mLastUpdate_Call,&date);
  
	SPRINTF(dateString,"&sinceTime=%d-%.2d-%.2dT%.2d%%3a%.2d%%3a%.2dZ",date.wYear,date.wMonth,date.wDay,date.wHour,date.wMinute, date.wSecond);
	SPRINTF(aux,"%s%s%s","/services/calllog/Retrieve?sonarID=",pMe->pSonarIdEncoded,dateString);

	pMe->serviceCalled = CONN_REQ_CALL_RETRIEVE_TS;
	GoToURL(pMe,aux);


	FREE(dateString);
	FREE(aux);
}

static void CheckForUpdates(HelloSonar *pMe, Conn_Reqs serviceReq)
{
	JulianType date;
	char * dateString;
	char * aux;
	uint32  lastUpdate;
	char stringAux[32];

#ifdef DEBUG_STACK
	DBGPRINTF("CheckForUpdates");
#endif

	dateString = MALLOC(sizeof(char)*50);
	aux = MALLOC(sizeof(char)*255);

	switch(serviceReq)
	{
	case CONN_REQ_CONTACT_RETRIEVE_TS:
		lastUpdate = pMe->mLastUpdate_Contact;
		STRCPY(stringAux, "contactslist");
		break;
	case CONN_REQ_WIDGET_RETRIEVE_TS:
		lastUpdate = pMe->mLastUpdate_Widget;
		STRCPY(stringAux, "widgetslist");
		break;
	}

	GETJULIANDATE(lastUpdate,&date);
  
	SPRINTF(dateString,"&sinceTime=%d-%.2d-%.2dT%.2d%%3a%.2d%%3a%.2dZ",date.wYear,date.wMonth,date.wDay,date.wHour,date.wMinute, date.wSecond);
	SPRINTF(aux,"%s%s%s%s%s","/services/",stringAux,"/Retrieve?sonarID=",pMe->pSonarIdEncoded,dateString);
		
	pMe->serviceCalled = serviceReq;
	GoToURL(pMe,aux);

	FREE(dateString);
	FREE(aux);
}

static void getSonarId(HelloSonar *pMe)
{
	int len;
	char tempString[64];
	MD5_CTX mdContext;
	int i;

	if (pMe->pSonarId == NULL)
	{
		len = STRLEN("demo") + 1;
		pMe->pSonarId = MALLOC (sizeof(char)*len);
		STRCPY(pMe->pSonarId, "demo");
	}

	STRCPY(tempString, pMe->pSonarId);

	FREEIF(pMe->pSonarIdEncoded);

	len = STRLEN (tempString);
	
	MD5Init (&mdContext);
	MD5Update (&mdContext, tempString, len);
	MD5Final (&mdContext);

	for (i = 0; i < 16; i++)
		SPRINTF(&(tempString[i*2]), "%02x", mdContext.digest[i]);

	tempString[32] = '\0';

	len = STRLEN (tempString);

	pMe->pSonarIdEncoded = MALLOC(sizeof(char)*(33));

	STRCPY(pMe->pSonarIdEncoded, tempString);	

	//STRCPY(pMe->pSonarId, "6906e253937a65226311e2278861e3a3");	/* ac@oneforcemobile.com */
	//STRCPY(pMe->pSonarId, "fe01ce2a7fbac8fafaed7c982a04e229");		/* demo@hellosonar.com */
}

static void putConnectionState(HelloSonar *pMe, int newConnState)
{
	int i;

	for (i=0; i<CONN_MAX_STATES; i++)
	{
		if (pMe->nConnState[i]==CONN_REQ_IDLE)
		{
			pMe->nConnState[i] = newConnState;
			break;
		}
		else if (pMe->nConnState[i]==newConnState)
		{
			break;
		}
	}
}

static int getActualConnectionState(HelloSonar *pMe)
{
	return pMe->nConnState[0];
}

static void removeActualConnectionState(HelloSonar *pMe)
{
	int i;

	pMe->nConnState[0]=CONN_REQ_IDLE;

	for (i=1;i<CONN_MAX_STATES;i++)
	{
		if (pMe->nConnState[i]==CONN_REQ_IDLE)
		{
			break;
		}
		else
		{
			pMe->nConnState[i-1] = pMe->nConnState[i];
			pMe->nConnState[i]	 = CONN_REQ_IDLE;
		}
	}

}

static void appendBuffer2File(HelloSonar *pMe, IFile *pFile, char *buffer)
{
	int size;
	int i;

	i = STRLEN(buffer);

	buffer[i] = FILE_ROW_SEPARATOR;
	i++;
	buffer[i] = '\0';

	size = sizeof(char) * i;

	if (pFile != NULL)
	{
		IFILE_Write(pFile, buffer, size);
		//IFILE_Seek(pFile, _SEEK_CURRENT, size);
	}

	buffer[0] = '\0';
}

static void addField2Buffer(HelloSonar *pMe, char *buffer, char*data)
{
	int i = STRLEN(buffer);

	if (data!= NULL)
	{
		STRCPY(&(buffer[i]), data);
		i += STRLEN(data);
	}
	
	buffer[i] = FILE_FIELD_SEPARATOR;
	i++;
	buffer[i] = '\0';
}

static int processContactData(HelloSonar *pMe, char *data)
{
	int i=0;
	int len = STRLEN(data);
	int j = 0;
	int k = 0;
	int fieldCounter;

	char *aux;
	Contact *tmpContact;

	while (i<len)
	{
		fieldCounter = 0;

		while (data[j] != FILE_ROW_SEPARATOR || fieldCounter< 25)
		{
			if (j >= len-1)
				return i;

			if (data[j] == FILE_FIELD_SEPARATOR)
				fieldCounter++;

			j++;
		}

		tmpContact = MALLOC(sizeof(Contact));
		tmpContact->pEmail		= MALLOC(sizeof(LblString)*CONTACT_MAX_LBL_STRING);
		tmpContact->pAddress	= MALLOC(sizeof(LblString)*CONTACT_MAX_LBL_STRING);
		tmpContact->pTelephone	= MALLOC(sizeof(LblString)*CONTACT_MAX_LBL_STRING);

		k = extractNextField(pMe,  &(data[i]), &aux);
		if (k!=-1)
		{
			tmpContact->id = ATOI(aux);
			FREE(aux);
			i += k;
		}
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &aux);
		if (k!=-1)
		{
			tmpContact->favorite = ATOI(aux);
			FREE(aux);
			i += k;
		}
		else
			i++;


		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pDisplayName));
		if (k!=-1)
			i += k;
		else
			i++;

		if (tmpContact->pDisplayName != NULL)
		{
			tmpContact->pSearchName		= MALLOC(sizeof(char)*(STRLEN(tmpContact->pDisplayName)+1));
			STRCPY(tmpContact->pSearchName, tmpContact->pDisplayName);
			STRUPPER(tmpContact->pSearchName);
		}

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pFirstName));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pLastName));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pStatus));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pImageURL));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pTelephone[0].pLabel));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pTelephone[0].pString));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pTelephone[1].pLabel));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pTelephone[1].pString));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pTelephone[2].pLabel));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pTelephone[2].pString));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pEmail[0].pLabel));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pEmail[0].pString));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pEmail[1].pLabel));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pEmail[1].pString));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pEmail[2].pLabel));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pEmail[2].pString));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pAddress[0].pLabel));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pAddress[0].pString));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pAddress[1].pLabel));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pAddress[1].pString));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pAddress[2].pLabel));
		if (k!=-1)
			i += k;
		else
			i++;

		k = extractNextField(pMe,  &(data[i]), &(tmpContact->pAddress[2].pString));
		

		i = j+1;


		AddFullContact2Array(pMe, tmpContact);
	}

	return i;
}

static void loadContactDataFromDevice(HelloSonar *pMe)
{
	IFile *pFile;
	FileInfo fileInfo;
	char *data;
	char *aux;

	int bufferSize;
	uint32 fileIterator;
	int j = 0;

	data = MALLOC(sizeof(char) * (FILE_CONTACT_LENGTH+1));
	aux = MALLOC(sizeof(char) * 64);

	pFile = IFILEMGR_OpenFile(pMe->fmgr, FILE_CONTACT, _OFM_READ);


	if (pFile != NULL)
	{
		IFILE_GetInfo(pFile,&fileInfo);

		IFILE_Read(pFile, data, 128);

		while (data[j] != FILE_ROW_SEPARATOR)
		{
			j++;
		}

		STRNCPY(aux, data, j);
		pMe->mLastUpdate_Contact = ATOI(aux); 
		FREE(aux);
		j++;

		IFILE_Seek(pFile, _SEEK_START, j);
		fileIterator = j;

		while (fileIterator < fileInfo.dwSize)
		{
			if (fileIterator + FILE_CONTACT_LENGTH < fileInfo.dwSize)
			{
				bufferSize = FILE_CONTACT_LENGTH;
			}
			else
			{
				bufferSize = fileInfo.dwSize - fileIterator;
			}

			IFILE_Read(pFile, data, bufferSize);

			data[bufferSize] = '\0';

			fileIterator +=  processContactData(pMe, data);

			IFILE_Seek(pFile, _SEEK_START, fileIterator);
		}

		IFILE_Release(pFile);
	}

	FREE(data);
}

static void saveContactData(HelloSonar *pMe)
{
	char		*data;
	char		*aux;
	IFile		*pFile;
	Contact		*contactAux;
	ListNode	*nodeAux;
	int			i;

	data	= MALLOC(sizeof(char)*FILE_CONTACT_LENGTH);
	aux		= MALLOC(sizeof(char)*128);

	IFILEMGR_Remove(pMe->fmgr, FILE_CONTACT); 
	pFile = IFILEMGR_OpenFile(pMe->fmgr, FILE_CONTACT, _OFM_CREATE);

	SPRINTF(data, "%d", pMe->mLastUpdate_Contact);

	appendBuffer2File(pMe, pFile, data);

	nodeAux = pMe->contactsList.pFirstElement;

	while(nodeAux != NULL)
	{
		contactAux = nodeAux->pNode;

		SPRINTF(aux, "%d", contactAux->id);
		addField2Buffer(pMe, data, aux);

		SPRINTF(aux, "%d", contactAux->favorite);
		addField2Buffer(pMe, data, aux);

		addField2Buffer(pMe, data, contactAux->pDisplayName);
		addField2Buffer(pMe, data, contactAux->pFirstName);
		addField2Buffer(pMe, data, contactAux->pLastName);
		addField2Buffer(pMe, data, contactAux->pStatus);
		addField2Buffer(pMe, data, contactAux->pImageURL);

		if (contactAux->pTelephone!=NULL)
		{
			for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
			{
				addField2Buffer(pMe, data, contactAux->pTelephone[i].pLabel);
				addField2Buffer(pMe, data, contactAux->pTelephone[i].pString);
			}
		}
		else
		{
			for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
			{
				addField2Buffer(pMe, data, NULL);
				addField2Buffer(pMe, data, NULL);
			}
		}

		if (contactAux->pEmail!=NULL)
		{
			for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
			{
				addField2Buffer(pMe, data, contactAux->pEmail[i].pLabel);
				addField2Buffer(pMe, data, contactAux->pEmail[i].pString);
			}
		}
		else
		{
			for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
			{
				addField2Buffer(pMe, data, NULL);
				addField2Buffer(pMe, data, NULL);
			}
		}

		if (contactAux->pAddress!=NULL)
		{
			for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
			{
				addField2Buffer(pMe, data, contactAux->pAddress[i].pLabel);
				addField2Buffer(pMe, data, contactAux->pAddress[i].pString);
			}
		}
		else
		{
			for (i = 0; i < CONTACT_MAX_LBL_STRING; i++)
			{
				addField2Buffer(pMe, data, NULL);
				addField2Buffer(pMe, data, NULL);
			}
		}

		appendBuffer2File(pMe, pFile, data);

		nodeAux =(ListNode *) nodeAux->pNext;
	}

	IFILE_Release(pFile);
	FREE(data);
	FREE(aux);
}

static void saveSettings(HelloSonar *pMe)
{
	char *data;
	int i = 0;

	data = MALLOC(sizeof(char) * FILE_SETTINGS_LENGTH);

	STRCPY(data, pMe->pSonarId);

	i = STRLEN(pMe->pSonarId);

	data[i] = FILE_FIELD_SEPARATOR;

	i++;

	switch(pMe->currentThemeColor)
	{
	case THEME_RED:
		STRCPY(&(data[i]), "RED");
		data[i+3] = FILE_FIELD_SEPARATOR;
		break;
	case THEME_GREEN:
		STRCPY(&(data[i]), "GREEN");
		data[i+4] = FILE_FIELD_SEPARATOR;
		break;
	case THEME_ORANGE:
		STRCPY(&(data[i]), "ORANGE");
		data[i+4] = FILE_FIELD_SEPARATOR;
		break;
	case THEME_BLUE:
		STRCPY(&(data[i]), "BLUE");
		data[i+4] = FILE_FIELD_SEPARATOR;
		break;
	}

	writeDataToFile(pMe, data, FILE_SETTINGS);

	FREE(data);
}

static void loadSettings(HelloSonar *pMe)
{
	char *data;
	char *aux;
	int	i = 0;
	int j = 0;

	data = MALLOC(sizeof(char) * FILE_SETTINGS_LENGTH);

	readDataFromFile(pMe, data, FILE_SETTINGS);

	j = extractNextField(pMe,  &(data[i]), &(pMe->pSonarId));

	if (j != -1)
	{
		i += j;
	}
	else
	{
		pMe->pSonarId = MALLOC (sizeof(char)*5);
		STRCPY(pMe->pSonarId, "demo");
	}


	j = extractNextField(pMe,  &(data[i]), &aux);

	if (j != -1)
	{
		i += j;
	}
	else
	{
		aux = MALLOC (sizeof(char)*6);
		STRCPY(aux, "GREEN");
	}

	getSonarId(pMe);
	changeWallpaper(pMe, aux);

	FREE(data);
	FREEIF(aux);
}

static int extractNextField(HelloSonar *pMe, char *data, char **dest)
{
	int j = 0;
	int len = STRLEN(data);

	while (data[j] != FILE_FIELD_SEPARATOR)
	{
		j++;

		if (j>= len)
		{
			j = -1;
			break;
		}
	}

	if (j>0)
	{
		*dest = MALLOC (sizeof(char)*(j+1));
		STRNCPY(*dest, data, j);
		j++;
	}
	else
	{
		*dest = NULL;
		j = -1;
	}

	return j;
}

static void writeDataToFile(HelloSonar *pMe, char *data, char *fileName)
{
	IFile *pFile;
	int size;
 
	IFILEMGR_Remove(pMe->fmgr, fileName);
 
	pFile = IFILEMGR_OpenFile(pMe->fmgr, fileName, _OFM_CREATE);

	size = sizeof(char) * STRLEN (data);

	if (pFile != NULL)
	{
		IFILE_Write(pFile, data, size);
		IFILE_Release(pFile);
	}
}

static void readDataFromFile(HelloSonar *pMe, char *data, char *fileName)
{
	IFile *pFile;
	FileInfo fileInfo;

 
	pFile = IFILEMGR_OpenFile(pMe->fmgr, fileName, _OFM_READ);

	if (pFile != NULL)
	{
		IFILE_GetInfo(pFile,&fileInfo);
		IFILE_Read(pFile, data, fileInfo.dwSize);
		IFILE_Release(pFile);

		data[fileInfo.dwSize] = '\0';
	}
}

static void encodeBase64 (char *input, char *output)
{
    int cols, bits, c, char_count;

	int len;
	int i = 0;
	int j = 0;

	char alphabet[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";



	len = STRLEN(input);

    char_count = 0;
    bits = 0;
    cols = 0;

	while (i<len) 
	{
		c = input[i];
		if (c > 255) 
		{
			DBGPRINTF("encountered char > 255 (decimal %d)", c);
			c = 'a';
		}

		bits += c;
		char_count++;
		if (char_count == 3)
		{
			output[j++] = alphabet[bits >> 18];
			output[j++] = alphabet[(bits >> 12) & 0x3f];
			output[j++] = alphabet[(bits >> 6) & 0x3f];
			output[j++] = alphabet[bits & 0x3f];

			bits = 0;
			char_count = 0;
		}
		else
		{
			bits <<= 8;
		}
		i++;
    }
    if (char_count != 0) 
	{
		bits <<= 16 - (8 * char_count);
		output[j++] = alphabet[bits >> 18];
		output[j++] = alphabet[(bits >> 12) & 0x3f];

		if (char_count == 1) 
		{
			output[j++] = '=';
			output[j++] = '=';
		}
		else
		{
			output[j++] = alphabet[(bits >> 6) & 0x3f];
			output[j++] = '=';
		}

    }

	output[j] = '\0';

}

static void loadFont(HelloSonar *pMe, Font_Types fontId)
{
	uint8	*data;
	uint8	*dataIterator;
	Font	*fnt;

	int i;
	int	c;



	data = ISHELL_LoadResData(pMe->piShell, HELLOSONAR_RES_FILE, (short)(IDB_BIN_FONT_01_5501+(fontId*2)), RESTYPE_IMAGE);
	dataIterator = data + *data;

	fnt = &(pMe->pFont[fontId]);

	fnt->font_count = *dataIterator;
	
	dataIterator++;

	fnt->font_x = MALLOC (sizeof(int)*255);
	fnt->font_y = MALLOC (sizeof(int)*255);
	fnt->font_w = MALLOC (sizeof(int)*255);
	fnt->font_h = MALLOC (sizeof(int)*255);

	fnt->pFont = ISHELL_LoadResImage(pMe->piShell, HELLOSONAR_RES_FILE, (short)(IDB_FONT_01_5500+(fontId*2)));
	 
	for( i = 0; i < fnt->font_count; i++ )
	{
		c = (int) *dataIterator;
		dataIterator++;
		
		if (c<255)
		{
			fnt->font_x[c] = (int) *dataIterator;
			dataIterator++;
			fnt->font_y[c] = (int) *dataIterator;
			dataIterator++; 
			fnt->font_w[c] = (int) *dataIterator;
			dataIterator++; 
			fnt->font_h[c] = (int) *dataIterator;
			dataIterator++; 
		}
		else
		{
			dataIterator += 4;
		}
	}

	ISHELL_FreeResData(pMe->piShell, data);
}

static int getStringWidht(HelloSonar *pMe, Font_Types fontId, char *string)
{
	int width = 0;
	int len = STRLEN(string);
	int i;

	for (i=0; i<len; i++)
	{
		width += pMe->pFont[fontId].font_w[string[i]];
	}

	return width;
}

static void drawString(HelloSonar *pMe, Font_Types fontId, char *string, int x, int y, int align)
{
	int i = 0;
	int stringWidth;
	int len;
	char c;
	AEERect *clip;
	Font *fnt;

	boolean noClipping	= TRUE;
	boolean ahead		= TRUE;


	clip = MALLOC(sizeof(AEERect));
	len = STRLEN(string);
	stringWidth = getStringWidht(pMe, fontId, string);

	fnt = &(pMe->pFont[fontId]);

	if( align > 0 )
	{

		if (align & FONT_USE_CLIP)
			noClipping = FALSE;

		if (align & FONT_ALIGN_CENTER)
			x -= (stringWidth >>1);
		else if (align & FONT_ALIGN_RIGHT)
		{
			ahead = FALSE;
			i = len -1;
			x -= fnt->font_w[string[i]];
		}
	}

	while(TRUE)
	{
		c = string[i];

		if( noClipping = TRUE )
		{
			clip->x = x;
			clip->y	= y;
			clip->dx = fnt->font_w[c];
			clip->dy = fnt->font_h[c];

			IDISPLAY_SetClipRect(pMe->piDisplay, clip);
			
			IIMAGE_Draw(fnt->pFont, x - fnt->font_x[c],  y - fnt->font_y[c]);
		}

		if (ahead==TRUE)
		{
			i++;
			if (i >= len)
				break;
			x += fnt->font_w[c];
		}
		else
		{
			i--;
			if (i<0)
				break;
			x -= fnt->font_w[string[i]];
		}

	}

	IDISPLAY_SetClipRect(pMe->piDisplay, NULL);

	FREE(clip);
}

static void drawStringInArea(HelloSonar *pMe, Font_Types fontId, char *string, int x, int y, int width, int height, int align)
{
	int len = STRLEN(string);
	int i = 0;
	char c;
	char word[64];
	int wordPos = 0;
	char line[256];
	int linePos = 0;
	int currentLineWidth = 0;
	int yAux = y;

	while (i<len)
	{
		c = string[i];

		if (c!= ' ' && c != 10)
		{
			word[wordPos] = c;
			wordPos ++;
		}
		else if (wordPos > 0)
		{
			word[wordPos] = '\0';
			if (currentLineWidth + getStringWidht(pMe, fontId, word) < width)
			{
				word[wordPos] = ' ';
				word[wordPos+1] = '\0';
				STRCPY(&(line[linePos]), word);
				linePos += STRLEN(word);
				currentLineWidth += getStringWidht(pMe, fontId, word);

				wordPos = 0;
			}
			else
			{
				line[linePos] = '\0';

				if (yAux + 2 + (pMe->pFont[fontId].font_h[32])*2 >= (y + height))
				{
					if (linePos>3)
					{
						line[linePos-1] = '.';
						line[linePos-2] = '.';
						line[linePos-3] = '.';
					}
				}

				drawString(pMe, fontId, line, x, yAux, align);
				linePos = 0;

				yAux += pMe->pFont[fontId].font_h[32] + 2;

				if (yAux + pMe->pFont[fontId].font_h[32]  >= ( y + height))
					break;

				currentLineWidth = 0;

				word[wordPos]	= ' ';
				word[wordPos+1] = '\0';
				STRCPY(&(line[linePos]), word);
				linePos += STRLEN(word);
				currentLineWidth += getStringWidht(pMe, fontId, word);
				wordPos = 0;
			}
		}

		i++;
	}

	if (wordPos > 0)
	{
		word[wordPos] = '\0';
		if (currentLineWidth + getStringWidht(pMe, fontId, word) < width)
		{
			word[wordPos] = ' ';
			word[wordPos+1] = '\0';
			STRCPY(&(line[linePos]), word);
			linePos += STRLEN(word);
			currentLineWidth += getStringWidht(pMe, fontId, word);

			wordPos = 0;
		}
		else
		{
			line[linePos] = '\0';

				if (yAux + 2 + (pMe->pFont[fontId].font_h[32])*2 >= (y + height))
				{
					if (linePos>3)
					{
						line[linePos-1] = '.';
						line[linePos-2] = '.';
						line[linePos-3] = '.';
					}
				}

				drawString(pMe, fontId, line, x, yAux, align);
				linePos = 0;

				yAux += pMe->pFont[fontId].font_h[32] + 2;

				if (yAux + pMe->pFont[fontId].font_h[32]  >= ( y + height))
					return;

				currentLineWidth = 0;

				word[wordPos]	= ' ';
				word[wordPos+1] = '\0';
				STRCPY(&(line[linePos]), word);
				linePos += STRLEN(word);
				currentLineWidth += getStringWidht(pMe, fontId, word);
				wordPos = 0;
		}
	}

	if (linePos >0)
	{
		line[linePos] = '\0';

		drawString(pMe, fontId, line, x, yAux, align);
	}
}

static void MD5Init (MD5_CTX *mdContext)
{
  mdContext->i[0] = mdContext->i[1] = (UINT4)0;

  /* Load magic initialization constants.
   */
  mdContext->buf[0] = (UINT4)0x67452301;
  mdContext->buf[1] = (UINT4)0xefcdab89;
  mdContext->buf[2] = (UINT4)0x98badcfe;
  mdContext->buf[3] = (UINT4)0x10325476;
}

static void MD5Update (MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen)
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* update number of bits */
  if ((mdContext->i[0] + ((UINT4)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((UINT4)inLen << 3);
  mdContext->i[1] += ((UINT4)inLen >> 29);

  while (inLen--) {
    /* add new character to buffer, increment mdi */
    mdContext->in[mdi++] = *inBuf++;

    /* transform if necessary */
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
                (((UINT4)mdContext->in[ii+2]) << 16) |
                (((UINT4)mdContext->in[ii+1]) << 8) |
                ((UINT4)mdContext->in[ii]);
      Transform (mdContext->buf, in);
      mdi = 0;
    }
  }
}

static void Transform (UINT4 *buf, UINT4 *in)
{
  UINT4 a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
  FF ( a, b, c, d, in[ 0], S11, 3614090360U); /* 1 */
  FF ( d, a, b, c, in[ 1], S12, 3905402710U); /* 2 */
  FF ( c, d, a, b, in[ 2], S13,  606105819U); /* 3 */
  FF ( b, c, d, a, in[ 3], S14, 3250441966U); /* 4 */
  FF ( a, b, c, d, in[ 4], S11, 4118548399U); /* 5 */
  FF ( d, a, b, c, in[ 5], S12, 1200080426U); /* 6 */
  FF ( c, d, a, b, in[ 6], S13, 2821735955U); /* 7 */
  FF ( b, c, d, a, in[ 7], S14, 4249261313U); /* 8 */
  FF ( a, b, c, d, in[ 8], S11, 1770035416U); /* 9 */
  FF ( d, a, b, c, in[ 9], S12, 2336552879U); /* 10 */
  FF ( c, d, a, b, in[10], S13, 4294925233U); /* 11 */
  FF ( b, c, d, a, in[11], S14, 2304563134U); /* 12 */
  FF ( a, b, c, d, in[12], S11, 1804603682U); /* 13 */
  FF ( d, a, b, c, in[13], S12, 4254626195U); /* 14 */
  FF ( c, d, a, b, in[14], S13, 2792965006U); /* 15 */
  FF ( b, c, d, a, in[15], S14, 1236535329U); /* 16 */

  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, in[ 1], S21, 4129170786U); /* 17 */
  GG ( d, a, b, c, in[ 6], S22, 3225465664U); /* 18 */
  GG ( c, d, a, b, in[11], S23,  643717713U); /* 19 */
  GG ( b, c, d, a, in[ 0], S24, 3921069994U); /* 20 */
  GG ( a, b, c, d, in[ 5], S21, 3593408605U); /* 21 */
  GG ( d, a, b, c, in[10], S22,   38016083U); /* 22 */
  GG ( c, d, a, b, in[15], S23, 3634488961U); /* 23 */
  GG ( b, c, d, a, in[ 4], S24, 3889429448U); /* 24 */
  GG ( a, b, c, d, in[ 9], S21,  568446438U); /* 25 */
  GG ( d, a, b, c, in[14], S22, 3275163606U); /* 26 */
  GG ( c, d, a, b, in[ 3], S23, 4107603335U); /* 27 */
  GG ( b, c, d, a, in[ 8], S24, 1163531501U); /* 28 */
  GG ( a, b, c, d, in[13], S21, 2850285829U); /* 29 */
  GG ( d, a, b, c, in[ 2], S22, 4243563512U); /* 30 */
  GG ( c, d, a, b, in[ 7], S23, 1735328473U); /* 31 */
  GG ( b, c, d, a, in[12], S24, 2368359562U); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, in[ 5], S31, 4294588738U); /* 33 */
  HH ( d, a, b, c, in[ 8], S32, 2272392833U); /* 34 */
  HH ( c, d, a, b, in[11], S33, 1839030562U); /* 35 */
  HH ( b, c, d, a, in[14], S34, 4259657740U); /* 36 */
  HH ( a, b, c, d, in[ 1], S31, 2763975236U); /* 37 */
  HH ( d, a, b, c, in[ 4], S32, 1272893353U); /* 38 */
  HH ( c, d, a, b, in[ 7], S33, 4139469664U); /* 39 */
  HH ( b, c, d, a, in[10], S34, 3200236656U); /* 40 */
  HH ( a, b, c, d, in[13], S31,  681279174U); /* 41 */
  HH ( d, a, b, c, in[ 0], S32, 3936430074U); /* 42 */
  HH ( c, d, a, b, in[ 3], S33, 3572445317U); /* 43 */
  HH ( b, c, d, a, in[ 6], S34,   76029189U); /* 44 */
  HH ( a, b, c, d, in[ 9], S31, 3654602809U); /* 45 */
  HH ( d, a, b, c, in[12], S32, 3873151461U); /* 46 */
  HH ( c, d, a, b, in[15], S33,  530742520U); /* 47 */
  HH ( b, c, d, a, in[ 2], S34, 3299628645U); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, in[ 0], S41, 4096336452U); /* 49 */
  II ( d, a, b, c, in[ 7], S42, 1126891415U); /* 50 */
  II ( c, d, a, b, in[14], S43, 2878612391U); /* 51 */
  II ( b, c, d, a, in[ 5], S44, 4237533241U); /* 52 */
  II ( a, b, c, d, in[12], S41, 1700485571U); /* 53 */
  II ( d, a, b, c, in[ 3], S42, 2399980690U); /* 54 */
  II ( c, d, a, b, in[10], S43, 4293915773U); /* 55 */
  II ( b, c, d, a, in[ 1], S44, 2240044497U); /* 56 */
  II ( a, b, c, d, in[ 8], S41, 1873313359U); /* 57 */
  II ( d, a, b, c, in[15], S42, 4264355552U); /* 58 */
  II ( c, d, a, b, in[ 6], S43, 2734768916U); /* 59 */
  II ( b, c, d, a, in[13], S44, 1309151649U); /* 60 */
  II ( a, b, c, d, in[ 4], S41, 4149444226U); /* 61 */
  II ( d, a, b, c, in[11], S42, 3174756917U); /* 62 */
  II ( c, d, a, b, in[ 2], S43,  718787259U); /* 63 */
  II ( b, c, d, a, in[ 9], S44, 3951481745U); /* 64 */

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}

static void MD5Final (MD5_CTX *mdContext)
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;
  unsigned int padLen;

  /* save number of bits */
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* pad out to 56 mod 64 */
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  MD5Update (mdContext, PADDING, padLen);

  /* append length in bits and transform */
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
            (((UINT4)mdContext->in[ii+2]) << 16) |
            (((UINT4)mdContext->in[ii+1]) << 8) |
            ((UINT4)mdContext->in[ii]);
  Transform (mdContext->buf, in);

  /* store buffer in digest */
  for (i = 0, ii = 0; i < 4; i++, ii += 4) {
    mdContext->digest[ii] = (unsigned char)(mdContext->buf[i] & 0xFF);
    mdContext->digest[ii+1] =
      (unsigned char)((mdContext->buf[i] >> 8) & 0xFF);
    mdContext->digest[ii+2] =
      (unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
    mdContext->digest[ii+3] =
      (unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
  }
}
