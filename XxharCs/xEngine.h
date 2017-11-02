#include <Windows.h>
#include "clientdll.h"

INT xpfnClientCmd( char *szCmdString );
INT xpfnCenterPrint( char *String );
INT xCreateVisibleEntity( int type, struct cl_entity_s *ent );
struct cl_entity_s xGetEntityByIndex( int idx );
VOID xpfnConsolePrint( char *String );
VOID xpfnSetScreenFade( struct screenfade_s *fade );
VOID xpfnGetScreenFade( struct screenfade_s *fade );
VOID xSetViewAngles( float* fAngels );
INT xpfnGetScreenInfo( SCREENINFO *pscrinfo );
INT xpfnDrawCharacter( int x, int y, int number, int r, int g, int b );
INT xpfnDrawConsoleString( int x, int y, char *string );
VOID xpfnDrawSetTextColor( float r, float g, float b );
VOID xpfnDrawConsoleStringLen( const char *string, int *length, int *height );
struct cl_entity_s xGetViewModel( void );

extern DWORD dwClientCmd;
extern DWORD dwCenterPrint;
extern DWORD dwCreateVisibleEntity;
extern DWORD dwGetEntityByIndex;
extern DWORD dwConsolePrint;
extern DWORD dwSetScreenFade;
extern DWORD dwGetScreenFade;
extern DWORD dwSetViewAngles;
extern DWORD dwDrawConsoleString;
extern DWORD dwDrawSetTextColor;
extern DWORD dwGetViewModel;
extern DWORD dwDrawCharacter;
extern DWORD dwGetScreenInfo;
extern DWORD dwDrawConsoleStringLen;

/*
When i class it up it crashes hl...

class CXEngine
{
	public:
		INT xpfnClientCmd( char *szCmdString );
		INT xpfnCenterPrint( char *String );
		INT xCreateVisibleEntity( int type, struct cl_entity_s *ent );
		struct cl_entity_s xGetEntityByIndex( int idx );
		VOID xpfnConsolePrint( char *String );
		VOID xpfnSetScreenFade( struct screenfade_s *fade );
		VOID xpfnGetScreenFade( struct screenfade_s *fade );
		VOID xSetViewAngles( float* fAngels );
		INT xpfnGetScreenInfo( SCREENINFO *pscrinfo );
		INT xpfnDrawCharacter( int x, int y, int number, int r, int g, int b );
		INT xpfnDrawConsoleString( int x, int y, char *string );
		VOID xpfnDrawSetTextColor( float r, float g, float b );
		VOID xpfnDrawConsoleStringLen( const char *string, int *length, int *height );
		struct cl_entity_s xGetViewModel( void );
};*/

