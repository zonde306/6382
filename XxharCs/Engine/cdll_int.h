/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  cdll_int.h
//
// 4-23-98  
// JOHN:  client dll interface declarations
//

#ifndef CDLL_INT_H
#define CDLL_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "const.h"

// this file is included by both the engine and the client-dll,
// so make sure engine declarations aren't done twice

typedef int HCSPRITE;	// handle to a graphic

#define SCRINFO_SCREENFLASH 1
#define SCRINFO_STRETCHED	2

typedef struct
{
	int		iSize;
	int		iWidth;
	int		iHeight;
	int		iFlags;
	int		iCharHeight;
	short	charWidths[256];
} SCREENINFO;


typedef struct client_data_s
{
	// fields that cannot be modified  (ie. have no effect if changed)
	vec3_t origin;

	// fields that can be changed by the cldll
	vec3_t viewangles;
	int		iWeaponBits;
	float	fov;	// field of view
} client_data_t;

typedef struct client_sprite_s
{
	char szName[64];
	char szSprite[64];
	int hspr;
	int iRes;
	wrect_t rc;
} client_sprite_t;

typedef struct
{
	int		effect;
	byte	r1, g1, b1, a1;		// 2 colors for effects
	byte	r2, g2, b2, a2;
	float	x;
	float	y;
	float	fadein;
	float	fadeout;
	float	holdtime;
	float	fxtime;
	const char *pName;
	const char *pMessage;
} client_textmessage_t;

typedef struct
{
	char *name;
	short ping;
	byte thisplayer;  // TRUE if this is the calling player

  // stuff that's unused at the moment,  but should be done
	byte spectator;
	byte packetloss;

	char *model;
	short topcolor;
	short bottomcolor;

} hud_player_info_t;

#define	MAX_ALIAS_NAME	32
typedef struct cmdalias_s
{
	struct cmdalias_s	*next;
	char	name[MAX_ALIAS_NAME];
	char	*value;
} cmdalias_t;

typedef struct sequenceCommandLine_ sequenceCommandLine_s;
struct sequenceCommandLine_
{
	int						commandType;		// Specifies the type of command
	client_textmessage_t	clientMessage;		// Text HUD message struct
	char*					speakerName;		// Targetname of speaking entity
	char*					listenerName;		// Targetname of entity being spoken to
	char*					soundFileName;		// Name of sound file to play
	char*					sentenceName;		// Name of sentences.txt to play
	char*					fireTargetNames;	// List of targetnames to fire
	char*					killTargetNames;	// List of targetnames to remove
	float					delay;				// Seconds 'till next command
	int						repeatCount;		// If nonzero, reset execution pointer to top of block (N times, -1 = infinite)
	int						textChannel;		// Display channel on which text message is sent
	int						modifierBitField;	// Bit field to specify what clientmessage fields are valid
	sequenceCommandLine_s*	nextCommandLine;	// Next command (linked list)
};

typedef struct sequenceEntry_ sequenceEntry_s;
struct sequenceEntry_
{
	char*					fileName;		// Name of sequence file without .SEQ extension
	char*					entryName;		// Name of entry label in file
	sequenceCommandLine_s*	firstCommand;	// Linked list of commands in entry
	sequenceEntry_s*		nextEntry;		// Next loaded entry
	qboolean				isGlobal;		// Is entry retained over level transitions?
};

typedef struct sentenceEntry_ sentenceEntry_s;
struct sentenceEntry_
{
	char*					data;			// sentence data (ie "We have hostiles" )
	sentenceEntry_s*		nextEntry;		// Next loaded entry
	qboolean				isGlobal;		// Is entry retained over level transitions?
	unsigned int			index;			// this entry's position in the file.
};

typedef enum
{
	NA_UNUSED,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX,
} netadrtype_t;

typedef struct netadr_s
{
	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned char	ipx[10];
	unsigned short	port;
} netadr_t;

typedef struct net_response_s
{
	// NET_SUCCESS or an error code
	int			error;

	// Context ID
	int			context;
	// Type
	int			type;

	// Server that is responding to the request
	netadr_t	remote_address;

	// Response RTT ping time
	double		ping;
	// Key/Value pair string ( separated by backlash \ characters )
	// WARNING:  You must copy this buffer in the callback function, because it is freed
	//  by the engine right after the call!!!!
	// ALSO:  For NETAPI_REQUEST_SERVERLIST requests, this will be a pointer to a linked list of net_adrlist_t's
	void		*response;
} net_response_t;

typedef void(*net_api_response_func_t) (struct net_response_s *response);
typedef struct net_api_s
{
	// APIs
	void(*InitNetworking)(void);
	void(*Status) (struct net_status_s *status);
	void(*SendRequest) (int context, int request, int flags, double timeout, struct netadr_s *remote_address, net_api_response_func_t response);
	void(*CancelRequest) (int context);
	void(*CancelAllRequests) (void);
	char		*(*AdrToString) (struct netadr_s *a);
	int(*CompareAdr) (struct netadr_s *a, struct netadr_s *b);
	int(*StringToAdr) (char *s, struct netadr_s *a);
	const char *(*ValueForKey) (const char *s, const char *key);
	void(*RemoveKey) (char *s, const char *key);
	void(*SetValueForKey) (char *s, const char *key, const char *value, int maxsize);
} net_api_t;

typedef HSPRITE(*pfnEngSrc_pfnSPR_Load_t)			(const char *szPicName);
typedef int(*pfnEngSrc_pfnSPR_Frames_t)			(HSPRITE hPic);
typedef int(*pfnEngSrc_pfnSPR_Height_t)			(HSPRITE hPic, int frame);
typedef int(*pfnEngSrc_pfnSPR_Width_t)			(HSPRITE hPic, int frame);
typedef void(*pfnEngSrc_pfnSPR_Set_t)				(HSPRITE hPic, int r, int g, int b);
typedef void(*pfnEngSrc_pfnSPR_Draw_t)			(int frame, int x, int y, const struct rect_s *prc);
typedef void(*pfnEngSrc_pfnSPR_DrawHoles_t)		(int frame, int x, int y, const struct rect_s *prc);
typedef void(*pfnEngSrc_pfnSPR_DrawAdditive_t)	(int frame, int x, int y, const struct rect_s *prc);
typedef void(*pfnEngSrc_pfnSPR_EnableScissor_t)	(int x, int y, int width, int height);
typedef void(*pfnEngSrc_pfnSPR_DisableScissor_t)	(void);
typedef struct client_sprite_s	*	(*pfnEngSrc_pfnSPR_GetList_t)			(char *psz, int *piCount);
typedef void(*pfnEngSrc_pfnFillRGBA_t)			(int x, int y, int width, int height, int r, int g, int b, int a);
typedef int(*pfnEngSrc_pfnGetScreenInfo_t) 		(struct SCREENINFO_s *pscrinfo);
typedef void(*pfnEngSrc_pfnSetCrosshair_t)		(HSPRITE hspr, wrect_t rc, int r, int g, int b);
typedef struct cvar_s *				(*pfnEngSrc_pfnRegisterVariable_t)	(char *szName, char *szValue, int flags);
typedef float(*pfnEngSrc_pfnGetCvarFloat_t)		(char *szName);
typedef char*						(*pfnEngSrc_pfnGetCvarString_t)		(char *szName);
typedef int(*pfnEngSrc_pfnAddCommand_t)			(char *cmd_name, void(*pfnEngSrc_function)(void));
typedef int(*pfnEngSrc_pfnHookUserMsg_t)			(char *szMsgName, pfnUserMsgHook pfn);
typedef int(*pfnEngSrc_pfnServerCmd_t)			(char *szCmdString);
typedef int(*pfnEngSrc_pfnClientCmd_t)			(char *szCmdString);
typedef void(*pfnEngSrc_pfnPrimeMusicStream_t)	(char *szFilename, int looping);
typedef void(*pfnEngSrc_pfnGetPlayerInfo_t)		(int ent_num, struct hud_player_info_s *pinfo);
typedef void(*pfnEngSrc_pfnPlaySoundByName_t)		(char *szSound, float volume);
typedef void(*pfnEngSrc_pfnPlaySoundByNameAtPitch_t)	(char *szSound, float volume, int pitch);
typedef void(*pfnEngSrc_pfnPlaySoundVoiceByName_t)		(char *szSound, float volume, int pitch);
typedef void(*pfnEngSrc_pfnPlaySoundByIndex_t)	(int iSound, float volume);
typedef void(*pfnEngSrc_pfnAngleVectors_t)		(const float * vecAngles, float * forward, float * right, float * up);
typedef struct client_textmessage_s*(*pfnEngSrc_pfnTextMessageGet_t)		(const char *pName);
typedef int(*pfnEngSrc_pfnDrawCharacter_t)		(int x, int y, int number, int r, int g, int b);
typedef int(*pfnEngSrc_pfnDrawConsoleString_t)	(int x, int y, char *string);
typedef void(*pfnEngSrc_pfnDrawSetTextColor_t)	(float r, float g, float b);
typedef void(*pfnEngSrc_pfnDrawConsoleStringLen_t)(const char *string, int *length, int *height);
typedef void(*pfnEngSrc_pfnConsolePrint_t)		(const char *string);
typedef void(*pfnEngSrc_pfnCenterPrint_t)			(const char *string);
typedef int(*pfnEngSrc_GetWindowCenterX_t)		(void);
typedef int(*pfnEngSrc_GetWindowCenterY_t)		(void);
typedef void(*pfnEngSrc_GetViewAngles_t)			(float *);
typedef void(*pfnEngSrc_SetViewAngles_t)			(float *);
typedef int(*pfnEngSrc_GetMaxClients_t)			(void);
typedef void(*pfnEngSrc_Cvar_SetValue_t)			(char *cvar, float value);
typedef int(*pfnEngSrc_Cmd_Argc_t)					(void);
typedef char *						(*pfnEngSrc_Cmd_Argv_t)				(int arg);
typedef void(*pfnEngSrc_Con_Printf_t)				(char *fmt, ...);
typedef void(*pfnEngSrc_Con_DPrintf_t)			(char *fmt, ...);
typedef void(*pfnEngSrc_Con_NPrintf_t)			(int pos, char *fmt, ...);
typedef void(*pfnEngSrc_Con_NXPrintf_t)			(struct con_nprint_s *info, char *fmt, ...);
typedef const char *				(*pfnEngSrc_PhysInfo_ValueForKey_t)	(const char *key);
typedef const char *				(*pfnEngSrc_ServerInfo_ValueForKey_t)(const char *key);
typedef float(*pfnEngSrc_GetClientMaxspeed_t)		(void);
typedef int(*pfnEngSrc_CheckParm_t)				(char *parm, char **ppnext);
typedef void(*pfnEngSrc_Key_Event_t)				(int key, int down);
typedef void(*pfnEngSrc_GetMousePosition_t)		(int *mx, int *my);
typedef int(*pfnEngSrc_IsNoClipping_t)			(void);
typedef struct cl_entity_s *		(*pfnEngSrc_GetLocalPlayer_t)		(void);
typedef struct cl_entity_s *		(*pfnEngSrc_GetViewModel_t)			(void);
typedef struct cl_entity_s *		(*pfnEngSrc_GetEntityByIndex_t)		(int idx);
typedef float(*pfnEngSrc_GetClientTime_t)			(void);
typedef void(*pfnEngSrc_V_CalcShake_t)			(void);
typedef void(*pfnEngSrc_V_ApplyShake_t)			(float *origin, float *angles, float factor);
typedef int(*pfnEngSrc_PM_PointContents_t)		(float *point, int *truecontents);
typedef int(*pfnEngSrc_PM_WaterEntity_t)			(float *p);
typedef struct pmtrace_s *			(*pfnEngSrc_PM_TraceLine_t)			(float *start, float *end, int flags, int usehull, int ignore_pe);
typedef struct model_s *			(*pfnEngSrc_CL_LoadModel_t)			(const char *modelname, int *index);
typedef int(*pfnEngSrc_CL_CreateVisibleEntity_t)	(int type, struct cl_entity_s *ent);
typedef const struct model_s *		(*pfnEngSrc_GetSpritePointer_t)		(HSPRITE hSprite);
typedef void(*pfnEngSrc_pfnPlaySoundByNameAtLocation_t)	(char *szSound, float volume, float *origin);
typedef unsigned short(*pfnEngSrc_pfnPrecacheEvent_t)		(int type, const char* psz);
typedef void(*pfnEngSrc_pfnPlaybackEvent_t)		(int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
typedef void(*pfnEngSrc_pfnWeaponAnim_t)			(int iAnim, int body);
typedef float(*pfnEngSrc_pfnRandomFloat_t)			(float flLow, float flHigh);
typedef int32_t(*pfnEngSrc_pfnRandomLong_t)			(int32_t lLow, int32_t lHigh);
typedef void(*pfnEngSrc_pfnHookEvent_t)			(char *name, void(*pfnEvent)(struct event_args_s *args));
typedef int(*pfnEngSrc_Con_IsVisible_t)			();
typedef const char *				(*pfnEngSrc_pfnGetGameDirectory_t)	(void);
typedef struct cvar_s *				(*pfnEngSrc_pfnGetCvarPointer_t)		(const char *szName);
typedef const char *				(*pfnEngSrc_Key_LookupBinding_t)		(const char *pBinding);
typedef const char *				(*pfnEngSrc_pfnGetLevelName_t)		(void);
typedef void(*pfnEngSrc_pfnGetScreenFade_t)		(struct screenfade_s *fade);
typedef void(*pfnEngSrc_pfnSetScreenFade_t)		(struct screenfade_s *fade);
typedef void *						(*pfnEngSrc_VGui_GetPanel_t)         ();
typedef void(*pfnEngSrc_VGui_ViewportPaintBackground_t) (int extents[4]);
typedef byte*						(*pfnEngSrc_COM_LoadFile_t)				(char *path, int usehunk, int *pLength);
typedef char*						(*pfnEngSrc_COM_ParseFile_t)			(char *data, char *token);
typedef void(*pfnEngSrc_COM_FreeFile_t)				(void *buffer);
typedef struct triangleapi_s *		pTriAPI;
typedef struct efx_api_s *			pEfxAPI;
typedef struct event_api_s *		pEventAPI;
typedef struct demo_api_s *			pDemoAPI;
typedef struct net_api_s *			pNetAPI;
typedef struct IVoiceTweak_s *		pVoiceTweak;
typedef int(*pfnEngSrc_IsSpectateOnly_t) (void);
typedef struct model_s *			(*pfnEngSrc_LoadMapSprite_t)			(const char *filename);
typedef void(*pfnEngSrc_COM_AddAppDirectoryToSearchPath_t) (const char *pszBaseDir, const char *appName);
typedef int(*pfnEngSrc_COM_ExpandFilename_t)				 (const char *fileName, char *nameOutBuffer, int nameOutBufferSize);
typedef const char *				(*pfnEngSrc_PlayerInfo_ValueForKey_t)(int playerNum, const char *key);
typedef void(*pfnEngSrc_PlayerInfo_SetValueForKey_t)(const char *key, const char *value);
typedef qboolean(*pfnEngSrc_GetPlayerUniqueID_t)(int iPlayer, char playerID[16]);
typedef int(*pfnEngSrc_GetTrackerIDForPlayer_t)(int playerSlot);
typedef int(*pfnEngSrc_GetPlayerForTrackerID_t)(int trackerID);
typedef int(*pfnEngSrc_pfnServerCmdUnreliable_t)(char *szCmdString);
typedef void(*pfnEngSrc_GetMousePos_t)(struct tagPOINT *ppt);
typedef void(*pfnEngSrc_SetMousePos_t)(int x, int y);
typedef void(*pfnEngSrc_SetMouseEnable_t)(qboolean fEnable);
typedef struct cvar_s *				(*pfnEngSrc_GetFirstCVarPtr_t)();
typedef unsigned int(*pfnEngSrc_GetFirstCmdFunctionHandle_t)();
typedef unsigned int(*pfnEngSrc_GetNextCmdFunctionHandle_t)(unsigned int cmdhandle);
typedef const char *				(*pfnEngSrc_GetCmdFunctionName_t)(unsigned int cmdhandle);
typedef float(*pfnEngSrc_GetClientOldTime_t)();
typedef float(*pfnEngSrc_GetServerGravityValue_t)();
typedef struct model_s	*			(*pfnEngSrc_GetModelByIndex_t)(int index);
typedef void(*pfnEngSrc_pfnSetFilterMode_t)(int mode);
typedef void(*pfnEngSrc_pfnSetFilterColor_t)(float r, float g, float b);
typedef void(*pfnEngSrc_pfnSetFilterBrightness_t)(float brightness);
typedef sequenceEntry_s*			(*pfnEngSrc_pfnSequenceGet_t)(const char *fileName, const char* entryName);
typedef void(*pfnEngSrc_pfnSPR_DrawGeneric_t)(int frame, int x, int y, const struct rect_s *prc, int src, int dest, int w, int h);
typedef sentenceEntry_s*			(*pfnEngSrc_pfnSequencePickSentence_t)(const char *sentenceName, int pickMethod, int* entryPicked);
// draw a complete string
typedef int(*pfnEngSrc_pfnDrawString_t)		(int x, int y, const char *str, int r, int g, int b);
typedef int(*pfnEngSrc_pfnDrawStringReverse_t)		(int x, int y, const char *str, int r, int g, int b);
typedef const char *				(*pfnEngSrc_LocalPlayerInfo_ValueForKey_t)(const char *key);
typedef int(*pfnEngSrc_pfnVGUI2DrawCharacter_t)		(int x, int y, int ch, unsigned int font);
typedef int(*pfnEngSrc_pfnVGUI2DrawCharacterAdd_t)	(int x, int y, int ch, int r, int g, int b, unsigned int font);
typedef unsigned int(*pfnEngSrc_COM_GetApproxWavePlayLength) (const char * filename);
typedef void *						(*pfnEngSrc_pfnGetCareerUI_t)();
typedef void(*pfnEngSrc_Cvar_Set_t)			(char *cvar, char *value);
typedef int(*pfnEngSrc_pfnIsPlayingCareerMatch_t)();
typedef double(*pfnEngSrc_GetAbsoluteTime_t) (void);
typedef void(*pfnEngSrc_pfnProcessTutorMessageDecayBuffer_t)(int *buffer, int bufferLength);
typedef void(*pfnEngSrc_pfnConstructTutorMessageDecayBuffer_t)(int *buffer, int bufferLength);
typedef void(*pfnEngSrc_pfnResetTutorMessageDecayData_t)();
typedef void(*pfnEngSrc_pfnFillRGBABlend_t)			(int x, int y, int width, int height, int r, int g, int b, int a);
typedef int(*pfnEngSrc_pfnGetAppID_t)			(void);
typedef cmdalias_t*				(*pfnEngSrc_pfnGetAliases_t)		(void);
typedef void(*pfnEngSrc_pfnVguiWrap2_GetMouseDelta_t) (int *x, int *y);

// this is by no means complete,  or even accurate
typedef struct cl_enginefuncs_s
{
	// sprite handlers
	HCSPRITE						( *pfnSPR_Load )			( const char *szPicName );
	int							( *pfnSPR_Frames )			( HCSPRITE hPic );
	int							( *pfnSPR_Height )			( HCSPRITE hPic, int frame );
	int							( *pfnSPR_Width )			( HCSPRITE hPic, int frame );
	void						( *pfnSPR_Set )				( HCSPRITE hPic, int r, int g, int b );
	void						( *pfnSPR_Draw )			( int frame, int x, int y, const wrect_t *prc );
	void						( *pfnSPR_DrawHoles )		( int frame, int x, int y, const wrect_t *prc );
	void						( *pfnSPR_DrawAdditive )	( int frame, int x, int y, const wrect_t *prc );
	void						( *pfnSPR_EnableScissor )	( int x, int y, int width, int height );
	void						( *pfnSPR_DisableScissor )	( void );
	client_sprite_t				*( *pfnSPR_GetList )			( char *psz, int *piCount );

	// screen handlers
	void						( *pfnFillRGBA )			( int x, int y, int width, int height, int r, int g, int b, int a );
	int							( *pfnGetScreenInfo ) 		( SCREENINFO *pscrinfo );
	void						( *pfnSetCrosshair )		( HCSPRITE hspr, wrect_t rc, int r, int g, int b );

	// cvar handlers
	struct cvar_s				*( *pfnRegisterVariable )	( char *szName, char *szValue, int flags );
	float						( *pfnGetCvarFloat )		( char *szName );
	char*						( *pfnGetCvarString )		( char *szName );

	// command handlers
	int							( *pfnAddCommand )			( char *cmd_name, void (*function)(void) );
	int							( *pfnHookUserMsg )			( char *szMsgName, pfnUserMsgHook pfn );
	int							( *pfnServerCmd )			( char *szCmdString );
	int							( *pfnClientCmd )			( char *szCmdString );

	void						( *pfnGetPlayerInfo )		( int ent_num, hud_player_info_t *pinfo );

	// sound handlers
	void						( *pfnPlaySoundByName )		( char *szSound, float volume );
	void						( *pfnPlaySoundByIndex )	( int iSound, float volume );

	// vector helpers
	void						( *pfnAngleVectors )		( const float * vecAngles, float * forward, float * right, float * up );

	// text message system
	client_textmessage_t		*( *pfnTextMessageGet )		( const char *pName );
	int							( *pfnDrawCharacter )		( int x, int y, int number, int r, int g, int b );
	int							( *pfnDrawConsoleString )	( int x, int y, char *string );
	void						( *pfnDrawSetTextColor )	( float r, float g, float b );
	void						( *pfnDrawConsoleStringLen )(  const char *string, int *length, int *height );

	void						( *pfnConsolePrint )		( const char *string );
	void						( *pfnCenterPrint )			( const char *string );


// Added for user input processing
	int							( *GetWindowCenterX )		( void );
	int							( *GetWindowCenterY )		( void );
	void						( *GetViewAngles )			( float * );
	void						( *SetViewAngles )			( float * );
	int							( *GetMaxClients )			( void );
	void						( *Cvar_SetValue )			( char *cvar, float value );

	int       					(*Cmd_Argc)					(void);	
	char						*( *Cmd_Argv )				( int arg );
	void						( *Con_Printf )				( char *fmt, ... );
	void						( *Con_DPrintf )			( char *fmt, ... );
	void						( *Con_NPrintf )			( int pos, char *fmt, ... );
	void						( *Con_NXPrintf )			( struct con_nprint_s *info, char *fmt, ... );

	const char					*( *PhysInfo_ValueForKey )	( const char *key );
	const char					*( *ServerInfo_ValueForKey )( const char *key );
	float						( *GetClientMaxspeed )		( void );
	int							( *CheckParm )				( char *parm, char **ppnext );
	void						( *Key_Event )				( int key, int down );
	void						( *GetMousePosition )		( int *mx, int *my );
	int							( *IsNoClipping )			( void );

	struct cl_entity_s			*( *GetLocalPlayer )		( void );
	struct cl_entity_s			*( *GetViewModel )			( void );
	struct cl_entity_s			*( *GetEntityByIndex )		( int idx );

	float						( *GetClientTime )			( void );
	void						( *V_CalcShake )			( void );
	void						( *V_ApplyShake )			( float *origin, float *angles, float factor );

	int							( *PM_PointContents )		( float *point, int *truecontents );
	int							( *PM_WaterEntity )			( float *p );
	struct pmtrace_s			*( *PM_TraceLine )			( float *start, float *end, int flags, int usehull, int ignore_pe );

	struct model_s				*( *CL_LoadModel )			( const char *modelname, int *index );
	int							( *CL_CreateVisibleEntity )	( int type, struct cl_entity_s *ent );

	const struct model_s *		( *GetSpritePointer )		( HCSPRITE hSprite );
	void						( *pfnPlaySoundByNameAtLocation )	( char *szSound, float volume, float *origin );

	unsigned short				( *pfnPrecacheEvent )		( int type, const char* psz );
	void						( *pfnPlaybackEvent )		( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
	void						( *pfnWeaponAnim )			( int iAnim, int body );
	float						( *pfnRandomFloat )			( float flLow, float flHigh );
	long						( *pfnRandomLong )			( long lLow, long lHigh );
	void						( *pfnHookEvent )			( char *name, void ( *pfnEvent )( struct event_args_s *args ) );
	int							(*Con_IsVisible)			();
	const char					*( *pfnGetGameDirectory )	( void );
	struct cvar_s				*( *pfnGetCvarPointer )		( const char *szName );
	const char					*( *Key_LookupBinding )		( const char *pBinding );
	const char					*( *pfnGetLevelName )		( void );
	void						( *pfnGetScreenFade )		( struct screenfade_s *fade );
	void						( *pfnSetScreenFade )		( struct screenfade_s *fade );
	void                        *( *VGui_GetPanel )         ( );
	void                         ( *VGui_ViewportPaintBackground ) (int extents[4]);

	byte*						(*COM_LoadFile)				( char *path, int usehunk, int *pLength );
	char*						(*COM_ParseFile)			( char *data, char *token );
	void						(*COM_FreeFile)				( void *buffer );
		
	struct triangleapi_s		*pTriAPI;
	struct efx_api_s			*pEfxAPI;
	struct event_api_s			*pEventAPI;
	struct demo_api_s			*pDemoAPI;
	struct net_api_s			*pNetAPI;
	struct IVoiceTweak_s		*pVoiceTweak;

	// returns 1 if the client is a spectator only (connected to a proxy), 0 otherwise or 2 if in dev_overview mode
	int							( *IsSpectateOnly ) ( void );
	struct model_s				*( *LoadMapSprite )			( const char *filename );

	// file search functions
	void						( *COM_AddAppDirectoryToSearchPath ) ( const char *pszBaseDir, const char *appName );
	int							( *COM_ExpandFilename)				 ( const char *fileName, char *nameOutBuffer, int nameOutBufferSize );

	// User info
	// playerNum is in the range (1, MaxClients)
	// returns NULL if player doesn't exit
	// returns "" if no value is set
	const char					*( *PlayerInfo_ValueForKey )( int playerNum, const char *key );
	void						( *PlayerInfo_SetValueForKey )( const char *key, const char *value );

	// Gets a unique ID for the specified player. This is the same even if you see the player on a different server.
	// iPlayer is an entity index, so client 0 would use iPlayer=1.
	// Returns false if there is no player on the server in the specified slot.
	qboolean					(*GetPlayerUniqueID)(int iPlayer, char playerID[16]);

	// TrackerID access
	int							(*GetTrackerIDForPlayer)(int playerSlot);
	int							(*GetPlayerForTrackerID)(int trackerID);

	// Same as pfnServerCmd, but the message goes in the unreliable stream so it can't clog the net stream
	// (but it might not get there).
	int							( *pfnServerCmdUnreliable )( char *szCmdString );

	pfnEngSrc_GetMousePos_t					pfnGetMousePos;
	pfnEngSrc_SetMousePos_t					pfnSetMousePos;
	pfnEngSrc_SetMouseEnable_t				pfnSetMouseEnable;
	pfnEngSrc_GetFirstCVarPtr_t				GetFirstCvarPtr;
	pfnEngSrc_GetFirstCmdFunctionHandle_t	GetFirstCmdFunctionHandle;
	pfnEngSrc_GetNextCmdFunctionHandle_t	GetNextCmdFunctionHandle;
	pfnEngSrc_GetCmdFunctionName_t			GetCmdFunctionName;
	pfnEngSrc_GetClientOldTime_t			hudGetClientOldTime;
	pfnEngSrc_GetServerGravityValue_t		hudGetServerGravityValue;
	pfnEngSrc_GetModelByIndex_t				hudGetModelByIndex;
	pfnEngSrc_pfnSetFilterMode_t			pfnSetFilterMode;
	pfnEngSrc_pfnSetFilterColor_t			pfnSetFilterColor;
	pfnEngSrc_pfnSetFilterBrightness_t		pfnSetFilterBrightness;
	pfnEngSrc_pfnSequenceGet_t				pfnSequenceGet;
	pfnEngSrc_pfnSPR_DrawGeneric_t			pfnSPR_DrawGeneric;
	pfnEngSrc_pfnSequencePickSentence_t		pfnSequencePickSentence;
	pfnEngSrc_pfnDrawString_t				pfnDrawString;
	pfnEngSrc_pfnDrawStringReverse_t				pfnDrawStringReverse;
	pfnEngSrc_LocalPlayerInfo_ValueForKey_t		LocalPlayerInfo_ValueForKey;
	pfnEngSrc_pfnVGUI2DrawCharacter_t		pfnVGUI2DrawCharacter;
	pfnEngSrc_pfnVGUI2DrawCharacterAdd_t	pfnVGUI2DrawCharacterAdd;
	pfnEngSrc_COM_GetApproxWavePlayLength	COM_GetApproxWavePlayLength;
	pfnEngSrc_pfnGetCareerUI_t				pfnGetCareerUI;
	pfnEngSrc_Cvar_Set_t					Cvar_Set;
	pfnEngSrc_pfnIsPlayingCareerMatch_t		pfnIsCareerMatch;
	pfnEngSrc_pfnPlaySoundVoiceByName_t	pfnPlaySoundVoiceByName;
	pfnEngSrc_pfnPrimeMusicStream_t		pfnPrimeMusicStream;
	pfnEngSrc_GetAbsoluteTime_t				GetAbsoluteTime;
	pfnEngSrc_pfnProcessTutorMessageDecayBuffer_t		pfnProcessTutorMessageDecayBuffer;
	pfnEngSrc_pfnConstructTutorMessageDecayBuffer_t		pfnConstructTutorMessageDecayBuffer;
	pfnEngSrc_pfnResetTutorMessageDecayData_t		pfnResetTutorMessageDecayData;
	pfnEngSrc_pfnPlaySoundByNameAtPitch_t	pfnPlaySoundByNameAtPitch;
	pfnEngSrc_pfnFillRGBABlend_t					pfnFillRGBABlend;
	pfnEngSrc_pfnGetAppID_t					pfnGetAppID;
	pfnEngSrc_pfnGetAliases_t				pfnGetAliasList;
	pfnEngSrc_pfnVguiWrap2_GetMouseDelta_t pfnVguiWrap2_GetMouseDelta;
} cl_enginefunc_t;

#ifndef IN_BUTTONS_H
#include "in_buttons.h"
#endif

#define CLDLL_INTERFACE_VERSION		7

extern void ClientDLL_Init( void ); // from cdll_int.c
extern void ClientDLL_Shutdown( void );
extern void ClientDLL_HudInit( void );
extern void ClientDLL_HudVidInit( void );
extern void	ClientDLL_UpdateClientData( void );
extern void ClientDLL_Frame( double time );
extern void ClientDLL_HudRedraw( int intermission );
extern void ClientDLL_MoveClient( struct playermove_s *ppmove );
extern void ClientDLL_ClientMoveInit( struct playermove_s *ppmove );
extern char ClientDLL_ClientTextureType( char *name );

extern void ClientDLL_CreateMove( float frametime, struct usercmd_s *cmd, int active );
extern void ClientDLL_ActivateMouse( void );
extern void ClientDLL_DeactivateMouse( void );
extern void ClientDLL_MouseEvent( int mstate );
extern void ClientDLL_ClearStates( void );
extern int ClientDLL_IsThirdPerson( void );
extern void ClientDLL_GetCameraOffsets( float *ofs );
extern int ClientDLL_GraphKeyDown( void );
extern struct kbutton_s *ClientDLL_FindKey( const char *name );
extern void ClientDLL_CAM_Think( void );
extern void ClientDLL_IN_Accumulate( void );
extern void ClientDLL_CalcRefdef( struct ref_params_s *pparams );
extern int ClientDLL_AddEntity( int type, struct cl_entity_s *ent );
extern void ClientDLL_CreateEntities( void );

extern void ClientDLL_DrawNormalTriangles( void );
extern void ClientDLL_DrawTransparentTriangles( void );
extern void ClientDLL_StudioEvent( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
extern void ClientDLL_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
extern void ClientDLL_TxferLocalOverrides( struct entity_state_s *state, const struct clientdata_s *client );
extern void ClientDLL_ProcessPlayerState( struct entity_state_s *dst, const struct entity_state_s *src );
extern void ClientDLL_TxferPredictionData ( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
extern void ClientDLL_ReadDemoBuffer( int size, unsigned char *buffer );
extern int ClientDLL_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
extern int ClientDLL_GetHullBounds( int hullnumber, float *mins, float *maxs );

extern void ClientDLL_VGui_ConsolePrint(const char* text);

extern int ClientDLL_Key_Event( int down, int keynum, const char *pszCurrentBinding );
extern void ClientDLL_TempEntUpdate( double ft, double ct, double grav, struct tempent_s **ppFreeTE, struct tempent_s **ppActiveTE, int ( *addTEntity )( struct cl_entity_s *pEntity ), void ( *playTESound )( struct tempent_s *pTemp, float damp ) );
extern struct cl_entity_s *ClientDLL_GetUserEntity( int index );
extern void ClientDLL_VoiceStatus(int entindex, qboolean bTalking);
extern void ClientDLL_DirectorEvent(unsigned char command, unsigned int firstObject, unsigned int secondObject, unsigned int flags);


#ifdef __cplusplus
}
#endif

#endif // CDLL_INT_H
