#include "8086 To PPC.h"
#include "Mac Vars.h"

#undef EXTERN
#define EXTERN extern
#include "Blitter.h"
#include "Control.h"
#include "Mallocx.h"
#include "Palette.h"
#include "Use DrawSprockets.h"
#include "Resource.h"
#include "Sprocket Input.h"
#include "Vbl.h"

/********************************************************/														 
/*           3D ENGINE - Football   13/04/95		        */
/********************************************************/

// By Laurent Noel


/*********************** SWITCHES ***********************/

#define LINK


/********************* HEADER FILES *********************/
#ifdef __INTEL__
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <conio.h>
#include <i86.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <sys\stat.h>
#include "c:\sos\include\sos.h"

#include "3deng.h"
#include "data.h"
#include "vesa.h"
#include "mallocx.h"
#endif

#ifdef __POWERPC__
#include <stdlib.h>
#include <stdio.h>
//#include <malloc.h>
#include <math.h>
#include <string.h>
//#include <conio.h>
//#include <i86.h>
//#include <io.h>
//#include <fcntl.h>
//#include <direct.h>
//#include <sys\stat.h>
//#include "c:\sos\include\sos.h"

#include "3deng.h"
#include "Render.h"
#include "data.h"
//#include "vesa.h"
//#include "mallocx.h"
#endif

#include "Game.equ"
#include "Fap.equ"

#include "files.c"
#include "fgfx.c"

#include "adefines.h"

#if USERAVEENGINE==YES
#include "RAGE Starter Code.h"
#include "RAGE Utilities.h"

extern short dataResRave;
extern float RAGE_cur_camx;
extern float RAGE_cur_camz;
extern float RAGE_cur_camy;
extern float RAGE_cur_tx;
extern float RAGE_cur_tz;
extern float RAGE_cur_ty;

#endif

extern Boolean UseRaveEngine, Frontend;

/***************** PRE-DEFINED CONSTANTS ****************/

#define SCREENDIST 15	// Screen clip plane z distance, set to avoid overflow
#define CUTOFFDIST 5		// Cutoff plane z distance (can be -ve!), set for nearest visibility

#define MAXOBJ	100		// Max number of objects in one sort
#define MAXPOL 1000		// Max number of polys in one sort
#define MAXPTS 500		// Max number of points in one object
#define MAXLOOP 100 		// Max number of looping maps

#define MAX_FILES 500

// Undefined in C++ (?!)
#define min(a,b) (((a)<(b))?(a):(b))
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#define abs(a) (((a)>=0)?(a):-(a))


int bm_kgrid[]={ BM_KGRID1, BM_KGRID2, BM_KGRID3, BM_KGRID4, BM_KGRID5, BM_KGRID6 };

/*********************** PROTOTYPES *********************/
extern void sprite3d(short x,short y,short ref);	// Depth sorted sprites, (Neil's code)

// Dumps for various buffer types
void dumpMCGA(scrpt dispx,scrpt dispy);
//void dumpMCGAs(scrpt dispx,scrpt dispy);
//void dumpV256(scrpt dispx,scrpt dispy);
void dumpvidi1(scrpt dispx,scrpt dispy);
void dumpnull(scrpt dispx,scrpt dispy);

// Miscellaneous
extern float log_factor;	// No of logic frames/update

extern int Language;
//void draw_sprite(buff_info *buff,int sprite_no,int x,int y,BYTE col);
int draw_string(buff_info *buff,int font_no,int x,int y,char *string,BYTE col,short just);

#ifdef LINK
//extern  char keys[128];		// Key pressed array (indexed by scan-code)
//extern  char key_togs[128];// Key toggled array (indexed by scan-code)
extern  int icth,isth;		// Integer cos and sin of theta
extern  volatile int count;// Timer count
extern  int network_on;			// Network game
#else
//extern char keys[128];		// Key pressed array (indexed by scan-code)
//extern char key_togs[128];	// Key toggled array (indexed by scan-code)
extern int icth,isth;		// Integer cos and sin of theta
extern volatile int count;	// Timer count
extern network_on;			// Network game
#endif

// Wind data
extern short wind_on;
extern float wind_x,wind_y;
extern float wind_speed;

extern volatile Boolean paused;

extern char match_half;
extern int Language;

#if USERAVEENGINE==YES
//dword PitchTextureX[] = { 0x00000000, 0x00400000, 0x00400000, 0x00000000 };
//dword PitchTextureY[] = { 0x00B00000, 0x00B00000, 0x00780000, 0x00780000 };
dword PitchTextureX[] = { 0x00000000, 0x00FF0000, 0x00FF0000, 0x00000000 };
dword PitchTextureY[] = { 0x00000000, 0x00000000, 0x00FF0000, 0x00FF0000 };
#endif

/********************** GLOBAL VARS *********************/

// Setup data
setup_info setup;				// Initialisation data
int performance;		 		// Machine performance index
int icth, isth;

// Screen buffer info
buff_info main_buff; // = {NULL, 320,200,320,200,0.5,0.5, dumpnull};		// Current main screen buffer info
buff_info vidi_buff;													// Current vidi-buffer info
buff_info menu_buff;													// Current menu buffer info
buff_info_ext render_buff;												// Extended buffer info for rendering
#if USERAVEENGINE==YES
buff_info LORES_buff= {NULL, 320, 240, 320, 240, 0.5, 0.5, dumpScreen};
buff_info HIRES_buff= {NULL, 640, 480, 640, 480, 1.0, 1.0, dumpScreen};	// Current main screen buffer info
buff_info LORES_buffs[] ={
						{NULL, 320,240, 320,240, 0.5,   0.5,   NULL}
						};
buff_info HIRES_buffs[] ={
						  {NULL, 640,480, 640,480, 1,     1,     NULL}
						 };
#else
buff_info LORES_buff= {NULL, 320, 200, 320, 200, 0.5, 0.5, dumpScreen};
buff_info HIRES_buff= {NULL, 640, 400, 640, 400, 1.0, 1.0, dumpScreen};	// Current main screen buffer info
buff_info LORES_buffs[] ={
						{NULL, 320,200, 320,200, 0.5,   0.5,   NULL},
						{NULL, 320,200, 296,184, 0.4625,0.46,  NULL},
						{NULL, 320,200, 320,158, 0.5,   0.3975,NULL},
						{NULL, 320,200, 264,166, 0.4125,0.4175,NULL},
						{NULL, 320,200, 296,142, 0.4625,0.355, NULL},
						{NULL, 320,200, 240,150, 0.375, 0.375, NULL},
						{NULL, 320,200, 264,124, 0.4125,0.3125,NULL},
						{NULL, 320,200, 216,132, 0.3375,0.3325,NULL},
						{NULL, 320,200, 240,108, 0.375, 0.2725,NULL},
						{NULL, 320,200, 216, 92, 0.3375,0.23,  NULL},
						};
buff_info HIRES_buffs[] ={
						  {NULL, 640,400, 640,400, 1,     1,     NULL},
						  {NULL, 640,400, 584,366, 0.9125,0.9175,NULL},
						  {NULL, 640,400, 640,316, 1,     0.79,  NULL},
						  {NULL, 640,400, 528,332, 0.825, 0.8325,NULL},
						  {NULL, 640,400, 584,284, 0.9125,0.71,  NULL},
						  {NULL, 640,400, 480,300, 0.75,  0.75,  NULL},
						  {NULL, 640,400, 528,250, 0.825, 0.625, NULL},
						  {NULL, 640,400, 424,266, 0.6625,0.6675,NULL},
						  {NULL, 640,400, 480,216, 0.75,  0.54,  NULL},
						  {NULL, 640,400, 424,184, 0.6625,0.46,  NULL},
						 };
#endif
buff_info vidi_buff_1={NULL, 256,256, 128,80, 0.175,0.183, dumpnull};
buff_info vidi_buff_2={NULL, 256,256, 128,80, 0.175,0.183, dumpvidi1};

int mappages;					// No of texture-map pages being used

// Viewpoint vars
floatpt cth,sth,cph,sph;	// Cos & sin of theta and phi, the viewing angles
short q,oq;						// Perspective scaling
floatpt qr,qa;					// Precalculations derived from above
floatpt scrdr,persc;			// Used for 'alternative perspective scaling', see below
qmat rot,tran;					// Quick matrices for view and object rotation

// Object/polygon/point lists
objs *objlist;		// List of objects [object z, object polygon list]
objs *curobj;		// Current object pointer
pols *pollist;		// List of polygons [polygon z, polygon point list]
pols *curpol;		// Current polygon pointer
scrpt *ptslist;	// List of polygon point data [nsides, col, nsides*{x,y}]
scrpt *curpts;		// Current point pointer(!)
rotpt *rotlist;	// List of rotated pts for object
char *animdata;
BYTE *sky;


int clip_tex;		// Current clip texture map

#ifdef COUNT
int numpols;	 	// Count for number of polygons drawn
#endif


floatpt mcap_ballx,mcap_bally,mcap_ballz;

/***************** INITIALISED STRUCTURES ***************/

// Unit matrix
mat unit=
	{
	{1.,0.,0.,0.},
	{0.,1.,0.,0.},
	{0.,0.,1.,0.},
	{0.,0.,0.,1.},
	};


plyrdat *ppt;

//////////////////
// MEMORY AREAS //

// Texture map pages
#define X_BM 6  // Bitmap offset for extra maps
#define S_BM 8  // Bitmap offset for stand maps
#define MAPPAGES		S_BM+7
#define MAPPAGES_4		S_BM+2

typedef BYTE mappage[256*256];
mappage		*maps;

char * mapsel[20];
dword	td[5]   = { 0 };

//Handle		Sky;
//Handle		Maps;
//Handle		Textures;
//Handle		Objlist;
//Handle		Ptslist;
//Handle		Pollist;
//Handle		Filters;
//Handle		Rotlist;
//Handle		Plyrtwtb;
//Handle		Player_p;
//Handle		AnimData;

//int isth, icth;

/////////////////
// SCREEN INFO //

// Palette
//BYTE pal[768];

// Polygon filters
#define FILTERS 2

filter *filters;



// Joolz
extern short dataResFile, dataResFile2, mainResFile;
extern float angle;

long		 resType;
Boolean Scanner;

short RenderCount=0;

//////////////////////
// TEXTURE MAP INFO //


// Team players:
// Texture 13 (1+12): Head A (End 61 (1+5*12))
// Texture 73 (61+12): Head B (End 121 (61+5*12))
// Texture 133 (121+12): Torso A (End 181 (121+5*12))
// Texture 193 (181+12): Torso B (End 241 (181+5*12))
// Texture 244 (241+3): Lower Leg A (End 248 (241+7))
// Texture 251 (248+3): Upper Arm A (End 255 (248+7))
// Texture 258 (255+3): Upper Leg A (End 262 (255+7))
// Texture 265 (262+3): Lower Arm A (End 269 (262+7))
// Texture 272 (269+3): Lower Leg B (End 276 (269+7))
// Texture 279 (276+3): Upper Arm B (End 283 (276+7))
// Texture 286 (283+3): Upper Leg B (End 290 (283+7))
// Texture 293 (290+3): Lower Arm B (End 297 (290+7))
// Texture 309 (297+12): Foot (End 357 (297+5*12))


// Referee & keeper:
// Texture 369 (357+12): Torso Referee (End 417 (357+5*12))
// Texture 429 (417+12): Torso Keeper (End 477 (417+5*12))
// Texture 480 (477+3): Lower Leg Referee (End 484 (477+7))
// Texture 487 (484+3): Upper Arm Referee (End 491 (484+7))
// Texture 494 (491+3): Upper Leg Referee (End 498 (491+7))
// Texture 501 (498+3): Lower Arm Referee (End 505 (498+7))
// Texture 508 (505+3): Lower Leg Keeper (End 512 (505+7))
// Texture 515 (512+3): Upper Arm Keeper (End 519 (512+7))
// Texture 522 (519+3): Upper Leg Keeper (End 526 (519+7))
// Texture 529 (526+3): Lower Arm Keeper (End 533 (526+7))

#define X_TM 533		// Offset for extra maps
#define X_TM_NO (52+412)	// No of extra maps 
#define N_TM (X_TM+52)

#define S_TM (X_TM+X_TM_NO)// Offset for stand maps
#define S_TM_MAX 100			// Maximum no of stadium maps

#define NOTEXTURES S_TM+S_TM_MAX

#define CLIP_TEXTURES 200

texture *textures;

// Misc looping textures
int noloop=6;
struct {
	int pos;
	texture tex;
	} texloop[]=
	{
	 {
	  0x1c040,
	  {0x000000,0x000000,0x3fff00,0x3fff00,
	   0x000000,0x3fff00,0x3fff00,0x000000}
	 },
	 {
	  0x1c040,
	  {0x000000,0x000000,0x1fff00,0x1fff00,
	   0x000000,0x3fff00,0x3fff00,0x000000}
	 },
	 {
	  0x1c040,
	  {0x000000,0x000000,0x7fff00,0x7fff00,
	   0x000000,0x7fff00,0x7fff00,0x000000}
	 },
	 {
	  0x1c080,
	  {0x000000,0x000000,0x3fff00,0x3fff00,
	   0x000000,0x7fff00,0x7fff00,0x000000}
	 },
	 {
	  0x1c040,
	  {0x000000,0x000000,0xbfff00,0xbfff00,
	   0x000000,0xbfff00,0xbfff00,0x000000}
	 },
	 {
	  0x1c0c0,
	  {0x000000,0x000000,0x5fff00,0x5fff00,
	   0x000000,0xbfff00,0xbfff00,0x000000}
	 },
	};



#if 0
char *llang[6][29]=
{
	{
	"GOAL SCORED BY: ",
	"BOOKED, YELLOW CARD: ",
	"RED CARD, SENT OFF: ",
	"DIRECT FREE KICK",
	"INDIRECT FREE KICK",
	"PENALTY KICK",
	"CORNER KICK",
	"THROW IN",
	"GOAL KICK",
	"KICK OFF",
	"OFFSIDE",
	"PLAYER INJURED: ",
	"FULL TIME",
	"EXTRA TIME",
	"HALF TIME",
	"PENALTY SHOOTOUT",
	"FINAL SCORE AFTER PENALTIES",
	"OWN GOAL BY: ",
	"REPLAY",
	"SUBSTITUTION",
	"FORMATION",
	"CANCEL",
	"SELECT",
	"OFF",
	"SUBSTITUTE",
	"ON",
	"DONE",
	"OPTIONS",
	"REFEREE : ",
	},
	{
	"BUT DE: ",
	"AVERT., C. JAUNE: ",
	"C. ROUGE, RENV.: ",
	"COUP FR. DIRECT",
	"COUP FR. INDIR",
	"PENALTY",
	"CORNER",
	"TOUCHE",
	"TIR AU BUT",
	"CP. D<ENVOI",
	"HORS JEU",
	"JOUEUR BLESSE: ",
	"TPS TOTAL",
	"PROLONG.",
	"MI=TEMPS",
	"PENALTIES",
	"SCORE FINAL APRES PEN.",
	"BUT CTRE SOM CAMP DE: ",
	"REPLAY",
	"REMPLACEMENT",
	"FORMATION",
	"ANNULER",
	"SELECT",
	"OFF",
	"REMPLACANT",
	"ON",
	"FAIT",
	"OPTIONS",
	"ARBITRE : ",
	},
	{
	"TORSCHUETZE: ",
	"GELBE KARTE: ",
	"ROTE KARTE: ",
	"DIREKTER FREISTOSS",
	"FREISTOSS",
	"ELFMETER",
	"ECKBALL",
	"EINWURF",
	"ABSTOSS",
	"ANSTOSS",
	"ABSEITS",
	"SPIELER VERLETZT: ",
	"ABPFIFF",
	"VERLAENGERUNG",
	"HALBZEIT",
	"ELFMETERSCHIESSEN",
	"ERGEBNIS NACH ELFMETERSCHIESSEN",
	"EIGENTOR VON ",
	"WIEDERHOLUNG",
	"AUSWECHSLUNG",
	"AUFSTELLUNG",
	"ABBRUCH",
	"AUSWAEHLEN",
	"RAUS",
	"AUSWECHSELN",
	"REIN",
	"FERTIG",
	"OPTIONEN",
	"SCHIEDSRICHTER : ",
	},
	{
	"GOAL SCORED BY: ",
	"BOOKED, YELLOW CARD: ",
	"RED CARD, SENT OFF: ",
	"DIRECT FREE KICK",
	"INDIRECT FREE KICK",
	"PENALTY KICK",
	"CORNER KICK",
	"THROW IN",
	"GOAL KICK",
	"KICK OFF",
	"OFFSIDE",
	"PLAYER INJURED: ",
	"FULL TIME",
	"EXTRA TIME",
	"HALF TIME",
	"PENALTY SHOOTOUT",
	"FINAL SCORE AFTER PENALTIES",
	"OWN GOAL BY: ",
	"REPLAY",
	"SUBSTITUTION",
	"FORMATION",
	"CANCEL",
	"SELECT",
	"OFF",
	"SUBSTITUTE",
	"ON",
	"DONE",
	"OPTIONS",
	"REFEREE : ",
	},
	{
	"BUT DE: ",
	"AVERT., C. JAUNE: ",
	"C. ROUGE, RENV.: ",
	"COUP FR. DIRECT",
	"COUP FR. INDIR",
	"PENALTY",
	"CORNER",
	"TOUCHE",
	"TIR AU BUT",
	"CP. D<ENVOI",
	"HORS JEU",
	"JOUEUR BLESSE: ",
	"TPS TOTAL",
	"PROLONG.",
	"MI=TEMPS",
	"PENALTIES",
	"SCORE FINAL APRES PEN.",
	"BUT CTRE SOM CAMP DE: ",
	"REPLAY",
	"REMPLACEMENT",
	"FORMATION",
	"ANNULER",
	"SELECT",
	"OFF",
	"REMPLACANT",
	"ON",
	"FAIT",
	"OPTIONS",
	"ARBITRE : ",
	},
	{
	"M>L SCORET AV: ",
	"GULT KORT: ",
	"R&DT KORT, SENDT UT: ",
	"DIREKTE FRISPARK",
	"INDIREKTE FRISPARK",
	"STRAFFESPARK",
	"CORNER",
	"INNKAST",
	"UTSPILL FRA M>L",
	"AVSPARK",
	"OFFSIDE",
	"SKADD SPILLER: ",
	"FULL TID",
	"OVERTID",
	"HALVTID",
	"STRAFFEKONKURRANSE",
	"SLUTTRESULTAT ETTER STRAFFER",
	"SELVM>L VED: ",
	"REPRISE",
	"INNBYTTE",
	"FORMASJON",
	"AVBRYT",
	"VELG",
	"AV",
	"INNBYTTER",
	"P>",
	"FERDIG",
	"VALG",
	"DOMMER : ",
	},
};
#endif

// Stadia objects
#define MAX_STAD_PTS 400
#define MAX_STAD_FACES 400
obj stad1,stad2,stad3,stad4;
static datapt stad1_p[MAX_STAD_PTS*3],stad2_p[MAX_STAD_PTS*3],stad3_p[MAX_STAD_PTS*3],stad4_p[MAX_STAD_PTS*3];
static word stad1_f[MAX_STAD_FACES*6],stad2_f[MAX_STAD_FACES*6],stad3_f[MAX_STAD_FACES*6],stad4_f[MAX_STAD_FACES*6];
float st_w,st_l,st_h;

/********************* FUNCTION CODE ********************/
void InvertWord(BYTE *data, int howmany)
{
BYTE b0;

	for(int i=0;i<howmany;i++)
	  {
	  b0 = *data;
	  *data = *(data+1);
	  *(data+1) = b0;
	  data+=2;
	  }
}

void InvertFloat(BYTE *data, int howmany)
{
BYTE b0;

	for(int i=0;i<howmany;i++)
	  {
	  b0 = data[0]; data[0] = data[3]; data[3] = b0;
	  b0 = data[1]; data[1] = data[2]; data[2] = b0;
	  data+=4;
	  }
}

/*******************/
/* MATRIX ROUTINES */
void matcop(mat dest,mat src)	// Matrix copy
{
//short i;
//floatpt *p0=&dest[0][0];
//floatpt *p1=&src[0][0];
double *p0 = (double *)dest;
double *p1 = (double *)src;

	p0[0] = p1[0];
	p0[1] = p1[1];
	p0[2] = p1[2];
	p0[3] = p1[3];
	p0[4] = p1[4];
	p0[5] = p1[5];
	p0[6] = p1[6];
	p0[7] = p1[7];
//	memcpy(dest,src,sizeof(mat));
/*	
	for (i=0;i<4;i++)
	  {
	  *p0++=*p1++;
	  *p0++=*p1++;
	  *p0++=*p1++;
	  *p0++=*p1++;
	  }
*/
}

void matmul(mat m1,mat m2)	// Matrix multiply
{
	short i,j;

	mat res;
	floatpt *wr=&res[0][0];

	for (i=0;i<4;i++)
//		for (j=0;j<4;j++)
//			*wr++=m1[0][j]*m2[i][0] + m1[1][j]*m2[i][1] + m1[2][j]*m2[i][2] + m1[3][j]*m2[i][3];
	  {
			*wr++=m1[0][0]*m2[i][0] + m1[1][0]*m2[i][1] + m1[2][0]*m2[i][2] + m1[3][0]*m2[i][3];
			*wr++=m1[0][1]*m2[i][0] + m1[1][1]*m2[i][1] + m1[2][1]*m2[i][2] + m1[3][1]*m2[i][3];
			*wr++=m1[0][2]*m2[i][0] + m1[1][2]*m2[i][1] + m1[2][2]*m2[i][2] + m1[3][2]*m2[i][3];
			*wr++=m1[0][3]*m2[i][0] + m1[1][3]*m2[i][1] + m1[2][3]*m2[i][2] + m1[3][3]*m2[i][3];
	  }
	matcop(m1,res);
	}

void matmultr(mat m1,mat m2)
// Reversed matrix multiply.
	{
	short i,j;

	mat res;
	floatpt *wr=&res[0][0];

	for (i=0;i<4;i++)
//		for (j=0;j<4;j++)
//			*wr++=m1[0][j]*m2[0][i] + m1[1][j]*m2[1][i] + m1[2][j]*m2[2][i] + m1[3][j]*m2[3][i];
	  {
			*wr++=m1[0][0]*m2[0][i] + m1[1][0]*m2[1][i] + m1[2][0]*m2[2][i] + m1[3][0]*m2[3][i];
			*wr++=m1[0][1]*m2[0][i] + m1[1][1]*m2[1][i] + m1[2][1]*m2[2][i] + m1[3][1]*m2[3][i];
			*wr++=m1[0][2]*m2[0][i] + m1[1][2]*m2[1][i] + m1[2][2]*m2[2][i] + m1[3][2]*m2[3][i];
			*wr++=m1[0][3]*m2[0][i] + m1[1][3]*m2[1][i] + m1[2][3]*m2[2][i] + m1[3][3]*m2[3][i];
	  }
	matcop(m1,res);
	}

#if USERAVEENGINE==YES
void Mymemcpy( BYTE *sc, BYTE *src, int width)
{
double *screen = (double *)sc;
double *source = (double *)src;
double a,b,c,d,e,f,g,h;

		width /= 64;
		while( width--)
			{
			a = source[0]; b = source[1]; c = source[2]; d = source[3];
			e = source[4]; f = source[5]; g = source[6]; h = source[7];
			screen[0] = a; screen[1] = b; screen[2] = c; screen[3] = d;
			screen[4] = e; screen[5] = f; screen[6] = g; screen[7] = h;
			source += 8; screen += 8;
			}
}

asm void MyMemset( BYTE *sc, int width, long data)
{
	lwz		SCREEN, sc
	addi	SCREEN, SCREEN , -4

	lwz		EAX, data

	lwz		TEMP1, width
	srawi	TEMP1, TEMP1, 5
	mtctr	TEMP1
@fill
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	stwu	EAX, 4(SCREEN)
	bdnz	@fill
	
	blr
}

/*********************************************/
/* DRAW GROUND (SPECIALIZED TEXTURE MAPPING) */
/*********************************************/
/* DRAW GROUND (SPECIALIZED TEXTURE MAPPING) */
/* Joolz 1996				      */
asm void DrawTexture2(long w, BYTE *sc, BYTE *data, short pan)
{
	lwz		r3, td

	lwz		ESI, 6(r3)	// edx
	lha		TEMP1, pan
	rlwimi	ESI, TEMP1, 0, 16, 31

	lwz		EDX, 14(r3)
	lwz		TEMP1, 6(r3)
	rlwimi	EDX, TEMP1, 0, 16, 31

	lhz		EBX, 16(r3)

	lwz		ECX, 18(r3)
	lhz		TEMP1, 12(r3)
	rlwimi	ECX, TEMP1, 0, 16, 31

	MOVCHBL
	ANDCXSI(TEMP1)
	
	lwz		EBX, 10(r3)
	lwz		TEMP1, 2(r3)
	rlwimi	EBX, TEMP1, 0, 16, 31
	
	MOVBHDL
	
	li		TEMP1,0
	rlwimi	EDX, TEMP1, 0, 16, 31
	
	lwz		DATA, data
	lwz		SCREEN, sc
	lwz 	EJX, palette_buffer16

	lwz		TEMP1, w
	srawi	TEMP1, TEMP1, 3
	mtctr	TEMP1
@lpp2
	ANDBXSI(TEMP1)
	MOVXBX(EAX)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)
	add		EAX, EAX, EAX
	lhzx	EAX, EAX, EJX
	sthu	EAX, -2(SCREEN)

	ANDBXSI(TEMP1)
	MOVXBX(EAX)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)
	add		EAX, EAX, EAX
	lhzx	EAX, EAX, EJX
	sthu	EAX, -2(SCREEN)

	ANDBXSI(TEMP1)
	MOVXBX(EAX)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)
	add		EAX, EAX, EAX
	lhzx	EAX, EAX, EJX
	sthu	EAX, -2(SCREEN)

	ANDBXSI(TEMP1)
	MOVXBX(EAX)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)
	add		EAX, EAX, EAX
	lhzx	EAX, EAX, EJX
	sthu	EAX, -2(SCREEN)

	ANDBXSI(TEMP1)
	MOVXBX(EAX)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)
	add		EAX, EAX, EAX
	lhzx	EAX, EAX, EJX
	sthu	EAX, -2(SCREEN)

	ANDBXSI(TEMP1)
	MOVXBX(EAX)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)
	add		EAX, EAX, EAX
	lhzx	EAX, EAX, EJX
	sthu	EAX, -2(SCREEN)

	ANDBXSI(TEMP1)
	MOVXBX(EAX)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)
	add		EAX, EAX, EAX
	lhzx	EAX, EAX, EJX
	sthu	EAX, -2(SCREEN)

	ANDBXSI(TEMP1)
	MOVXBX(EAX)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)
	add		EAX, EAX, EAX
	lhzx	EAX, EAX, EJX
	sthu	EAX, -2(SCREEN)

	bdnz	@lpp2
	blr
}

static void RavePitchSky( float vr, float vwy, datapt vx, datapt vy, datapt vz, BYTE *buffer, int offset)
{
scrpt y,ys[4],ye,yd;
rotpt ry[4],rz[4],ze;
float t,d,z,tx,tz,dw1,dw2,sa;

BYTE *cpyline;
unsigned short *scrline;
char *flrsel;
word w,psh,pan;
int a;

	if( setup.detail.pitch)
		{
		// Find highest pt of pitch
		ry[0]=-st_l*rot.e.r10+st_w*rot.e.r12+rot.e.r13;
		rz[0]=-st_l*rot.e.r20+st_w*rot.e.r22+rot.e.r23;
		ry[1]=(1280+st_l)*rot.e.r10+st_w*rot.e.r12+rot.e.r13;
		rz[1]=(1280+st_l)*rot.e.r20+st_w*rot.e.r22+rot.e.r23;
		ry[2]=(1280+st_l)*rot.e.r10-(800+st_w)*rot.e.r12+rot.e.r13;
		rz[2]=(1280+st_l)*rot.e.r20-(800+st_w)*rot.e.r22+rot.e.r23;
		ry[3]=-st_l*rot.e.r10-(800+st_w)*rot.e.r12+rot.e.r13;
		rz[3]=-st_l*rot.e.r20-(800+st_w)*rot.e.r22+rot.e.r23;

		if (rz[0]>0)
			ye=q*ry[0]/rz[0],ze=rz[0];
		else 
			ye=-render_buff.clip_ymid,ze=0;
		if (rz[1]>ze) ye=q*ry[1]/rz[1],ze=rz[1];
		if (rz[2]>ze) ye=q*ry[2]/rz[2],ze=rz[2];
		if (rz[3]>ze) ye=q*ry[3]/rz[3],ze=rz[3];
		if (ye>render_buff.clip_ymid) ye=render_buff.clip_ymid;

		y=-render_buff.clip_ymid;
		t=q*sph+y*cph;						// Repeated divisor & incrementor

		scrline=(unsigned short *)(buffer+479*offset);

		if( setup.detail.pitch == 2)
			{
			psh=16;
			pan=0x3f3f;
			flrsel=mapsel[mappages+noloop+1];
			}
		else
			{
			psh=15;
			pan=0x1f1f;
			flrsel=mapsel[mappages+noloop];
			}

		offset /=2;
		while (t<0 && y<ye)
			{				 
			d=vy/t;								// Depth (unscaled)
			z=d*(q*cph-y*sph);					// Floor depth
			tx=vx-z*sth;						// X pos of pt
			tz=-vz-z*cth;						// Z pos of pt

			dw2=-d*sth;
			dw1=d*cth;
			td[1]=(dword)((tz-dw2*render_buff.clip_xmid)*(1<<psh));
			td[2]=(dword)((tx-dw1*render_buff.clip_xmid)*(1<<psh));
			td[3]=(dword)(dw2*(1<<psh))&0xffffff;
			td[4]=(dword)(dw1*(1<<psh))&0xffffff;
			y++; t+=cph;
			DrawTexture2( 640, (BYTE *)(scrline+640), (BYTE *)flrsel, pan);
			scrline-=offset;
			}
		offset *=2;
		}

	// Find lowest pt of sky
#define ST_DP 200
	ry[0]=-(st_l+ST_DP)*rot.e.r10+st_h*rot.e.r11+(st_w+ST_DP)*rot.e.r12+rot.e.r13;
	rz[0]=-(st_l+ST_DP)*rot.e.r20+st_h*rot.e.r21+(st_w+ST_DP)*rot.e.r22+rot.e.r23;
	ry[1]=(1280+st_l+ST_DP)*rot.e.r10+st_h*rot.e.r11+(st_w+ST_DP)*rot.e.r12+rot.e.r13;
	rz[1]=(1280+st_l+ST_DP)*rot.e.r20+st_h*rot.e.r21+(st_w+ST_DP)*rot.e.r22+rot.e.r23;
	ry[2]=(1280+st_l+ST_DP)*rot.e.r10+st_h*rot.e.r11-(800+st_w+ST_DP)*rot.e.r12+rot.e.r13;
	rz[2]=(1280+st_l+ST_DP)*rot.e.r20+st_h*rot.e.r21-(800+st_w+ST_DP)*rot.e.r22+rot.e.r23;
	ry[3]=-(st_l+ST_DP)*rot.e.r10+st_h*rot.e.r11-(800+st_w+ST_DP)*rot.e.r12+rot.e.r13;
	rz[3]=-(st_l+ST_DP)*rot.e.r20+st_h*rot.e.r21-(800+st_w+ST_DP)*rot.e.r22+rot.e.r23;

	if (rz[0]>0)
		ye=q*ry[0]/rz[0],ze=rz[0];
	else 
		ye=render_buff.clip_ymid,ze=0;
	if (rz[1]>ze) ye=q*ry[1]/rz[1],ze=rz[1];
	if (rz[2]>ze) ye=q*ry[2]/rz[2],ze=rz[2];
	if (rz[3]>ze) ye=q*ry[3]/rz[3],ze=rz[3];
	if (ye<-render_buff.clip_ymid) ye=-render_buff.clip_ymid;

	sa=atan((float)render_buff.clip_wid/q);
//	sw=3;//(int)(0.5+2*PI/sa);
	sa=2*PI*(1/3);

	if (sky && setup.detail.sky)
		{
		if ((yd=ye+q*sph/cph)<0) yd=0;
		if (479-yd<240-ye) yd=240+ye;
		if (sth<0)
			cpyline=sky+1280*(479-yd)-(((int)(1280*acos((double)cth)/sa)%1280)&0xfffffffe);
		else
			cpyline=sky+1280*(479-yd)+(((int)(1280*acos((double)cth)/sa)%1280)&0xfffffffe);

		y=240-ye-1;

		scrline=(unsigned short *)(buffer+y*offset);

		offset /= 2;
		while (y-->=0)
			{
			Mymemcpy( (BYTE *)scrline, cpyline, 1280);
			scrline-=offset;
			cpyline-=1280;
			}
		}
	else
		{
		y=240-ye-1;
		scrline=(unsigned short *)(buffer+y*offset);
		offset /= 2;
		while (y-->=0)
			{
			MyMemset( (BYTE *)scrline, 640, 0x01d901d9);
			scrline-=offset;
			}
		}
}

void RenderPitchSky( datapt viewx, datapt viewy, datapt viewz, datapt targx, datapt targy, datapt targz, BYTE *buffer, int width)
{
float hr, r;

	// Prepare viewing vector vars
	viewz=-viewz,targz=-targz;
	targx-=viewx;
	targy-=viewy;
	targz-=viewz;

	hr=targx*targx+targz*targz;
	if (hr<1) hr=1,targz=1;
	r=sqrt(hr+targy*targy);
	hr=sqrt(hr);
	cth=targz/hr;sth=targx/hr;
	cph=hr/r;sph=targy/r;
	icth=32767*cth; isth=32767*sth;

	RavePitchSky( hr, targy, viewx, viewy, -viewz, buffer, width);
}

#else

/*********************************************/
/* DRAW GROUND (SPECIALIZED TEXTURE MAPPING) */
/*********************************************/
/* DRAW GROUND (SPECIALIZED TEXTURE MAPPING) */
/* Joolz 1996				      */
asm void DrawTexture1(long w, BYTE *sc, BYTE *data, short pan)
{
	lwz	r3, td

	lwz	ESI, 6(r3)	// edx
	lha TEMP1, pan
	rlwimi ESI, TEMP1, 0, 16, 31

	lwz EDX, 14(r3)
	lwz TEMP1, 6(r3)
	rlwimi EDX, TEMP1, 0, 16, 31

	lhz EBX, 16(r3)

	lwz ECX, 18(r3)
	lhz TEMP1, 12(r3)
	rlwimi ECX, TEMP1, 0, 16, 31

	MOVCHBL
	ANDCXSI(TEMP1)
	
	lwz EBX, 10(r3)
	lwz TEMP1, 2(r3)
	rlwimi EBX, TEMP1, 0, 16, 31
	
	MOVBHDL
	
	li	TEMP1,0
	rlwimi EDX, TEMP1, 0, 16, 31
	
	lwz	DATA, data
	lwz	SCREEN, sc
//	addi	SCREEN, SCREEN, 4	// For Auto Update

	lwz		TEMP1, w
	srawi	TEMP1, TEMP1, 3
	mtctr	TEMP1
@lpp
	ANDBXSI(TEMP1)
	MOVXBX(EAX)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)

	ANDBXSI(TEMP1)
	MOVXBX(TEMP1)
	MOVAH(TEMP1)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)

	ANDBXSI(TEMP1)
	MOVXBX(TEMP1)
	MOVEAXL(TEMP1)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)

	ANDBXSI(TEMP1)
	MOVXBX(TEMP1)
	MOVEAXH(TEMP1)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)
			
	stwu	EAX, -4(SCREEN)		// save all 4 bytes at once
	
	ANDBXSI(TEMP1)
	MOVXBX(EAX)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)

	ANDBXSI(TEMP1)
	MOVXBX(TEMP1)
	MOVAH(TEMP1)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)

	ANDBXSI(TEMP1)
	MOVXBX(TEMP1)
	MOVEAXL(TEMP1)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)

	ANDBXSI(TEMP1)
	MOVXBX(TEMP1)
	MOVEAXH(TEMP1)
	ADDESIEDX
	ADCEBXECX
	ADCBHDH(TEMP1,TEMP2)
			
	stwu	EAX, -4(SCREEN)		// save all 4 bytes at once
	bdnz	@lpp
	blr
}

void ground(float vr,float vwy,datapt vx,datapt vy,datapt vz)
{
scrpt y,ys[4],ye,yd;
rotpt ry[4],rz[4],ze;
float t,d,z,tx,tz,dw1,dw2,sa;

BYTE *scr,*scrline,*cpyline;
char *flrsel;
word w,psh,pan;

	// static dword td[5]={0,0,0,0,0};

	// Find highest pt of pitch
	ry[0]=-st_l*rot.e.r10+st_w*rot.e.r12+rot.e.r13;
	rz[0]=-st_l*rot.e.r20+st_w*rot.e.r22+rot.e.r23;
	ry[1]=(1280+st_l)*rot.e.r10+st_w*rot.e.r12+rot.e.r13;
	rz[1]=(1280+st_l)*rot.e.r20+st_w*rot.e.r22+rot.e.r23;
	ry[2]=(1280+st_l)*rot.e.r10-(800+st_w)*rot.e.r12+rot.e.r13;
	rz[2]=(1280+st_l)*rot.e.r20-(800+st_w)*rot.e.r22+rot.e.r23;
	ry[3]=-st_l*rot.e.r10-(800+st_w)*rot.e.r12+rot.e.r13;
	rz[3]=-st_l*rot.e.r20-(800+st_w)*rot.e.r22+rot.e.r23;

	if (rz[0]>0)
		ye=q*ry[0]/rz[0],ze=rz[0];
	else 
		ye=-render_buff.clip_ymid,ze=0;
	if (rz[1]>ze) ye=q*ry[1]/rz[1],ze=rz[1];
	if (rz[2]>ze) ye=q*ry[2]/rz[2],ze=rz[2];
	if (rz[3]>ze) ye=q*ry[3]/rz[3],ze=rz[3];
	if (ye>render_buff.clip_ymid) ye=render_buff.clip_ymid;

	y=-render_buff.clip_ymid;
	t=q*sph+y*cph;						// Repeated divisor & incrementor

	scrline=render_buff.clip_end+render_buff.clip_wid;

	switch(setup.detail.pitch)
		{
		case 0:
				psh = 15;
				pan = 0x0f0f;
				flrsel = mapsel[mappages+noloop];
				break;
		case 1:
				psh=15;
				pan=0x1f1f;
				flrsel=mapsel[mappages+noloop];
				break;
		default:
				psh=16;
				pan=0x3f3f;
				flrsel=mapsel[mappages+noloop+1];
		}

	while (t<0 && y<ye)
		{				 
		d=vy/t;								// Depth (unscaled)
		z=d*(q*cph-y*sph);				// Floor depth
		tx=vx-z*sth;						// X pos of pt
		tz=-vz-z*cth;						// Z pos of pt

		dw2=-d*sth;
		dw1=d*cth;
		td[1]=(dword)((tz-dw2*render_buff.clip_xmid)*(1<<psh));
		td[2]=(dword)((tx-dw1*render_buff.clip_xmid)*(1<<psh));
		td[3]=(dword)(dw2*(1<<psh))&0xffffff;
		td[4]=(dword)(dw1*(1<<psh))&0xffffff;
		y++; t+=cph;
		DrawTexture1( render_buff.clip_wid, scrline, (BYTE *)flrsel, pan);
		scrline-=render_buff.buff_wid;
		}

			// Find lowest pt of sky
#define ST_DP 200
	ry[0]=-(st_l+ST_DP)*rot.e.r10+st_h*rot.e.r11+(st_w+ST_DP)*rot.e.r12+rot.e.r13;
	rz[0]=-(st_l+ST_DP)*rot.e.r20+st_h*rot.e.r21+(st_w+ST_DP)*rot.e.r22+rot.e.r23;
	ry[1]=(1280+st_l+ST_DP)*rot.e.r10+st_h*rot.e.r11+(st_w+ST_DP)*rot.e.r12+rot.e.r13;
	rz[1]=(1280+st_l+ST_DP)*rot.e.r20+st_h*rot.e.r21+(st_w+ST_DP)*rot.e.r22+rot.e.r23;
	ry[2]=(1280+st_l+ST_DP)*rot.e.r10+st_h*rot.e.r11-(800+st_w+ST_DP)*rot.e.r12+rot.e.r13;
	rz[2]=(1280+st_l+ST_DP)*rot.e.r20+st_h*rot.e.r21-(800+st_w+ST_DP)*rot.e.r22+rot.e.r23;
	ry[3]=-(st_l+ST_DP)*rot.e.r10+st_h*rot.e.r11-(800+st_w+ST_DP)*rot.e.r12+rot.e.r13;
	rz[3]=-(st_l+ST_DP)*rot.e.r20+st_h*rot.e.r21-(800+st_w+ST_DP)*rot.e.r22+rot.e.r23;

	if (rz[0]>0)
		ye=q*ry[0]/rz[0],ze=rz[0];
	else 
		ye=render_buff.clip_ymid,ze=0;
	if (rz[1]>ze) ye=q*ry[1]/rz[1],ze=rz[1];
	if (rz[2]>ze) ye=q*ry[2]/rz[2],ze=rz[2];
	if (rz[3]>ze) ye=q*ry[3]/rz[3],ze=rz[3];
	if (ye<-render_buff.clip_ymid) ye=-render_buff.clip_ymid;

	sa=atan((float)render_buff.clip_wid/q);
//	sw=3;//(int)(0.5+2*PI/sa);
	sa=2*PI*(1/3);

#if ALLOW_8MEG
	if (metup.M8 && setup.detail.sky)
#else
	if (sky && setup.detail.sky)
#endif
		{
		if ((yd=ye+q*sph/cph)<0) yd=0;
		if (479-yd<240-ye) yd=240+ye;
		if (sth<0)
			cpyline=sky+640*(480-yd)-(int)(640*acos((double)cth)/sa)%640;
		else
			cpyline=sky+640*(479-yd)+(int)(640*acos((double)cth)/sa)%640;

		y=render_buff.clip_ymid-ye-1;

		scrline=render_buff.buff_start+y*render_buff.buff_wid;
		while (y-->=0)
			{
			memcpy(scrline,cpyline,render_buff.clip_wid);
			scrline-=render_buff.buff_wid;
			cpyline-=640;
			}
		}
	else
		{
		y=render_buff.clip_ymid-ye-1;

		scrline=render_buff.buff_start+y*render_buff.buff_wid;
		while (y-->=0)
			{
			memset(scrline,204,render_buff.clip_wid);
			scrline-=render_buff.buff_wid;
			}
		}
}
#endif

/******************************/
/* GET OBJECT DEPTH STRUCTURE */

// Macro for objects with no detail levels
#define objdep(ob) ((ob).x*rot.e.r20+(ob).y*rot.e.r21+(ob).z*rot.e.r22+rot.e.r23)

// Routine for objects with detail levels
float objdepd(obj *ob, det *detail)
	{
	rotpt oz,zq;
	if ((oz=ob->x*rot.e.r20+ob->y*rot.e.r21+ob->z*rot.e.r22+rot.e.r23)<0)
		zq=0;
	else
		zq=oz/q;
	while (detail->zscale>zq) detail++;
	ob->points=detail->dpoints;
	ob->faces=detail->dfaces;
	return oz;
	}



/*************************************/
/* ADD ROTATED FACES TO POLYGON LIST */

// Add ordinary polygon to list
void addpoly(face obf,word np,word col)
	{
	word p;
	rotpt dx1,dy1,dx2,dy2,tz1,tz2;
	rpt tpt1,tpt2,tpt3;

	// Unroll first three face points in subsequent code
	tpt1=&rotlist[obf[0]<<2];
	tpt2=&rotlist[obf[1]<<2];
	tpt3=&rotlist[obf[2]<<2];

	// Find min z
	tz2=tpt1[0];
	if (tpt2[0]<tz2) tz2=tpt2[0];
	if (tpt3[0]<tz2) tz2=tpt3[0];
	for (p=3;p<np;p++)
		if ((tz1=rotlist[obf[p]<<2])<tz2) tz2=tz1;

	// If visible...
	if (tz2>=CUTOFFDIST)
		{
		// Facing check
		dx1=dx2=tpt1[1];
		dy1=dy2=tpt1[2];
		dx1-=tpt2[1];
		dy1-=tpt2[2];
		dx2-=tpt3[1];
		dy2-=tpt3[2];

		if (dx1*dy2<dx2*dy1)
			{
			// Transfer rotated points to polygon list
			// and calculate average z
			curpol->pts=curpts;
			curpts[0]=np;
			curpts[1]=col;

/*
if (col>=256)
	{
 	tz1=tpt1[0];
	curpts[2]=max(tz1,CUTOFFDIST);
	curpts[3]=tpt1[1];
	curpts[4]=tpt1[2];
	tz1+=tpt2[0];
	curpts[5]=max(tpt2[0],CUTOFFDIST);
	curpts[6]=tpt2[1];
	curpts[7]=tpt2[2];
	tz1+=tpt3[0];
	curpts[8]=max(tpt3[0],CUTOFFDIST);
	curpts[9]=tpt3[1];
	curpts[10]=tpt3[2];
	curpts+=11;
	for (p=3;p<np;p++)
		{
		tpt1=&rotlist[obf[p]<<2];
		tz1+=tpt1[0];
		curpts[0]=max(tpt1[0],CUTOFFDIST);
  		curpts[1]=tpt1[1];
		curpts[2]=tpt1[2];
		curpts+=3;
		}
	}
else
	{
	}
*/
			tz1=tpt1[0];
			curpts[2]=tpt1[1];
			curpts[3]=tpt1[2];
			tz1+=tpt2[0];
			curpts[4]=tpt2[1];
			curpts[5]=tpt2[2];
			tz1+=tpt3[0];
			curpts[6]=tpt3[1];
			curpts[7]=tpt3[2];
			curpts+=8;
			for (p=3;p<np;p++)
				{
				tpt1=&rotlist[obf[p]<<2];
				tz1+=tpt1[0];
				curpts[0]=tpt1[1];
				curpts[1]=tpt1[2];
				curpts+=2;
				}
			curpol->z=tz1/np;
			curpol++;
			}
		}
	}

	
// Add line to list
void
addline(face obf,word col)
	{
	rotpt tz;
	rpt tpt1,tpt2;

	// Find min z
	tpt1=&rotlist[obf[0]<<2];
	tpt2=&rotlist[obf[1]<<2];
	tz=min(tpt1[0],tpt2[0]); 

	// If visible...
	if (tz>=CUTOFFDIST)
		{
		// Transfer rotated line points to polygon
		// list and calculate average z
		curpol->pts=curpts;
		curpol->z=(tpt1[0]+tpt2[0])/2;
		curpts[0]=2;
		curpts[1]=col;
		curpts[2]=tpt1[1];
		curpts[3]=tpt1[2];
		curpts[4]=tpt2[1];
		curpts[5]=tpt2[2];
		curpts+=6;
  		curpol++;		          
		}
	}


// Add 3d cylinder map to list
void
add3dcmap(face obf,word col)
	{
	rotpt dx1,dy1,dx2,dy2,tz1,tz2;
	rpt tpt1,tpt2,tpt3;

	// Find min z
	tpt1=&rotlist[obf[0]<<2];
	tpt2=&rotlist[obf[1]<<2];
	tz1=min(tpt1[0],tpt2[0]); 

	// If visible...
	if (tz1>=CUTOFFDIST)
		{
		curpts[0]=4;
		curpol->pts=curpts;
		curpol->z=(tpt1[0]+tpt2[0])/2;

		// Calculate bounding rectangle for cylinder texture map

		/*
		tpt1[0]=0;
		tpt1[1]=0;
		tpt1[2]=0;
		tpt2[0]=0;
		tpt2[1]=0;
		tpt2[2]=0;
		obf[2]=0;
		obf[3]=0;
		*/

		dx1=tpt1[1]-tpt2[1];
		dy1=tpt1[2]-tpt2[2];
		if ((dx2=dx1*dx1+dy1*dy1)==0) dx1=dx2=1;
		tz2=obf[2]*qa/(sqrt(dx2)*(tpt1[0]+tpt2[0]));
		tz1=100*(tpt2[0]-tpt1[0])/obf[3];
		if (tz1>=0)
			{				   
			if (tz1>0.78062475)
				if (tz1>0.92702481) col-=3;
				else col-=2;
			else
				if (tz1>0.48412292) col--;
//			if (tz1>0.78062475)
//				if (tz1>0.92702481) col-=3;
//				else col-=2;
//			else
//				if (tz1>0.48412292) col--;
//			col-=(int)((1-sqrt(1-tz1*tz1))*4.3);
			tz1*=tz2;
			dy2=tz2*dy1;
			dx2=tz2*dx1;
			dy1*=tz1;
			dx1*=tz1;
			curpts[1]=col;
			curpts[2]=tpt1[1]+dx1-dy2;
			curpts[3]=tpt1[2]+dy1+dx2;
			curpts[4]=tpt1[1]+dx1+dy2;
			curpts[5]=tpt1[2]+dy1-dx2;
			curpts[6]=tpt2[1]-dx1+dy2;
			curpts[7]=tpt2[2]-dy1-dx2;
			curpts[8]=tpt2[1]-dx1-dy2;
			curpts[9]=tpt2[2]-dy1+dx2;
			}
		else
			{
			if (tz1<-0.78062475)
				if (tz1<-0.92702481) col+=3;
				else col+=2;
			else
				if (tz1<-0.48412292) col++;
//			col+=(int)((1-sqrt(1-tz1*tz1))*4.3);
			tz1*=tz2;
			dy2=tz2*dy1;
			dx2=tz2*dx1;
			dy1*=tz1;
			dx1*=tz1;
			curpts[1]=col;
			curpts[2]=tpt1[1]-dx1-dy2;
			curpts[3]=tpt1[2]-dy1+dx2;
			curpts[4]=tpt1[1]-dx1+dy2;
			curpts[5]=tpt1[2]-dy1-dx2;
			curpts[6]=tpt2[1]+dx1+dy2;
			curpts[7]=tpt2[2]+dy1-dx2;
			curpts[8]=tpt2[1]+dx1-dy2;
			curpts[9]=tpt2[2]+dy1+dx2;
			}
		curpts+=10;
  		curpol++;		          
		}
	}


// Add 3d eliptical cylinder map to list
void
add3demap(face obf,word col)
	{
	rotpt dx1,dy1,dx2,dy2,tz1,tz2,rx,ry,rz;
	rpt tpt1,tpt2,tpt3;

	// Find min z
	tpt1=&rotlist[obf[0]<<2];
	tpt2=&rotlist[obf[1]<<2];
	tz1=min(tpt1[0],tpt2[0]); 

	// If visible...
	if (tz1>=CUTOFFDIST)
		{
		curpts[0]=4;
		curpol->pts=curpts;
		curpol->z=(tpt1[0]+tpt2[0])/2;

		// Calculate bounding rectangle for elliptical cylinder texture map
//		tpt3=&rotlist[obf[2]<<2];
//		dx1=tpt1[1]-tpt2[1];
//		dy1=tpt1[2]-tpt2[2];
//		rx=((tpt3[1]-render_buff.clip_xmid)*tpt3[0]-(tpt2[1]-render_buff.clip_xmid)*tpt2[0])/q;
//		ry=((tpt3[2]-render_buff.clip_ymid)*tpt3[0]-(tpt2[2]-render_buff.clip_ymid)*tpt2[0])/q;
//		if ((dx2=dx1*dx1+dy1*dy1)<2) return;
//		dx2=sqrt(dx2);
//		dy2=100*(tpt1[0]-tpt2[0])/obf[5];
//		tz1=(rx*dy1-ry*dx1)/dx2;
//		tz2=qa/(dx2*(tpt1[0]+tpt2[0]));

		tpt3=&rotlist[obf[2]<<2];
		dx1=tpt1[1]-tpt2[1];
		dy1=tpt1[2]-tpt2[2];
		rx=(tpt3[1]-tpt2[1])*tpt3[0]/q;
		ry=(tpt3[2]-tpt2[2])*tpt3[0]/q;
		if ((dx2=dx1*dx1+dy1*dy1)==0) dx1=dx2=1;
		dy2=sqrt(dx2);
		tz2=qa/(dy2*(tpt1[0]+tpt2[0]));

		tz1=(rx*dy1-ry*dx1)/dy2;

if (tz1<-1) tz1=-1;
if (tz1>1) tz1=1;

		if (tpt3[0]>tpt2[0])
//		if (rx*dx1<ry*dy1)
			if (tz1>-.2588)
				if (tz1<=.7071)
					if (tz1<=.2588)
						col-=9;
					else
						col-=10;
				else
					{
					if (tz1<=.9659)
						col-=11;
					}
			else				
				if (tz1>-.7071)
					col-=8;
				else
					if (tz1>-.9659)
						col-=7;
					else
						col-=6;
		else
			if (tz1<=.2588)
				if (tz1>-.7071)
					if (tz1>-.2588)
						col-=3;
					else
						col-=4;
				else
					if (tz1>-.9659)
						col-=5;
					else
						col-=6;
			else
				if (tz1<=.7071)
					col-=2;
				else
					if (tz1<=.9659)
						col-=1;

//		tz1*=tz1;
//		tz1*=(obf[3]-obf[4])*(obf[3]+obf[4]);
//		dx2=tz2*sqrt(tz1+obf[4]*obf[4]);

		tz1*=tz1;
		tz1*=(obf[3]-obf[4])*(obf[3]+obf[4]);
		dx2=tz2*sqrt(tz1+obf[4]*obf[4]);
		dy2=100*(tpt1[0]-tpt2[0])/obf[5];

		if (dy2>=0)
			{				   
			if (dy2>0.555570233)
				if (dy2>0.836286155) col-=36;
				else col-=24;
			else
				if (dy2>0.195090322) col-=12;
			dy2*=tz2*sqrt(obf[3]*obf[3]-tz1);
			tz1=dy2*dx1;
			tz2=dy2*dy1;
			dx1*=dx2;
			dy1*=dx2;
			curpts[1]=col;
			curpts[2]=tpt2[1]+dy1-tz1;
			curpts[3]=tpt2[2]-dx1-tz2;
			curpts[4]=tpt2[1]-dy1-tz1;
			curpts[5]=tpt2[2]+dx1-tz2;
			curpts[6]=tpt1[1]-dy1+tz1;
			curpts[7]=tpt1[2]+dx1+tz2;
			curpts[8]=tpt1[1]+dy1+tz1;
			curpts[9]=tpt1[2]-dx1+tz2;
			}
		else
			{
			if (dy2<-0.258819045)
				if (dy2<-0.707106781) col+=12;
				else col+=12;
			dy2*=tz2*sqrt(obf[3]*obf[3]-tz1);
			tz1=dy2*dx1;
			tz2=dy2*dy1;
			dx1*=dx2;
			dy1*=dx2;
			curpts[1]=col;
			curpts[2]=tpt2[1]+dy1+tz1;
			curpts[3]=tpt2[2]-dx1+tz2;
			curpts[4]=tpt2[1]-dy1+tz1;
			curpts[5]=tpt2[2]+dx1+tz2;
			curpts[6]=tpt1[1]-dy1-tz1;
			curpts[7]=tpt1[2]+dx1-tz2;
			curpts[8]=tpt1[1]+dy1-tz1;
			curpts[9]=tpt1[2]-dx1-tz2;
			}
		curpts+=10;
  		curpol++;		          
		}
	}

// Add polygon to list, with clipping, (note: optimised for unclipped objects)
void
addpolyc(face obf,word np,word col)
	{
	word p,pc,tp,i;
	dword *texx,*texy,*texs;
	rotpt dx1,dy1,dx2,dy2,tz1,tz2,qrz,qrz2;
	rpt tpt1,tpt2,tpt3;

	// Unroll first three face points in subsequent code
	tpt1=&rotlist[obf[0]<<2];
	tpt2=&rotlist[obf[1]<<2];
	tpt3=&rotlist[obf[2]<<2];

	// If any of first 3 pts need clipping, facing check is invalid, so go to clipping code now
	if ((tz1=tpt1[0])<SCREENDIST||tpt2[0]<SCREENDIST||tpt3[0]<SCREENDIST)
		{
		p=0;tz1=0;
		curpol->pts=curpts;
		curpts[0]=np;
		curpts[1]=col;
		curpts+=2;
		goto pclip;
		}

	// Facing check
	dx1=dx2=tpt1[1];
	dy1=dy2=tpt1[2];
	dx1-=tpt2[1];
	dy1-=tpt2[2];
	dx2-=tpt3[1];
	dy2-=tpt3[2];
	if (dx1*dy2<dx2*dy1)
		{
		// Transfer rotated points to polygon list and 
		// calculate average z. Go to clipping code if necessary
		curpol->pts=curpts;
		curpts[0]=np;
		curpts[1]=col;
		curpts[2]=tpt1[1];
		curpts[3]=tpt1[2];
		curpts[4]=tpt2[1];
		curpts[5]=tpt2[2];
		tz1+=tpt2[0];
		curpts[6]=tpt3[1];
		curpts[7]=tpt3[2];
		tz1+=tpt3[0];
		curpts+=8;
		for (p=3;p<np;p++)
			{
			tpt1=&rotlist[obf[p]<<2];
			if ((tz2=tpt1[0])<SCREENDIST) goto pclip; 
			tz1+=tz2;
			curpts[0]=tpt1[1];
			curpts[1]=tpt1[2];
			curpts+=2;
			}
		curpol->z=tz1/np;
		curpol++;
		}
	return;

	// Polygon needs clipping 
	pclip:;
	if (col<0)
		{
		// Texture map...
		if (col<-2000)
			{
			texs=(dword *)&textures[-col-2000];
			col=-2000-clip_tex;
			}
		else
			{
			texs=(dword *)&textures[-col];
			col=-clip_tex;
			}
		*(curpol->pts+1)=col;
		texy=(dword *)&textures[clip_tex];
		texx=(dword *)&textures[++clip_tex];
		for (i=0;i<p;i++)
			texy[i]=texs[i];
		for (i=0;i<p;i++)
			texx[i]=texs[np+i];

		tp=(p+np-1)%np;
		tpt2=&rotlist[obf[tp]<<2];
		pc=p;
		do
			{
			tz1+=tpt1[0];
			if (tpt1[0]>=SCREENDIST)
				{
				if (tpt2[0]<SCREENDIST)
					{
					qrz=(SCREENDIST-tpt2[0])/(tpt1[0]-tpt2[0]);
					qrz2=tpt1[0]/SCREENDIST;
					curpts[0]=tpt2[1]+qrz*((tpt1[1]-render_buff.clip_xmid)*qrz2-tpt2[1])+render_buff.clip_xmid;
					curpts[1]=tpt2[2]+qrz*((tpt1[2]-render_buff.clip_ymid)*qrz2-tpt2[2])+render_buff.clip_ymid;
					curpts+=2;

					texy[pc]=texs[tp]+qrz*(texs[p]-texs[tp]);
					texx[pc]=texs[tp+np]+qrz*(texs[p+np]-texs[tp+np]);

					if (++pc==3)
						{
						dx1=dx2=curpts[-6];
						dy1=dy2=curpts[-5];
						dx1-=curpts[-4];
						dy1-=curpts[-3];
						dx2-=curpts[-2];
						dy2-=curpts[-1];
						if (dx1*dy2>=dx2*dy1)
							{
							curpts=curpol->pts;
							return;
							}
						}
					}
				curpts[0]=tpt1[1];
				curpts[1]=tpt1[2];
				curpts+=2;

				texy[pc]=texs[p];
				texx[pc]=texs[p+np];

				if (++pc==3)
					{
					dx1=dx2=curpts[-6];
					dy1=dy2=curpts[-5];
					dx1-=curpts[-4];
					dy1-=curpts[-3];
					dx2-=curpts[-2];
					dy2-=curpts[-1];
					if (dx1*dy2>=dx2*dy1)
						{
						curpts=curpol->pts;
						return;
						}
					}
				}
			else
				if (tpt2[0]>=SCREENDIST)
					{
					qrz=(SCREENDIST-tpt1[0])/(tpt2[0]-tpt1[0]);
					qrz2=tpt2[0]/SCREENDIST;
					curpts[0]=tpt1[1]+qrz*((tpt2[1]-render_buff.clip_xmid)*qrz2-tpt1[1])+render_buff.clip_xmid;
					curpts[1]=tpt1[2]+qrz*((tpt2[2]-render_buff.clip_ymid)*qrz2-tpt1[2])+render_buff.clip_ymid;
					curpts+=2;

					texy[pc]=texs[p]+qrz*(texs[tp]-texs[p]);
					texx[pc]=texs[p+np]+qrz*(texs[tp+np]-texs[p+np]);

					if (++pc==3)
						{
						dx1=dx2=curpts[-6];
						dy1=dy2=curpts[-5];
						dx1-=curpts[-4];
						dy1-=curpts[-3];
						dx2-=curpts[-2];
						dy2-=curpts[-1];
						if (dx1*dy2>=dx2*dy1)
							{
							curpts=curpol->pts;
							return;
							}
						}
					}
			tpt2=tpt1;
			tp=p;
			tpt1=&rotlist[obf[(++p)%np]<<2];
			}while (p<np);
		p%=np;
		if (tpt1[0]>=SCREENDIST && tpt2[0]<SCREENDIST)
			{
			qrz=(SCREENDIST-tpt2[0])/(tpt1[0]-tpt2[0]);
			qrz2=tpt1[0]/SCREENDIST;
			curpts[0]=tpt2[1]+qrz*((tpt1[1]-render_buff.clip_xmid)*qrz2-tpt2[1])+render_buff.clip_xmid;
			curpts[1]=tpt2[2]+qrz*((tpt1[2]-render_buff.clip_ymid)*qrz2-tpt2[2])+render_buff.clip_ymid;
			curpts+=2;

			texy[pc]=texs[tp]+qrz*(texs[p]-texs[tp]);
			texx[pc]=texs[tp+np]+qrz*(texs[p+np]-texs[tp+np]);

			if (++pc==3)
				{
				dx1=dx2=curpts[-6];
				dy1=dy2=curpts[-5];
				dx1-=curpts[-4];
				dy1-=curpts[-3];
				dx2-=curpts[-2];
				dy2-=curpts[-1];
				if (dx1*dy2>=dx2*dy1)
					{
					curpts=curpol->pts;
					return;
					}
				}
			}

		for (i=0;i<pc;i++)
			texy[pc+i]=texx[i];
		if (pc>4) clip_tex++;

		}
	else
		{
		tpt2=&rotlist[obf[(p+np-1)%np]<<2];
		pc=p;
		do
			{
			tz1+=tpt1[0];
			if (tpt1[0]>=SCREENDIST)
				{
				if (tpt2[0]<SCREENDIST)
					{
					qrz=(SCREENDIST-tpt2[0])/(tpt1[0]-tpt2[0]);
					qrz2=tpt1[0]/SCREENDIST;
					curpts[0]=tpt2[1]+qrz*((tpt1[1]-render_buff.clip_xmid)*qrz2-tpt2[1])+render_buff.clip_xmid;
					curpts[1]=tpt2[2]+qrz*((tpt1[2]-render_buff.clip_ymid)*qrz2-tpt2[2])+render_buff.clip_ymid;
					curpts+=2;
					if (++pc==3)
						{
						dx1=dx2=curpts[-6];
						dy1=dy2=curpts[-5];
						dx1-=curpts[-4];
						dy1-=curpts[-3];
						dx2-=curpts[-2];
						dy2-=curpts[-1];
						if (dx1*dy2>=dx2*dy1)
							{
							curpts=curpol->pts;
							return;
							}
						}
					}
				curpts[0]=tpt1[1];
				curpts[1]=tpt1[2];
				curpts+=2;
				if (++pc==3)
					{
					dx1=dx2=curpts[-6];
					dy1=dy2=curpts[-5];
					dx1-=curpts[-4];
					dy1-=curpts[-3];
					dx2-=curpts[-2];
					dy2-=curpts[-1];
					if (dx1*dy2>=dx2*dy1)
						{
						curpts=curpol->pts;
						return;
						}
					}
				}
			else
				if (tpt2[0]>=SCREENDIST)
					{
					qrz=(SCREENDIST-tpt1[0])/(tpt2[0]-tpt1[0]);
					qrz2=tpt2[0]/SCREENDIST;
					curpts[0]=tpt1[1]+qrz*((tpt2[1]-render_buff.clip_xmid)*qrz2-tpt1[1])+render_buff.clip_xmid;
					curpts[1]=tpt1[2]+qrz*((tpt2[2]-render_buff.clip_ymid)*qrz2-tpt1[2])+render_buff.clip_ymid;
					curpts+=2;
					if (++pc==3)
						{
						dx1=dx2=curpts[-6];
						dy1=dy2=curpts[-5];
						dx1-=curpts[-4];
						dy1-=curpts[-3];
						dx2-=curpts[-2];
						dy2-=curpts[-1];
						if (dx1*dy2>=dx2*dy1)
							{
							curpts=curpol->pts;
							return;
							}
						}
					}
			tpt2=tpt1;
			tpt1=&rotlist[obf[(++p)%np]<<2];
			}while (p<np);
		if (tpt1[0]>=SCREENDIST && tpt2[0]<SCREENDIST)
			{
			qrz=(SCREENDIST-tpt2[0])/(tpt1[0]-tpt2[0]);
			qrz2=tpt1[0]/SCREENDIST;
			curpts[0]=tpt2[1]+qrz*((tpt1[1]-render_buff.clip_xmid)*qrz2-tpt2[1])+render_buff.clip_xmid;
			curpts[1]=tpt2[2]+qrz*((tpt1[2]-render_buff.clip_ymid)*qrz2-tpt2[2])+render_buff.clip_ymid;
			curpts+=2;
			if (++pc==3)
				{
				dx1=dx2=curpts[-6];
				dy1=dy2=curpts[-5];
				dx1-=curpts[-4];
				dy1-=curpts[-3];
				dx2-=curpts[-2];
				dy2-=curpts[-1];
				if (dx1*dy2>=dx2*dy1)
					{
					curpts=curpol->pts;
					return;
					}
				}
			}
		}
   // All pts clipped?
	if (pc==0)
		curpts=curpol->pts;
	else
		{
		*(curpol->pts)=pc;
		curpol->z=tz1/np;
		curpol++;
		}
 	}


// Add line to list, with clipping
void
addlinec(face obf,word col)
	{
	rotpt qrz,qrz2;
	rpt tpt1,tpt2;

	// Transfer rotated (/clipped) points to polygon list
	// and calculate average z
	tpt1=&rotlist[obf[0]<<2];
	tpt2=&rotlist[obf[1]<<2];
	if (tpt1[0]<SCREENDIST)
		{
		if (tpt2[0]<SCREENDIST) return;
		qrz=(SCREENDIST-tpt1[0])/(tpt2[0]-tpt1[0]);
		qrz2=tpt2[0]/SCREENDIST;
		curpts[2]=tpt1[1]+qrz*((tpt2[1]-render_buff.clip_xmid)*qrz2-tpt1[1])+render_buff.clip_xmid;
		curpts[3]=tpt1[2]+qrz*((tpt2[2]-render_buff.clip_ymid)*qrz2-tpt1[2])+render_buff.clip_ymid;
		}
	else
		{
		curpts[2]=tpt1[1];
		curpts[3]=tpt1[2];
		}
	if (tpt2[0]<SCREENDIST)
		{
		qrz=(SCREENDIST-tpt2[0])/(tpt1[0]-tpt2[0]);
		qrz2=tpt1[0]/SCREENDIST;
		curpts[4]=tpt2[1]+qrz*((tpt1[1]-render_buff.clip_xmid)*qrz2-tpt2[1])+render_buff.clip_xmid;
		curpts[5]=tpt2[2]+qrz*((tpt1[2]-render_buff.clip_ymid)*qrz2-tpt2[2])+render_buff.clip_ymid;
		}
	else
		{
		curpts[4]=tpt2[1];
		curpts[5]=tpt2[2];
		}
	curpol->pts=curpts;
	curpts[0]=2;
	curpts[1]=col;
	curpts+=6;
	curpol->z=(tpt1[0]+tpt2[0])/2;
	curpol++;
	}


// Add faces to polygon list, with no z clipping
void addpols(obj *ob)
	{
	face obf=ob->faces;
	word f=*obf++;
	word np,col;

#ifdef COUNT
	numpols+=f;
#endif

	while(f--)
		{
		np=obf[0];
		col=obf[1];
		obf+=2;

		if (np>2)
			{
			addpoly(obf, np, col); 
			obf+=np;
			}
		else
			if (np==2)
				{
				addline(obf, col);
				obf+=2;
				}
			else
			 	if (np==0)
					{
					add3dcmap(obf, col);
					obf+=4;
					}
				else
					{
					add3demap(obf, col);
					obf+=6;
					}
		}
	}


// Add faces to polygon list, with z clipping
void addpolsc(obj *ob)
	{
	face obf=ob->faces;
	word f=*obf++;	
	word np,col;

#ifdef COUNT
	numpols+=f;
#endif

	while(f--)
		{
		np=obf[0];
		col=obf[1];
		obf+=2;

		if (np>2) // Deal with polygons...
			{
			addpolyc(obf,np,col);
			obf+=np;
			}
		else	
			{
			addlinec(obf,col);
			obf+=2;
			}
		}
	}



/******************************************/
/* ROTATE OBJECTS AND ADD TO POLYGON LIST */

// Several routines here, for various cases:
// addobjxy	: objects with x & y axis rotation and no z-clipping
// addobjy	: objects with y-axis rotation only and no z-clipping
// addobjyc	: objects with y-axis rotation only with z-clipping
// addobjn	: objects with no rotation and no z-clipping
// addobjnc	: objects with no rotation with z-clipping
// addobjf	: objects with no rotation, all y=0, and no z-clipping
// addobjfc	: objects with no rotation, all y=0, with z-clipping


// Routine for objects with x & y axis rotation, no z-clipping
void addobjxy(obj *ob,floatpt cxrot,floatpt sxrot)
	{
	pt inpt=ob->points;
	rpt outpt=rotlist;
	word p=(word)*inpt++; // No of pts currently stored as datapt type!
	datapt x,y,z;
	floatpt crot,srot;
	rotpt	rx,ry,rz,qrz;

	// Get object depth
	if ((curobj->z=objdep(*ob))>CUTOFFDIST)
	{
	curobj->polys=curpol;

	// Transform matrix derived from view rotation and object rotation
	// Assumes r01=0 in view rotation
	crot=ob->crot;
	srot=ob->srot;
	x=ob->x;		  
	y=ob->y;
	z=ob->z;
/*
	tran.e.r00=rot.e.r00*crot-rot.e.r02*srot*cxrot; // Rotate in x then y
	tran.e.r01=rot.e.r02*sxrot;
	tran.e.r02=rot.e.r00*srot+rot.e.r02*crot*cxrot;
	tran.e.r03=rot.e.r00*x+rot.e.r01*y+rot.e.r02*z+rot.e.r03;
	tran.e.r10=rot.e.r10*crot-rot.e.r11*srot*sxrot-rot.e.r12*srot*cxrot;
	tran.e.r11=rot.e.r11*cxrot+rot.e.r12*sxrot;
	tran.e.r12=rot.e.r10*srot-rot.e.r11*crot*sxrot+rot.e.r12*crot*cxrot;
	tran.e.r13=rot.e.r10*x+rot.e.r11*y+rot.e.r12*z+rot.e.r13;
	tran.e.r20=rot.e.r20*crot-rot.e.r21*srot*sxrot-rot.e.r22*srot*cxrot;
	tran.e.r21=rot.e.r21*cxrot+rot.e.r22*sxrot;
	tran.e.r22=rot.e.r20*srot-rot.e.r21*crot*sxrot+rot.e.r22*crot*cxrot;
	tran.e.r23=rot.e.r20*x+rot.e.r21*y+rot.e.r22*z+rot.e.r23;
*/
	tran.e.r00= rot.e.r00*crot-rot.e.r02*srot;		// Rotate in y then x
	tran.e.r01=-rot.e.r00*srot*sxrot-rot.e.r02*crot*sxrot;
	tran.e.r02= rot.e.r00*srot*cxrot+rot.e.r02*crot*cxrot;
	tran.e.r03= rot.e.r00*x+rot.e.r01*y+rot.e.r02*z+rot.e.r03;
	tran.e.r10= rot.e.r10*crot-rot.e.r12*srot;
	tran.e.r11=-rot.e.r10*srot*sxrot+rot.e.r11*cxrot-rot.e.r12*crot*sxrot;
	tran.e.r12= rot.e.r10*srot*cxrot+rot.e.r11*sxrot+rot.e.r12*crot*cxrot;
	tran.e.r13= rot.e.r10*x+rot.e.r11*y+rot.e.r12*z+rot.e.r13;
	tran.e.r20= rot.e.r20*crot-rot.e.r22*srot;
	tran.e.r21=-rot.e.r20*srot*sxrot+rot.e.r21*cxrot-rot.e.r22*crot*sxrot;
	tran.e.r22= rot.e.r20*srot*cxrot+rot.e.r21*sxrot+rot.e.r22*crot*cxrot;
	tran.e.r23= rot.e.r20*x+rot.e.r21*y+rot.e.r22*z+rot.e.r23;

	// Rotate all points of object
	for (;p;p--)
		{
		x=inpt[0];
		y=inpt[1];
		z=inpt[2];
		inpt+=3;
		rx=x*tran.e.r00+y*tran.e.r01+z*tran.e.r02+tran.e.r03;
		ry=x*tran.e.r10+y*tran.e.r11+z*tran.e.r12+tran.e.r13;
		rz=x*tran.e.r20+y*tran.e.r21+z*tran.e.r22+tran.e.r23;

		// Get perspective scaling of point
		if (rz<SCREENDIST)
			{
			qrz=1.5-rz*scrdr; 	// Alternative perspective scaling...
			qrz*=qrz; 		 		//	mimics proper perspective for small...	
			qrz=persc*(qrz+.75);	// and negative z values.
			}
		else
			qrz=q/rz;	// Normal scaling

		//Add screen points to output list (with z values)
		outpt[0]=rz;
		outpt[1]=rx*qrz+render_buff.clip_xmid;
		outpt[2]=ry*qrz+render_buff.clip_ymid;
		outpt+=4;	// add 4 so indexing can be done with shifts
		}
	addpols(ob);
	curobj->nopols=curpol-curobj->polys;
	curobj++;
	}
	}


// Routine for objects with y axis rotation, no z-clipping
void addobjy(obj *ob)
	{
	pt inpt=ob->points;
	rpt outpt=rotlist;
	word p=(word)*inpt++; // No of pts currently stored as datapt type!
	datapt x,y,z;
	floatpt crot,srot;
	rotpt	rx,ry,rz,qrz;


	// Get object depth
	if ((curobj->z=objdep(*ob))>CUTOFFDIST)
	{
	curobj->polys=curpol;

	// Transform matrix derived from view rotation and object rotation
	// Assumes r01=0 in view rotation and that r11 & r21 remain the same
	crot=ob->crot;
	srot=ob->srot;
	x=ob->x;
	y=ob->y;
	z=ob->z;
	tran.e.r00=rot.e.r00*crot-rot.e.r02*srot;
	tran.e.r02=rot.e.r00*srot+rot.e.r02*crot;
	tran.e.r03=rot.e.r00*x+rot.e.r01*y+rot.e.r02*z+rot.e.r03;
	tran.e.r10=rot.e.r10*crot-rot.e.r12*srot;
	tran.e.r12=rot.e.r10*srot+rot.e.r12*crot;
	tran.e.r13=rot.e.r10*x+rot.e.r11*y+rot.e.r12*z+rot.e.r13;
	tran.e.r20=rot.e.r20*crot-rot.e.r22*srot;
	tran.e.r22=rot.e.r20*srot+rot.e.r22*crot;
	tran.e.r23=rot.e.r20*x+rot.e.r21*y+rot.e.r22*z+rot.e.r23;

	// Rotate all points of object
	for (;p;p--)
		{
		x=inpt[0];
		y=inpt[1];
		z=inpt[2];
		inpt+=3;
		rx=x*tran.e.r00+z*tran.e.r02+tran.e.r03; // r01 is 0
		ry=x*tran.e.r10+y*rot.e.r11+z*tran.e.r12+tran.e.r13;
		rz=x*tran.e.r20+y*rot.e.r21+z*tran.e.r22+tran.e.r23;

		// Get perspective scaling of point
		if (rz<SCREENDIST)
			{
			qrz=1.5-rz*scrdr; 	// Alternative perspective scaling...
			qrz*=qrz; 		 		//	mimics proper perspective for small...	
			qrz=persc*(qrz+.75);	// and negative z values.
/*
			qrz=2-rz*scrdr;		// Improved alternative perspective..
			qrz=qrz*qrz*qrz;	   // slightly higher complexity.
			qrz=persc*(qrz+2);
			qrz/=3;
*/			}
		else
			qrz=q/rz;	// Normal scaling

		//Add screen points to output list (with z values)
		outpt[0]=rz;
		outpt[1]=rx*qrz+render_buff.clip_xmid;
		outpt[2]=ry*qrz+render_buff.clip_ymid;
		outpt+=4;	// add 4 so indexing can be done with shifts
		}
	addpols(ob);
	curobj->nopols=curpol-curobj->polys;
	curobj++;
	}
	}

// Routine for objects with y axis rotation, no z-clipping, z flipped (only players)
void addobjyf(obj *ob)
	{
	pt inpt=ob->points;
	rpt outpt=rotlist;
	word p=(word)*inpt++; // No of pts currently stored as datapt type!
	datapt x,y,z;
	floatpt crot,srot;
	rotpt	rx,ry,rz,qrz;


	// Get object depth
	if ((curobj->z=objdep(*ob))>CUTOFFDIST)
	{
	curobj->polys=curpol;

	// Transform matrix derived from view rotation and object rotation
	// Assumes r01=0 in view rotation and that r11 & r21 remain the same
	crot=ob->crot;
	srot=ob->srot;
	x=ob->x;
	y=ob->y;
	z=ob->z;
	tran.e.r00=rot.e.r00*crot-rot.e.r02*srot;
	tran.e.r02=rot.e.r00*srot+rot.e.r02*crot;
	tran.e.r03=rot.e.r00*x+rot.e.r01*y+rot.e.r02*z+rot.e.r03;
	tran.e.r10=rot.e.r10*crot-rot.e.r12*srot;
	tran.e.r12=rot.e.r10*srot+rot.e.r12*crot;
	tran.e.r13=rot.e.r10*x+rot.e.r11*y+rot.e.r12*z+rot.e.r13;
	tran.e.r20=rot.e.r20*crot-rot.e.r22*srot;
	tran.e.r22=rot.e.r20*srot+rot.e.r22*crot;
	tran.e.r23=rot.e.r20*x+rot.e.r21*y+rot.e.r22*z+rot.e.r23;

	// Rotate all points of object
	for (;p;p--)
		{
		x=inpt[0];
		y=inpt[1];
		z=-inpt[2];
		inpt+=3;
		rx=x*tran.e.r00+z*tran.e.r02+tran.e.r03; // r01 is 0
		ry=x*tran.e.r10+y*rot.e.r11+z*tran.e.r12+tran.e.r13;
		rz=x*tran.e.r20+y*rot.e.r21+z*tran.e.r22+tran.e.r23;

		// Get perspective scaling of point
		if (rz<SCREENDIST)
			{
			qrz=1.5-rz*scrdr; 	// Alternative perspective scaling...
			qrz*=qrz; 		 		//	mimics proper perspective for small...	
			qrz=persc*(qrz+.75);	// and negative z values.
/*
			qrz=2-rz*scrdr;		// Improved alternative perspective..
			qrz=qrz*qrz*qrz;	   // slightly higher complexity.
			qrz=persc*(qrz+2);
			qrz/=3;
*/			}
		else
			qrz=q/rz;	// Normal scaling

		//Add screen points to output list (with z values)
		outpt[0]=rz;
		outpt[1]=rx*qrz+render_buff.clip_xmid;
		outpt[2]=ry*qrz+render_buff.clip_ymid;
		outpt+=4;	// add 4 so indexing can be done with shifts
		}
	addpols(ob);
	curobj->nopols=curpol-curobj->polys;
	curobj++;
	}
	}


// Routine for objects with y axis rotation & z-clipping
void addobjyc(obj *ob)
	{
	pt inpt=ob->points;
	rpt outpt=rotlist;
	word p=(word)*inpt++; // No of pts currently stored as datapt type!
	datapt x,y,z;
	floatpt crot,srot;
	rotpt	rx,ry,rz,qrz;

	// Get object depth
	curobj->z=objdep(*ob);
	curobj->polys=curpol;

	// Transform matrix derived from view rotation and object rotation
	// Assumes r01=0 in view rotation and that r11 & r21 remain the same
	crot=ob->crot;
	srot=ob->srot;
	x=ob->x;
	y=ob->y;
	z=ob->z;
	tran.e.r00=rot.e.r00*crot-rot.e.r02*srot;
	tran.e.r02=rot.e.r00*srot+rot.e.r02*crot;
	tran.e.r03=rot.e.r00*x+rot.e.r01*y+rot.e.r02*z+rot.e.r03;
	tran.e.r10=rot.e.r10*crot-rot.e.r12*srot;
	tran.e.r12=rot.e.r10*srot+rot.e.r12*crot;
	tran.e.r13=rot.e.r10*x+rot.e.r11*y+rot.e.r12*z+rot.e.r13;
	tran.e.r20=rot.e.r20*crot-rot.e.r22*srot;
	tran.e.r22=rot.e.r20*srot+rot.e.r22*crot;
	tran.e.r23=rot.e.r20*x+rot.e.r21*y+rot.e.r22*z+rot.e.r23;

	// Rotate all points of object
	for (;p;p--)
		{
		x=inpt[0];
		y=inpt[1];
		z=inpt[2];
		inpt+=3;
		rx=x*tran.e.r00+z*tran.e.r02+tran.e.r03; // r01 is 0
		ry=x*tran.e.r10+y*rot.e.r11+z*tran.e.r12+tran.e.r13;
		rz=x*tran.e.r20+y*rot.e.r21+z*tran.e.r22+tran.e.r23;

		// Get perspective scaling & add screen points to output list (with z values)
		outpt[0]=rz;
		if (rz<SCREENDIST)
			{
			outpt[1]=rx*persc;	//These values used in clip calculation
			outpt[2]=ry*persc;
			}
		else
			{
			qrz=q/rz;	// Normal scaling
			outpt[1]=rx*qrz+render_buff.clip_xmid;
			outpt[2]=ry*qrz+render_buff.clip_ymid;
			}
		outpt+=4;	// add 4 so indexing can be done with shifts
		}
	addpolsc(ob);
	curobj->nopols=curpol-curobj->polys;
	curobj++;
	}


// Routine for objects with no rotation, no z-clipping
void addobjn(obj *ob)
	{
	pt inpt=ob->points;
	rpt outpt=rotlist;
	word p=(word)*inpt++; // No of pts currently stored as datapt type!
	datapt x,y,z,tranx,trany,tranz;
	rotpt	rx,ry,rz,qrz;

	// Get object depth
	if ((curobj->z=objdep(*ob))>CUTOFFDIST)
	{
	curobj->polys=curpol;

	tranx=ob->x;
	trany=ob->y;
	tranz=ob->z;
	// Rotate all points of object
	for (;p;p--)
		{
		x=tranx+inpt[0];
		y=trany+inpt[1];
		z=tranz+inpt[2];
		inpt+=3;
		rx=x*rot.e.r00+z*rot.e.r02+rot.e.r03; // r01 is 0
		ry=x*rot.e.r10+y*rot.e.r11+z*rot.e.r12+rot.e.r13;
		rz=x*rot.e.r20+y*rot.e.r21+z*rot.e.r22+rot.e.r23;

		// Get perspective scaling of point
		if (rz<SCREENDIST)
			{
			qrz=1.5-rz*scrdr; 	// Alternative perspective scaling...
			qrz*=qrz; 		 		//	mimics proper perspective for small...	
			qrz=persc*(qrz+.75);	// and negative z values.
			}
		else
			qrz=q/rz;	// Normal scaling

		//Add screen points to output list (with z values)
		outpt[0]=rz;
		outpt[1]=rx*qrz+render_buff.clip_xmid;
		outpt[2]=ry*qrz+render_buff.clip_ymid;
		outpt+=4;	// add 4 so indexing can be done with shifts
		}
	addpols(ob);
	curobj->nopols=curpol-curobj->polys;
	curobj++;
	}
	}


// Routine for sprite objects (no rotation, no z-clipping)
void addobjsp(datapt *sprpt, word *refpt)
	{
	word p=(word)*(sprpt++); // No of sprites currently stored as datapt type!
	datapt x,y,z;
	rotpt	rx,ry,rz,qrz;

	// Rotate all sprite points and add to polygon list for sorting
	for (;p;p--)
		{
		curobj->polys=curpol;
		x=sprpt[0];
		y=sprpt[1];
		z=sprpt[2];
		sprpt+=3;
		rx=x*rot.e.r00+z*rot.e.r02+rot.e.r03; // r01 is 0
		ry=x*rot.e.r10+y*rot.e.r11+z*rot.e.r12+rot.e.r13;
		rz=x*rot.e.r20+y*rot.e.r21+z*rot.e.r22+rot.e.r23;

		// Get perspective scaling of point
		if (rz<SCREENDIST)
			{
			qrz=1.5-rz*scrdr; 	// Alternative perspective scaling...
			qrz*=qrz; 		 		//	mimics proper perspective for small...	
			qrz=persc*(qrz+.75);	// and negative z values.
			}
		else
			qrz=q/rz;	// Normal scaling

		rx=rx*qrz+render_buff.clip_xmid;
		if (rz<CUTOFFDIST)
			if (rx>=0)
			 	rx=render_buff.clip_wid<<1;
			else
				rx=-render_buff.clip_wid;

 		curpol->pts=curpts;
		curobj->z=curpol->z=rz; 
		curpts[0]=1;
		curpts[1]=*(refpt++);
		curpts[2]=rx;
		curpts[3]=ry*qrz+render_buff.clip_ymid;
		curpts+=4;
		curpol++;
		curobj->nopols=1;
		curobj++;
		}
	}


// Routine for objects with no rotation & z-clipping
void addobjnc(obj *ob)
	{
	pt inpt=ob->points;
	rpt outpt=rotlist;
	word p=(word)*inpt++; // No of pts currently stored as datapt type!
	datapt x,y,z,tranx,trany,tranz;
	rotpt	rx,ry,rz,qrz;

	// Get object depth
	curobj->z=objdep(*ob);
	curobj->polys=curpol;

	tranx=ob->x;
	trany=ob->y;
	tranz=ob->z;
	// Rotate all points of object
	for (;p;p--)
		{
		x=tranx+inpt[0];
		y=trany+inpt[1];
		z=tranz+inpt[2];
		inpt+=3;
		rx=x*rot.e.r00+z*rot.e.r02+rot.e.r03; // r01 is 0
		ry=x*rot.e.r10+y*rot.e.r11+z*rot.e.r12+rot.e.r13;
		rz=x*rot.e.r20+y*rot.e.r21+z*rot.e.r22+rot.e.r23;

		// Get perspective scaling & add screen points to output list (with z values)
		outpt[0]=rz;
		if (rz<SCREENDIST)
			{
			outpt[1]=rx*persc;	//These values used in clip calculation
			outpt[2]=ry*persc;
			}
		else
			{
			qrz=q/rz;	// Normal scaling
			outpt[1]=rx*qrz+render_buff.clip_xmid;
			outpt[2]=ry*qrz+render_buff.clip_ymid;
			}
		outpt+=4;	// add 4 so indexing can be done with shifts
		}
	addpolsc(ob);
	curobj->nopols=curpol-curobj->polys;
	curobj++;
	}


// Routine for objects with no rotation and all y=0, no z-clipping
void addobjf(obj *ob)
	{
	pt inpt=ob->points;
	rpt outpt=rotlist;
	word p=(word)*inpt++; // No of pts currently stored as datapt type!
	datapt x,z,tranx,tranz;
	rotpt	rx,ry,rz,qrz;

	// Get object depth
	if ((curobj->z=objdep(*ob))>CUTOFFDIST)
	{
	curobj->polys=curpol;

	tranx=ob->x;
	tranz=ob->z;
	// Rotate all points of object
	for (;p;p--)
		{
		x=tranx+inpt[0];
		z=tranz+inpt[2];
		inpt+=3;
		rx=x*rot.e.r00+z*rot.e.r02+rot.e.r03;
		ry=x*rot.e.r10+z*rot.e.r12+rot.e.r13;
		rz=x*rot.e.r20+z*rot.e.r22+rot.e.r23;

		// Get perspective scaling of point
		if (rz<SCREENDIST)
			{
			qrz=1.5-rz*scrdr; 	// Alternative perspective scaling...
			qrz*=qrz; 		 		//	mimics proper perspective for small...	
			qrz=persc*(qrz+.75);	// and negative z values.
			}
		else
			qrz=q/rz;	// Normal scaling

		//Add screen points to output list (with z values)
		outpt[0]=rz;
		outpt[1]=rx*qrz+render_buff.clip_xmid;
		outpt[2]=ry*qrz+render_buff.clip_ymid;
		outpt+=4;	// add 4 so indexing can be done with shifts
		}
	addpols(ob);
	curobj->nopols=curpol-curobj->polys;
	curobj++;
	}
	}


// Routine for objects with no rotation and all y=0, with z-clipping
void addobjfc(obj *ob)
	{
	pt inpt=ob->points;
	rpt outpt=rotlist;
	word p=(word)*inpt++; // No of pts currently stored as datapt type!
	datapt x,z,tranx,tranz;
	rotpt	rx,ry,rz,qrz;

	// Get object depth
	curobj->z=objdep(*ob);
	curobj->polys=curpol;

	tranx=ob->x;
	tranz=ob->z;
	// Rotate all points of object
	for (;p;p--)
		{
		x=tranx+inpt[0];
		z=tranz+inpt[2];
		inpt+=3;
		rx=x*rot.e.r00+z*rot.e.r02+rot.e.r03;
		ry=x*rot.e.r10+z*rot.e.r12+rot.e.r13;
		rz=x*rot.e.r20+z*rot.e.r22+rot.e.r23;

		// Get perspective scaling & add screen points to output list (with z values)
  		outpt[0]=rz;
		if (rz<SCREENDIST)
			{
			outpt[1]=rx*persc;	//These values used in clip calculation
			outpt[2]=ry*persc;
			}
		else
			{
			qrz=q/rz;	// Normal scaling
			outpt[1]=rx*qrz+render_buff.clip_xmid;
			outpt[2]=ry*qrz+render_buff.clip_ymid;
			}
		outpt+=4;	// add 4 so indexing can be done with shifts
		}
	addpolsc(ob);
	curobj->nopols=curpol-curobj->polys;
	curobj++;
	}




/************************************************/
/* SORT, DISPLAY AND RESET OBJECT/POLYGON LISTS */

// Macro to reset object/polygon/point lists
#define clearlists curobj=objlist,curpol=pollist,curpts=ptslist;


// Object depth comparison for qsort
int objcmp(void const *obj1, void const *obj2)
	{
	// Avoids unnessecary float to int conversions, assumes true=1
	return (((objs *)obj1)->z<=((objs *)obj2)->z ? ((objs *)obj1)->z<((objs *)obj2)->z : -1);
	}


// Polygon depth comparison for qsort
int polcmp(void const *pol1, void const *pol2)
	{
	// Avoids unnessecary float to int conversions, assumes true=1
	return (((pols *)pol1)->z<=((pols *)pol2)->z ? ((pols *)pol1)->z<((pols *)pol2)->z : -1);
	}


// Sort the polygons within a single object
#define sortobj(ob) qsort((ob)->polys,(ob)->nopols,sizeof(pols),polcmp);

// Draw polygons after sorting
void dispols()
	{
	objs *cobj=objlist;
	short i,n;
	word np,col;
	dword *polytex;

	// Sort objects from z value
	qsort(objlist,curobj-objlist,sizeof(objs),objcmp);

	// Draw the polygons associated with the sorted list
	while (cobj!=curobj)
		{
		n=cobj->nopols;
		curpol=cobj++->polys;
		for (i=0;i<n;i++)
			{
			curpts=curpol++->pts;
			np=(word)curpts[0];
			col=(word)curpts[1];
			curpts+=2;

			if (col>=512) col=257;

			// Choose routine to use from np & col
			if (col<-2000)
 				{
				polytex=(dword *)&textures[-col-2000];
				polyt((pnt *)curpts,np,mapsel[*(((BYTE *)polytex))],polytex+np,polytex);
				continue;
				}
			if (col<0)
 				{
				polytex=(dword *)&textures[-col];
				polym((pnt *)curpts,np,mapsel[*(((BYTE *)polytex))],polytex+np,polytex);
				continue;
				}
			if (col>=512)
	 			{
				polytex=(dword *)&texloop[(col&255)].tex;
//				polytex=(dword *)&textures[(col&255)+S_TM];
				polyl((pnt *)curpts,np,mapsel[mappages-2+(col&255)],polytex+np,polytex);
//				polyl((pnt *)curpts,np,mapsel[mappages-2+(col>>8)],polytex+np,polytex);
				continue;
				}
			if (col>=256)
 				{
				polyf((pnt *)curpts,np,filters[col&255]);
				continue;
				}
			if (np<=2)				     
				if (np==1)
					{sprite3d(curpts[0]/render_buff.scale_x,(render_buff.clip_hgt-curpts[1])/render_buff.scale_y,col);curpts+=2;}
				else		   
					line((pnt *)curpts,col);
			else
				polyb((pnt *)curpts,np,col);
			}
	 
		}
		clearlists
	}



// Draw polygons without sorting
void dispolsn()
	{
	objs *cobj=objlist;
	short i,n;
	word np,col;
	dword *polytex;

	while (cobj!=curobj)
		{
		n=cobj->nopols;
		curpol=cobj++->polys;
		for (i=0;i<n;i++)
			{
			curpts=curpol++->pts;
			np=(word)curpts[0];
			col=(word)curpts[1];
			curpts+=2;

			if (col>=512) col=257;

			// Choose routine to use from np & col
//			if (np == -4)
//				{
//				polyt((pnt *)curpts, np, mapsel[6], &PitchTextureX[0], &PitchTextureY[0]);
//				polyj((pnt *)curpts, 4);
//				continue;
//				}
			if (col<-2000)
 				{
				polytex=(dword *)&textures[-col-2000];
				polyt((pnt *)curpts,np,mapsel[*(((BYTE *)polytex))],polytex+np,polytex);
				continue;
				}
			if (col<0)
 				{
				polytex=(dword *)&textures[-col];
				polym((pnt *)curpts,np,mapsel[*(((BYTE *)polytex))],polytex+np,polytex);
				continue;
				}
			if (col>=512)
	 			{
				polytex=(dword *)&texloop[(col&255)].tex;
				polyl((pnt *)curpts,np,mapsel[mappages-2+(col&255)],polytex+np,polytex);
				continue;
				}
			if (col>=256)
 				{
				polyf((pnt *)curpts,np,filters[col&255]);
				continue;
				}
			if (np<=2)
				if (np==1)
					{sprite3d(curpts[0]/render_buff.scale_x,(render_buff.clip_hgt-curpts[1])/render_buff.scale_y,col);curpts+=2;}
				else		   
					line((pnt *)curpts,col);
			else
				polyb((pnt *)curpts,np,col);
			}
		}
	clearlists
	}




void dispolsnx()
	{
	objs *cobj=objlist;
	short i,n;
	word np,col;
	dword *polytex;

	while (cobj!=curobj)
		{
		n=cobj->nopols;
		curpol=cobj++->polys;
		for (i=0;i<n;i++)
			{
			curpts=curpol++->pts;
			np=(word)curpts[0];
			col=(word)curpts[1];
			curpts+=2;

if (col>=512) col=257;
			
			// Choose routine to use from np & col
			if (col<-2000)
 				{
				polytex=(dword *)&textures[-col-2000];
				polyt((pnt *)curpts,np,mapsel[*(((BYTE *)polytex))],polytex+np,polytex);
				continue;
				}
			if (col<0)
 				{
				polytex=(dword *)&textures[-col];
				polym((pnt *)curpts,np,mapsel[*(((BYTE *)polytex))],polytex+np,polytex);
				continue;
				}
			if (col>=512)
	 			{
				polytex=(dword *)&texloop[(col&255)].tex;
//				polytex=(dword *)&textures[(col&255)+S_TM];
				polyl((pnt *)curpts,np,mapsel[mappages-2+(col&255)],polytex+np,polytex);
//	 			polyl((pnt *)curpts,np,mapsel[mappages-2+(col>>8)],polytex+np,polytex);
				continue;
				}
			if (col>=256)
 				{
				polyf((pnt *)curpts,np,filters[col&255]);
				continue;
				}
			if (np<=2)
				if (np==1)
					{sprite3d(curpts[0]/render_buff.scale_x,(render_buff.clip_hgt-curpts[1])/render_buff.scale_y,col);curpts+=2;}
				else		   
					line((pnt *)curpts,col);
			else
				poly((pnt *)curpts,np,col);
			}
 
		}
		clearlists
	}

// Draw Just The Pitch
static void dispol_pitch()
{
objs *cobj=objlist;
short i,n;
word np,col;
dword *polytex;

	while (cobj!=curobj)
		{
		n=cobj->nopols;
		curpol=cobj++->polys;
		for (i=0;i<n;i++)
			{
			curpts=curpol++->pts;
			np=(word)curpts[0];
			col=(word)curpts[1];
			curpts+=2;
			
			polyj((pnt *)curpts, 4);
			}
		}
	clearlists
}

// Make a filter array for polygon filters
void makefilter(filter filt,byte fr,byte fg,byte fb,float mr,float mg,float mb)
	{
	word r1,g1,b1,r2,g2,b2;
	word x,s,c,l1,l2,d1,d2;
	if ((l1=sqrt(fr*fr+fg*fg+fb*fb))==0) l1=1;
	for (x=0;x<768;x+=3)	 
		{
		l2=sqrt(palette_buffer[x]*palette_buffer[x]+palette_buffer[x+1]*palette_buffer[x+1]+palette_buffer[x+2]*palette_buffer[x+2]);
		r1=palette_buffer[x]+mr*(fr*l2/l1-palette_buffer[x]);
		if (r1<0) r1=0;if (r1>255) r1=255;
		g1=palette_buffer[x+1]+mg*(fg*l2/l1-palette_buffer[x+1]);
		if (g1<0) g1=0;if (g1>255) g1=255;
		b1=palette_buffer[x+2]+mb*(fb*l2/l1-palette_buffer[x+2]);
		if (b1<0) b1=0;if (b1>255) b1=255;
		r1=(fr-palette_buffer[x])*mr+palette_buffer[x];
		g1=(fg-palette_buffer[x+1])*mg+palette_buffer[x+1];
		b1=(fb-palette_buffer[x+2])*mb+palette_buffer[x+2];
		d1=128;
		for (s=0;s<768-16*3;s+=3)
			{
		 	r2=(palette_buffer[s]-r1);
			g2=(palette_buffer[s+1]-g1);
			b2=(palette_buffer[s+2]-b1);
			d2=sqrt(r2*r2+g2*g2+b2*b2);
			if (d2<d1) d1=d2,c=s;
			}
		filt[x/3]=c/3;
		}
	}

int readfile(int fileno, BYTE *address)
{
Handle handle;
Boolean flag = false;

	fileno = NORMRES(fileno) ;
again:;
	handle = GetResource( resType, fileno);
	if( handle)
		{
		memcpy(address, *handle, GetHandleSize(handle));
		ReleaseResource( handle);
		}
#if MEM_DEBUG==ON
	else
		fprintf(DebugFile, " Load %d to %lx failed , Error %d, Handle %lx\n", fileno, address, ResError(), handle);
#endif

	return (handle == NULL);
}

//char *readfileblock(int fileno, int blockno, int blocks, BYTE *address)
//{
//int flen;
//BYTE *ptr;

//	fprintf( DebugFile, "Anim Load %d (%d) - %d\n", blockno, NORMRES(fileno), blocks);
//	flen=load_offsets[fileno/8].size;
//	if ((ptr=readdatafilesection(fileno,address+flen*blockno/blocks,flen*blockno/blocks,flen*(blockno+1)/blocks-flen*blockno/blocks))==NULL)
//		printf("Error loading data file %d\n",fileno);
//	return (ptr);
//	return NULL;
//}

#define FRM_WID 128
#define FRM_HGT 80
#define FRM_LEN (FRM_WID*FRM_HGT)
#define ANIM_SPACE 280000
//#define ANIM_SPACE 172000

int anim_start[]={0,10241,10241,10241,10241,10241,10241,10241,10241+91600,10241,10241+126777,10241,10241+127983};

char *buff_ptr;

typedef struct { int no,time;char *frmptr; } anim_info;
anim_info vidi;
int vidi_in,vidi_anim,vcount;

void init_anim( anim_info &anim, int no, int vco)
{
int i;
char *buffp=(char *)(maps[S_BM])+stadlist[setup.stadium].vmap;
char *animp=anim.frmptr=animdata+anim_start[no];

	vidi_in = no;
	vcount = vco;
	
	anim.no=no;
	anim.time=count;
		
	for (i=0;i<FRM_HGT;i++)
		{
		memcpy(buffp,animp,FRM_WID);
		buffp+=256;
		animp+=FRM_WID;
		}
	anim.frmptr=animp;
}


void stop_anim(anim_info &anim)
{
	if( Language == American || Language == Canadian)
		readfile(COL_VR,(BYTE *)&palette_buffer[240*3]);
	else
		readfile(COL_VIDI,(BYTE *)&palette_buffer[240*3]);
	init_anim( anim, 0, 0);
	PaletteChange = true;
//	vidi_in=vidi_anim=0;
	vidi_anim = 0;
	ReloadTextureMap( S_BM);
}


void do_anim(anim_info &anim)
	{
	int i,j,ttime,tval,tlen;
	int buffd=256-FRM_WID;
//	char *buffp=buff_ptr;
	char *buffp=(char *)(maps[S_BM])+stadlist[setup.stadium].vmap;
	char *animp;

	if (anim.no && (ttime=count)>anim.time+10)
		if (!*anim.frmptr)
		 	{
			if (vidi_anim==7||vidi_anim==9||vidi_anim==11)
				vidi_anim++;
			init_anim( anim, vidi_anim, vcount);
		 	}
		else
			{
			if (--vcount<=0)
				stop_anim(anim);
			else if(vcount < 10)
				{
				for( i=0; i<FRM_HGT; i++)
					{
					memset( buffp, 0, FRM_WID);
					buffp+=256;
					}
#if USERAVEENGINE==YES
				ReloadTextureMap( S_BM);
#endif
				}
			else
				{
				animp=anim.frmptr;
				anim.time=ttime;

				tlen=FRM_WID;
				while (tval=*animp++)
					{

					while (tlen<tval)	
						{
						tval-=tlen;
						while (tlen--) (*buffp++) = (*animp++);
						tlen=FRM_WID,buffp+=buffd;
						}
					tlen-=tval;
					while (tval--) (*buffp++) = (*animp++);

					tval=*animp++;
					while (tlen<tval)	
						{
						tval-=tlen;
						while (tlen--) buffp++;
						tlen=FRM_WID,buffp+=buffd;
						}
					tlen-=tval;
					while (tval--) buffp++;
					}
				anim.frmptr=animp;
#if USERAVEENGINE==YES
				ReloadTextureMap( S_BM);
#endif
				}
	  		}
	}


int spool;

void do_extra_time_anim(int sp)
{
	if (!animdata) return;
	vidi_anim=1;
	spool=sp;
	if (spool==10||vidi_in==1)
		{
		spool=0;
//		vidi_in=1;
//		vcount=200;
		init_anim( vidi, 1, 200);
		PaletteChange = true;
		return;
		}
	if( !spool)
		{
		readfile(FAP_XET,(BYTE *)animdata+10241);
		readfile(COL_XET,(BYTE *)&palette_buffer[240*3]);
		spool=9;
		}
//	readfileblock(FAP_XET,spool,10,(BYTE *)animdata+10241);
	spool++;
}

void do_foul_anim(int sp)
{
	if (!animdata) return;
	vidi_anim=2;
	spool=sp;
	if (spool==10||vidi_in==2)
		{
		spool=0;
//		vidi_in=2;
//		vcount=100;
		init_anim( vidi, 2, 100);
		PaletteChange = true;
		return;
		}
	if( !spool)
		{
		readfile(FAP_XFOU,(BYTE *)animdata+10241);
		readfile(COL_XFOU,(BYTE *)&palette_buffer[240*3]);
		spool=9;
		}
//	readfileblock(FAP_XFOU,spool,10,(BYTE *)animdata+10241);
	spool++;
}

void do_full_time_anim(int sp)
{
	if (!animdata) return;
	vidi_anim=3;
	spool=sp;
	if (spool==10||vidi_in==3)
		{
		spool=0;
//		vidi_in=3;
//		vcount=200;
		init_anim( vidi, 3, 200);
		PaletteChange = true;
		return;
		}
	if( !spool)
		{
		readfile(FAP_XFT,(BYTE *)animdata+10241);
		readfile(COL_XFT,(BYTE *)&palette_buffer[240*3]);
		spool=9;
		}
//	readfileblock(FAP_XFT,spool,10,(BYTE *)animdata+10241);
	spool++;
}

void do_half_time_anim(int sp)
{
	if (!animdata) return;
	vidi_anim=4;
	spool=sp;
	if (spool==10||vidi_in==4)
		{
		spool=0;
//		vidi_in=4;
//		vcount=100;
		init_anim( vidi, 4, 100);
		PaletteChange = true;
		return;
		}
	if( !spool)
		{
		readfile(FAP_XHT,(BYTE *)animdata+10241);
		readfile(COL_XHT,(BYTE *)&palette_buffer[240*3]);
		spool=9;
		}
//	readfileblock(FAP_XHT,spool,10,(BYTE *)animdata+10241);
	spool++;
}

void do_penalty_anim(int sp)
{
	if (!animdata) return;
	vidi_anim=5;
	spool=sp;
	if (spool==10||vidi_in==5)
		{
		spool=0;
//		vidi_in=5;
//		vcount=100;
		init_anim( vidi, 5, 100);
		PaletteChange = true;
		return;
		}
	if( !spool)
		{
		readfile(FAP_XPEN,(BYTE *)animdata+10241);
		readfile(COL_XPEN,(BYTE *)&palette_buffer[240*3]);
		spool=9;
		}
//	readfileblock(FAP_XPEN,spool,10,(BYTE *)animdata+10241);
	spool++;
}

/*
void
do_goal_anim(int sp)
	{
	if (!animdata) return;
	if (vidi_in!=6) readfile(FAP_XGOA,(BYTE *)animdata+10241);
	readfile(COL_XGOA,(BYTE *)&palette_buffer[240*3]);
	PaletteChange = true;
	vidi_anim=vidi_in=6;
	vcount=200;
	init_anim(vidi,6);
	}
*/
void do_goal_anim(int sp)
{
	if (!animdata) return;
	vidi_anim=6;
	spool=sp;
	if (spool==10||vidi_in==6)
		{
		spool=0;
//		vidi_in=6;
//		vcount=200;
		init_anim( vidi, 6, 100);
		PaletteChange = true;
		return;
		}
	if( !spool)
		{
	if( Language == American || Language == Canadian)
		{
		readfile(FAP_XGOA,(BYTE *)animdata+10241);
		readfile(COL_XGOA,(BYTE *)&palette_buffer[240*3]);
		}
	else
		{
		readfile(FAP_BOOST,(BYTE *)animdata+10241);
		readfile(COL_BOOST,(BYTE *)&palette_buffer[240*3]);
		}
		spool=9;
		}
////#ifndef BBS
////	readfileblock(FAP_BOOST,spool,10,(BYTE *)animdata+10241);
////#else
//	readfileblock(FAP_XGOA,spool,10,(BYTE *)animdata+10241);
////#endif
	spool++;
	}

void do_win_f_anim(int sp)
{
	if (!animdata) return;
	vidi_anim=7;
	spool=sp;
	if (spool==10||vidi_in==7)
		{
		spool=0;
//		vidi_in=7;
//		vcount=100000;
		init_anim( vidi, 7, 50000);
		PaletteChange = true;
		return;
		}
	if( !spool)
		{
		readfile(COL_XWNF,(BYTE *)&palette_buffer[240*3]);
		readfile(FAP_XWNF1,(BYTE *)animdata+10241);
		readfile(FAP_XWNF2,(BYTE *)animdata+10241+91600);
		spool=9;
		}
//	readfileblock(FAP_XWNF1,spool,10,(BYTE *)animdata+10241);
//	readfileblock(FAP_XWNF2,spool,10,(BYTE *)animdata+10241+91600);
	spool++;
}

void do_win_g_anim(int sp)
{
	if (!animdata) return;
	vidi_anim=9;
	spool=sp;
	if (spool==10||vidi_in==9)
		{
		spool=0;
//		vidi_in=9;
//		vcount=100000;
		init_anim( vidi, 9, 50000);
		PaletteChange = true;
		return;
		}
	if( !spool)
		{
		readfile(COL_XWNG,(BYTE *)&palette_buffer[240*3]);
		readfile(FAP_XWNG1,(BYTE *)animdata+10241);
		readfile(FAP_XWNG2,(BYTE *)animdata+10241+126777);
		spool=9;
		}
//	readfileblock(FAP_XWNG1,spool,10,(BYTE *)animdata+10241);
//	readfileblock(FAP_XWNG2,spool,10,(BYTE *)animdata+10241+126777);
	spool++;
}

void do_win_s_anim(int sp)
{
	if (!animdata) return;
	vidi_anim=11;
	spool=sp;
	if (spool==10||vidi_in==11)
		{
		spool=0;
//		vidi_in=11;
//		vcount=100000;
		init_anim( vidi, 11, 100000);
		PaletteChange = true;
		return;
		}
	if( !spool)
		{
		readfile(COL_XWNS,(BYTE *)&palette_buffer[240*3]);
		readfile(FAP_XWNS1,(BYTE *)animdata+10241);
		readfile(FAP_XWNS2,(BYTE *)animdata+10241+127983);
		spool=9;
		}
//	readfileblock(FAP_XWNS1,spool,10,(BYTE *)animdata+10241);
//	readfileblock(FAP_XWNS2,spool,10,(BYTE *)animdata+10241+127983);
	spool++;
}


/*********************/
/* IN-GAME FRONT END */

#define NO_MENUS 23
buff_info menu_buff_l[NO_MENUS]=
	{
	 {NULL, 320,64, 320,64, 0, 0, dumpnull},
	 {NULL, 320,64, 320,64, 0, 0, dumpnull},
	 {NULL, 320,64, 320,64, 0, 0, dumpnull},
	 {NULL, 320,64, 224,64, 0, 0, dumpnull},
	 {NULL, 320,64, 224,64, 0, 0, dumpnull},
	 {NULL, 320,64, 192,64, 0, 0, dumpnull},
	 {NULL, 320,64, 192,64, 0, 0, dumpnull},
	 {NULL, 320,64, 192,64, 0, 0, dumpnull},
	 {NULL, 320,64, 192,64, 0, 0, dumpnull},
	 {NULL, 320,64, 192,64, 0, 0, dumpnull},
	 {NULL, 320,64, 224,64, 0, 0, dumpnull},
	 {NULL, 320,64, 320,64, 0, 0, dumpnull},
	 {NULL, 320,128, 288,128, 0, 0, dumpnull},
	 {NULL, 320,128, 288,128, 0, 0, dumpnull},
	 {NULL, 320,128, 288,128, 0, 0, dumpnull},
	 {NULL, 320,128, 288,128, 0, 0, dumpnull},
	 {NULL, 320,128, 288,128, 0, 0, dumpnull},
	 {NULL, 320,64, 320,64, 0, 0, dumpnull},
	 {NULL, 320,128, 160,128, 0, 0, dumpnull},
	 {NULL, 320,112, 160,112, 0, 0, dumpnull},
	 {NULL, 320,128, 224,128, 0, 0, dumpnull},
	 {NULL, 320,80, 288,80, 0, 0, dumpnull},
	 {NULL, 320,64, 320,64, 0, 0, dumpnull},
	};
	
buff_info menu_buff_h[NO_MENUS]=
	{
	 {NULL, 640,128, 640,128, 0, 0, dumpnull},
	 {NULL, 640,128, 640,128, 0, 0, dumpnull},
	 {NULL, 640,128, 640,128, 0, 0, dumpnull},
	 {NULL, 640,128, 448,128, 0, 0, dumpnull},
	 {NULL, 640,128, 448,128, 0, 0, dumpnull},
	 {NULL, 640,128, 384,128, 0, 0, dumpnull},
	 {NULL, 640,128, 384,128, 0, 0, dumpnull},
	 {NULL, 640,128, 384,128, 0, 0, dumpnull},
	 {NULL, 640,128, 384,128, 0, 0, dumpnull},
	 {NULL, 640,128, 384,128, 0, 0, dumpnull},
	 {NULL, 640,128, 448,128, 0, 0, dumpnull},
	 {NULL, 640,128, 640,128, 0, 0, dumpnull},
	 {NULL, 640,256, 576,256, 0, 0, dumpnull},
	 {NULL, 640,256, 576,256, 0, 0, dumpnull},
	 {NULL, 640,256, 576,256, 0, 0, dumpnull},
	 {NULL, 640,256, 576,256, 0, 0, dumpnull},
	 {NULL, 640,256, 576,256, 0, 0, dumpnull},
	 {NULL, 640,128, 640,128, 0, 0, dumpnull},
	 {NULL, 640,256, 320,256, 0, 0, dumpnull},
	 {NULL, 640,224, 320,224, 0, 0, dumpnull},
	 {NULL, 640,256, 448,256, 0, 0, dumpnull},
	 {NULL, 640,160, 576,160, 0, 0, dumpnull},
	 {NULL, 640,128, 640,128, 0, 0, dumpnull},
	};

struct mtime {
  	unsigned short min;
	float secs;
};

extern mtime match_time;
extern int team_a_goals;
extern int team_b_goals;

extern goal_table	goals[];
extern team_info	game_data[MAX_TEAMS];
extern match_data match_info;

float slco;
int slide,oslide,mcount,mmenu,mhgt,rdmenu;
int mdata1,mdata2,mdata3;

void draw_sprite(buff_info *buff,int sprite_no,int x,int y,BYTE col)
{
BYTE *spt, *spd, *spf;
int sd, w, h, i, j, count = 1, ts, ds;
BYTE c;

	spd=maps[X_BM+spr_data[sprite_no].page]+spr_data[sprite_no].y*256+spr_data[sprite_no].x;
	w=min(spr_data[sprite_no].wid,buff->clip_wid-x);
	h=min(spr_data[sprite_no].hgt,buff->clip_hgt-y);
	ds=256-w;

	spt=buff->buff_start+y*buff->buff_wid+x;
	ts=buff->buff_wid-w;
	for (i=0;i<h;i++)
		{
		for (j=0;j<w;j++)
			{
			c=*spd++;
			if (c) if (!--c) *spt=col; else *spt=c;
			spt++;
			}
		spt+=ts;
		spd+=ds;
		}
}

void draw_sprite_d(buff_info *buff,int sprite_no,int x,int y,BYTE col)
{
int sd;
BYTE *spt;
BYTE *spd=maps[X_BM+spr_data[sprite_no].page]+spr_data[sprite_no].y*256+spr_data[sprite_no].x;
int w=min(spr_data[sprite_no].wid,buff->clip_wid-x);
int h=min(spr_data[sprite_no].hgt,buff->clip_hgt-y);
int ts=buff->buff_wid-(w<<1);
int ds=256-w;
int i,j;
BYTE c;

	spt=buff->buff_start+y*buff->buff_wid+x;

	if (w<0||h<0) return;
	for (i=0;i<h;i++)
		{
		for (j=0;j<w;j++)
			{
			c=*spd++;
			if (c) if (!--c) *spt=col; else *spt=c;
			spt++;
			if (c) if (!--c) *spt=col; else *spt=c;
			spt++;
			}
		spt+=ts;
		spd-=w;
		for (j=0;j<w;j++)
			{
			c=*spd++;
			if (c) if (!--c) *spt=col; else *spt=c;
			spt++;
			if (c) if (!--c) *spt=col; else *spt=c;
			spt++;
			}
		spt+=ts;
		spd+=ds;
		}
}

void draw_sprite_s(buff_info *buff,int sprite_no,int x,int y,BYTE col)
{
int sd;
BYTE *spt;
BYTE *spd=maps[X_BM+spr_data[sprite_no].page]+spr_data[sprite_no].y*256+spr_data[sprite_no].x;
int w=min(spr_data[sprite_no].wid,buff->clip_wid-x);
int h=min(spr_data[sprite_no].hgt,buff->clip_hgt-y);
int ts=buff->buff_wid-w;
int ds=256-w;
int i,j;
BYTE c;

	spt=buff->buff_start+y*buff->buff_wid+x;

	if (w<0||h<0) return;
	for (i=0;i<h;i++)
		{
		for (j=0;j<w;j++)
			{
			c=*spd++;
			if (--c) *spt=c;
			spt++;
			}
		spt+=ts;
		spd+=ds;
		}
}

void draw_sprite_vf(buff_info *buff,int sprite_no,int x,int y,BYTE col)
{
int sd;
BYTE *spt;
BYTE *spd=maps[X_BM+spr_data[sprite_no].page]+spr_data[sprite_no].y*256+spr_data[sprite_no].x+(spr_data[sprite_no].hgt-1)*256;
int w=min(spr_data[sprite_no].wid,buff->clip_wid-x);
int h=min(spr_data[sprite_no].hgt,buff->clip_hgt-y);
int ts=buff->buff_wid-w;
int ds=256+w;
int i,j;
BYTE c;

	spt=buff->buff_start+y*buff->buff_wid+x;

	for (i=0;i<h;i++)
		{
		for (j=0;j<w;j++)
			{
			c=*spd++;
			if (c) if (!--c) *spt=col; else *spt=c;
			spt++;
			}
		spt+=ts;
		spd-=ds;
		}
}

void draw_sprite_r(buff_info *buff,int sprite_no,int x,int y,BYTE col)
{
int sd;
BYTE *spt;
BYTE *spd=maps[X_BM+spr_data[sprite_no].page]+spr_data[sprite_no].y*256+spr_data[sprite_no].x;
BYTE *tspd;
int w=min(spr_data[sprite_no].wid,buff->clip_wid-x);
int h=min(spr_data[sprite_no].hgt,buff->clip_hgt-y);
int ts=buff->buff_wid-w;
int ds=256-w;
int i,j;
BYTE c;

	spt=buff->buff_start+y*buff->buff_wid+x;

	if (w<0||h<0) return;
	for (i=0;i<h;i++)
		{
		tspd=spd;
		for (j=0;j<w;j++)
			{
			c=*spd;
			spd+=256;
			if (c) if (!--c) *spt=col; else *spt=c;
			spt++;
			}
		spt+=ts;
		spd=tspd+1;
		}
}

void draw_sprite_hf(buff_info *buff,int sprite_no,int x,int y,BYTE col)
{
int sd;
BYTE *spt;
BYTE *spd=maps[X_BM+spr_data[sprite_no].page]+spr_data[sprite_no].y*256+spr_data[sprite_no].x+spr_data[sprite_no].wid-1;
int w=min(spr_data[sprite_no].wid,buff->clip_wid-x);
int h=min(spr_data[sprite_no].hgt,buff->clip_hgt-y);
int ts=buff->buff_wid-w;
int ds=256+w;
int i,j;
BYTE c;

	spt=buff->buff_start+y*buff->buff_wid+x;

	for (i=0;i<h;i++)
		{
		for (j=0;j<w;j++)
			{
			c=*spd--;
			if (c) if (!--c) *spt=col; else *spt=c;
			spt++;
			}
		spt+=ts;
		spd+=ds;
		}
}

void draw_sprite_vhf(buff_info *buff,int sprite_no,int x,int y,BYTE col)
{
int sd;
BYTE *spt;
BYTE *spd=maps[X_BM+spr_data[sprite_no].page]+spr_data[sprite_no].y*256+spr_data[sprite_no].x+(spr_data[sprite_no].hgt-1)*256+spr_data[sprite_no].wid-1;
int w=min(spr_data[sprite_no].wid,buff->clip_wid-x);
int h=min(spr_data[sprite_no].hgt,buff->clip_hgt-y);
int ts=buff->buff_wid-w;
int ds=256-w;
int i,j;
BYTE c;

	spt=buff->buff_start+y*buff->buff_wid+x;

	for (i=0;i<h;i++)
		{
		for (j=0;j<w;j++)
			{
			c=*spd--;
			if (c) if (!--c) *spt=col; else *spt=c;
			spt++;
			}
		spt+=ts;
		spd-=ds;
		}
}

static char CheckCharacter( char c)
{
	if (c=='-') 
		c = ';';
	else if (c==' ') 
		c = ';';
	else if (c=='.') 
		c = '@';
	else if (c==',') 
		c = '?';
	else if (c=='o' || c=='O') 
		c='0';
	if (c>='a'&&c<='z')
		c-=32;
	return c-48;
}

int
string_len(int font_no,char *string)
	{
	BYTE c;
	int len=0;
	while (c=*string++)
		{
		c = CheckCharacter( c);
		len+=font_data[font_no].prp[c]+1;
		}
	return(len);
	}

int draw_string(buff_info *buff,int font_no,int x,int y,char *string,BYTE col,short just)
{
BYTE c,ec;
int len;

	len=string_len(font_no,string);
	switch (just)
		{
		case 1:
			x-=len;
			break;
		case 2:
			x-=len>>1;
			break;
		}
	while (c=*string++)
		{
		c = CheckCharacter( c);
		ec=c+font_data[font_no].off;
		spr_data[0].page=font_data[font_no].page;
		spr_data[0].x=font_data[font_no].x+font_data[font_no].wid*(ec%font_data[font_no].n);
		spr_data[0].y=font_data[font_no].y+font_data[font_no].hgt*(ec/font_data[font_no].n);
		spr_data[0].wid=font_data[font_no].prp[c];
		spr_data[0].hgt=font_data[font_no].hgt;
		draw_sprite(buff,0,x,y,col);
		x+=font_data[font_no].prp[c]+1;
		}
	return (len);
}

static int draw_string_2col(buff_info *buff,int font_no,int x,int y,char *string,BYTE col,short just, BYTE col2)
{
BYTE c,ec;
int len;

	len=string_len(font_no,string);
	switch (just)
		{
		case 1:
			x-=len;
			break;
		case 2:
			x-=len>>1;
			break;
		}
	while (c=*string)
		{
		c = CheckCharacter(c);
		ec=c+font_data[font_no].off;
		spr_data[0].page=font_data[font_no].page;
		spr_data[0].x=font_data[font_no].x+font_data[font_no].wid*(ec%font_data[font_no].n);
		spr_data[0].y=font_data[font_no].y+font_data[font_no].hgt*(ec/font_data[font_no].n);
		spr_data[0].wid=font_data[font_no].prp[c];
		spr_data[0].hgt=font_data[font_no].hgt;
		draw_sprite(buff,0,x,y,col);
		x+=font_data[font_no].prp[c]+1;
		if( *string++==':')
		  col = col2;
		}
	return (len);
}

void draw_menu_box(buff_info *buff)
{
short f,w,wd,hd;
BYTE *col=filters[0];
BYTE *sc;

	if (match_info.res_toggle)
		{
		if ((hd=min(buff->buff_hgt-(19+32)*2,buff->clip_hgt-19-32))>0)
			{
			wd=buff->clip_wid-20*2;
			sc=buff->buff_start+(19+32)*buff->buff_wid+20;
			for (f=hd;f;f--)
				{
				w=wd;
				do {*sc=col[*sc];sc++;} while (--w);
				sc+=render_buff.buff_wid-wd;
				}
			}
		draw_sprite(buff,21,11,10+32,0);
		draw_sprite_hf(buff,21,buff->clip_wid-32,10+32,0);
		draw_sprite_vhf(buff,21,buff->clip_wid-32,buff->buff_hgt-32-32,0);
		draw_sprite_vf(buff,21,11,buff->buff_hgt-32-32,0);
		for (f=32;f<buff->clip_wid-32;f+=32)
			{
			draw_sprite(buff,22,f,10+32,0);
			draw_sprite(buff,22,f,buff->buff_hgt-19-32,0);
			}
		for (f=32+32;f<buff->buff_hgt-32-32;f+=32)
			{
			draw_sprite(buff,23,11,f,0);
			draw_sprite(buff,23,buff->clip_wid-20,f,0);
			}
		}
#if USERAVEENGINE!=YES
	else
		{
		if ((hd=min(buff->buff_hgt-(10+16)*2,buff->clip_hgt-10-16))>0)
			{
			wd=buff->clip_wid-10*2;
			sc=buff->buff_start+(10+16)*buff->buff_wid+10;
			for (f=hd;f;f--)
				{
				w=wd;
				do {*sc=col[*sc];sc++;} while (--w);
				sc+=render_buff.buff_wid-wd;
				}
			}
		draw_sprite(buff,32,5,5+16,0);
		draw_sprite_hf(buff,32,buff->clip_wid-16,5+16,0);
		draw_sprite_vhf(buff,32,buff->clip_wid-16,buff->buff_hgt-16-16,0);
		draw_sprite_vf(buff,32,5,buff->buff_hgt-16-16,0);
		for (f=16;f<buff->clip_wid-16;f+=32)
			{
			draw_sprite(buff,33,f,5+16,0);
			draw_sprite(buff,33,f,buff->buff_hgt-10-16,0);
			}
		for (f=16+16;f<buff->buff_hgt-32-16;f+=32)
			{
			draw_sprite(buff,34,5,f,0);
			draw_sprite(buff,34,buff->clip_wid-10,f,0);
			}
		if (f<buff->buff_hgt-16-16)
			{
			draw_sprite(buff,83,5,f,0);
			draw_sprite(buff,83,buff->clip_wid-10,f,0);
			}
		}
#endif
	}


void goal_menu(buff_info *buff, int team, int player, int time)
{
char tstr[128];
int shirt;

	if (!team)
		shirt=ppt[player+((match_half&1)?11:0)].number-1;
	else
		shirt=ppt[player+((match_half&1)?0:11)].number-1;

	strcpy( tstr, GetResString(0)); strcat( tstr, " ");
	strcat( tstr, game_data[!team ? setup.team_a:setup.team_b].players[player].name);
	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_sprite(buff,2+shirt,16,21,0);
		draw_string_2col( buff, 0, 160, 30, tstr, 207, 2, 31);
		draw_sprite(buff,40,278,18,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string_2col( buff, 2, 320, 56, tstr, 207, 2, 31);
		draw_sprite_s(buff,shirt+(!team ?53:68), 42, 51, 0);
		draw_sprite(buff,29,556,38,0);
		}
	}

void booked_menu(buff_info *buff, int team, int player)
{
char tstr[128];
int shirt;

	if (!team)
		shirt=ppt[player+((match_half&1)?11:0)].number-1;
	else
		shirt=ppt[player+((match_half&1)?0:11)].number-1;

	strcpy( tstr, GetResString(1)); strcat( tstr, " ");
	strcat( tstr, game_data[!team ? setup.team_a:setup.team_b].players[player].name);
	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_sprite(buff,2+shirt,16,21,0);
		draw_string_2col( buff, 0, 160, 30, tstr, 157, 2, 31);
		draw_sprite(buff,51,276,9,0);
		draw_sprite(buff,52,276,11,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string_2col( buff, 2, 320, 56, tstr, 157, 2, 31);
		draw_sprite_s(buff,shirt + (!team ? 53:68),42,51,0);
		draw_sprite(buff,45,552,18,0);
		draw_sprite(buff,46,552,24,0);
		}
	}

void off_menu(buff_info *buff, int team, int player)
{
char tstr[128];
	int shirt;
	if (!team)
		shirt=ppt[player+((match_half&1)?11:0)].number-1;
	else
		shirt=ppt[player+((match_half&1)?0:11)].number-1;

	strcpy( tstr, GetResString(2)); strcat( tstr, " ");
	strcat( tstr, game_data[!team ? setup.team_a:setup.team_b].players[player].name);
	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_sprite(buff,2+shirt,16,21,0);
		draw_string_2col( buff, 0, 160, 30, tstr,127, 2, 31);
		draw_sprite(buff,50,276,9,0);
		draw_sprite(buff,52,276,11,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string_2col( buff, 2, 320, 56, tstr,127, 2, 31);
		draw_sprite_s( buff, shirt+(!team ?53:68), 42, 51, 0);
		draw_sprite(buff,44,552,18,0);
		draw_sprite(buff,46,552,24,0);
		}
	}

void direct_menu(buff_info *buff, int team)
{
char *p = GetResString(3);

	draw_menu_box(buff);
#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_string(buff,0,112,30,p,31,2);
		draw_sprite(buff,52,188,11,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string(buff,2,224,56,p,31,2);
		draw_sprite(buff,46,376,24,0);
		}
}

void indirect_menu(buff_info *buff, int team)
{
char *p = GetResString(4);

	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_string(buff,0,112,30,p,31,2);
		draw_sprite(buff,52,188,11,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string(buff,2,224,56,p,31,2);
		draw_sprite(buff,46,376,24,0);
		}
}

void penalty_menu(buff_info *buff, int team)
{
char *p = GetResString(5);

	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_string(buff,0,96,30,p,31,2);
		draw_sprite(buff,52,151,11,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string(buff,2,192,56,p,31,2);
		draw_sprite(buff,46,302,24,0);
		}
}

void corner_menu(buff_info *buff, int team)
{
char *p = GetResString(6);

	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_string(buff,0,96,30,p,31,2);
		draw_sprite(buff,40,151,18,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string(buff,2,192,56,p,31,2);
		draw_sprite(buff,29,302,38,0);
		}
}

void throw_menu(buff_info *buff, int team)
{
char *p = GetResString(7);

	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_string(buff,0,96,30,p,31,2);
		draw_sprite(buff,40,151,18,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string(buff,2,192,56,p,31,2);
		draw_sprite(buff,29,302,38,0);
		}
}

void goalkick_menu(buff_info *buff, int team)
{
char *p = GetResString(8);

	draw_menu_box(buff);
	
#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_string(buff,0,96,30,p,31,2);
		draw_sprite(buff,40,151,18,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string(buff,2,192,56,p,31,2);
		draw_sprite(buff,29,302,38,0);
		}
}

void kickoff_menu(buff_info *buff, int team)
{
char *p = GetResString( 9);

	draw_menu_box(buff);
	
#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_string(buff,0,96,30,p,31,2);
		draw_sprite(buff,40,151,18,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string(buff,2,192,56,p,31,2);
		draw_sprite(buff,29,302,38,0);
		}
}

void offside_menu(buff_info *buff, int team)
{
char *p = GetResString( 10);

	draw_menu_box(buff);
	
#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_string(buff,0,112,30,p,31,2);
		draw_sprite(buff,52,188,11,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string(buff,2,224,56,p,31,2);
		draw_sprite(buff,46,376,24,0);
		}
}

void injured_menu(buff_info *buff, int team, int player)
{
char tstr[128];
	int shirt;
	if (!team)
		shirt=ppt[player+((match_half&1)?11:0)].number-1;
	else
		shirt=ppt[player+((match_half&1)?0:11)].number-1;

	strcpy( tstr, GetResString(11));
	strcat( tstr, " ");
	strcat( tstr, game_data[!team ? setup.team_a:setup.team_b].players[player].name);
	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_sprite(buff,2+shirt,16,21,0);
		draw_string_2col( buff, 0, 160, 30, tstr, 127, 2, 31);
		draw_sprite(buff,52,280,11,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_sprite_s(buff,shirt+(!team ? 53:68),42,51,0);
		draw_string_2col( buff, 2, 320, 56, tstr, 127, 2, 31);
		draw_sprite(buff,46,560,24,0);
		}
	}

extern referee_info referee_data[];

void ref_menu(buff_info *buff)
{
char tstr[128];

	strcpy( tstr, GetResString(28));
	strcat( tstr, " ");
	strcat( tstr, referee_data[match_info.match_referee].name);

	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,52,10,11,0);
		draw_string_2col( buff, 0, 160, 30, tstr, 207, 2, 31);
		draw_sprite(buff,52,284,11,0);
		}
	else
#endif
		{
		draw_sprite(buff,46,20,24,0);
		draw_string_2col( buff, 2, 320, 56, tstr, 207, 2, 31);
		draw_sprite(buff,46,568,24,0);
		}
}

void	GOAL_SCORED( int team, int scorer, int time )
{
	int	goal_slot	=	-1;
	int	goal_count	=	0;

		// FIND AN EMPTY GOAL SLOT WITHIN GOAL DATA...

		while	( goal_slot==-1 )
		{
					
			if ( goals[goal_count].goal_data.used == NULL )
			{
			 	goal_slot				=	goal_count;
				goals[goal_slot].goal_data.used		=	ACTIVE;				
			}
		     	
			else
		
			{
			goal_count++;
			
				if ( goal_count>90 )
				{
					goals[0].goal_data.used	=	NULL;
					goal_count		=	0;
				}
			}			
		}


	goals[goal_slot].goal_data.time		=	time+1;				

		// CALCULATE NORMAL GOAL....


		if	( (time & O_G)==0 )
		{
			
			if ( scorer<11 )
			{
				goals[goal_slot].goal_data.scorer	=	game_data[setup.team_a].players[scorer].squad_number;	//was scorer-1
				goals[goal_slot].goal_data.used|=		CREDIT_TEAMa;
				match_info.team_a_score++;
			}
			else
			{
				goals[goal_slot].goal_data.scorer	=	game_data[setup.team_b].players[scorer-11].squad_number;   //was scorer-12
				goals[goal_slot].goal_data.used|=		CREDIT_TEAMb;
				match_info.team_b_score++;
			}
		}


		// CALCULATE OWN GOAL....

		if	( (time & O_G)!=0 )
		{

		goals[goal_slot].goal_data.used		=	O_G >> 8;				

			if ( scorer<11 )     
			{
				goals[goal_slot].goal_data.scorer	=	game_data[setup.team_a].players[scorer].squad_number;	     //was scorer-1
				goals[goal_slot].goal_data.used|=		CREDIT_TEAMb;
				match_info.team_b_score++;
			}

			else
			{
				goals[goal_slot].goal_data.scorer	=	game_data[setup.team_b].players[scorer-11].squad_number;     //was scorer-12
				goals[goal_slot].goal_data.used|=		CREDIT_TEAMa;
				match_info.team_a_score++;
			}
		}
}


void
score_breakdown_l(buff_info *buff)
	{
	int g,gc,sf;
	int l;
	int m;
	char tstr[50];

	g=0;
	m=52;
	while (goals[g].goal_data.used)
		{
		if ((goals[g].goal_data.used&5)==4)
			{
			l=0;
			if (!(goals[g].goal_data.time&512))
				{
				for (sf=0;sf<22;sf++)
					if (game_data[setup.team_a].players[sf].squad_number==goals[g].goal_data.scorer) break;
				sprintf(tstr,"%s ",&game_data[setup.team_a].players[sf].name[game_data[setup.team_a].players[sf].goal_index]);
				}
			else
				{
				for (sf=0;sf<22;sf++)
					if (game_data[setup.team_b].players[sf].squad_number==goals[g].goal_data.scorer) break;
				sprintf(tstr,"%s ",&game_data[setup.team_b].players[sf].name[game_data[setup.team_b].players[sf].goal_index]);
				}
			l+=draw_string(buff,0,l+40,m,tstr,157,0);
			gc=g;
			do	{
				if (goals[gc].goal_data.time&768)
					{
					if (l>=50)	if ((m+=8,l=0)>52+5*8) goto mend1;
					if (goals[gc].goal_data.time&256)
						sprintf(tstr,"%d PEN",goals[gc].goal_data.time&255);
					else
						sprintf(tstr,"%d O.G",goals[gc].goal_data.time&255);
					}
				else
					{
					if (l>=78)	if ((m+=8,l=0)>52+5*8) goto mend1;
					sprintf(tstr,"%d",goals[gc].goal_data.time);
					}
				l+=draw_string(buff,0,l+40,m,tstr,157,0);
				goals[gc].goal_data.used|=1;
				do {
					gc++;
					if ((goals[gc].goal_data.used&5)==4 && goals[gc].goal_data.scorer==goals[g].goal_data.scorer && (goals[gc].goal_data.time&512)==(goals[g].goal_data.time&512))
						{
						l+=draw_string(buff,0,l+40,m,",",157,0);
						break;
						}
					} while (goals[gc].goal_data.used);
				} while (goals[gc].goal_data.used);
			if ((m+=8)>52+5*8) goto mend1;
			}
		g++;
		}
	mend1:;
	mhgt=((m+40)>>4)<<4;

	g=0;
	m=52;
	while (goals[g].goal_data.used)
		{
		if ((goals[g].goal_data.used&9)==8)
			{
			l=0;
			if (!(goals[g].goal_data.time&512))
				{
				for (sf=0;sf<22;sf++)
					if (game_data[setup.team_b].players[sf].squad_number==goals[g].goal_data.scorer) break;
				sprintf(tstr,"%s ",&game_data[setup.team_b].players[sf].name[game_data[setup.team_b].players[sf].goal_index]);
				}
			else
				{
				for (sf=0;sf<22;sf++)
					if (game_data[setup.team_a].players[sf].squad_number==goals[g].goal_data.scorer) break;
				sprintf(tstr,"%s ",&game_data[setup.team_a].players[sf].name[game_data[setup.team_a].players[sf].goal_index]);
				}
			l+=draw_string(buff,0,l+178,m,tstr,157,0);
			gc=g;
			do	{
				if (goals[gc].goal_data.time&768)
					{
					if (l>=50)	if ((m+=8,l=0)>52+5*8) goto mend2;
					if (goals[gc].goal_data.time&256)
						sprintf(tstr,"%d PEN",goals[gc].goal_data.time&255);
					else
						sprintf(tstr,"%d O.G",goals[gc].goal_data.time&255);
					}
				else
					{
					if (l>=78)	if ((m+=8,l=0)>52+5*8) goto mend2;
					sprintf(tstr,"%d",goals[gc].goal_data.time);
					}
				l+=draw_string(buff,0,l+178,m,tstr,157,0);
				goals[gc].goal_data.used|=1;
				do {
					gc++;
					if ((goals[gc].goal_data.used&9)==8 && goals[gc].goal_data.scorer==goals[g].goal_data.scorer && (goals[gc].goal_data.time&512)==(goals[g].goal_data.time&512))
						{
						l+=draw_string(buff,0,l+178,m,",",157,0);
						break;
						}
					} while (goals[gc].goal_data.used);
				} while (goals[gc].goal_data.used);
			if ((m+=8)>52+5*8) goto mend2;
			}
		g++;
		}
	mend2:;
	if (mhgt<((m+40)>>4)<<4) mhgt=((m+40)>>4)<<4;

	gc=0;
	while (goals[gc].goal_data.used)
		goals[gc++].goal_data.used&=0xfe;
	}


void
score_breakdown_h(buff_info *buff)
	{
	int g,gc,sf;
	int l;
	int m;
	char tstr[50];

	g=0;
	m=104;
	while (goals[g].goal_data.used)
		{
		if ((goals[g].goal_data.used&5)==4)
			{
			l=0;
			if (!(goals[g].goal_data.time&512))
				{
				for (sf=0;sf<22;sf++)
					if (game_data[setup.team_a].players[sf].squad_number==goals[g].goal_data.scorer) break;
				sprintf(tstr,"%s ",&game_data[setup.team_a].players[sf].name[game_data[setup.team_a].players[sf].goal_index]);
				}
			else
				{
				for (sf=0;sf<22;sf++)
					if (game_data[setup.team_b].players[sf].squad_number==goals[g].goal_data.scorer) break;
				sprintf(tstr,"%s ",&game_data[setup.team_b].players[sf].name[game_data[setup.team_b].players[sf].goal_index]);
				}
			l+=draw_string(buff,2,l+80,m,tstr,157,0);
			gc=g;
			do	{
				if (goals[gc].goal_data.time&768)
					{
					if (l>=120)	if ((m+=13,l=0)>52+6*13) goto mend3;
					if (goals[gc].goal_data.time&256)
						sprintf(tstr,"%d PEN",goals[gc].goal_data.time&255);
					else
						sprintf(tstr,"%d O.G",goals[gc].goal_data.time&255);
					}
				else
					{
					if (l>=176)	if ((m+=13,l=0)>52+6*13) goto mend3;
					sprintf(tstr,"%d",goals[gc].goal_data.time);
					}
				l+=draw_string(buff,2,l+80,m,tstr,157,0);
				goals[gc].goal_data.used|=1;
				do {
					gc++;
					if ((goals[gc].goal_data.used&5)==4 && goals[gc].goal_data.scorer==goals[g].goal_data.scorer && (goals[gc].goal_data.time&512)==(goals[g].goal_data.time&512))
						{
						l+=draw_string(buff,2,l+80,m,",",157,0);
						break;
						}
					} while (goals[gc].goal_data.used);
				} while (goals[gc].goal_data.used);
			if ((m+=13)>104+6*13) goto mend3;
			}
		g++;
		}
	mend3:;
	mhgt=((m+84)>>5)<<4;

	g=0;
	m=104;
	while (goals[g].goal_data.used)
		{
		if ((goals[g].goal_data.used&9)==8)
			{
			l=0;
			if (!(goals[g].goal_data.time&512))
				{
				for (sf=0;sf<22;sf++)
					if (game_data[setup.team_b].players[sf].squad_number==goals[g].goal_data.scorer) break;
				sprintf(tstr,"%s ",&game_data[setup.team_b].players[sf].name[game_data[setup.team_b].players[sf].goal_index]);
				}
			else
				{
				for (sf=0;sf<22;sf++)
					if (game_data[setup.team_a].players[sf].squad_number==goals[g].goal_data.scorer) break;
				sprintf(tstr,"%s ",&game_data[setup.team_a].players[sf].name[game_data[setup.team_a].players[sf].goal_index]);
				}
			l+=draw_string(buff,2,l+356,m,tstr,157,0);
			gc=g;
			do	{
				if (goals[gc].goal_data.time&768)
					{
					if (l>=120)	if ((m+=13,l=0)>104+6*13) goto mend4;
					if (goals[gc].goal_data.time&256)
						sprintf(tstr,"%d PEN",goals[gc].goal_data.time&255);
					else
						sprintf(tstr,"%d O.G",goals[gc].goal_data.time&255);
					}
				else
					{
					if (l>=176)	if ((m+=13,l=0)>104+6*13) goto mend4;
					sprintf(tstr,"%d",goals[gc].goal_data.time);
					}
				l+=draw_string(buff,2,l+356,m,tstr,157,0);
				goals[gc].goal_data.used|=1;
				do {
					gc++;
					if ((goals[gc].goal_data.used&9)==8 && goals[gc].goal_data.scorer==goals[g].goal_data.scorer && (goals[gc].goal_data.time&512)==(goals[g].goal_data.time&512))
						{
						l+=draw_string(buff,2,l+356,m,",",157,0);
						break;
						}
					} while (goals[gc].goal_data.used);
				} while (goals[gc].goal_data.used);
			if ((m+=13)>104+6*13) goto mend4;
			}
		g++;
		}
	mend4:;
	if (mhgt<((m+84)>>5)<<4) mhgt=((m+84)>>5)<<4;

	gc=0;
	while (goals[gc].goal_data.used)
		goals[gc++].goal_data.used&=0xfe;
	}


void 
fulltime_menu(buff_info *buff)
	{
char *p = GetResString( 12);
char tstr[50];

	draw_menu_box(buff);
	
#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30,10,11,0);
		draw_sprite(buff,31,243,11,0);
//		draw_string(buff,1,144,30,lang[match_info.language][12],207,2);
		draw_string(buff,1,144,30,p,207,2);
		draw_string(buff,0,110,42,game_data[setup.team_a].Nickname,31,1);
		sprintf(tstr,"%d",team_a_goals);
		draw_string(buff,0,131,42,tstr,31,1);
		sprintf(tstr,"%d",team_b_goals);
		draw_string(buff,0,157,42,tstr,31,0);
		draw_string(buff,0,178,42,game_data[setup.team_b].Nickname,31,0);
//		draw_sprite(buff,49,134,38,0);
		score_breakdown_l(buff);
		}
	else
#endif
		{
		draw_sprite(buff,19,20,24,0);
		draw_sprite(buff,20,486,24,0);
//		draw_string(buff,2,288,56,lang[match_info.language][12],207,2);
		draw_string(buff,2,288,56,p,207,2);
		draw_string(buff,2,220,80,game_data[setup.team_a].Nickname,31,1);
		sprintf(tstr,"%d",team_a_goals);
		draw_string(buff,2,262,80,tstr,31,1);
		sprintf(tstr,"%d",team_b_goals);
		draw_string(buff,2,314,80,tstr,31,0);
		draw_string(buff,2,356,80,game_data[setup.team_b].Nickname,31,0);
//		draw_sprite(buff,43,268,72,0);
		score_breakdown_h(buff);
		}
	}

void 
extratime_menu(buff_info *buff)
	{
char *p = GetResString( 13);
char tstr[50];

	draw_menu_box(buff);
	
#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30,10,11,0);
		draw_sprite(buff,31,243,11,0);
//		draw_string(buff,1,144,30,lang[match_info.language][13],207,2);
		draw_string(buff,1,144,30,p,207,2);
		draw_string(buff,0,110,42,game_data[setup.team_a].Nickname,31,1);
		sprintf(tstr,"%d",team_a_goals);
		draw_string(buff,0,131,42,tstr,31,1);
		sprintf(tstr,"%d",team_b_goals);
		draw_string(buff,0,157,42,tstr,31,0);
		draw_string(buff,0,178,42,game_data[setup.team_b].Nickname,31,0);
//		draw_sprite(buff,49,134,38,0);
		score_breakdown_l(buff);
		}
	else
#endif
		{
		draw_sprite(buff,19,20,24,0);
		draw_sprite(buff,20,486,24,0);
//		draw_string(buff,2,288,56,lang[match_info.language][13],207,2);
		draw_string(buff,2,288,56,p,207,2);
		draw_string(buff,2,220,80,game_data[setup.team_a].Nickname,31,1);
		sprintf(tstr,"%d",team_a_goals);
		draw_string(buff,2,262,80,tstr,31,1);
		sprintf(tstr,"%d",team_b_goals);
		draw_string(buff,2,314,80,tstr,31,0);
		draw_string(buff,2,356,80,game_data[setup.team_b].Nickname,31,0);
//		draw_sprite(buff,43,268,72,0);
		score_breakdown_h(buff);
		}
	}

void 
halftime_menu(buff_info *buff)
	{
char *p = GetResString( 14);
char tstr[50];

	draw_menu_box(buff);
#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30,10,11,0);
		draw_sprite(buff,31,243,11,0);
//		draw_string(buff,1,144,30,lang[match_info.language][14],207,2);
		draw_string(buff,1,144,30,p,207,2);
		draw_string(buff,0,110,42,game_data[setup.team_a].Nickname,31,1);
		sprintf(tstr,"%d",team_a_goals);
		draw_string(buff,0,131,42,tstr,31,1);
		sprintf(tstr,"%d",team_b_goals);
		draw_string(buff,0,157,42,tstr,31,0);
		draw_string(buff,0,178,42,game_data[setup.team_b].Nickname,31,0);
//		draw_sprite(buff,49,134,38,0);
		score_breakdown_l(buff);
		}
	else
#endif
		{
		draw_sprite(buff,19,20,24,0);
		draw_sprite(buff,20,486,24,0);
//		draw_string(buff,2,288,56,lang[match_info.language][14],207,2);
		draw_string(buff,2,288,56,p,207,2);
		draw_string(buff,2,220,80,game_data[setup.team_a].Nickname,31,1);
		sprintf(tstr,"%d",team_a_goals);
		draw_string(buff,2,262,80,tstr,31,1);
		sprintf(tstr,"%d",team_b_goals);
		draw_string(buff,2,314,80,tstr,31,0);
		draw_string(buff,2,356,80,game_data[setup.team_b].Nickname,31,0);
//		draw_sprite(buff,43,268,72,0);
		score_breakdown_h(buff);
		}
	}


void penalty_shootout_menu(buff_info *buff)
	{
char *p = GetResString( 15);
char tstr[60];
	
	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30,10,11,0);
		draw_sprite(buff,31,243,11,0);
		draw_string(buff,1,144,30,p,207,2);

		draw_string(buff,0,110,42,game_data[setup.team_a].Nickname,31,1);
		sprintf(tstr,"%d",team_a_goals);
		draw_string(buff,0,131,42,tstr,31,1);

		sprintf(tstr,"%2d",team_b_goals);
		draw_string(buff,0,157,42,tstr,31,0);
		draw_string(buff,0,178,42,game_data[setup.team_b].Nickname,31,0);

//		draw_sprite(buff,49,134,38,0);
		score_breakdown_l(buff);
		}
	else
#endif
		{
		draw_sprite(buff,19,20,24,0);
		draw_sprite(buff,20,486,24,0);
//		draw_string(buff,2,288,56,lang[match_info.language][15],207,2);
		draw_string(buff,2,288,56,p,207,2);
		draw_string(buff,2,220,80,game_data[setup.team_a].Nickname,31,1);
		sprintf(tstr,"%d",team_a_goals);
		draw_string(buff,2,262,80,tstr,31,1);
		sprintf(tstr,"%d",team_b_goals);
		draw_string(buff,2,314,80,tstr,31,0);
		draw_string(buff,2,356,80,game_data[setup.team_b].Nickname,31,0);
//		draw_sprite(buff,43,268,72,0);
		score_breakdown_h(buff);
		}
	}


void final_score_menu(buff_info *buff)
	{
char *p = GetResString( 16);
char tstr[50];

	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30,10,11,0);
		draw_sprite(buff,31,243,11,0);
		draw_string(buff,1,144,30,p,207,2);
		draw_string(buff,0,110,42,game_data[setup.team_a].Nickname,31,1);
		sprintf(tstr,"%d",team_a_goals);
		draw_string(buff,0,131,42,tstr,31,1);
		sprintf(tstr,"%d",team_b_goals);
		draw_string(buff,0,157,42,tstr,31,0);
		draw_string(buff,0,178,42,game_data[setup.team_b].Nickname,31,0);
		score_breakdown_l(buff);
		}
	else
#endif
		{
		draw_sprite(buff,19,20,24,0);
		draw_sprite(buff,20,486,24,0);
		draw_string(buff,2,288,56,p,207,2);
		draw_string(buff,2,220,80,game_data[setup.team_a].Nickname,31,1);
		sprintf(tstr,"%d",team_a_goals);
		draw_string(buff,2,262,80,tstr,31,1);
		sprintf(tstr,"%d",team_b_goals);
		draw_string(buff,2,314,80,tstr,31,0);
		draw_string(buff,2,356,80,game_data[setup.team_b].Nickname,31,0);
		score_breakdown_h(buff);
		}
	}

void owngoal_menu(buff_info *buff, int team, int player, int time)
	{
char *p = GetResString( 17);
	int shirt;
	if (!team)
		shirt=ppt[player+((match_half&1)?11:0)].number-1;
	else
		shirt=ppt[player+((match_half&1)?0:11)].number-1;

	draw_menu_box(buff);

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,30+team,10,11,0);
		draw_sprite(buff,2+shirt,16,21,0);
		draw_string(buff,0,160,30,p,207,1);
		draw_string(buff,0,160,30,game_data[!team ? setup.team_a:setup.team_b].players[player].name,31,0);
		draw_sprite(buff,40,278,18,0);
		}
	else
#endif
		{
		draw_sprite(buff,19+team,20,24,0);
		draw_string(buff,2,320,56,p,207,1);
		draw_sprite_s(buff,shirt + (!team ? 53:68),42,51,0);
		draw_string(buff,2,320,56,game_data[!team ? setup.team_a:setup.team_b].players[player].name,31,0);
		draw_sprite(buff,29,556,38,0);
		}
	}

void draw_m_butt(buff_info *buff,int x,int y,char *string,BYTE col) //,char *legend)
{
#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,36,x,y,0);
		draw_string(buff,0,x+23,y+5,string,col,2);
		}
	else
#endif
		{
		draw_sprite(buff,25,x,y,0);
		draw_string(buff,2,x+45,y+8,string,col,2);
		}
}

void draw_l_butt(buff_info *buff,int x,int y,char *string,BYTE col) //,char *legend)
{
#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,35,x,y,0);
		draw_string(buff,0,x+41,y+5,string,col,2);
		}
	else
#endif
		{
		draw_sprite(buff,24,x,y,0);
		draw_string(buff,2,x+82,y+8,string,col,2);
		}
}

void draw_s_butt(buff_info *buff,int x,int y,char *string,BYTE col) // ,char *legend)
{
#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		draw_sprite(buff,37,x,y,0);
		draw_string(buff,0,x+15,y+5,string,col,2);
		}
	else
#endif
		{
		draw_sprite(buff,26,x,y,0);
		draw_string(buff,2,x+29,y+8,string,col,2);
		}
}

struct
	{
	int formation;
	int sub_off,sub_on;
	}option_info;
int sub_c;

extern short replay_seconds;
extern char allow_replay;

int opt_b;
void options_menu(buff_info *buff, int team, int user_no)
	{
char tstr[50];

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
//		if (opt_b!=0) draw_l_butt(buff,62,32,lang[match_info.language][18],(replay_seconds==0||!allow_replay)?2:31,"F1");
//		if (opt_b!=1) draw_l_butt(buff,62,48,lang[match_info.language][19],(!match_info.substitutes||sub_c<2||user_no<=0)?2:31,"F2");
//		if (opt_b!=2) draw_l_butt(buff,62,64,lang[match_info.language][20],user_no<=0?2:31,"F3");
//		if (opt_b!=3) draw_l_butt(buff,62,80,lang[match_info.language][21],31,"F4");
		if (opt_b!=0) draw_l_butt(buff,62,32,GetResString( 18),(replay_seconds==0||!allow_replay)?2:31); // ,"F1");
		if (opt_b!=1) draw_l_butt(buff,62,48,GetResString( 19),(!match_info.substitutes||sub_c<2||user_no<=0)?2:31); //,"F2");
		if (opt_b!=2) draw_l_butt(buff,62,64,GetResString( 20),user_no<=0?2:31); //,"F3");
		if (opt_b!=3) draw_l_butt(buff,62,80,GetResString( 21),31); //,"F4");
		draw_menu_box(buff);
//		if (opt_b==0) draw_l_butt(buff,62,32,lang[match_info.language][18],31,"F1");
//		if (opt_b==1) draw_l_butt(buff,62,48,lang[match_info.language][19],31,"F2");
//		if (opt_b==2) draw_l_butt(buff,62,64,lang[match_info.language][20],31,"F3");
//		if (opt_b==3) draw_l_butt(buff,62,80,lang[match_info.language][21],31,"F4");
		if (opt_b==0) draw_l_butt(buff,62,32,GetResString( 18),31); //,"F1");
		if (opt_b==1) draw_l_butt(buff,62,48,GetResString( 19),31); //,"F2");
		if (opt_b==2) draw_l_butt(buff,62,64,GetResString( 20),31); //,"F3");
		if (opt_b==3) draw_l_butt(buff,62,80,GetResString( 21),31); //,"F4");
		if (user_no>0)
			{
	 		draw_sprite(buff,30+team,18,38,0);
			sprintf(tstr,"PLAYER %d",user_no);
			draw_string(buff,0,36,87,tstr,157,2);
			}
		else
	 		draw_sprite(buff,52,22,45,0);
		}
	else
#endif
		{
//		if (opt_b!=0) draw_l_butt(buff,124,64,lang[match_info.language][18],(replay_seconds==0||!allow_replay)?2:31,"F1");
//		if (opt_b!=1) draw_l_butt(buff,124,96,lang[match_info.language][19],(sub_c<2||user_no<=0)?2:31,"F2");
//		if (opt_b!=2) draw_l_butt(buff,124,128,lang[match_info.language][20],user_no<=0?2:31,"F3");
//		if (opt_b!=3) draw_l_butt(buff,124,160,lang[match_info.language][21],31,"F4");
		if (opt_b!=0) draw_l_butt(buff,124,64,GetResString( 18),(replay_seconds==0||!allow_replay)?2:31); //,"F1");
		if (opt_b!=1) draw_l_butt(buff,124,96,GetResString( 19),(sub_c<2||user_no<=0)?2:31); //,"F2");
		if (opt_b!=2) draw_l_butt(buff,124,128,GetResString( 20),user_no<=0?2:31); //,"F3");
		if (opt_b!=3) draw_l_butt(buff,124,160,GetResString( 21),31); // ,"F4");
		draw_menu_box(buff);
//		if (opt_b==0) draw_l_butt(buff,124,64,lang[match_info.language][18],31,"F1");
//		if (opt_b==1) draw_l_butt(buff,124,96,lang[match_info.language][19],31,"F2");
//		if (opt_b==2) draw_l_butt(buff,124,128,lang[match_info.language][20],31,"F3");
//		if (opt_b==3) draw_l_butt(buff,124,160,lang[match_info.language][21],31,"F4");
		if (opt_b==0) draw_l_butt(buff,124,64,GetResString( 18),31); //,"F1");
		if (opt_b==1) draw_l_butt(buff,124,96,GetResString( 19),31); //,"F2");
		if (opt_b==2) draw_l_butt(buff,124,128,GetResString( 20),31); //,"F3");
		if (opt_b==3) draw_l_butt(buff,124,160,GetResString( 21),31); //,"F4");
		if (user_no>0)
			{
			draw_sprite(buff,19+team,36,76,0);
			sprintf(tstr,"PLAYER %d",user_no);
			draw_string(buff,2,72,174,tstr,157,2);
			}
		else
	 		draw_sprite(buff,46,45,90,0);
		}
	}

char forms[10][6]= { "4=3=3", "5=3=2", "6=3=1", "4=2=4", "5=1=4", "4=0=6", "3=1=6", "4=1=5", "5=2=3", "4=4=2" };

int for_b;
void 
formation_menu(buff_info *buff, int team, int user_no)
	{
char tstr[50];

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		if (for_b!=0) draw_l_butt(buff,62,32,forms[option_info.formation],31); //,"F1");
		if (for_b!=1) draw_l_butt(buff,62,48,GetResString( 22),31); //,"F2");
		if (for_b!=2) draw_l_butt(buff,62,64,GetResString( 21),31); //,"F3");
		draw_menu_box(buff);
		if (for_b==0) draw_l_butt(buff,62,32,forms[option_info.formation],31); //,"F1");
		if (for_b==1) draw_l_butt(buff,62,48,GetResString( 22),31); //,"F2");
		if (for_b==2) draw_l_butt(buff,62,64,GetResString( 21),31); //,"F3");
		draw_sprite(buff,30+team,18,34,0);
		}
	else
#endif
		{
		if (for_b!=0) draw_l_butt(buff,124,64,forms[option_info.formation],31); //,"F1");
		if (for_b!=1) draw_l_butt(buff,124,96,GetResString( 22),31); //,"F2");
		if (for_b!=2) draw_l_butt(buff,124,128,GetResString( 21),31); //,"F3");
		draw_menu_box(buff);
		if (for_b==0) draw_l_butt(buff,124,64,forms[option_info.formation],31); //,"F1");
		if (for_b==1) draw_l_butt(buff,124,96,GetResString( 22),31); //,"F2");
		if (for_b==2) draw_l_butt(buff,124,128,GetResString( 21),31); //,"F3");
		draw_sprite(buff,19+team,36,68,0);
		}
	}

int sub_b;
void substitution_menu(buff_info *buff, int team, int user_no)
	{
char tstr[50];

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		if (sub_b!=0) draw_m_butt(buff,88,32,GetResString( 23),31); //,"F1");
		if (sub_b!=1) draw_l_butt(buff,70,48,GetResString( 24),31); //,"F2");
		if (sub_b!=2) draw_l_butt(buff,70,64,GetResString( 21),31); //,"F3");
		if (sub_b!=3) draw_m_butt(buff,88,80,GetResString( 25),31); //,"F4");
		draw_menu_box(buff);
		if (sub_b==0) draw_m_butt(buff,88,32,GetResString( 23),31); //,"F1");
		if (sub_b==1) draw_l_butt(buff,70,48,GetResString( 24),31); //,"F2");
		if (sub_b==2) draw_l_butt(buff,70,64,GetResString( 21),31); //,"F3");
		if (sub_b==3) draw_m_butt(buff,88,80,GetResString( 25),31); //,"F4");
		draw_sprite(buff,30+team,23,48,0);
		draw_sprite(buff,30+team,166,38,0);
		if (!team)
			{
			draw_string(buff,0,41,37,&game_data[setup.team_a].players[option_info.sub_off].name[game_data[setup.team_a].players[option_info.sub_off].goal_index],157,2);
			draw_sprite(buff,2+option_info.sub_off,29,58,0);
			draw_string(buff,0,183,85,&game_data[setup.team_a].players[option_info.sub_on].name[game_data[setup.team_a].players[option_info.sub_on].goal_index],157,2);
			draw_sprite(buff,2+option_info.sub_on,172,48,0);
			}
		else
			{
			draw_string(buff,0,41,37,&game_data[setup.team_b].players[option_info.sub_off].name[game_data[setup.team_b].players[option_info.sub_off].goal_index],157,2);
			draw_sprite(buff,2+option_info.sub_off,29,58,0);
			draw_string(buff,0,183,85,&game_data[setup.team_b].players[option_info.sub_on].name[game_data[setup.team_b].players[option_info.sub_on].goal_index],157,2);
			draw_sprite(buff,2+option_info.sub_on,172,48,0);
			}
		}
	else
#endif
		{
		if (sub_b!=0) draw_m_butt(buff,176,64,GetResString( 23),31); //,"F1");
		if (sub_b!=1) draw_l_butt(buff,140,96,GetResString( 24),31); //,"F2");
		if (sub_b!=2) draw_l_butt(buff,140,128,GetResString( 21),31); //,"F3");
		if (sub_b!=3) draw_m_butt(buff,176,160,GetResString( 25),31); //,"F4");
		draw_menu_box(buff);
		if (sub_b==0) draw_m_butt(buff,176,64,GetResString( 23),31); //,"F1");
		if (sub_b==1) draw_l_butt(buff,140,96,GetResString( 24),31); //,"F2");
		if (sub_b==2) draw_l_butt(buff,140,128,GetResString( 21),31); //,"F3");
		if (sub_b==3) draw_m_butt(buff,176,160,GetResString( 25),31); //,"F4");
		draw_sprite(buff,19+team,46,96,0);
		draw_sprite(buff,19+team,332,76,0);
		if (!team)
			{
			draw_string(buff,2,81,72,&game_data[setup.team_a].players[option_info.sub_off].name[game_data[setup.team_a].players[option_info.sub_off].goal_index],157,2);
			draw_sprite_s(buff,53+option_info.sub_off,68,123,0);
			draw_string(buff,2,366,168,&game_data[setup.team_a].players[option_info.sub_on].name[game_data[setup.team_a].players[option_info.sub_on].goal_index],157,2);
			draw_sprite_s(buff,53+option_info.sub_on,354,103,0);
			}
		else
			{
			draw_string(buff,2,81,72,&game_data[setup.team_b].players[option_info.sub_off].name[game_data[setup.team_b].players[option_info.sub_off].goal_index],157,2);
			draw_sprite_s(buff,68+option_info.sub_off,68,123,0);
			draw_string(buff,2,366,168,&game_data[setup.team_b].players[option_info.sub_on].name[game_data[setup.team_b].players[option_info.sub_on].goal_index],157,2);
			draw_sprite_s(buff,68+option_info.sub_on,354,103,0);
			}
		}
	}

int rep_b;
int play,speed;
float reppos;

void replay_menu(buff_info *buff, int user_no)
{
char *p = GetResString( 26);
char tstr[50];

#if USERAVEENGINE!=YES
	if (!match_info.res_toggle)
		{
		if (rep_b!=0) 
			{
			draw_s_butt(buff,112,32,"",31); //,"F1");
			draw_sprite_hf(buff,39,121,35,0);
			draw_sprite_hf(buff,39,127,35,0);
			}
		if (rep_b!=1) 
			{
			draw_s_butt(buff,146,32,"",31); //,"F2");
			draw_sprite(buff,39,155,35,0);
			draw_sprite(buff,39,161,35,0);
			}
		if (rep_b!=2) draw_s_butt(buff,185,32,"",31); //,"F3");
		if (!play) draw_sprite(buff,39,197,35,0);
		if (rep_b!=3) draw_m_butt(buff,225,32,p,31); //,"F4");
		draw_menu_box(buff);
		sprintf(tstr,"%.2f",reppos);
		draw_m_butt(buff,17,32,tstr,157); //,NULL);
		sprintf(tstr,"%d",speed);
		draw_s_butt(buff,69,32,tstr,31); //,NULL);
		if (rep_b==0) 
			{
			draw_s_butt(buff,112,32,"",31); //,"F1");
			draw_sprite_hf(buff,39,121,35,0);
			draw_sprite_hf(buff,39,127,35,0);
			}
		if (rep_b==1) 
			{
			draw_s_butt(buff,146,32,"",31); //,"F2");
			draw_sprite(buff,39,155,35,0);
			draw_sprite(buff,39,161,35,0);
			}
		if (rep_b==2) draw_s_butt(buff,185,32,"",31); //,"F3");
		if (play||rep_b==2) draw_sprite(buff,39,197,35,0);
		if (rep_b==3) draw_m_butt(buff,225,32,p,31); //,"F4");
		}
	else
#endif
		{
		if (rep_b!=0) 
			{
			draw_s_butt(buff,224,64,"",31); //,"F1");
			draw_sprite_hf(buff,28,240,70,0);
			draw_sprite_hf(buff,28,252,70,0);
			}
		if (rep_b!=1) 
			{
			draw_s_butt(buff,292,64,"",31); //,"F2");
			draw_sprite(buff,28,310,70,0);
			draw_sprite(buff,28,322,70,0);
			}
		if (rep_b!=2) draw_s_butt(buff,370,64,"",31); //,"F3");
		if (!play) draw_sprite(buff,28,394,70,0);
		if (rep_b!=3) draw_m_butt(buff,447,64,p,31); //,"F4");
		draw_menu_box(buff);
		sprintf(tstr,"%.2f",reppos);
		draw_m_butt(buff,34,64,tstr,157); //,NULL);
		sprintf(tstr,"%d",speed);
		draw_s_butt(buff,138,64,tstr,31); //,NULL);
		if (rep_b==0) 
			{
			draw_s_butt(buff,224,64,"",31); //,"F1");
			draw_sprite_hf(buff,28,240,70,0);
			draw_sprite_hf(buff,28,252,70,0);
			}
		if (rep_b==1) 
			{
			draw_s_butt(buff,292,64,"",31); //,"F2");
			draw_sprite(buff,28,310,70,0);
			draw_sprite(buff,28,322,70,0);
			}
		if (rep_b==2) draw_s_butt(buff,370,64,"",31); // ,"F3");
		if (play||rep_b==2) draw_sprite(buff,28,394,70,0);
		if (rep_b==3) draw_m_butt(buff,447,64,p,31); //,"F4");
		}
}


void do_goal_menu(int team, int player, int time)
	{
	mdata1=team;
	mdata2=player;
	mdata3=time;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=1;
	}

void 
do_booked_menu(int team, int player)
	{
	mdata1=team;
	mdata2=player;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=2;
	}

void 
do_off_menu(int team, int player)
	{
	mdata1=team;
	mdata2=player;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=3;
	}

void 
do_direct_menu(int team)
	{
	mdata1=team;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=4;
	}

void 
do_indirect_menu(int team)
	{
	mdata1=team;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=5;
	}

void 
do_penalty_menu(int team)
	{
	mdata1=team;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=6;
	}

void 
do_corner_menu(int team)
	{
	mdata1=team;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=7;
	}

void 
do_throw_menu(int team)
	{
	mdata1=team;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=8;
	}

void 
do_goalkick_menu(int team)
	{
	mdata1=team;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=9;
	}

void 
do_kickoff_menu(int team)
	{
	mdata1=team;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=10;
	}

void 
do_offside_menu(int team)
	{
	mdata1=team;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=11;
	}

void 
do_injured_menu(int team, int player)
	{
	mdata1=team;
	mdata2=player;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=12;
	}

void 
do_fulltime_menu()
	{
	slco=0;
	slide=oslide=108;
	mcount=240000;
	if (mmenu) rdmenu=1;
	mmenu=13;
	mhgt=128;
	}

void 
do_extratime_menu()
	{
	slco=0;
	slide=oslide=108;
	mcount=240000;
	if (mmenu) rdmenu=1;
	mmenu=14;
	mhgt=128;
	}

void do_halftime_menu()
{
	slco=0;
	slide=oslide=108;
	mcount=240000;
	if (mmenu) rdmenu=1;
	mmenu=15;
	mhgt=128;
}

void 
do_penalty_shootout_menu()
	{
	slco=0;
	slide=oslide=108;
	mcount=340;
	if (mmenu) rdmenu=1;
	mmenu=16;
	mhgt=128;
	}

void 
do_final_score_menu()
	{
	slco=0;
	slide=oslide=108;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=17;
	mhgt=128;
	}

void 
do_owngoal_menu(int team, int player, int time)
	{
	mdata1=team;
	mdata2=player;
	mdata3=time;
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=18;
	}

void 
do_options_menu(int team,int user_no)
	{
	mdata1=team;
	mdata2=user_no+1;
	slco=0;
	slide=oslide=108;
	mcount=1000000;
	if (mmenu) rdmenu=1;
	mmenu=19;
	}

void 
do_formation_menu(int team,int user_no)
	{
	mdata1=team;
	mdata2=user_no+1;
	slco=0;
	slide=oslide=92;
	mcount=1000000;
	if (mmenu) rdmenu=1;
	mmenu=20;
	}

void 
do_substitution_menu(int team,int user_no)
	{
	mdata1=team;
	mdata2=user_no+1;
	slco=0;
	slide=oslide=108;
	mcount=1000000;
	if (mmenu) rdmenu=1;
	mmenu=21;
	}

void 
do_replay_menu(int user_no)
	{
	mdata1=user_no+1;
	slco=0;
	slide=oslide=34;
	mcount=1000000;
	if (mmenu) rdmenu=1;
	mmenu=22;
	}

void 
do_ref_menu()
	{
	slco=0;
	slide=oslide=44;
	mcount=240;
	if (mmenu) rdmenu=1;
	mmenu=23;
	}


struct user_vectors{
	float x;
	float y;
	char m;
	char f;
	};

extern user_vectors users_dir[20];
extern void convert_inputs();
#ifndef LINK
extern void get_user_inputs();
extern void process_user_inputs();
extern volatile int readptr;
extern volatile int writeptr;
#else
extern  void get_user_inputs();
extern  void process_user_inputs();
extern  volatile int readptr;
extern  volatile int writeptr;
#endif

#define RIGHT_DIR 1
#define LEFT_DIR 2
#define DOWN_DIR 4
#define UP_DIR 8
#define FIRE1_DIR 16
#define FIRE2_DIR 32
//#define F1_DIR 64
//#define F2_DIR 128
//#define F3_DIR 256
//#define F4_DIR 512
#define BUTT_WAIT 65
static int count_ur;
static int count_ul;
static int count_ud;
static int count_uu;
static int count_uf1;
static int count_uf2;
static int uf1_p;
static int uf2_p;
static int uf3_p;
static int uf4_p;

int get_dir(int user_no)
{
int out;

	GetKeys(keyMap);
//#ifdef LINK
//	if (user_no>=0)
//		{
////		icth=32767;
////		isth=0;
//		get_user_inputs();
//		process_user_inputs();
//		writeptr++;
//		writeptr&=511;
//		convert_inputs();
//		readptr++;
//		readptr&=511;
//		}
//	else
//#endif
		{
		users_dir[0].x=0;
		users_dir[0].y=0;
		users_dir[0].f=0;
#ifdef LINK
//		if (keys[0xcd]) users_dir[0].x=1;
//		if (keys[0xcb]) users_dir[0].x=-1;
//		if (keys[0xd0]) users_dir[0].y=1;
//		if (keys[0xc8]) users_dir[0].y=-1;
		if (keyMap[3]&0x00000008) 
		  users_dir[0].x=-1;
		if (keyMap[3]&0x00000010)
		  users_dir[0].x=1;
		if (keyMap[3]&0x00000040)
		  users_dir[0].y=-1;
		if (keyMap[3]&0x00000020)
		  users_dir[0].y=1;
#else
//		if (keys[0x4d]) users_dir[0].x=1;
//		if (keys[0x4b]) users_dir[0].x=-1;
//		if (keys[0x50]) users_dir[0].y=1;
//		if (keys[0x48]) users_dir[0].y=-1;
#endif
		if (keyMap[1]&0x10000000) users_dir[0].f=1;
		if (keyMap[2]&0x00100000) users_dir[0].f=1;
		user_no=0;
		}

	out=0;
	if (users_dir[user_no].x>0.1) 
		{if (count>count_ur+50) out|=RIGHT_DIR,count_ur=count+100;}
	else
		if (users_dir[user_no].x<-0.1)
			{if (count>count_ul+50) out|=LEFT_DIR,count_ul=count+100;}
		else
			count_ul=count_ur=0;
	if (users_dir[user_no].y>0.1) 
		{if (count>count_ud+50) out|=DOWN_DIR,count_ud=count+100;}
	else
		if (users_dir[user_no].y<-0.1)
			{if (count>count_uu+50) out|=UP_DIR,count_uu=count+100;}
		else
			count_uu=count_ud=0;
	if (users_dir[user_no].f&1)
			{if (count>count_uf1+50) out|=FIRE1_DIR,count_uf1=count+100;}
		else
			count_uf1=0;
	if (users_dir[user_no].f&2)
			{if (count>count_uf2+50) out|=FIRE2_DIR,count_uf2=count+100;}
		else
			count_uf2=0;
/*
	if (!(keyMap[3]&0x00000004))
	  uf1_p=0; 
	else if (!uf1_p) 
	  out|=F1_DIR|FIRE1_DIR,uf1_p=1;

	if (!(keyMap[3]&0x00000001))
	  uf2_p=0; 
	else if (!uf2_p) 
	  out|=F2_DIR|FIRE1_DIR,uf2_p=1;

	if (!(keyMap[3]&0x08000000))
	  uf3_p=0; 
	else if (!uf3_p) 
	  out|=F3_DIR|FIRE1_DIR,uf3_p=1;

	if (!(keyMap[3]&0x00004000))
	  uf4_p=0;
	else if (!uf4_p) 
	  out|=F4_DIR|FIRE1_DIR,uf4_p=1;
*/

	return(out);
	}


extern void do_replay_render3d(float pos);
extern void setup_replay();
extern void reset_replay();
extern void frame_count();

void
replay(buff_info *buffer,datapt viewx,datapt viewy,datapt viewz,datapt targx,datapt targy,
        datapt targz,datapt vdist,plyrdat *plyrtb,datapt *xyz_ptr,word *ref_ptr,int user_no)
	{
	int dir,repcount;
	float repstep;
	rep_b=3;
	uf1_p=uf2_p=uf3_p=uf4_p=1;
	play=1;
	speed=9;
	reppos=0;
	setup_replay();
	do_replay_menu(user_no);
	do
		{
		render3d(buffer,viewx,viewy,viewz,targx,targy,targz,vdist,plyrtb,xyz_ptr,ref_ptr);
		buffer->dump(0,0);
		frame_count();
		} while (slide>0);
	do
		{
		dir=get_dir(user_no);
		if (dir&LEFT_DIR) rep_b=((rep_b+3)%4);
		if (dir&RIGHT_DIR) rep_b=(++rep_b%4);
		repcount=count;
		do_replay_render3d(reppos);
		buffer->dump(0,0);
		frame_count();
		repstep=(count-repcount)/100.;
		if (dir&(FIRE1_DIR|FIRE2_DIR))
			{
			switch(rep_b)
				{
				case 0:
					if (speed>0) --speed;
					break;
				case 1:
					if (speed<10) ++speed;
					break;
				case 2:
					if (speed)
						play=!play;
					else
						reppos+=repstep*3/9.;
					break;
				case 3:
					goto re_exit;
				}
			}
			if (play) reppos+=repstep*speed/9.;
			if (reppos>=replay_seconds) reppos=0,play=0;
		} while (!(keyMap[1]&0x00002000));
	while ((keyMap[1]&0x00002000)) GetKeys(keyMap);
	re_exit:;
	if (user_no>=0) 
		while (users_dir[user_no].f&3)
			get_dir(user_no);
	reset_replay();
	mcount=1;
	}

extern void	SubstituePlayers(int team_no, int player1, int player2);
extern void	sub_player(short team_no, short player1, short player2);

char sub_pending;
int sub_p_a,sub_p_b;

void
substitution(buff_info *buffer,datapt viewx,datapt viewy,datapt viewz,datapt targx,datapt targy,
        datapt targz,datapt vdist,plyrdat *plyrtb,datapt *xyz_ptr,word *ref_ptr,int team)
	{
	int dir;
	int user_no;
	user_no=(team==0)?sub_p_a:sub_p_b;
	option_info.sub_off=0;
	option_info.sub_on=11;
	while (game_data[(team==0)?setup.team_a:setup.team_b].players[option_info.sub_off].flags&2)
		option_info.sub_off++;
	while (!game_data[(team==0)?setup.team_a:setup.team_b].players[option_info.sub_on].flags&1)
		option_info.sub_on++;
	sub_b=0;
	uf1_p=uf2_p=uf3_p=uf4_p=1;
	do_substitution_menu(team, user_no);
	do
		{
		render3d(buffer,viewx,viewy,viewz,targx,targy,targz,vdist,plyrtb,xyz_ptr,ref_ptr);
		buffer->dump(0,0);
		frame_count();
		} while (slide>0);
	do
		{
		dir=get_dir(user_no);
//		if (dir&F1_DIR) sub_b=0;
//		if (dir&F2_DIR) sub_b=1;
//		if (dir&F3_DIR) sub_b=2;
//		if (dir&F4_DIR) sub_b=3;
		if (dir&UP_DIR) sub_b=(sub_b+3)%4;
		if (dir&DOWN_DIR) sub_b=(++sub_b%4);
		render3d(buffer,viewx,viewy,viewz,targx,targy,targz,vdist,plyrtb,xyz_ptr,ref_ptr);
		buffer->dump(0,0);
		frame_count();
		if (dir&(FIRE1_DIR|FIRE2_DIR))
			{
			switch(sub_b)
				{
				case 0:
					do
						{
						if (dir&FIRE1_DIR) 
						  option_info.sub_off = (++option_info.sub_off)%11;
						else
						  option_info.sub_off = (option_info.sub_off+10)%11;
						} while (game_data[(team==0)?setup.team_a:setup.team_b].players[option_info.sub_off].flags&2);
					break;
				case 1:
					sub_player(team,option_info.sub_off,option_info.sub_on);
					SubstituePlayers((team==0)?setup.team_a:setup.team_b,option_info.sub_off,option_info.sub_on);
					goto su_exit;
				case 2:
					goto su_exit;
				case 3:
					do
						{
						if (dir&FIRE1_DIR) { if (++option_info.sub_on>14) option_info.sub_on=11; }
						else { if (--option_info.sub_on<11) option_info.sub_on=14; }
						} while (!game_data[(team==0)?setup.team_a:setup.team_b].players[option_info.sub_on].flags&1);
					break;
				}
			}
		} while (!(keyMap[1]&0x00002000));
	while ((keyMap[1]&0x00002000)) GetKeys(keyMap);
	su_exit:;
	if (user_no>=0) 
		while (users_dir[user_no].f&3)
			get_dir(user_no);
	sub_pending&=((team==0)?2:1);
	mcount=1;
	}

extern char tactics_a,tactics_b;
extern void load_new_tactics();

void
formation(buff_info *buffer,datapt viewx,datapt viewy,datapt viewz,datapt targx,datapt targy,
        datapt targz,datapt vdist,plyrdat *plyrtb,datapt *xyz_ptr,word *ref_ptr,int user_no,int team)
	{
	int dir;
	int oform=option_info.formation;
	for_b=0;
	uf1_p=uf2_p=uf3_p=uf4_p=1;
	do_formation_menu(team, user_no);
	do
		{
		render3d(buffer,viewx,viewy,viewz,targx,targy,targz,vdist,plyrtb,xyz_ptr,ref_ptr);
		buffer->dump(0,0);
		frame_count();
		} while (slide>0);
	do
		{
		dir=get_dir(user_no);
		if (dir&UP_DIR) for_b=(for_b+2)%3;
		if (dir&DOWN_DIR) for_b=(++for_b)%3;
		render3d(buffer,viewx,viewy,viewz,targx,targy,targz,vdist,plyrtb,xyz_ptr,ref_ptr);
		buffer->dump(0,0);
		frame_count();
		if (dir&(FIRE1_DIR|FIRE2_DIR))
			{
			switch(for_b)
				{
				case 0:
					if (dir&FIRE1_DIR) 
					  option_info.formation = (++option_info.formation)%10;
					else
					  option_info.formation = (option_info.formation+9)%10;
					break;
				case 1:
					if (team==0) tactics_a=option_info.formation;
					else tactics_b=option_info.formation;
					load_new_tactics();
					goto fo_exit;
				case 2:
					option_info.formation=oform;
					goto fo_exit;
				}
			}
		} while (!(keyMap[1]&0x00002000));
	while ((keyMap[1]&0x00002000)) GetKeys(keyMap);
	fo_exit:;
	mcount=1;
	}

void
options(buff_info *buffer,datapt viewx,datapt viewy,datapt viewz,datapt targx,datapt targy,
        datapt targz,datapt vdist,plyrdat *plyrtb,datapt *xyz_ptr,word *ref_ptr,int user_no,int team)
	{
	int dir;
	opt_b=0;
	uf1_p=uf2_p=uf3_p=uf4_p=1;
	if (replay_seconds==0||!allow_replay) opt_b=3;
	if (user_no<0)
		{
		if (replay_seconds==0||!allow_replay) return;
		sub_c=0;
		}
	else
		{
		sub_c=(game_data[(team==0)?setup.team_a:setup.team_b].players[11].flags&1);
		sub_c+=(game_data[(team==0)?setup.team_a:setup.team_b].players[12].flags&1);
		sub_c+=(game_data[(team==0)?setup.team_a:setup.team_b].players[13].flags&1);
		sub_c+=(game_data[(team==0)?setup.team_a:setup.team_b].players[14].flags&1);
		}
	option_info.formation=(team==0)?tactics_a:tactics_b;
	op_loop:
	do_options_menu(team, user_no);
	Input_Activate(false);
	do
		{
		render3d(buffer,viewx,viewy,viewz,targx,targy,targz,vdist,plyrtb,xyz_ptr,ref_ptr);
		buffer->dump(0,0);
		frame_count();
		} while (slide>0);
	do
		{
		dir=get_dir(user_no);
//		if (dir&F1_DIR) if (replay_seconds==0||!allow_replay) dir^=FIRE1_DIR; else opt_b=0;
//		if (dir&F2_DIR) if (!match_info.substitutes||sub_c<2) dir^=FIRE1_DIR; else opt_b=1;
//		if (dir&F3_DIR) if (user_no<0) dir^=FIRE1_DIR; else opt_b=2;
//		if (dir&F4_DIR) opt_b=3;
re_opt:;
		if (dir&UP_DIR) 
			{
			if (user_no<0 && opt_b==3) { opt_b=2;goto re_opt; }
			if ((sub_c<2 || !match_info.substitutes) && opt_b==2 ) { opt_b=1;goto re_opt; }
			if ((replay_seconds==0||!allow_replay) && opt_b==1) { opt_b=0;goto re_opt; }
			opt_b = (opt_b+3)%4;
			}
		if (dir&DOWN_DIR)
			{
			if (user_no<0 && opt_b==1) { opt_b=2;goto re_opt; }
			if ((sub_c<2 || !match_info.substitutes) && opt_b==0) { opt_b=1;goto re_opt; }
			if ((replay_seconds==0||!allow_replay) && opt_b==3) { opt_b=0;goto re_opt; }
			++opt_b%=4;
			}
		render3d(buffer,viewx,viewy,viewz,targx,targy,targz,vdist,plyrtb,xyz_ptr,ref_ptr);
		buffer->dump(0,0);
		frame_count();
		if (dir&(FIRE1_DIR|FIRE2_DIR))
			{
			switch(opt_b)
				{
				case 0:
					replay(buffer,viewx,viewy,viewz,targx,targy,targz,vdist,plyrtb,xyz_ptr,ref_ptr,user_no);
					goto op_exit;
				case 1:
					if (team==0)
						{
						sub_pending^=1;
						sub_p_a=user_no;
						}
					else
						{
						sub_pending^=2;
						sub_p_b=user_no;
						}
//					substitution(buffer,viewx,viewy,viewz,targx,targy,targz,vdist,plyrtb,xyz_ptr,ref_ptr,user_no,team);
  					goto op_exit;
				case 2:
					formation(buffer,viewx,viewy,viewz,targx,targy,targz,vdist,plyrtb,xyz_ptr,ref_ptr,user_no,team);
					if (for_b==2)
						goto op_loop;
					else
						goto op_exit;
				case 3:
					goto op_exit;
				}
			}
		} while (!(keyMap[1]&0x00002000));
	while ((keyMap[1]&0x00002000)) GetKeys(keyMap);
	op_exit:;
	if (user_no>=0) 
		while (users_dir[user_no].f&3)
			get_dir(user_no);
	mcount=1;
	Input_Activate(true);
	}

void adjust_menu()
	{
	menu_buff_l[mmenu-1].buff_hgt=menu_buff_l[mmenu-1].clip_hgt=mhgt;
	menu_buff_l[mmenu-1].buff_start=LORES_buff.buff_start+(LORES_buff.clip_hgt-menu_buff_l[mmenu-1].buff_hgt)*LORES_buff.buff_wid+((LORES_buff.clip_wid-menu_buff_l[mmenu-1].clip_wid)>>1);

	menu_buff_h[mmenu-1].buff_hgt=menu_buff_h[mmenu-1].clip_hgt=mhgt<<1;
	menu_buff_h[mmenu-1].buff_start=HIRES_buff.buff_start+(HIRES_buff.clip_hgt-menu_buff_h[mmenu-1].buff_hgt)*HIRES_buff.buff_wid+((HIRES_buff.clip_wid-menu_buff_h[mmenu-1].clip_wid)>>1);

	if (slide>mhgt-20) slide=oslide=mhgt-20;
	}


//extern int coo;
//float null;
//obj hd;
//ptlist hdp={ 4, -50,0,-50, 50,0,-50, 50,0,50, -50,0,50 };
//facelist hdf={ 1, 4,-X_TM-2000, 0,3,2,1};

static char old_screen_size = 10;


/************************/
/* MAIN DISPLAY ROUTINE */
void render3d(buff_info *buffer,datapt viewx,datapt viewy,datapt viewz,datapt targx,datapt targy,
              datapt targz,datapt vdist,plyrdat *plyrtb,datapt *xyz_ptr,word *ref_ptr)
	{
	short i,j;
	mat tmp;
	obj *thisobj;
	objs *oldobj;
	plyrdat *plyrpt;
	plyrtwdat *plyrpto;
	mcap *mc1,*mc2;
	datapt *tw,*tw1,*tw2;
	floatpt twnstp,pt1,pt2;
	short plyrhi;
	static char vidi_l;
	static short balld;
	short an,ano,xf,xfo;
//	static short kco=0;
	static float hcoo=0;
	static float fcoo=0;
	static char last_vidi_wall_on = false;
	BYTE v,ra,rb,*vrnd;

	rotpt rxa[4],rya[4],rza[4];
	rotpt rxb[4],ryb[4],rzb[4];

	float hr,r;
	float fs,fx,fy;
	float dx1,dy1,dx2,dy2,dz1,dz2;
	float vn,vm,vs1,vs2,vs3,vs4;

	RenderCount++;
	
	if (vidi_anim && buffer==&vidi_buff)
		return;

#if USERAVEENGINE==YES
	match_info.size_toggle = 0;		// Only allow full screen
#else
	if( mmenu && match_info.size_toggle)
		ClearBuffer();
#endif

	if( match_info.size_toggle != old_screen_size)
		{
		LORES_buff.clip_wid = LORES_buffs[match_info.size_toggle].clip_wid;
		LORES_buff.clip_hgt = LORES_buffs[match_info.size_toggle].clip_hgt;
		LORES_buff.scale_x = LORES_buffs[match_info.size_toggle].scale_x;
		LORES_buff.scale_y = LORES_buffs[match_info.size_toggle].scale_y;
		LORES_buff.buff_start = ((LORES_buff.buff_wid-LORES_buff.clip_wid)/2) + (((LORES_buff.buff_hgt-LORES_buff.clip_hgt)/2)*LORES_buff.buff_wid) + ((BYTE *)screenptr);

		HIRES_buff.clip_wid = HIRES_buffs[match_info.size_toggle].clip_wid;
		HIRES_buff.clip_hgt = HIRES_buffs[match_info.size_toggle].clip_hgt;
		HIRES_buff.scale_x = HIRES_buffs[match_info.size_toggle].scale_x;
		HIRES_buff.scale_y = HIRES_buffs[match_info.size_toggle].scale_y;
		HIRES_buff.buff_start = ((HIRES_buff.buff_wid-HIRES_buff.clip_wid)/2) + (((HIRES_buff.buff_hgt-HIRES_buff.clip_hgt)/2)*HIRES_buff.buff_wid) + ((BYTE *)screenptr);
		
		if(match_info.res_toggle)
			main_buff=HIRES_buff;
		else
			main_buff=LORES_buff;

		old_screen_size = setup.screen_size;
		}

	// Set extended buffer info
	render_buff.buff_start=buffer->buff_start;
	render_buff.buff_wid=buffer->buff_wid;
	render_buff.buff_hgt=buffer->buff_hgt;
	render_buff.clip_wid=buffer->clip_wid;
	render_buff.clip_hgt=buffer->clip_hgt;
	render_buff.scale_x=buffer->scale_x;
	render_buff.scale_y=buffer->scale_y;
	render_buff.dump=buffer->dump;
	render_buff.clip_endl=buffer->buff_start+buffer->clip_hgt*buffer->buff_wid;
	render_buff.clip_end=render_buff.clip_endl-buffer->buff_wid;
	render_buff.clip_widl=buffer->clip_wid-1;
	render_buff.clip_hgtl=buffer->clip_hgt-1;
	render_buff.clip_xmid=buffer->clip_wid>>1;
	render_buff.clip_ymid=buffer->clip_hgt>>1;

#if USERAVEENGINE==YES
	RAGE_cur_camx = viewx; RAGE_cur_camz = viewy; RAGE_cur_camy = viewz;
	RAGE_cur_tx = targx; RAGE_cur_tz = targy; RAGE_cur_ty = targz;
	match_info.size_toggle = 0;		// Only allow full screen
	RaveStartFrame();
#else
	if( mmenu && match_info.size_toggle)
		ClearBuffer();
#endif
	
	// Set perspective based info
	oq=vdist;
	q=vdist*((buffer->scale_x+buffer->scale_y)/2);
	qr=(floatpt)1.0/q; 
	qa=(floatpt)q*0.02;
	scrdr=1.0/SCREENDIST;
	persc=q*scrdr;

	// Prepare viewing vector vars
	viewz=-viewz,targz=-targz;
	targx-=viewx;
	targy-=viewy;
	targz-=viewz;

	hr=targx*targx+targz*targz;
	if (hr<1) hr=1,targz=1;
	r=sqrt(hr+targy*targy);
	hr=sqrt(hr);
	cth=targz/hr;sth=targx/hr;
	cph=hr/r;sph=targy/r;
	icth=32767*cth; isth=32767*sth;

	// Make global rotation matrix
	matcop(rot.arr,unit);
	matcop(tran.arr,unit);
	matcop(tmp,unit);
	rot.arr[0][3]=-viewx;
	rot.arr[1][3]=-viewy;
  	rot.arr[2][3]=-viewz;
	tmp[0][0]=cth;
	tmp[0][2]=-sth;				             
	tmp[1][0]=-sth*sph;
	tmp[1][1]=cph;
	tmp[1][2]=-cth*sph;
	tmp[2][0]=sth*cph;			 
	tmp[2][1]=sph;
	tmp[2][2]=cth*cph;
	matmul(rot.arr,tmp);

	clip_tex=NOTEXTURES+1;

	ppt=plyrtb;
	
#ifdef COUNT
	numpols=0;	 
#endif


//// Now render all objects from furthest to nearest
	clearlists

//// GRASS
#if USERAVEENGINE==YES
	if( !setup.detail.pitch)
		{
		addobjnc( &pitch);
//		dispol_pitch();
		}
#else
	ground(hr,targy,viewx,viewy,-viewz);
#endif

//// FAR STANDS
	vn=st_w*rot.e.r22+rot.e.r23;
	vs1=vn*rot.e.r00-(st_w*rot.e.r02+rot.e.r03)*rot.e.r20;
	vn=(-800-st_w)*rot.e.r22+rot.e.r23;
	vs2=vn*rot.e.r00-((-800-st_w)*rot.e.r02+rot.e.r03)*rot.e.r20;

	vn=-st_l*rot.e.r20+rot.e.r23;
	vs3=vn*rot.e.r02-(-st_l*rot.e.r00+rot.e.r03)*rot.e.r22;
	vn=(1280+st_l)*rot.e.r20+rot.e.r23;
	vs4=vn*rot.e.r02-((1280+st_l)*rot.e.r00+rot.e.r03)*rot.e.r22;

	if (setup.stadium<90)
		{
		if (vs3>=0)
			addobjnc(&stad3),numpols+=1000;
		if (vs4<0)
			addobjnc(&stad1),numpols+=1000;
		if (vs1>=0)
			addobjnc(&stad2),numpols+=1000;
		if (vs2<0)
			addobjnc(&stad4),numpols+=1000;
		}
	else
		{
		if (vs1>=0)
			addobjnc(&stad2);
		if (vs2<0)
			addobjnc(&stad4);
		if (vs3>=0)
			addobjnc(&stad3);
		if (vs4<0)
			addobjnc(&stad1);
		}

	dispolsn();

	//// PITCH MARKINGS
	objdepd(&l1,l1_d);
	addobjfc(&l1);
	objdepd(&l2,l2_d);
	addobjfc(&l2);
	objdepd(&l3,l3_d);
	addobjfc(&l3);
	objdepd(&l4,l4_d);
	addobjfc(&l4);
	objdepd(&l5,l5_d);
	addobjfc(&l5);
	objdepd(&l6,l6_d);
	addobjfc(&l6);

	if (setup.detail.lines)
		{
		objdepd(&circle,circle_d);
		addobjfc(&circle);
		objdepd(&semi1,s_circle_d1);
		addobjfc(&semi1);
		objdepd(&semi2,s_circle_d2);
		addobjfc(&semi2);
		}
	addobjfc(&spot1);
	addobjfc(&spot2);
	addobjfc(&spot3);

//// SHADOWS/HIGHLIGHTS

	// Player shadows
	hcoo+=log_factor;
	while (hcoo>=4) hcoo-=4;
	plyrpt=(plyrdat *)plyrtb;
	for (i=0;i<NPLAYERS;i++)
		{
		if (plyrpt->type)
			{
			if (plyrpt->htype||setup.detail.shadows)
				{
				if ((plyrpt->htype!=4 && plyrpt->htype!=6) || hcoo>=2)
					{
					plhilight[plyrpt->htype][plyrpt->hcol].x=plyrpt->x;
  					plhilight[plyrpt->htype][plyrpt->hcol].z=-plyrpt->z;
					if (plyrpt->htype==3 || plyrpt->htype==4)
						{
						plhilight[plyrpt->htype][plyrpt->hcol].crot=plyrpt->crot;
	  					plhilight[plyrpt->htype][plyrpt->hcol].srot=-plyrpt->srot;
		  				addobjyc(&plhilight[plyrpt->htype][plyrpt->hcol]);
						}
					else
	  					addobjfc(&plhilight[plyrpt->htype][plyrpt->hcol]);
					}
				}
			}
		plyrpt++;
		}

	// Ball shadow
	if (!balld) 
		{
		ballshad.x=plyrpt->x;
		ballshad.z=-plyrpt->z;
		}
	else
		{
		ballshad.x=mcap_ballx;
		ballshad.z=mcap_bally;
		}
	addobjfc(&ballshad);

	// Draw lines & shadows in order given (no sort).
	dispolsnx();
							

//// PLAYERS

//	coo++;

	balld=0;
	thisobj=player;
	plyrpt=plyrtb;
	plyrpto=plyrtwtb;
	for (i=0;i<NPLAYERS;i++)
		{

		// Saying something
		if (plyrpt->sprite>=0)
			{
//			*(xyz_ptr+(int)*xyz_ptr*3+1)=plyrpt->x;
//			*(xyz_ptr+(int)*xyz_ptr*3+2)=plyrpt->y+30;
//			*(xyz_ptr+(int)*xyz_ptr*3+3)=-plyrpt->z;
//			*(ref_ptr+(int)*xyz_ptr)=plyrpt->sprite;
//			(*xyz_ptr)++;
			}

		// Is he on?
		if (plyrpt->type)
			{

			// Set object data
			thisobj->x=plyrpt->x;			
			thisobj->y=plyrpt->y;
			thisobj->z=-plyrpt->z;
 			thisobj->crot=-plyrpt->crot;
			thisobj->srot=plyrpt->srot;

			// Tweening
			an=plyrpt->anim;
			ano=plyrpto->anim;
			if (xf=(an&1 && an>=MC_BFOOTBL && an<=MC_TROTF))
				mc1=&mcaps[an-1];
			else
				mc1=&mcaps[an];
			if (an!=ano||plyrpto->tween>=0)
				{
				xfo=(ano&1 && ano>=MC_BFOOTBL && ano<=MC_TROTF);
				if (plyrpto->tween<0)
					{
					plyrpto->tween=0;
					plyrpto->tstep=plyrpt->fstep*2*log_factor;
					plyrpto->animto=an;
					}
				else if (plyrpto->animto!=an)
					{
					tw=(plyrpto->twnfrm=plyrpto->twnpts2)+1;
					tw1=plyrpto->twnpts1+1;
					for (j=PLYRPTS*3;j;j--)
						*(tw++)=*(tw1++);
					plyrpto->tween=0;
					plyrpto->tstep=plyrpt->fstep*2*log_factor;
					plyrpto->anim=ano=plyrpto->animto;
					plyrpto->animto=an;
					xfo=(ano&1 && ano>=MC_BFOOTBL && ano<=MC_TROTF);
					}
				plyrpto->tween+=plyrpto->tstep;
				if (plyrpto->tween>=1)
					{
					plyrpto->tween=-1;
					thisobj->points=player_p[mc1->cappts+(short)(plyrpt->frame*mc1->capfrms)];
					plyrpto->twnfrm=thisobj->points;
					plyrpto->anim=plyrpto->animto;
					}
				else
					{
					tw1=player_p[mc1->cappts+(short)(plyrpt->frame*mc1->capfrms)]+1;
					tw2=plyrpto->twnfrm+1;
					tw=(thisobj->points=plyrpto->twnpts1)+1;
					if ((xf&&!xfo)||(!xf&&xfo))
						{
						pt1=plyrpto->tween;
						pt2=1-pt1;
						tw[0*3]  =tw1[0*3]  *pt1+tw2[0*3]  *pt2;
						tw[0*3+1]=tw1[0*3+1]*pt1+tw2[0*3+1]*pt2;
						tw[0*3+2]=tw1[0*3+2]*pt1-tw2[0*3+2]*pt2;
						tw[1*3]  =tw1[1*3]  *pt1+tw2[1*3]  *pt2;
						tw[1*3+1]=tw1[1*3+1]*pt1+tw2[1*3+1]*pt2;
						tw[1*3+2]=tw1[1*3+2]*pt1-tw2[1*3+2]*pt2;
						tw[2*3]  =tw1[2*3]  *pt1+tw2[2*3]  *pt2;
						tw[2*3+1]=tw1[2*3+1]*pt1+tw2[2*3+1]*pt2;
						tw[2*3+2]=tw1[2*3+2]*pt1-tw2[2*3+2]*pt2;
						tw[3*3]  =tw1[3*3]  *pt1+tw2[3*3]  *pt2;
						tw[3*3+1]=tw1[3*3+1]*pt1+tw2[3*3+1]*pt2;
						tw[3*3+2]=tw1[3*3+2]*pt1-tw2[3*3+2]*pt2;
						tw[4*3]  =tw1[4*3]  *pt1+tw2[4*3]  *pt2;
						tw[4*3+1]=tw1[4*3+1]*pt1+tw2[4*3+1]*pt2;
						tw[4*3+2]=tw1[4*3+2]*pt1-tw2[4*3+2]*pt2;
						tw[5*3]  =tw1[5*3]  *pt1+tw2[8*3]  *pt2;
						tw[5*3+1]=tw1[5*3+1]*pt1+tw2[8*3+1]*pt2;
						tw[5*3+2]=tw1[5*3+2]*pt1-tw2[8*3+2]*pt2;
						tw[6*3]  =tw1[6*3]  *pt1+tw2[9*3]  *pt2;
						tw[6*3+1]=tw1[6*3+1]*pt1+tw2[9*3+1]*pt2;
						tw[6*3+2]=tw1[6*3+2]*pt1-tw2[9*3+2]*pt2;
						tw[7*3]  =tw1[7*3]  *pt1+tw2[10*3]  *pt2;
						tw[7*3+1]=tw1[7*3+1]*pt1+tw2[10*3+1]*pt2;
						tw[7*3+2]=tw1[7*3+2]*pt1-tw2[10*3+2]*pt2;
						tw[8*3]  =tw1[8*3]  *pt1+tw2[5*3]  *pt2;
						tw[8*3+1]=tw1[8*3+1]*pt1+tw2[5*3+1]*pt2;
						tw[8*3+2]=tw1[8*3+2]*pt1-tw2[5*3+2]*pt2;
						tw[9*3]  =tw1[9*3]  *pt1+tw2[6*3]  *pt2;
						tw[9*3+1]=tw1[9*3+1]*pt1+tw2[6*3+1]*pt2;
						tw[9*3+2]=tw1[9*3+2]*pt1-tw2[6*3+2]*pt2;
						tw[10*3]  =tw1[10*3]  *pt1+tw2[7*3]  *pt2;
						tw[10*3+1]=tw1[10*3+1]*pt1+tw2[7*3+1]*pt2;
						tw[10*3+2]=tw1[10*3+2]*pt1-tw2[7*3+2]*pt2;
						tw[11*3]  =tw1[11*3]  *pt1+tw2[12*3]  *pt2;
						tw[11*3+1]=tw1[11*3+1]*pt1+tw2[12*3+1]*pt2;
						tw[11*3+2]=tw1[11*3+2]*pt1-tw2[12*3+2]*pt2;
						tw[12*3]  =tw1[12*3]  *pt1+tw2[11*3]  *pt2;
						tw[12*3+1]=tw1[12*3+1]*pt1+tw2[11*3+1]*pt2;
						tw[12*3+2]=tw1[12*3+2]*pt1-tw2[11*3+2]*pt2;
						tw[13*3]  =tw1[13*3]  *pt1+tw2[17*3]  *pt2;
						tw[13*3+1]=tw1[13*3+1]*pt1+tw2[17*3+1]*pt2;
						tw[13*3+2]=tw1[13*3+2]*pt1-tw2[17*3+2]*pt2;
						tw[14*3]  =tw1[14*3]  *pt1+tw2[18*3]  *pt2;
						tw[14*3+1]=tw1[14*3+1]*pt1+tw2[18*3+1]*pt2;
						tw[14*3+2]=tw1[14*3+2]*pt1-tw2[18*3+2]*pt2;
						tw[15*3]  =tw1[15*3]  *pt1+tw2[19*3]  *pt2;
						tw[15*3+1]=tw1[15*3+1]*pt1+tw2[19*3+1]*pt2;
						tw[15*3+2]=tw1[15*3+2]*pt1-tw2[19*3+2]*pt2;
						tw[17*3]  =tw1[17*3]  *pt1+tw2[13*3]  *pt2;
						tw[17*3+1]=tw1[17*3+1]*pt1+tw2[13*3+1]*pt2;
						tw[17*3+2]=tw1[17*3+2]*pt1-tw2[13*3+2]*pt2;
						tw[18*3]  =tw1[18*3]  *pt1+tw2[14*3]  *pt2;
						tw[18*3+1]=tw1[18*3+1]*pt1+tw2[14*3+1]*pt2;
						tw[18*3+2]=tw1[18*3+2]*pt1-tw2[14*3+2]*pt2;
						tw[19*3]  =tw1[19*3]  *pt1+tw2[15*3]  *pt2;
						tw[19*3+1]=tw1[19*3+1]*pt1+tw2[15*3+1]*pt2;
						tw[19*3+2]=tw1[19*3+2]*pt1-tw2[15*3+2]*pt2;
						tw[16*3]  =tw1[16*3]  *pt1+tw2[20*3]  *pt2;
						tw[16*3+1]=tw1[16*3+1]*pt1+tw2[20*3+1]*pt2;
						tw[16*3+2]=tw1[16*3+2]*pt1-tw2[20*3+2]*pt2;
						tw[20*3]  =tw1[20*3]  *pt1+tw2[16*3]  *pt2;
						tw[20*3+1]=tw1[20*3+1]*pt1+tw2[16*3+1]*pt2;
						tw[20*3+2]=tw1[20*3+2]*pt1-tw2[16*3+2]*pt2;
						tw[21*3]  =tw1[21*3]  *pt1+tw2[22*3]  *pt2;
						tw[21*3+1]=tw1[21*3+1]*pt1+tw2[22*3+1]*pt2;
						tw[21*3+2]=tw1[21*3+2]*pt1-tw2[22*3+2]*pt2;
						tw[22*3]  =tw1[22*3]  *pt1+tw2[21*3]  *pt2;
						tw[22*3+1]=tw1[22*3+1]*pt1+tw2[21*3+1]*pt2;
						tw[22*3+2]=tw1[22*3+2]*pt1-tw2[21*3+2]*pt2;
						if (tw2[23*3+1]<0)
							{
							tw[23*3]  =tw1[23*3];
							tw[23*3+1]=tw1[23*3+1];
							tw[23*3+2]=tw1[23*3+2];
							}
						else if (tw1[23*3+1]<0)
							{
							tw[23*3]  =tw2[23*3];
							tw[23*3+1]=tw2[23*3+1];
							tw[23*3+2]=-tw2[23*3+2];
							}
						else
							{
							tw[23*3]  =tw1[23*3]  *pt1+tw2[23*3]  *pt2;
							tw[23*3+1]=tw1[23*3+1]*pt1+tw2[23*3+1]*pt2;
							tw[23*3+2]=tw1[23*3+2]*pt1-tw2[23*3+2]*pt2;
							}
						tw[24*3]  =tw1[24*3]  *pt1+tw2[25*3]  *pt2;
						tw[24*3+1]=tw1[24*3+1]*pt1+tw2[25*3+1]*pt2;
						tw[24*3+2]=tw1[24*3+2]*pt1-tw2[25*3+2]*pt2;
						tw[25*3]  =tw1[25*3]  *pt1+tw2[24*3]  *pt2;
						tw[25*3+1]=tw1[25*3+1]*pt1+tw2[24*3+1]*pt2;
						tw[25*3+2]=tw1[25*3+2]*pt1-tw2[24*3+2]*pt2;
						tw[26*3]  =tw1[26*3]  *pt1+tw2[27*3]  *pt2;
						tw[26*3+1]=tw1[26*3+1]*pt1+tw2[27*3+1]*pt2;
						tw[26*3+2]=tw1[26*3+2]*pt1-tw2[27*3+2]*pt2;
						tw[27*3]  =tw1[27*3]  *pt1+tw2[26*3]  *pt2;
						tw[27*3+1]=tw1[27*3+1]*pt1+tw2[26*3+1]*pt2;
						tw[27*3+2]=tw1[27*3+2]*pt1-tw2[26*3+2]*pt2;
						}
					else
						{
						pt1=plyrpto->tween;
						pt2=1-pt1;
						for (j=PLYRPTS*3;j;j--)
							*(tw++)=*(tw1++)*pt1+*(tw2++)*pt2;
						}
					}
				}
			else
				// No tween
				{
				thisobj->points=player_p[mc1->cappts+(short)(plyrpt->frame*mc1->capfrms)];
				plyrpto->twnfrm=thisobj->points;
				plyrpto->anim=an;
				}

			// Motion captured ball
			if (thisobj->points[1+23*3+1]>=0)
				{
				ball.y=-thisobj->points[1+23*3];
				if (xf)
					ball.z=-thisobj->points[1+23*3+2];
				else
					ball.z=thisobj->points[1+23*3+2];
				ball.x=-ball.y*thisobj->crot+ball.z*thisobj->srot+thisobj->x;
				ball.z=ball.y*thisobj->srot+ball.z*thisobj->crot+thisobj->z;
				ball.y=thisobj->points[1+23*3+1]+thisobj->y;
				balld=1;

// (ANDYF) Grab motion capture coordinates of ball...
				mcap_ballx=ball.x;
				mcap_bally=ball.z;
				mcap_ballz=ball.y;

				}

			// Wire frame mode
				{

				// Is he a goalie
				if (xf)
					if (i==0||i==11)
						{ thisobj->faces=playertypes[(plyrpt->type==1||1/*!setup.M8*/)?10:20]; }
					else
#if ALLOW_8MEG
						if (game_data[plyrpt->type==1?setup.team_a:setup.team_b].players[i%11].skin_tone) // &&setup.M8)
							thisobj->faces=playertypes[(short)plyrpt->type+15];
						else
#endif
							thisobj->faces=playertypes[(short)plyrpt->type+5];
				else
					if (i==0||i==11)
						{ thisobj->faces=playertypes[(plyrpt->type==1||1/*!setup.M8*/)?5:15]; }
					else
#if ALLOW_8MEG
						if (game_data[plyrpt->type==1?setup.team_a:setup.team_b].players[i%11].skin_tone) //&&setup.M8)
							thisobj->faces=playertypes[(short)plyrpt->type+10];
						else
#endif
							thisobj->faces=playertypes[(short)plyrpt->type];
				if (thisobj->faces==player_fl&&1/*!setup.M8*/) 
					thisobj->faces=player_fr;

				// Player number
	 			if (i<22)
					if (setup.detail.players)
						{
						thisobj->faces[0]=13;
						if (plyrpt->type==1)
							thisobj->faces[82]=-X_TM-2016-(plyrpt->number-1);
						else
							thisobj->faces[82]=-X_TM-2016-15-(plyrpt->number-1);
						}
					else
						thisobj->faces[0]=12;
				}

//plyrpt->frame=modf(plyrpt->frame+(float)1/mc1->capfrms,&null);
  
			// Draw object
			if (objdep(*thisobj)>15)
				{
				oldobj=curobj;
				if (xf)
					addobjyf(thisobj);
				else
					addobjy(thisobj);
				if (oldobj!=curobj) sortobj(oldobj);
				}
			thisobj++;
			}
		plyrpt++;
		plyrpto++;
		}						


//// BALL

	if (!balld)
		{
		ball.x=plyrpt->x;
		ball.y=plyrpt->y;
		ball.z=-plyrpt->z;
		}
	ball.crot=plyrpt->crot;
	ball.srot=plyrpt->srot;
	objdepd(&ball,ball_d);
	addobjxy(&ball,plyrpt->frame,plyrpt->fstep);
	plyrpt++;



//// GOALS

	oldobj=curobj;
	objdepd(&goal1_1,goal1_a);
	addobjnc(&goal1_1);
	if (oldobj!=curobj) {sortobj(oldobj);oldobj++;}
	objdepd(&goal2_1,goal2_a);
	addobjnc(&goal2_1);
	if (oldobj!=curobj) {sortobj(oldobj);oldobj++;}
	objdepd(&goal3_1,goal3_a);
	addobjnc(&goal3_1);
	if (oldobj!=curobj) {sortobj(oldobj);oldobj++;}
	objdepd(&goal4_1,goal4_a);
	addobjnc(&goal4_1);

	oldobj=curobj;
	objdepd(&goal1_2,goal1_b);
	addobjnc(&goal1_2);
	if (oldobj!=curobj) {sortobj(oldobj);oldobj++;}
	objdepd(&goal2_2,goal2_b);
	addobjnc(&goal2_2);
	if (oldobj!=curobj) {sortobj(oldobj);oldobj++;}
	objdepd(&goal3_2,goal3_b);
	addobjnc(&goal3_2);
	if (oldobj!=curobj) {sortobj(oldobj);oldobj++;}
	objdepd(&goal4_2,goal4_b);
	addobjnc(&goal4_2);


//// FLAGS

	if (setup.detail.lines)
		{
		oldobj=curobj;
		addobjnc(&flag_1);
		if (oldobj!=curobj) {sortobj(oldobj);oldobj++;}
		addobjnc(&flag_2);
		if (oldobj!=curobj) {sortobj(oldobj);oldobj++;}
		addobjnc(&flag_3);
		if (oldobj!=curobj) {sortobj(oldobj);oldobj++;}
		addobjnc(&flag_4);
		if (oldobj!=curobj) {sortobj(oldobj);oldobj++;}
	}


//// SPRITES

	if (buffer==&main_buff)
		addobjsp(xyz_ptr,ref_ptr);		
	

// Sort and render players, ball, goals and sprites.
	dispols();

//// SCANNER

	if (Scanner && buffer!=&vidi_buff && !mmenu)
#if USERAVEENGINE!=YES
		if (!match_info.res_toggle)
			{
			scrpt scanner_l[]={8,56, 44,56, 44,8, 8,8};
			polyf((pnt *)scanner_l,4,filters[0]);
			draw_sprite(buffer,88,9,buffer->clip_hgt-55,0);
			plyrpt=(plyrdat *)plyrtb;
			for (i=0;i<22;i++)
				{
				if (plyrpt->type)
					draw_sprite(buffer,89+(((i<11&&(!(match_half&1)))||(i>=11&&(match_half&1)))?0:2),8+(plyrpt->z*34)/800,buffer->clip_hgt-10-(plyrpt->x*46)/1280,0);
				plyrpt++;					  
				}
			plyrpt+=3;
 			draw_sprite(buffer,89,8+(plyrpt->z*34)/800,buffer->clip_hgt-10-(ball.x*46)/1280,31);
			}
		else
#endif
			{
			scrpt scanner_h[]={16,112, 88,112, 88,16, 16,16};
			polyf((pnt *)scanner_h,4,filters[0]);
			draw_sprite_d(buffer,88,18,buffer->clip_hgt-110,0);
			plyrpt=(plyrdat *)plyrtb;
			for (i=0;i<22;i++)
				{
				if (plyrpt->type)
					draw_sprite(buffer,90+(((i<11&&(!(match_half&1)))||(i>=11&&(match_half&1)))?0:2),15+2*(plyrpt->z*34)/800,buffer->clip_hgt-21-2*(plyrpt->x*46)/1280,0);
				plyrpt++;
				}
			plyrpt+=3;
			draw_sprite(buffer,89,16+2*(plyrpt->z*34)/800,buffer->clip_hgt-20-2*(ball.x*46)/1280,31);
			}


//// ANIMATE TEXTURES

	if (animdata && vidi_anim) 
		if (spool)
			switch(vidi_anim)
				{
				case 1: 
					do_extra_time_anim(spool); 
					break;
				case 2: 
					do_foul_anim(spool); 
					break;
				case 3: 
					do_full_time_anim(spool);
					break;
				case 4: 
					do_half_time_anim(spool); 
					break;
				case 5: 
					do_penalty_anim(spool); 
					break;
				case 6: 
					do_goal_anim(spool); 
					break;
				case 7: 
					do_win_f_anim(spool); 
					break;
				case 9: 
					do_win_g_anim(spool); 
					break;
				case 11: 
					do_win_s_anim(spool); 
					break;
				}
		else
			do_anim(vidi);
			
	if (wind_on)
		{
		fcoo+=log_factor;
		if (fcoo>=(128-wind_speed)/10)
			{
			if (flag_f[1+5*6+1]==-X_TM-2048)
				flag_f[1+5*6+1]=flag_f[1+5*6+6]=-X_TM-2046;
			else
				flag_f[1+5*6+1]=--flag_f[1+5*6+6];
			fcoo=0;
			}
		fs=(PI/2)*(wind_speed+16)/144;
		fx=8.5*sin(fs)+1;
		fy=8.5*cos(fs);
		flag_p[1+8*3]=fx*wind_x;
		flag_p[1+8*3+1]=15-fy;
		flag_p[1+8*3+2]=fx*wind_y;
		}
	else
		{
		flag_p[1+8*3]=1.751;
		flag_p[1+8*3+1]=6.629;
		flag_p[1+8*3+2]=1.751;
		}


	if (mmenu&&buffer!=&vidi_buff)
		{
		slco+=log_factor*5;
		if (mcount>0) 
			if (slide>0) while (slco>=1&&slide>0) slco--,slide--;
			else while (slco>=1) slco--,mcount--;
		else
			{
			while (slco>=1&&slide<oslide) slco--,slide++;
			if (slide>=oslide) mmenu=0,rdmenu=1;
			}

		if (!match_info.res_toggle)
			{
			menu_buff.buff_start=menu_buff_l[mmenu-1].buff_start+LORES_buff.buff_wid*(slide+12); //+(mmenu==22?10:0));
			menu_buff.buff_wid=menu_buff_l[mmenu-1].buff_wid;
			menu_buff.buff_hgt=menu_buff_l[mmenu-1].buff_hgt;
			menu_buff.clip_wid=menu_buff_l[mmenu-1].clip_wid;
			menu_buff.clip_hgt=menu_buff_l[mmenu-1].clip_hgt-(slide+12);
		  }
		else
			{
			menu_buff.buff_start=menu_buff_h[mmenu-1].buff_start+HIRES_buff.buff_wid*((slide+12)<<1); //(mmenu==22?10:0))<<1);
			menu_buff.buff_wid=menu_buff_h[mmenu-1].buff_wid;
			menu_buff.buff_hgt=menu_buff_h[mmenu-1].buff_hgt;
			menu_buff.clip_wid=menu_buff_h[mmenu-1].clip_wid;
			menu_buff.clip_hgt=menu_buff_h[mmenu-1].clip_hgt-((slide+12)<<1);
			}

		switch (mmenu)
			{
			case 1:	
				goal_menu(&menu_buff, mdata1, mdata2, mdata3);
				break;
			case 2:	
				booked_menu(&menu_buff, mdata1, mdata2);
				break;
			case 3:	
				off_menu(&menu_buff, mdata1, mdata2);
				break;
			case 4:	
				direct_menu(&menu_buff, mdata1);
				break;
			case 5:	
				indirect_menu(&menu_buff, mdata1);
				break;
			case 6:	
				penalty_menu(&menu_buff, mdata1);
				break;
			case 7:	
				corner_menu(&menu_buff, mdata1);
				break;
			case 8:	
				throw_menu(&menu_buff, mdata1);
				break;
			case 9:	
				goalkick_menu(&menu_buff, mdata1);
				break;
			case 10:	
				kickoff_menu(&menu_buff, mdata1);
				break;
			case 11:	
				offside_menu(&menu_buff, mdata1);
				break;
			case 12:	
				injured_menu(&menu_buff, mdata1, mdata2);
				break;
			case 13:	
				fulltime_menu(&menu_buff);
				adjust_menu();
				break;
			case 14:	
				extratime_menu(&menu_buff);
				adjust_menu();
				break;
			case 15:
				halftime_menu(&menu_buff);
				adjust_menu();
				break;
			case 16:	
				penalty_shootout_menu(&menu_buff);
				adjust_menu();
				break;
			case 17:	
				final_score_menu(&menu_buff);
				adjust_menu();
				break;
			case 18:	
				owngoal_menu(&menu_buff, mdata1, mdata2, mdata3);
				break;
			case 19:	
				options_menu(&menu_buff, mdata1, mdata2);
				break;
			case 20:	
				formation_menu(&menu_buff, mdata1, mdata2);
				break;
			case 21:	
				substitution_menu(&menu_buff, mdata1, mdata2);
				break;
			case 22:	
				replay_menu(&menu_buff, mdata1);
				break;
			case 23:	
				ref_menu(&menu_buff);
				break;
			}
		}

	if( last_vidi_wall_on != match_info.vidi)
		{
		if( !match_info.vidi)
			stop_anim( vidi);
		last_vidi_wall_on = match_info.vidi;
		}

//	if (key_togs[0x2f])
//		vidi_l=1;
//	else
//		{
//		if (vidi_l)
//			stop_anim(vidi);
//		vidi_l=0;
//		}

	if (buffer!=&vidi_buff)
		{
char tstr[50];
int mid=buffer->clip_wid>>1;
int end=buffer->clip_hgt;

		switch(mmenu)
			{
			case 19:	strcpy(tstr,GetResString( 27)); 
						break;
			case 20:	strcpy(tstr,GetResString( 20));
						break;
			case 21:	strcpy(tstr,GetResString( 19));
						break;
			case 22:	strcpy(tstr,GetResString( 18));
						break;
			default:	if(paused)
							strcpy(tstr, "PAUSED");
						else
							sprintf(tstr,"%2d:%02d",match_time.min,(int)match_time.secs);
						break;
			}
#if USERAVEENGINE!=YES
		if (!match_info.res_toggle)
			{
			draw_string(buffer,1,mid,1,tstr,31,2);
			if (!mmenu)
				{
	//			draw_string(&LORES_buff[0],end-8,140,0,game_data[setup.team_a].Nickname,31,1);
				i=draw_string(buffer,1,mid-20,end-8,game_data[setup.team_a].Nickname,32,1);
				if (sub_pending&1) draw_sprite(buffer,84,mid-20-i-16,end-16,0);
				sprintf(tstr,"%d=%d",team_a_goals,team_b_goals);
				draw_string(buffer,1,mid,end-8,tstr,31,2);
				i=draw_string(buffer,1,mid+20,end-8,game_data[setup.team_b].Nickname,56,0);
				if (sub_pending&2) draw_sprite(buffer,85,mid+20+i+2,end-16,0);
				}
			}
		else
#endif
			{							  
			draw_string(buffer,2,mid,1,tstr,31,2);
			if (!mmenu)
				{
	//			draw_string(&VESA_buff[0][0],end-14,mid-40,0,game_data[setup.team_a].Nickname,31,1);
				i=draw_string(buffer,2,mid-40,end-14,game_data[setup.team_a].Nickname,32,1);
				if (sub_pending&1) draw_sprite(buffer,86,mid-40-i-20,end-20,0);
				sprintf(tstr,"%d=%d",team_a_goals,team_b_goals);
				draw_string(buffer,2,mid,end-14,tstr,31,2);
				i=draw_string(buffer,2,mid+40,end-14,game_data[setup.team_b].Nickname,56,0);
				if (sub_pending&2) draw_sprite(buffer,87,mid+40+i+4,end-20,0);
				}
			}
		}
}



/************************/
/* BUFFER DUMP ROUTINES */
// Dump main buffer to MCGA 
void dumpScreen(scrpt dispx,scrpt dispy)
{
#if USERAVEENGINE==YES
	RaveEndFrame();
#else
	BlitBufferToScreen((double *)screenptr);
#endif
	RenderCount--;
}

// Null dump routine
void dumpnull(scrpt dispx,scrpt dispy) {}

// Dump vidi buffer to map page through filter (already there)
void dumpvidi1(scrpt dispx,scrpt dispy)
	{
	BYTE *scrpt;
	short i,j;
	if (vidi_anim) return;
	scrpt=render_buff.buff_start;
	for (i=0;i<render_buff.clip_hgt;i++)
		{
		for (j=0;j<render_buff.clip_wid;j++)
			{
			*scrpt=filters[1][*scrpt];
			scrpt++;
			}
		scrpt+=render_buff.buff_wid-render_buff.clip_wid;
		}
	}



/********************************/
/* INITIALISE DISPLAY & OBJECTS */

// Object 'constructor' function
void initobj(obj *object,ptlist points,facelist faces,datapt x,datapt y,datapt z,floatpt crot,floatpt srot)
	{
	object->points=points;
	object->faces=faces;
	object->x=x;
	object->y=y;
	object->z=z;
	object->crot=crot;
	object->srot=srot;
	};

static char Filename[]={"Dump0000.TGA"};

void DumpTga( void *buffer, int x, int y)
{
char start[] = {0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x18, 0x00, 0x00, 0x00, 0x00
			   ,0x00, 0x01		// Width
			   ,0x00, 0x01		// Height
			   ,0x08, 0x20
			   };
char *p;
char thispal[768];
FILE *fp;
char z;

	start[12] = x&255; start[13] = x>>8;
	start[14] = y&255; start[15] = y>>8;
	
		fp = fopen( Filename, "wb+");
		p = (char *)&Filename[7];
		while( ++(*p)==':')
			{
			*p = '0';
			p--;
			}
		fwrite( &start, 1, sizeof(start), fp);

		for( int a=0;a<256;a++)
			{
			thispal[a*3] = palette_buffer[a*3+2]<<2;
			thispal[a*3+1] = palette_buffer[a*3+1]<<2;
			thispal[a*3+2] = palette_buffer[a*3]<<2;
			}

		fwrite( &thispal, 1, 768, fp);
		fwrite( buffer, x, y, fp);
		fclose(fp);
}

void DumpAllMaps()
{
	for( int a=0; a<MAPPAGES_4; a++)
	DumpTga( maps[a], 256, 256);
}

int init3d_gremlin()
	{
	int i,j,mc_tot,skyno;
	word selector;
	BYTE *seg;
	BYTE *tptr,*tptr2;
	word *stp;
	dword *tlptr;

	old_screen_size = 10;
	
	HIRES_buff.buff_start  = LORES_buff.buff_start  = (BYTE *)screenptr;
	vidi_buff_1.buff_start = vidi_buff_2.buff_start = (BYTE *)screenptr;

	if(!match_info.res_toggle)
	  {
#if USERAVEENGINE==YES
	  BlitterMode = DOUBLE_320240;
#else
	  BlitterMode = DOUBLE_320200;
#endif
	  main_buff = LORES_buff;
	  }
	else
	  {
#if USERAVEENGINE==YES
	  BlitterMode = REAL_640480;
#else
	  BlitterMode = REAL_640400;
#endif
	  main_buff = HIRES_buff;
	  }
	
	for (i=0;i<MAPPAGES+noloop+2;i++)
		mapsel[i]=NULL;

	// Free memory?
//	if (setup.verbose) printf("Free memory = %d\n",get_mem_info());

	// Malloc data areas
//	if (setup.verbose) puts("Preparing memory areas...");
#if ALLOW_8MEG
	if (setup.M8)
		mcaps=mcaps8;
	else
#endif
		mcaps=mcaps4;

	mc_tot=0;
	for (i=0;i<MC_NO;i++)
		{
		mcaps[i].cappts=mc_tot;
		mc_tot+=mcaps[i].capfrms;
		}
		
	maps=NULL;
	sky=NULL,textures=NULL,filters=NULL,player_p=NULL,plyrtwtb=NULL;
	objlist=NULL,pollist=NULL,ptslist=NULL,rotlist=NULL,animdata=NULL;
	
//	if (setup.verbose) printf("Motion capture frames: %d\n",mc_tot);
#if ALLOW_8MEG
	if (setup.M8)
		maps = (mappage *)MallocX((mappages=MAPPAGES)*65536);
	else
#endif
		maps = (mappage *)MallocX( (mappages=MAPPAGES_4)*65536);
		
	textures = (texture *)MallocX((NOTEXTURES+CLIP_TEXTURES+1)*sizeof(texture));
	filters=(filter *)MallocX(FILTERS*sizeof(filter));
	player_p=(capfrm *)MallocX(mc_tot*sizeof(capfrm));
	plyrtwtb=(plyrtwdat *)MallocX(NPLAYERS*sizeof(plyrtwdat));
	objlist= (objs *)MallocX(MAXOBJ*sizeof(objs));
	pollist=(pols *)MallocX(MAXPOL*sizeof(pols));
	ptslist=(scrpt *)MallocX(MAXPOL*10*sizeof(scrpt));
	rotlist=(rotpt *)MallocX(MAXPTS*4*sizeof(rotpt));
	animdata =  (char *)MallocX(ANIM_SPACE);
#if USERAVEENGINE==YES
	sky =  (unsigned char *)MallocX( 640*480*2);
#else
	sky =  (unsigned char *)MallocX( 640*480);
#endif

	if( !textures || !filters || !player_p || !plyrtwtb || !objlist || !pollist || !ptslist || !rotlist)
	  goto init3d_error;

// Load texture maps and other files
//	if (setup.verbose) puts("Reading data files...");
// 	if (readrawfile(dataoffs,(BYTE *)load_offsets) == NULL) goto init3d_error;
//	if (opendatafile(datafile) < 0 ) goto init3d_error;

	UseResFile(dataResFile2); resType = RESTYPE_ACTR;

	if (setup.detail.stadia||network_on) 
		{
		if (network_on)
			{
			setup.detail.stadia=1;
			setup.stadium+=45;
			}
		else
			if (stadlist[setup.stadium+90].tmdfile==-1 || performance<=18)
				setup.stadium+=45;
			else
				setup.stadium+=90;
		}


#ifdef LINK
	skyno=stadlist[setup.stadium].skytypes[(count&1)?1:0];
#else
	skyno=stadlist[setup.stadium].skytypes[(rand()&1)?1:0];
#endif

	if (
	 	readfile(TMD_TEXDATA,(BYTE *)&textures[1]) ||
	   	readfile(teamlist[setup.team_a].headfile,maps[0]) ||
	   	readfile(teamlist[setup.team_b].headfile,maps[0]+128*256) ||
	   	readfile(teamlist[setup.team_a].torsofile,maps[1]) ||
	   	readfile(teamlist[setup.team_b_kit].torsofile,maps[2]) ||
	   	readfile(teamlist[setup.team_a].limbsfile,maps[3]) ||
	   	readfile(teamlist[setup.team_b_kit].limbsfile,maps[3]+80*256) ||
	   	readfile(BM_FEET,maps[3]+158*256) ||
	   	readfile(BM_REFKPTOR,maps[4]) ||
	   	readfile(BM_EXTRA1,maps[X_BM]) ||
		readfile(teamlist[setup.team_a].nosfile,maps[X_BM]+62*256) ||
	   	readfile(BM_EXTRA3,maps[X_BM+1]) ||
		readfile(stadlist[setup.stadium].sb1file,maps[S_BM]) ||
		readfile(stadlist[setup.stadium].sb2file,maps[S_BM+1]) ||
		
	   	readfile(PAL_FOOTY,palette_buffer) ||
	   	readfile(teamlist[setup.team_a].palfile,&palette_buffer[3*32]) ||
	   	readfile(teamlist[setup.team_b_kit].palfile,&palette_buffer[3*56]) ||
	   	readfile(teamlist[setup.team_a].skinfile,&palette_buffer[3*80]) ||
	   	readfile(teamlist[setup.team_b].skinfile,&palette_buffer[3*88]) ||
	   	readfile(stadlist[setup.stadium].pitchpfile,&palette_buffer[3*128]) ||
	   	readfile(teamlist[setup.team_a].homepfile,&palette_buffer[3*224]) ||
	   	readfile(teamlist[setup.team_b].awaypfile,&palette_buffer[3*232])
		)
 	  goto init3d_error;

	if( sky)
		{
#if USERAVEENGINE==YES
			UseResFile(dataResRave);
			LoadPictureToMemory( 128+skyno, (char *)sky);
			UseResFile(dataResFile2);
#else
			if( readfile(skylist[skyno].skypfile,&palette_buffer[3*208]) ||
				readfile(skylist[skyno].skyfile,sky))
			goto init3d_error;
#endif
		}

#if ALLOW_8MEG
	if (setup.M8)
	{
		if (
			readfile(teamlist[setup.team_a].torsofile,maps[S_BM+2]) ||
			readfile(teamlist[setup.team_b_kit].torsofile,maps[S_BM+3]) ||
		   	readfile(BM_XLIMBS,maps[S_BM+4]) ||
		   	readfile(teamlist[setup.team_a].limbsfile,maps[S_BM+4]) ||
		   	readfile(teamlist[setup.team_b_kit].limbsfile,maps[S_BM+4]+80*256) ||
		   	readfile(BM_XRFKPLIM,maps[S_BM+5]) ||
		   	readfile(BM_REFKPTOR,maps[S_BM+6]) ||
			)
 		  goto init3d_error;

		int st_a0;
		int st_b0_kit;
//		st_a0=14;//setup.team_a?36:14;
//		st_b0_kit=36;//setup.team_b_kit?36:14;
		st_a0=setup.team_a;
		st_b0_kit=setup.team_b_kit;

		switch(st_a0/42)
			{
			case 0: if (readfile(BM_KGRIDS1,maps[5])) 
					  goto init3d_error;
			case 1: if (readfile(BM_KGRIDS2,maps[5])) 
					  goto init3d_error;
			}
		j=st_a0%42;
		tptr=maps[5]+((j/7)*42+1)*256+(j%7)*36+1;
		tptr2=maps[X_BM+1]+111*256+220;
		for (i=0;i<40;i++)
			{
			memcpy(tptr2,tptr,35);
			tptr+=256;
			tptr2+=256;
			}
		switch(st_b0_kit/42)
			{
			case 0: if (readfile(BM_KGRIDS1,maps[5]))
					  goto init3d_error;
			case 1: if (readfile(BM_KGRIDS2,maps[5]))
					  goto init3d_error;
			}
		j=st_b0_kit%42;
		tptr=maps[5]+((j/7)*42+1)*256+(j%7)*36+1;
		tptr2=maps[X_BM+1]+151*256+220;
		for (i=0;i<40;i++)
			{
			memcpy(tptr2,tptr,35);
			tptr+=256;
			tptr2+=256;
			}
/*
		switch(st_a0/9)
			{
			case 0: if (readfile(BM_KGRID1,maps[5])) 
					  goto init3d_error;
			case 1: if (readfile(BM_KGRID2,maps[5]))
					  goto init3d_error;
			case 2: if (readfile(BM_KGRID3,maps[5]))
					  goto init3d_error;
			case 3: if (readfile(BM_KGRID4,maps[5]))
					  goto init3d_error;
			case 4: if (readfile(BM_KGRID5,maps[5]))
					  goto init3d_error;
			case 5: if (readfile(BM_KGRID6,maps[5]))
					  goto init3d_error;
			}
*/
		if (readfile(bm_kgrid[st_a0/9],maps[5])) 
		  goto init3d_error;

		j=st_a0%9;
		tptr=maps[5]+((j/3)*83+2)*256+(j%3)*70+1;
		tptr2=maps[X_BM+1];
		for (i=0;i<79;i++)
			{
			memcpy(tptr2,tptr,69);
			tptr+=256;
			tptr2+=256;
			}
/*
		switch(st_b0_kit/9)
			{
			case 0: if (readfile(BM_KGRID1,maps[5]))
					  goto init3d_error;
			case 1: if (readfile(BM_KGRID2,maps[5]))
					  goto init3d_error;
			case 2: if (readfile(BM_KGRID3,maps[5]))
					  goto init3d_error;
			case 3: if (readfile(BM_KGRID4,maps[5]))
					  goto init3d_error;
			case 4: if (readfile(BM_KGRID5,maps[5]))
					  goto init3d_error;
			case 5: if (readfile(BM_KGRID6,maps[5]))
					  goto init3d_error;
			}
*/
		if (readfile(bm_kgrid[st_b0_kit/9],maps[5])) 
		  goto init3d_error;

		j=st_b0_kit%9;
		tptr=maps[5]+((j/3)*83+2)*256+(j%3)*70+1;
		tptr2=maps[X_BM+1]+69;
		for (i=0;i<79;i++)
			{
			memcpy(tptr2,tptr,69);
			tptr+=256;
			tptr2+=256;
			}
    	if (readfile(BM_REFKPLIM,maps[5])) 
    	  goto init3d_error;

		tptr=maps[X_BM]+89*256+161;
		for (i=0;i<27;i++)
			{
			memcpy(tptr+91*256,tptr,92);
			tptr+=256;
			}
		if (readfile(teamlist[setup.team_b_kit].nosfile,maps[X_BM]+89*256))
		  goto init3d_error;
		tptr=maps[X_BM]+116*256+161;
		for (i=0;i<27;i++)
			{
			memcpy(tptr+64*256-92,tptr,92);
			tptr+=256;
			}
    	if (readfile(stadlist[setup.stadium].pitchfile,maps[X_BM]+116*256))
    	  goto init3d_error;
		tptr=maps[X_BM]+180*256+69;
		for (i=0;i<27;i++)
			{
			memcpy(tptr-64*256,tptr,184);
			tptr+=256;
			}
    	if (readfile(BM_EXTRA2,maps[X_BM]+180*256)) ||
			readfile(stadlist[setup.stadium].tmdfile,(BYTE *)&textures[S_TM]) ||
		 	readfile(stadlist[setup.stadium].s1pfile,(BYTE *)&stad1_p) ||
		 	readfile(stadlist[setup.stadium].s1ffile,(BYTE *)&stad1_f) ||
		 	readfile(stadlist[setup.stadium].s2pfile,(BYTE *)&stad2_p) ||
		 	readfile(stadlist[setup.stadium].s2ffile,(BYTE *)&stad2_f) ||
		 	readfile(stadlist[setup.stadium].s3pfile,(BYTE *)&stad3_p) ||
		 	readfile(stadlist[setup.stadium].s3ffile,(BYTE *)&stad3_f) ||
		 	readfile(stadlist[setup.stadium].s4pfile,(BYTE *)&stad4_p) ||
		 	readfile(stadlist[setup.stadium].s4ffile,(BYTE *)&stad4_f) ||
			readfile(MPB_AFOOTB,(BYTE *)&player_p[mcaps[MC_AFOOTB].cappts]) ||
	    	readfile(MPB_AFOOTC,(BYTE *)&player_p[mcaps[MC_AFOOTC].cappts]) ||
	    	readfile(MPB_ABODYB,(BYTE *)&player_p[mcaps[MC_ABODYB].cappts]) ||
	    	readfile(MPB_ABODYC,(BYTE *)&player_p[mcaps[MC_ABODYC].cappts]) ||
	    	readfile(MPB_AHEADB,(BYTE *)&player_p[mcaps[MC_AHEADB].cappts]) ||
	    	readfile(MPB_AHEADC,(BYTE *)&player_p[mcaps[MC_AHEADC].cappts]) ||
	    	readfile(MPB_AJUMPB,(BYTE *)&player_p[mcaps[MC_AJUMPB].cappts]) ||
	    	readfile(MPB_AJUMPC,(BYTE *)&player_p[mcaps[MC_AJUMPC].cappts]) ||
	    	readfile(MPB_BFOOTB,(BYTE *)&player_p[mcaps[MC_BFOOTBL].cappts]) ||
	    	readfile(MPB_BFOOTC,(BYTE *)&player_p[mcaps[MC_BFOOTCL].cappts]) ||
	    	readfile(MPB_BBODYB,(BYTE *)&player_p[mcaps[MC_BBODYBL].cappts]) ||
	    	readfile(MPB_BBODYC,(BYTE *)&player_p[mcaps[MC_BBODYCL].cappts]) ||
	    	readfile(MPB_BHEADB,(BYTE *)&player_p[mcaps[MC_BHEADBL].cappts]) ||
	    	readfile(MPB_BHEADC,(BYTE *)&player_p[mcaps[MC_BHEADCL].cappts]) ||
	    	readfile(MPB_BJUMPB,(BYTE *)&player_p[mcaps[MC_BJUMPBL].cappts]) ||
	    	readfile(MPB_CFOOTB,(BYTE *)&player_p[mcaps[MC_CFOOTBL].cappts]) || 
	    	readfile(MPB_CFOOTC,(BYTE *)&player_p[mcaps[MC_CFOOTCL].cappts]) || 
	    	readfile(MPB_CBODYB,(BYTE *)&player_p[mcaps[MC_CBODYBL].cappts]) ||
	    	readfile(MPB_CBODYC,(BYTE *)&player_p[mcaps[MC_CBODYCL].cappts]) ||
	    	readfile(MPB_CHEADB,(BYTE *)&player_p[mcaps[MC_CHEADBL].cappts]) ||
	    	readfile(MPB_CJUMPB,(BYTE *)&player_p[mcaps[MC_CJUMPBL].cappts]) || 
	    	readfile(MPB_SHOOT1,(BYTE *)&player_p[mcaps[MC_SHOOTR].cappts]) ||
	    	readfile(MPB_CHIP1,(BYTE *)&player_p[mcaps[MC_CHIPR].cappts]) ||
	    	readfile(MPB_SPASS1,(BYTE *)&player_p[mcaps[MC_PASSR].cappts]) ||
	    	readfile(MPB_CROSS1,(BYTE *)&player_p[mcaps[MC_CROSSR].cappts]) ||
	    	readfile(MPB_VOLLEY,(BYTE *)&player_p[mcaps[MC_VOLLEYR].cappts]) ||
//	    	readfile(MPB_TOE2,(BYTE *)&player_p[mcaps[MC_TOER].cappts]) ||
	    	readfile(MPB_DIAGPASS,(BYTE *)&player_p[mcaps[MC_DIAGPASSR].cappts]) ||
	    	readfile(MPB_DIAGBACK,(BYTE *)&player_p[mcaps[MC_DIAGBACKR].cappts]) ||
	    	readfile(MPB_SIDEPASS,(BYTE *)&player_p[mcaps[MC_SIDEPASSR].cappts]) ||
	    	readfile(MPB_BACKHEEL,(BYTE *)&player_p[mcaps[MC_BACKHEELR].cappts]) ||
	    	readfile(MPB_SVTOSTD,(BYTE *)&player_p[mcaps[MC_STOSL].cappts]) ||
	    	readfile(MPB_SVTOSTB,(BYTE *)&player_p[mcaps[MC_STOSBL].cappts]) ||
	    	readfile(MPB_TRAP,(BYTE *)&player_p[mcaps[MC_TRAPR].cappts]) ||
			readfile(MPB_RIDE4,(BYTE *)&player_p[mcaps[MC_RIDEL].cappts]) ||
	    	readfile(MPB_SOCKS2,(BYTE *)&player_p[mcaps[MC_SOCKSR].cappts]) ||
			readfile(MPB_TROTB,(BYTE *)&player_p[mcaps[MC_TROTB].cappts]) ||
			readfile(MPB_TROTG,(BYTE *)&player_p[mcaps[MC_TROTG].cappts]) ||
	    	readfile(MPB_TROTD1,(BYTE *)&player_p[mcaps[MC_TROTD].cappts]) ||
	    	readfile(MPB_TROTA,(BYTE *)&player_p[mcaps[MC_TROTA].cappts]) ||
	    	readfile(MPB_TROTE,(BYTE *)&player_p[mcaps[MC_TROTE].cappts]) ||
	    	readfile(MPB_RUN2,(BYTE *)&player_p[mcaps[MC_RUN].cappts]) ||
	    	readfile(MPB_JOG1,(BYTE *)&player_p[mcaps[MC_JOG].cappts]) ||
	    	readfile(MPB_BARGE,(BYTE *)&player_p[mcaps[MC_BARGE].cappts]) ||
	    	readfile(MPB_THROWW,(BYTE *)&player_p[mcaps[MC_THROWW].cappts]) ||
//	    	readfile(MPB_SHAMEW,(BYTE *)&player_p[mcaps[MC_SHAMEW].cappts]) ||
//	    	readfile(MPB_LIE1,(BYTE *)&player_p[mcaps[MC_LIE].cappts]) ||
			readfile(MPB_STAND1,(BYTE *)&player_p[mcaps[MC_STAND].cappts])  ||
	    	readfile(MPB_SHEADER,(BYTE *)&player_p[mcaps[MC_S_HEAD].cappts]) ||
	    	readfile(MPB_JHEADER,(BYTE *)&player_p[mcaps[MC_J_HEAD].cappts]) ||
	    	readfile(MPB_HDOWN1,(BYTE *)&player_p[mcaps[MC_D_HEAD].cappts]) ||
//	    	readfile(MPB_HEADUP,(BYTE *)&player_p[mcaps[MC_U_HEAD].cappts]) ||
	    	readfile(MPB_DHEADER,(BYTE *)&player_p[mcaps[MC_DV_HEAD].cappts]) ||
	    	readfile(MPB_CHEST1,(BYTE *)&player_p[mcaps[MC_CHEST].cappts]) ||
			readfile(MPB_STACKLE,(BYTE *)&player_p[mcaps[MC_S_TACKLE].cappts]) ||
			readfile(MPB_STEAL2,(BYTE *)&player_p[mcaps[MC_STEAL].cappts]) ||
	    	readfile(MPB_OVERHEAD,(BYTE *)&player_p[mcaps[MC_OVERHEAD].cappts]) ||
			readfile(MPB_THROWL,(BYTE *)&player_p[mcaps[MC_THROWL].cappts]) ||
	    	readfile(MPB_THROWS,(BYTE *)&player_p[mcaps[MC_THROWS].cappts]) ||
	    	readfile(MPB_FALLR,(BYTE *)&player_p[mcaps[MC_FALLR].cappts]) ||
//	    	readfile(MPB_FALLS,(BYTE *)&player_p[mcaps[MC_FALLS].cappts]) ||
	    	readfile(MPB_CELEB,(BYTE *)&player_p[mcaps[MC_CELEB].cappts]) ||
//	    	readfile(MPB_SHAME1,(BYTE *)&player_p[mcaps[MC_SHAME].cappts]) ||
	    	readfile(MPB_GETUP,(BYTE *)&player_p[mcaps[MC_GETUP].cappts]) ||
	    	readfile(MPB_GETUPF,(BYTE *)&player_p[mcaps[MC_GETUPF].cappts]) ||
			readfile(MPB_STANDB,(BYTE *)&player_p[mcaps[MC_STANDB].cappts]) ||
	    	readfile(MPB_BOUNCE,(BYTE *)&player_p[mcaps[MC_BOUNCE].cappts]) ||
	    	readfile(MPB_KICKOUT,(BYTE *)&player_p[mcaps[MC_KICKOUT].cappts]) ||
	    	readfile(MPB_HALFVOLL,(BYTE *)&player_p[mcaps[MC_HALFVOLL].cappts]) ||
	    	readfile(MPB_THROW,(BYTE *)&player_p[mcaps[MC_THROW].cappts]) ||
	    	readfile(MPB_ROLL,(BYTE *)&player_p[mcaps[MC_ROLL].cappts]) || 
	    	readfile(MPB_RUNWITHB,(BYTE *)&player_p[mcaps[MC_RUNWITHB].cappts]) || 
	    	readfile(MPB_WAVEUP,(BYTE *)&player_p[mcaps[MC_WAVEUP].cappts]) || 
//	    	readfile(MPB_PENALTY,(BYTE *)&player_p[mcaps[MC_PENALTY].cappts]) ||
//	    	readfile(MPB_PLEAD,(BYTE *)&player_p[mcaps[MC_PLEAD].cappts]) ||
	    	readfile(MPB_CRTOSTB1,(BYTE *)&player_p[mcaps[MC_CRTOSTB].cappts]) ||
	    	readfile(MPB_PICKBALL,(BYTE *)&player_p[mcaps[MC_PICKBALL].cappts])
			)
		 goto init3d_error;
	}
	else				
#endif
		{
/*
{
	for( int a=0; a<6;a++)
		{
		readfile(skylist[a].skypfile,&palette_buffer[3*208]);
		readfile(skylist[a].skyfile,sky);
		DumpTga( sky, 640, 480);
		}
}
*/
		int st_a1;
		int st_b1_kit;
//		st_a1=14;//setup.team_a?36:14;
//		st_b1_kit=36;//setup.team_b_kit?36:14;
		st_a1=setup.team_a;
		st_b1_kit=setup.team_b_kit;

		switch(st_a1/42)
			{
			case 0: if (readfile(BM_KGRIDS1,maps[5])) goto init3d_error;
					break;
			case 1: if (readfile(BM_KGRIDS2,maps[5]))
					  goto init3d_error;
			}
		j=st_a1%42;
		tptr=maps[5]+((j/7)*42+1)*256+(j%7)*36+1;
		tptr2=maps[X_BM+1]+111*256+220;
		for (i=0;i<40;i++)
			{
			memcpy(tptr2,tptr,35);
			tptr+=256;
			tptr2+=256;
			}
		switch(st_b1_kit/42)
			{
			case 0: if (readfile(BM_KGRIDS1,maps[5])) goto init3d_error;
					break;
			case 1: if (readfile(BM_KGRIDS2,maps[5]))
					  goto init3d_error;
			}
		j=st_b1_kit%42;
		tptr=maps[5]+((j/7)*42+1)*256+(j%7)*36+1;
		tptr2=maps[X_BM+1]+151*256+220;
		for (i=0;i<40;i++)
			{
			memcpy(tptr2,tptr,35);
			tptr+=256;
			tptr2+=256;
			}
/*			
		switch(st_a1/9)
			{
			case 0: if (readfile(BM_KGRID1,maps[5]))
					  goto init3d_error;
			case 1: if (readfile(BM_KGRID2,maps[5]))
					  goto init3d_error;
			case 2: if (readfile(BM_KGRID3,maps[5])) 
					  goto init3d_error;
			case 3: if (readfile(BM_KGRID4,maps[5]))
					  goto init3d_error;
			case 4: if (readfile(BM_KGRID5,maps[5]))
					  goto init3d_error;
			case 5: if (readfile(BM_KGRID6,maps[5]))
					  goto init3d_error;
			}
*/
		if (readfile(bm_kgrid[st_a1/9],maps[5]))
		  goto init3d_error;

		j=st_a1%9;
		tptr=maps[5]+((j/3)*83+2)*256+(j%3)*70+1;
		tptr2=maps[X_BM+1];
		for (i=0;i<79;i++)
			{
			memcpy(tptr2,tptr,69);
			tptr+=256;
			tptr2+=256;
			}
/*
		switch(st_b1_kit/9)
			{
			case 0: if (readfile(BM_KGRID1,maps[5]))
					  goto init3d_error;
			case 1: if (readfile(BM_KGRID2,maps[5])) 
					  goto init3d_error;
			case 2: if (readfile(BM_KGRID3,maps[5])) 
					  goto init3d_error;
			case 3: if (readfile(BM_KGRID4,maps[5]))
					  goto init3d_error;
			case 4: if (readfile(BM_KGRID5,maps[5]))
					  goto init3d_error;
			case 5: if (readfile(BM_KGRID6,maps[5]))
					  goto init3d_error;
			}
*/
		if (readfile(bm_kgrid[st_b1_kit/9],maps[5]))
		  goto init3d_error;

		j=st_b1_kit%9;
		tptr=maps[5]+((j/3)*83+2)*256+(j%3)*70+1;
		tptr2=maps[X_BM+1]+69;
		for (i=0;i<79;i++)
			{
			memcpy(tptr2,tptr,69);
			tptr+=256;
			tptr2+=256;
			}
    	if (readfile(BM_REFKPLIM,maps[5])) 
    	  goto init3d_error;

		tptr=maps[X_BM]+89*256+161;
		for (i=0;i<27;i++)
			{
			memcpy(tptr+91*256,tptr,92);
			tptr+=256;
			}
		if (readfile(teamlist[setup.team_b_kit].nosfile,maps[X_BM]+89*256))
		  goto init3d_error;
		tptr=maps[X_BM]+116*256+161;
		for (i=0;i<27;i++)
			{
			memcpy(tptr+64*256-92,tptr,92);
			tptr+=256;
			}
    	if (readfile(stadlist[setup.stadium].pitchfile,maps[X_BM]+116*256))
    	  goto init3d_error;
		tptr=maps[X_BM]+180*256+69;
		for (i=0;i<27;i++)
			{
			memcpy(tptr-64*256,tptr,184);
			tptr+=256;
			}
    	if (readfile(BM_EXTRA2,maps[X_BM]+180*256) ||
 			readfile(stadlist[setup.stadium].tmdfile,(BYTE *)&textures[S_TM]) ||
		 	readfile(stadlist[setup.stadium].s1pfile,(BYTE *)&stad1_p) ||
		 	readfile(stadlist[setup.stadium].s1ffile,(BYTE *)&stad1_f) ||
		 	readfile(stadlist[setup.stadium].s2pfile,(BYTE *)&stad2_p) ||
	 	 	readfile(stadlist[setup.stadium].s2ffile,(BYTE *)&stad2_f) ||
		 	readfile(stadlist[setup.stadium].s3pfile,(BYTE *)&stad3_p) ||
		 	readfile(stadlist[setup.stadium].s3ffile,(BYTE *)&stad3_f) ||
		 	readfile(stadlist[setup.stadium].s4pfile,(BYTE *)&stad4_p) ||
		 	readfile(stadlist[setup.stadium].s4ffile,(BYTE *)&stad4_f) ||
			readfile(MP4_AFOOTB,(BYTE *)&player_p[mcaps[MC_AFOOTB].cappts]) ||
	    	readfile(MP4_AFOOTC,(BYTE *)&player_p[mcaps[MC_AFOOTC].cappts]) ||
	    	readfile(MP4_ABODYB,(BYTE *)&player_p[mcaps[MC_ABODYB].cappts]) ||
	    	readfile(MP4_ABODYC,(BYTE *)&player_p[mcaps[MC_ABODYC].cappts]) ||
	    	readfile(MP4_AHEADB,(BYTE *)&player_p[mcaps[MC_AHEADB].cappts]) ||
	    	readfile(MP4_AHEADC,(BYTE *)&player_p[mcaps[MC_AHEADC].cappts]) ||
	    	readfile(MP4_AJUMPB,(BYTE *)&player_p[mcaps[MC_AJUMPB].cappts]) ||
	    	readfile(MP4_AJUMPC,(BYTE *)&player_p[mcaps[MC_AJUMPC].cappts]) ||
	    	readfile(MP4_BFOOTB,(BYTE *)&player_p[mcaps[MC_BFOOTBL].cappts]) ||
	    	readfile(MP4_BFOOTC,(BYTE *)&player_p[mcaps[MC_BFOOTCL].cappts]) ||
	    	readfile(MP4_BBODYB,(BYTE *)&player_p[mcaps[MC_BBODYBL].cappts]) ||
	    	readfile(MP4_BBODYC,(BYTE *)&player_p[mcaps[MC_BBODYCL].cappts]) ||
	    	readfile(MP4_BHEADB,(BYTE *)&player_p[mcaps[MC_BHEADBL].cappts]) ||
	    	readfile(MP4_BHEADC,(BYTE *)&player_p[mcaps[MC_BHEADCL].cappts]) ||
	    	readfile(MP4_BJUMPB,(BYTE *)&player_p[mcaps[MC_BJUMPBL].cappts]) ||
	    	readfile(MP4_CFOOTB,(BYTE *)&player_p[mcaps[MC_CFOOTBL].cappts]) || 
	    	readfile(MP4_CFOOTC,(BYTE *)&player_p[mcaps[MC_CFOOTCL].cappts]) || 
	    	readfile(MP4_CBODYB,(BYTE *)&player_p[mcaps[MC_CBODYBL].cappts]) ||
	    	readfile(MP4_CBODYC,(BYTE *)&player_p[mcaps[MC_CBODYCL].cappts]) ||
	    	readfile(MP4_CHEADB,(BYTE *)&player_p[mcaps[MC_CHEADBL].cappts]) ||
	    	readfile(MP4_CJUMPB,(BYTE *)&player_p[mcaps[MC_CJUMPBL].cappts]) || 
	    	readfile(MP4_SHOOT1,(BYTE *)&player_p[mcaps[MC_SHOOTR].cappts]) ||
	    	readfile(MP4_CHIP1,(BYTE *)&player_p[mcaps[MC_CHIPR].cappts]) ||
	    	readfile(MP4_SPASS1,(BYTE *)&player_p[mcaps[MC_PASSR].cappts]) ||
	    	readfile(MP4_CROSS1,(BYTE *)&player_p[mcaps[MC_CROSSR].cappts]) ||
	    	readfile(MP4_VOLLEY,(BYTE *)&player_p[mcaps[MC_VOLLEYR].cappts]) ||
//	    	readfile(MP4_TOE2,(BYTE *)&player_p[mcaps[MC_TOER].cappts]) ||
	    	readfile(MP4_DIAGPASS,(BYTE *)&player_p[mcaps[MC_DIAGPASSR].cappts]) ||
	    	readfile(MP4_DIAGBACK,(BYTE *)&player_p[mcaps[MC_DIAGBACKR].cappts]) ||
	    	readfile(MP4_SIDEPASS,(BYTE *)&player_p[mcaps[MC_SIDEPASSR].cappts]) ||
	    	readfile(MP4_BACKHEEL,(BYTE *)&player_p[mcaps[MC_BACKHEELR].cappts]) ||
	    	readfile(MP4_SVTOSTD,(BYTE *)&player_p[mcaps[MC_STOSL].cappts]) ||
	    	readfile(MP4_SVTOSTB,(BYTE *)&player_p[mcaps[MC_STOSBL].cappts]) ||
	    	readfile(MP4_TRAP,(BYTE *)&player_p[mcaps[MC_TRAPR].cappts]) ||
			readfile(MP4_RIDE4,(BYTE *)&player_p[mcaps[MC_RIDEL].cappts]) ||
	    	readfile(MP4_SOCKS2,(BYTE *)&player_p[mcaps[MC_SOCKSR].cappts]) ||
			readfile(MP4_TROTB,(BYTE *)&player_p[mcaps[MC_TROTB].cappts]) ||
			readfile(MP4_TROTG,(BYTE *)&player_p[mcaps[MC_TROTG].cappts]) ||
	    	readfile(MP4_TROTD1,(BYTE *)&player_p[mcaps[MC_TROTD].cappts]) ||
	    	readfile(MP4_TROTA,(BYTE *)&player_p[mcaps[MC_TROTA].cappts]) ||
	    	readfile(MP4_TROTE,(BYTE *)&player_p[mcaps[MC_TROTE].cappts]) ||
	    	readfile(MP4_RUN2,(BYTE *)&player_p[mcaps[MC_RUN].cappts]) ||
	    	readfile(MP4_JOG1,(BYTE *)&player_p[mcaps[MC_JOG].cappts]) ||
	    	readfile(MP4_BARGE,(BYTE *)&player_p[mcaps[MC_BARGE].cappts]) ||
	    	readfile(MP4_THROWW,(BYTE *)&player_p[mcaps[MC_THROWW].cappts]) ||
//	    	readfile(MP4_SHAMEW,(BYTE *)&player_p[mcaps[MC_SHAMEW].cappts]) ||
//	    	readfile(MP4_LIE1,(BYTE *)&player_p[mcaps[MC_LIE].cappts]) ||
			readfile(MP4_STAND1,(BYTE *)&player_p[mcaps[MC_STAND].cappts])  ||
	    	readfile(MP4_SHEADER,(BYTE *)&player_p[mcaps[MC_S_HEAD].cappts]) ||
	    	readfile(MP4_JHEADER,(BYTE *)&player_p[mcaps[MC_J_HEAD].cappts]) ||
	    	readfile(MP4_HDOWN1,(BYTE *)&player_p[mcaps[MC_D_HEAD].cappts]) ||
//	    	readfile(MP4_HEADUP,(BYTE *)&player_p[mcaps[MC_U_HEAD].cappts]) ||
	    	readfile(MP4_DHEADER,(BYTE *)&player_p[mcaps[MC_DV_HEAD].cappts]) ||
	    	readfile(MP4_CHEST1,(BYTE *)&player_p[mcaps[MC_CHEST].cappts]) ||
			readfile(MP4_STACKLE,(BYTE *)&player_p[mcaps[MC_S_TACKLE].cappts]) ||
			readfile(MP4_STEAL2,(BYTE *)&player_p[mcaps[MC_STEAL].cappts]) ||
	    	readfile(MP4_OVERHEAD,(BYTE *)&player_p[mcaps[MC_OVERHEAD].cappts]) ||
			readfile(MP4_THROWL,(BYTE *)&player_p[mcaps[MC_THROWL].cappts]) ||
	    	readfile(MP4_THROWS,(BYTE *)&player_p[mcaps[MC_THROWS].cappts]) ||
	    	readfile(MP4_FALLR,(BYTE *)&player_p[mcaps[MC_FALLR].cappts]) ||
//	    	readfile(MP4_FALLS,(BYTE *)&player_p[mcaps[MC_FALLS].cappts]) ||
	    	readfile(MP4_CELEB,(BYTE *)&player_p[mcaps[MC_CELEB].cappts]) ||
//	    	readfile(MP4_SHAME1,(BYTE *)&player_p[mcaps[MC_SHAME].cappts]) ||
	    	readfile(MP4_GETUP,(BYTE *)&player_p[mcaps[MC_GETUP].cappts]) ||
	    	readfile(MP4_GETUPF,(BYTE *)&player_p[mcaps[MC_GETUPF].cappts]) ||
			readfile(MP4_STANDB,(BYTE *)&player_p[mcaps[MC_STANDB].cappts]) ||
	    	readfile(MP4_BOUNCE,(BYTE *)&player_p[mcaps[MC_BOUNCE].cappts]) ||
	    	readfile(MP4_KICKOUT,(BYTE *)&player_p[mcaps[MC_KICKOUT].cappts]) ||
	    	readfile(MP4_HALFVOLL,(BYTE *)&player_p[mcaps[MC_HALFVOLL].cappts]) ||
	    	readfile(MP4_THROW,(BYTE *)&player_p[mcaps[MC_THROW].cappts]) ||
	    	readfile(MP4_ROLL,(BYTE *)&player_p[mcaps[MC_ROLL].cappts]) || 
	    	readfile(MP4_RUNWITHB,(BYTE *)&player_p[mcaps[MC_RUNWITHB].cappts]) || 
	    	readfile(MP4_WAVEUP,(BYTE *)&player_p[mcaps[MC_WAVEUP].cappts]) || 
//	    	readfile(MP4_PENALTY,(BYTE *)&player_p[mcaps[MC_PENALTY].cappts]) ||
//	    	readfile(MP4_PLEAD,(BYTE *)&player_p[mcaps[MC_PLEAD].cappts]) ||
	    	readfile(MP4_CRTOSTB1,(BYTE *)&player_p[mcaps[MC_CRTOSTB].cappts]) ||
	    	readfile(MP4_PICKBALL,(BYTE *)&player_p[mcaps[MC_PICKBALL].cappts])
			)
 			goto init3d_error;
		}

	UseResFile(mainResFile);
	
	// Making simple pitch
	st_w=stadlist[setup.stadium].st_w+100;
	st_l=stadlist[setup.stadium].st_l;
	st_h=stadlist[setup.stadium].st_h;
	for (i=0;i<19;i++)
		{
		pitch_p[1+i*3+2]=st_w;
		pitch_p[1+(i+19)*3+2]=-800-st_w;
		}
	pitch_p[1]=-st_l;
	pitch_p[1+19*3]=-st_l;
	pitch_p[1+18*3]=1280+st_l;
	pitch_p[1+37*3]=1280+st_l;
								 
	// Remapping textures
//	if (setup.verbose) puts("Remapping textures...");
	tlptr=(dword *)&textures[0];
	for (i=0;i<(NOTEXTURES+1)*8;i++) *tlptr++&=0xffffff00; // Correct unchecked carry in texture mapping (not a bug!)
	tptr=maps[0]+128*256;
	for (i=0;i<128*256;i++)
		{
		if (*tptr>=80 && *tptr<88) *tptr+=8;
		tptr++;
		}
	tptr=maps[2];
	for (i=0;i<65536;i++)
		{
		if (*tptr>=32 && *tptr<56) *tptr+=24;
		if (*tptr>=80 && *tptr<88) *tptr+=8;
		tptr++;
		}
	tptr=maps[3]+80*256;
	for (i=0;i<80*256;i++)
		{
		if (*tptr>=32 && *tptr<56) *tptr+=24;
		if (*tptr>=80 && *tptr<88) *tptr+=8;
		tptr++;
		}
	tptr=maps[X_BM]+89*256;
	for (i=0;i<27*256;i++)
		{
		if (*tptr>32 && *tptr<=56) *tptr+=24;
		if (*tptr>80 && *tptr<=88) *tptr+=8;
		tptr++;
		}
	tptr=maps[X_BM]+116*256+69;
	for (i=0;i<27;i++)
		{
		for (j=0;j<92;j++)
			{
			if (*tptr>32 && *tptr<=56) *tptr+=24;
			if (*tptr>80 && *tptr<=88) *tptr+=8;
			tptr++;
			}
		tptr+=164;
		}
	tptr=maps[0];
	for (i=0;i<X_BM*65536;i++) ++(*tptr++);
#if ALLOW_8MEG
	if (setup.M8)
		{
		tptr=maps[S_BM+2];
		for (i=0;i<65536;i++)
			{
			if (*tptr>=80 && *tptr<88) *tptr+=16;
			tptr++;
			}
		tptr=maps[S_BM+3];
		for (i=0;i<65536;i++)
			{
			if (*tptr>=32 && *tptr<56) *tptr+=24;
			if (*tptr>=80 && *tptr<88) *tptr+=16;
			tptr++;
			}
		tptr=maps[S_BM+4];
		for (i=0;i<(78*256);i++)
			{
			if (*tptr>=80 && *tptr<88) *tptr+=16;
			tptr++;
			}
		tptr=maps[S_BM+4]+80*256;
		for (i=0;i<(78*256);i++)
			{
			if (*tptr>=32 && *tptr<56) *tptr+=24;
			if (*tptr>=80 && *tptr<88) *tptr+=16;
			tptr++;
			}
		tptr=maps[S_BM+5];
		for (i=0;i<2*65536;i++)
			{
			if (*tptr>=80 && *tptr<88) *tptr+=8;
			tptr++;
			}
		tptr=maps[S_BM+2];
		for (i=0;i<5*65536;i++) ++(*tptr++);
		}
#endif

	// Prepare segment selectors for texture maps
	for (i=0;i<mappages+noloop+2;i++)
		mapsel[i]=(char *)&maps[i];

	for (i=mappages;i<mappages+noloop;i++)
		mapsel[i]=(char *)&maps[X_BM+(texloop[i-mappages].pos>>16)]+(texloop[i-mappages].pos&0xffff);
		
  	mapsel[i++]=(char *)&maps[X_BM]+148*256+64;

   	mapsel[i++]=(char *)&maps[X_BM]+116*256;
 
// Prepare filters
//	if (setup.verbose) puts("Making colour filters...");
	makefilter(filters[0],0,0,0,0.5,0.5,0.5);
	makefilter(filters[1],48,48,48,.5,.5,.5);


	// Initialise objects
//	if (setup.verbose) puts("Intialising data objects...");

	// Set players  
	for (i=0;i<11;i++)	
		initobj(&player[i],player_p[0],player_f1,0,0,0,1,0);
										            
	for (i=11;i<22;i++)
		initobj(&player[i],player_p[0],player_f2,0,0,0,1,0);

	initobj(&player[22],player_p[0],player_fr,0,0,0,1,0);
	initobj(&player[23],player_p[0],player_fl,0,0,0,1,0);
	initobj(&player[24],player_p[0],player_fl,0,0,0,1,0);

	// Prepare player tweening
	for (i=0;i<NPLAYERS;i++)
		{
		plyrtwtb[i].anim=MC_STAND;
		plyrtwtb[i].tween=-1;
		plyrtwtb[i].twnfrm=player[0].points;
		*plyrtwtb[i].twnpts1=*plyrtwtb[i].twnpts2=PLYRPTS;
		}

	// Set player shadows/highlights
	initobj(&plhilight[0][0],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][0],plhi_p,plhi1_f1,0,0,0,1,0);
	initobj(&plhilight[2][0],plhi_p,plhi2_f1,0,0,0,1,0);
	initobj(&plhilight[3][0],plhi_p,plhi3_f1,0,0,0,1,0);
	initobj(&plhilight[4][0],plhi_p,plhi3_f1,0,0,0,1,0);
	initobj(&plhilight[5][0],plhi_p,plhi4_f1,0,0,0,1,0);
	initobj(&plhilight[6][0],plhi_p,plhi4_f1,0,0,0,1,0);
	initobj(&plhilight[0][1],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][1],plhi_p,plhi1_f2,0,0,0,1,0);
	initobj(&plhilight[2][1],plhi_p,plhi2_f2,0,0,0,1,0);
	initobj(&plhilight[3][1],plhi_p,plhi3_f2,0,0,0,1,0);
	initobj(&plhilight[4][1],plhi_p,plhi3_f2,0,0,0,1,0);
	initobj(&plhilight[5][1],plhi_p,plhi4_f2,0,0,0,1,0);
	initobj(&plhilight[6][1],plhi_p,plhi4_f2,0,0,0,1,0);
	initobj(&plhilight[0][2],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][2],plhi_p,plhi1_f3,0,0,0,1,0);
	initobj(&plhilight[2][2],plhi_p,plhi2_f3,0,0,0,1,0);
	initobj(&plhilight[3][2],plhi_p,plhi3_f3,0,0,0,1,0);
	initobj(&plhilight[4][2],plhi_p,plhi3_f3,0,0,0,1,0);
	initobj(&plhilight[5][2],plhi_p,plhi4_f3,0,0,0,1,0);
	initobj(&plhilight[6][2],plhi_p,plhi4_f3,0,0,0,1,0);
	initobj(&plhilight[0][3],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][3],plhi_p,plhi1_f4,0,0,0,1,0);
	initobj(&plhilight[2][3],plhi_p,plhi2_f4,0,0,0,1,0);
	initobj(&plhilight[3][3],plhi_p,plhi3_f4,0,0,0,1,0);
	initobj(&plhilight[4][3],plhi_p,plhi3_f4,0,0,0,1,0);
	initobj(&plhilight[5][3],plhi_p,plhi4_f4,0,0,0,1,0);
	initobj(&plhilight[6][3],plhi_p,plhi4_f4,0,0,0,1,0);
	initobj(&plhilight[0][4],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][4],plhi_p,plhi1_f5,0,0,0,1,0);
	initobj(&plhilight[2][4],plhi_p,plhi2_f5,0,0,0,1,0);
	initobj(&plhilight[3][4],plhi_p,plhi3_f5,0,0,0,1,0);
	initobj(&plhilight[4][4],plhi_p,plhi3_f5,0,0,0,1,0);
	initobj(&plhilight[5][4],plhi_p,plhi4_f5,0,0,0,1,0);
	initobj(&plhilight[6][4],plhi_p,plhi4_f5,0,0,0,1,0);
	initobj(&plhilight[0][5],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][5],plhi_p,plhi1_ff,0,0,0,1,0);
	initobj(&plhilight[2][5],plhi_p,plhi2_ff,0,0,0,1,0);
	initobj(&plhilight[3][5],plhi_p,plhi3_ff,0,0,0,1,0);
	initobj(&plhilight[4][5],plhi_p,plhi3_ff,0,0,0,1,0);
	initobj(&plhilight[5][5],plhi_p,plhi4_ff,0,0,0,1,0);
	initobj(&plhilight[6][5],plhi_p,plhi4_ff,0,0,0,1,0);

	// Set ball
	initobj(&ball,footy_p,footy_f,640,0,-400,1,0);

	// Set ball shadow 
	initobj(&ballshad,ballshad_p,ballshad_f,640,0,-400,1,0);

	// Set simple pitch
	initobj(&pitch,pitch_p,pitch_f,0,0,0,1,0);
	
	// Set pitch lines
	initobj(&l1,l1_p1,l_f1a,210,0,-200,1,0);
	initobj(&l2,l2_p1,l_f1a,1070,0,-200,1,0);
	initobj(&l3,l3_p1,l_f1a,1070,0,-600,1,0);
	initobj(&l4,l4_p1,l_f1a,210,0,-600,1,0);
	initobj(&l5,l5_p1,l_f3a,640,0,-200,1,0);
	initobj(&l6,l6_p1,l_f3a,640,0,-600,1,0);
	initobj(&circle,circle_pa,circle_fa,640,0,-400,1,0);
	initobj(&semi1,s_circle_p1a,s_circle_f1a,123,0,-400,1,0);
	initobj(&semi2,s_circle_p2a,s_circle_f2a,1157,0,-400,1,0);
	initobj(&spot1,spot_p,spot_f,640,0,-400,1,0);
	initobj(&spot2,spot_p,spot_f,128,0,-400,1,0);
	initobj(&spot3,spot_p,spot_f,1152,0,-400,1,0);

	// Set goals
	initobj(&goal1_1,goal1a_p,goal_f1a,0,0,-443.5,1,0);
	initobj(&goal2_1,goal1a_p,goal_f1a,0,0,-356.5,1,0);
	initobj(&goal3_1,goal1a_p,goal_f1a,0,37,-400,1,0);
	initobj(&goal4_1,goal1a_p,goal_f1a,-28,0,-400,1,0);
	initobj(&goal1_2,goal1a_p,goal_f1a,1280,0,-443.5,1,0);
	initobj(&goal2_2,goal1a_p,goal_f1a,1280,0,-356.5,1,0);
	initobj(&goal3_2,goal1a_p,goal_f1a,1280,37,-400,1,0);
	initobj(&goal4_2,goal1a_p,goal_f1a,1308,0,-400,1,0);

	// Set flags
	initobj(&flag_1,flag_p,flag_f,0,0,0,1,0);
	initobj(&flag_2,flag_p,flag_f,1280,0,0,1,0);
	initobj(&flag_3,flag_p,flag_f,0,0,-800,1,0);
	initobj(&flag_4,flag_p,flag_f,1280,0,-800,1,0);

	// Set stands
	initobj(&stad1,stad1_p,stad1_f, stadlist[setup.stadium].s1x,stadlist[setup.stadium].s1y,stadlist[setup.stadium].s1z,1,0);
	initobj(&stad2,stad2_p,stad2_f, stadlist[setup.stadium].s2x,stadlist[setup.stadium].s2y,stadlist[setup.stadium].s2z,1,0);
	initobj(&stad3,stad3_p,stad3_f, stadlist[setup.stadium].s3x,stadlist[setup.stadium].s3y,stadlist[setup.stadium].s3z,1,0);
	initobj(&stad4,stad4_p,stad4_f, stadlist[setup.stadium].s4x,stadlist[setup.stadium].s4y,stadlist[setup.stadium].s4z,1,0);

	InvertWord((BYTE *)&stad1_f, MAX_STAD_FACES*6);
	InvertWord((BYTE *)&stad2_f, MAX_STAD_FACES*6);
	InvertWord((BYTE *)&stad3_f, MAX_STAD_FACES*6);
	InvertWord((BYTE *)&stad4_f, MAX_STAD_FACES*6);

	InvertFloat((BYTE *)&stad1_p, MAX_STAD_PTS*3);
	InvertFloat((BYTE *)&stad2_p, MAX_STAD_PTS*3);
	InvertFloat((BYTE *)&stad3_p, MAX_STAD_PTS*3);
	InvertFloat((BYTE *)&stad4_p, MAX_STAD_PTS*3);

	stp=stad1_f;
	i=*stp++;
	for(;i;i--)
	{
		j=*stp++;
		if (*stp<0) *stp-=S_TM-1;
		stp++;
		for (;j;j--) stp++;
	}
	stp=stad2_f;
	i=*stp++;
	for(;i;i--)
	{
		j=*stp++;
		if (*stp<0) *stp-=S_TM-1;
		stp++;
		for (;j;j--) stp++;
	}
	stp=stad3_f;
	i=*stp++;
	for(;i;i--)
	{
		j=*stp++;
		if (*stp<0) *stp-=S_TM-1;
		stp++;
		for (;j;j--) stp++;
	}
	stp=stad4_f;
	i=*stp++;
	for(;i;i--)
	{
		j=*stp++;
		if (*stp<0) *stp-=S_TM-1;
		stp++;
		for (;j;j--) stp++;
	}

	// Initialise animations
	buff_ptr=(char *)(maps[S_BM])+stadlist[setup.stadium].vmap;
	vidi_anim=0;
	vidi_in=0;

	UseResFile( dataResFile); resType = RESTYPE_FAP;

// 	if (readrawfile(fapoffs,(BYTE *)load_offsets)) goto init3d_error;
//	if (opendatafile(fapfile) < 0) goto init3d_error;
	
	if (animdata) 
		{
		if( Language == American || Language == Canadian)
			{
			readfile(FAP_VR,(BYTE *)animdata);
			readfile(COL_VR,(BYTE *)&palette_buffer[240*3]);
			}
		else
			{
			readfile(FAP_VIDI,(BYTE *)animdata);
			readfile(COL_VIDI,(BYTE *)&palette_buffer[240*3]);
			}
		init_anim( vidi, 0, 0);
		}

	// Prepare miscellaneous data
//	if (setup.verbose) puts("Preparing miscellaneous data...");
	

	// Set screen mode and set buffer info
//	if (setup.verbose) puts("Geting VESA information...");
//	LORES_buff.buff_start=(BYTE *)0xa0000;
//	for (i=0;i<SCRSIZES;i++)
//		{
//		LORES_buff[i].buff_start=scrb+320+((320-LORES_buff[i].clip_wid)>>1)+((200-LORES_buff[i].clip_hgt)>>1)*320;
//		for (j=0;j<VESAMODES;j++)
//			VESA_buff[j][i].buff_start=scrb+640+((VESA_buff[j][i].buff_wid-VESA_buff[j][i].clip_wid)>>1)+((VESA_buff[j][i].buff_hgt-VESA_buff[j][i].clip_hgt)>>1)*VESA_buff[j][i].buff_wid;
//		}
	if (setup.stadium>=90)
		{
		vidi_buff_1.buff_start=maps[S_BM]+7*256+10;
		vidi_buff_2.buff_start=maps[S_BM]+7*256+10;
		}
	else
		{
		vidi_buff_1.buff_start=maps[S_BM]+14*256+51;
		vidi_buff_2.buff_start=maps[S_BM]+14*256+51;
		}
	
	if (setup.vidi_type==0)
		vidi_buff=vidi_buff_1;
	else
		vidi_buff=vidi_buff_2;

	for (i=0;i<NO_MENUS;i++)
		{
		menu_buff_l[i].buff_start=LORES_buff.buff_start+(LORES_buff.clip_hgt-menu_buff_l[i].buff_hgt)*LORES_buff.buff_wid+((LORES_buff.clip_wid-menu_buff_l[i].clip_wid)>>1);
		menu_buff_h[i].buff_start=HIRES_buff.buff_start+(HIRES_buff.clip_hgt-menu_buff_h[i].buff_hgt)*HIRES_buff.buff_wid+((HIRES_buff.clip_wid-menu_buff_h[i].clip_wid)>>1);
		}

//	for (i=0;i<NO_MENUS;i++)
//		{
//		menu_buff_l[i].buff_start=LORES_buff.buff_start+(LORES_buff.clip_hgt-menu_buff_l[i].buff_hgt)*LORES_buff.buff_wid+((LORES_buff.clip_wid-menu_buff_l[i].clip_wid)>>1);
//		menu_buff_h[i].buff_start=HIRES_buff.buff_start+(HIRES_buff.clip_hgt-menu_buff_h[i].buff_hgt)*HIRES_buff.buff_wid+((HIRES_buff.clip_wid-menu_buff_h[i].clip_wid)>>1);
//		}

	return (0);

init3d_error:;
	end3d_gremlin();
	return (-1);

}

void end3d_gremlin()
{
	UseResFile( mainResFile);
	
	FreeX(rotlist);
	FreeX(ptslist);
	FreeX(pollist);
	FreeX(objlist);
	FreeX(plyrtwtb);
	FreeX(player_p);
	FreeX(filters);
	FreeX(textures);
	if(sky)
	  	FreeX(sky);
	if(animdata)
	  	FreeX(animdata);
	FreeX(maps);
	HeapPurge();
}

int timing()
{
#if USERAVEENGINE != YES
int i,j;
word *stp;
plyrdat *plyrwr,plyr[NPLAYERS+2];
datapt dp[]={0};
long ticks, frames;

	setup.M8=0;
	setup.team_a=0;
	setup.team_b=setup.team_b_kit=1;
	setup.stadium=45;
	setup.vidi_type=0;
	setup.start_res=0;
	setup.screen_size=0;
	setup.detail.stadia=1;
	setup.detail.pitch=1;
	setup.detail.lines=1;
	setup.detail.sky=1;
	setup.detail.players=1;
	setup.detail.shadows=1;
	
	LORES_buff.buff_start = (BYTE *)screenptr;
#if USERAVEENGINE==YES
	BlitterMode = DOUBLE_320240;
#else
	BlitterMode = DOUBLE_320200;
#endif

	maps=(mappage *)MallocX((mappages=MAPPAGES_4)*65536);
	textures=(texture *)MallocX((NOTEXTURES+CLIP_TEXTURES+1)*sizeof(texture));
	filters=(filter *)MallocX(FILTERS*sizeof(filter));
	player_p=(capfrm *)MallocX(100*sizeof(capfrm));
	plyrtwtb=(plyrtwdat *)MallocX(NPLAYERS*sizeof(plyrtwdat));
	objlist=(objs *)MallocX(MAXOBJ*sizeof(objs));
	pollist=(pols *)MallocX(MAXPOL*sizeof(pols));
	ptslist=(scrpt *)MallocX(MAXPOL*10*sizeof(scrpt));
	rotlist=(rotpt *)MallocX(MAXPTS*4*sizeof(rotpt));

	UseResFile(dataResFile2); resType = RESTYPE_ACTR;

	mcaps=mcaps4;

	if (readfile(TMD_TEXDATA,(BYTE *)&textures[1]) ||
		readfile(stadlist[setup.stadium].tmdfile,(BYTE *)&textures[S_TM]) ||
	 	readfile(stadlist[setup.stadium].s1pfile,(BYTE *)&stad1_p) ||
	 	readfile(stadlist[setup.stadium].s1ffile,(BYTE *)&stad1_f) ||
	 	readfile(stadlist[setup.stadium].s2pfile,(BYTE *)&stad2_p) ||
	 	readfile(stadlist[setup.stadium].s2ffile,(BYTE *)&stad2_f) ||
	 	readfile(stadlist[setup.stadium].s3pfile,(BYTE *)&stad3_p) ||
	 	readfile(stadlist[setup.stadium].s3ffile,(BYTE *)&stad3_f) ||
	 	readfile(stadlist[setup.stadium].s4pfile,(BYTE *)&stad4_p) ||
	 	readfile(stadlist[setup.stadium].s4ffile,(BYTE *)&stad4_f) ||
		readfile(MP4_STAND1,(BYTE *)&player_p[0])
	   )
	  goto timing_error;

	// Prepare segment selectors for texture maps
	for (i=0;i<mappages;i++)
		mapsel[i]=(char *)maps[i];

	for (i=mappages;i<mappages+noloop;i++)
		mapsel[i]=(char *)maps[X_BM+(texloop[i-mappages].pos>>16)]+(texloop[i-mappages].pos&0xffff);

  	mapsel[i]=(char *)maps[X_BM]+148*256+64;
  	mapsel[i+1]=(char *)maps[X_BM]+116*256;
 
	// Prepare filters
	makefilter(filters[0],0,0,0,0.5,0.5,0.5);
	makefilter(filters[1],48,48,48,.5,.5,.5);

	// Initialise objects
	// Set players  
	for (i=0;i<11;i++)	
		initobj(&player[i],player_p[0],player_f1,0,0,0,1,0);
										            
	for (i=11;i<22;i++)
		initobj(&player[i],player_p[0],player_f2,0,0,0,1,0);

	initobj(&player[22],player_p[0],player_fr,0,0,0,1,0);
	initobj(&player[23],player_p[0],player_fl,0,0,0,1,0);
	initobj(&player[24],player_p[0],player_fl,0,0,0,1,0);

	// Prepare player tweening
	for (i=0;i<NPLAYERS;i++)
		{
		plyrtwtb[i].anim=0;
		plyrtwtb[i].tween=-1;
		plyrtwtb[i].twnfrm=player[0].points;
		*plyrtwtb[i].twnpts1=*plyrtwtb[i].twnpts2=PLYRPTS;
		}

	// Set player shadows/highlights
	initobj(&plhilight[0][0],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][0],plhi_p,plhi1_f1,0,0,0,1,0);
	initobj(&plhilight[2][0],plhi_p,plhi2_f1,0,0,0,1,0);
	initobj(&plhilight[3][0],plhi_p,plhi3_f1,0,0,0,1,0);
	initobj(&plhilight[4][0],plhi_p,plhi3_f1,0,0,0,1,0);
	initobj(&plhilight[5][0],plhi_p,plhi4_f1,0,0,0,1,0);
	initobj(&plhilight[0][1],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][1],plhi_p,plhi1_f2,0,0,0,1,0);
	initobj(&plhilight[2][1],plhi_p,plhi2_f2,0,0,0,1,0);
	initobj(&plhilight[3][1],plhi_p,plhi3_f2,0,0,0,1,0);
	initobj(&plhilight[4][1],plhi_p,plhi3_f2,0,0,0,1,0);
	initobj(&plhilight[5][1],plhi_p,plhi4_f2,0,0,0,1,0);
	initobj(&plhilight[0][2],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][2],plhi_p,plhi1_f3,0,0,0,1,0);
	initobj(&plhilight[2][2],plhi_p,plhi2_f3,0,0,0,1,0);
	initobj(&plhilight[3][2],plhi_p,plhi3_f3,0,0,0,1,0);
	initobj(&plhilight[4][2],plhi_p,plhi3_f3,0,0,0,1,0);
	initobj(&plhilight[5][2],plhi_p,plhi4_f3,0,0,0,1,0);
	initobj(&plhilight[0][3],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][3],plhi_p,plhi1_f4,0,0,0,1,0);
	initobj(&plhilight[2][3],plhi_p,plhi2_f4,0,0,0,1,0);
	initobj(&plhilight[3][3],plhi_p,plhi3_f4,0,0,0,1,0);
	initobj(&plhilight[4][3],plhi_p,plhi3_f4,0,0,0,1,0);
	initobj(&plhilight[5][3],plhi_p,plhi4_f4,0,0,0,1,0);
	initobj(&plhilight[0][4],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][4],plhi_p,plhi1_f5,0,0,0,1,0);
	initobj(&plhilight[2][4],plhi_p,plhi2_f5,0,0,0,1,0);
	initobj(&plhilight[3][4],plhi_p,plhi3_f5,0,0,0,1,0);
	initobj(&plhilight[4][4],plhi_p,plhi3_f5,0,0,0,1,0);
	initobj(&plhilight[5][4],plhi_p,plhi4_f5,0,0,0,1,0);
	initobj(&plhilight[0][5],plshad_p,plshad_f,0,0,0,1,0);
	initobj(&plhilight[1][5],plhi_p,plhi1_ff,0,0,0,1,0);
	initobj(&plhilight[2][5],plhi_p,plhi2_ff,0,0,0,1,0);
	initobj(&plhilight[3][5],plhi_p,plhi3_ff,0,0,0,1,0);
	initobj(&plhilight[4][5],plhi_p,plhi3_ff,0,0,0,1,0);
	initobj(&plhilight[5][5],plhi_p,plhi4_ff,0,0,0,1,0);

	// Set ball
	initobj(&ball,footy_p,footy_f,640,0,-400,1,0);

	// Set ball shadow 
	initobj(&ballshad,ballshad_p,ballshad_f,640,0,-400,1,0);
	
	// Set pitch lines
	initobj(&l1,l1_p1,l_f1a,210,0,-200,1,0);
	initobj(&l2,l2_p1,l_f1a,1070,0,-200,1,0);
	initobj(&l3,l3_p1,l_f1a,1070,0,-600,1,0);
	initobj(&l4,l4_p1,l_f1a,210,0,-600,1,0);
	initobj(&l5,l5_p1,l_f3a,640,0,-200,1,0);
	initobj(&l6,l6_p1,l_f3a,640,0,-600,1,0);
	initobj(&circle,circle_pa,circle_fa,640,0,-400,1,0);
	initobj(&semi1,s_circle_p1a,s_circle_f1a,123,0,-400,1,0);
	initobj(&semi2,s_circle_p2a,s_circle_f2a,1157,0,-400,1,0);
	initobj(&spot1,spot_p,spot_f,640,0,-400,1,0);
	initobj(&spot2,spot_p,spot_f,128,0,-400,1,0);
	initobj(&spot3,spot_p,spot_f,1152,0,-400,1,0);

	// Set goals
	initobj(&goal1_1,goal1a_p,goal_f1a,0,0,-443.5,1,0);
	initobj(&goal2_1,goal1a_p,goal_f1a,0,0,-356.5,1,0);
	initobj(&goal3_1,goal1a_p,goal_f1a,0,37,-400,1,0);
	initobj(&goal4_1,goal1a_p,goal_f1a,-28,0,-400,1,0);
	initobj(&goal1_2,goal1a_p,goal_f1a,1280,0,-443.5,1,0);
	initobj(&goal2_2,goal1a_p,goal_f1a,1280,0,-356.5,1,0);
	initobj(&goal3_2,goal1a_p,goal_f1a,1280,37,-400,1,0);
	initobj(&goal4_2,goal1a_p,goal_f1a,1308,0,-400,1,0);

	// Set flags
	initobj(&flag_1,flag_p,flag_f,0,0,0,1,0);
	initobj(&flag_2,flag_p,flag_f,1280,0,0,1,0);
	initobj(&flag_3,flag_p,flag_f,0,0,-800,1,0);
	initobj(&flag_4,flag_p,flag_f,1280,0,-800,1,0);

	// Set stands
	initobj(&stad1,stad1_p,stad1_f, stadlist[setup.stadium].s1x,stadlist[setup.stadium].s1y,stadlist[setup.stadium].s1z,1,0);
	initobj(&stad2,stad2_p,stad2_f, stadlist[setup.stadium].s2x,stadlist[setup.stadium].s2y,stadlist[setup.stadium].s2z,1,0);
	initobj(&stad3,stad3_p,stad3_f, stadlist[setup.stadium].s3x,stadlist[setup.stadium].s3y,stadlist[setup.stadium].s3z,1,0);
	initobj(&stad4,stad4_p,stad4_f, stadlist[setup.stadium].s4x,stadlist[setup.stadium].s4y,stadlist[setup.stadium].s4z,1,0);

	InvertWord( (BYTE *) stad1_f, MAX_STAD_FACES*6);
	InvertWord( (BYTE *) stad2_f, MAX_STAD_FACES*6);
	InvertWord( (BYTE *) stad3_f, MAX_STAD_FACES*6);
	InvertWord( (BYTE *) stad4_f, MAX_STAD_FACES*6);

	InvertFloat( (BYTE *) stad1_p, MAX_STAD_PTS*3);
	InvertFloat( (BYTE *) stad2_p, MAX_STAD_PTS*3);
	InvertFloat( (BYTE *) stad3_p, MAX_STAD_PTS*3);
	InvertFloat( (BYTE *) stad4_p, MAX_STAD_PTS*3);

	stp=stad1_f;
	i=*stp++;
	for(;i;i--)
	{
		j=*stp++;
		if (*stp<0) *stp-=S_TM-1;
		stp++;
		for (;j;j--) stp++;
	}
	stp=stad2_f;
	i=*stp++;
	for(;i;i--)
	{
		j=*stp++;
		if (*stp<0) *stp-=S_TM-1;
		stp++;
		for (;j;j--) stp++;
	}
	stp=stad3_f;
	i=*stp++;
	for(;i;i--)
	{
		j=*stp++;
		if (*stp<0) *stp-=S_TM-1;
		stp++;
		for (;j;j--) stp++;
	}
	stp=stad4_f;
	i=*stp++;
	for(;i;i--)
	{
		j=*stp++;
		if (*stp<0) *stp-=S_TM-1;
		stp++;
		for (;j;j--) stp++;
	}

	LORES_buff.buff_start = (BYTE *)screenptr;
	main_buff=LORES_buff;

	plyrwr=plyr;            
	for (i=0;i<23;i++)
		{
		plyrwr->x=640;
		plyrwr->y=0;
		plyrwr->z=400;
		plyrwr->type=(i/11);
		plyrwr->number=i+1;
		plyrwr->htype=plyrwr->hcol=0;
		plyrwr->crot=1;
		plyrwr->srot=0;
		plyrwr->anim=0;
		plyrwr->frame=0;
		plyrwr->fstep=0.5;
		plyrwr->sprite=-1;
		plyrwr++;
		}

	for (i=0;i<2;i++)
		{
		plyrwr->x=640;
		plyrwr->y=0;
		plyrwr->z=400;
		plyrwr->type=4;
		plyrwr->number=i+1;
		plyrwr->htype=plyrwr->hcol=0;
		plyrwr->crot=1;
		plyrwr->srot=0;
		plyrwr->anim=0;
		plyrwr->frame=0;
		plyrwr->fstep=0.5;
		plyrwr->sprite=-1;
		plyrwr++;
		}

	plyrwr->x=640;
	plyrwr->y=20;
	plyrwr->z=400;
 	plyrwr->crot=1;
 	plyrwr->srot=0;
 	plyrwr->frame=1;
 	plyrwr->fstep=0;
	plyrwr->sprite=-1;

	Timing = false;

	frames = VblTicks = 0;
	performance = TickCount();
	while((TickCount()-performance)<60)	// one second for timing
		{
	  	render3d(&main_buff,0,20,0,1280,20,800,256,plyr,dp,NULL);
	  	main_buff.dump(0,0);
	  	frames++;
		}
	ticks = VblTicks;
	TicksPerSecond = ticks;
	
	EndTiming();
		
	Timing = true;

//	performance = frames;

	if( frames>31)
	  return 31;

	return frames;
timing_error:;
	EndTiming();
	return (-1);
#else
	Timing = true;
	return 31;
#endif
}

void EndTiming(void)
{
	UseResFile( mainResFile);
	FreeX(rotlist);
	FreeX(ptslist);
	FreeX(pollist);
	FreeX(objlist);
	FreeX(plyrtwtb);
	FreeX(player_p);
	FreeX(filters);
	FreeX(textures);
	FreeX(maps);
	HeapPurge();
}

float balla;
float balld;
float ballly;
float ballld;
int jno=300;

void drawman(buff_info *buffer, float anim, float angle, int dist)
{
datapt targx,targy,targz,viewx,viewy,viewz,vdist;

	short i,j;
	objs *oldobj;
	float hr,r;
	mat tmp;

	targx=targz=0;
	targy=10;
	viewx=dist*cos(angle);
	viewy=40;
	viewz=dist*sin(angle);
	vdist=buffer->buff_wid;

	// Set extended buffer info
	render_buff.buff_start=buffer->buff_start;
	render_buff.buff_wid=buffer->buff_wid;
	render_buff.buff_hgt=buffer->buff_hgt;
	render_buff.clip_wid=buffer->clip_wid;
	render_buff.clip_hgt=buffer->clip_hgt;
	render_buff.scale_x=buffer->scale_x;
	render_buff.scale_y=buffer->scale_y;
	render_buff.dump=buffer->dump;
	render_buff.clip_endl=buffer->buff_start+buffer->clip_hgt*buffer->buff_wid;
	render_buff.clip_end=render_buff.clip_endl-buffer->buff_wid;
	render_buff.clip_widl=buffer->clip_wid-1;
	render_buff.clip_hgtl=buffer->clip_hgt-1;
	render_buff.clip_xmid=buffer->clip_wid>>1;
	render_buff.clip_ymid=buffer->clip_hgt>>1;

	// Set perspective based info
	oq=vdist;
	q=vdist*((buffer->scale_x+buffer->scale_y)/2);
	qr=(floatpt)1/q; 
	qa=(floatpt)q*(2.0/100.0);
	scrdr=(float)(1/SCREENDIST);
	persc=q*scrdr;

	// Prepare viewing vector vars
	viewz=-viewz,targz=-targz;
	targx-=viewx;
	targy-=viewy;
	targz-=viewz;

	hr=targx*targx+targz*targz;
	if (hr<1) hr=1,targz=1;
	r=sqrt(hr+targy*targy);
	hr=sqrt(hr);
	cth=targz/hr;sth=targx/hr;
	cph=hr/r;sph=targy/r;

	// Make global rotation matrix
	matcop(rot.arr,unit);
	matcop(tran.arr,unit);
	matcop(tmp,unit);
	rot.arr[0][3]=-viewx;
	rot.arr[1][3]=-viewy;
  	rot.arr[2][3]=-viewz;
	tmp[0][0]=cth;
	tmp[0][2]=-sth;				             
	tmp[1][0]=-sth*sph;
	tmp[1][1]=cph;
	tmp[1][2]=-cth*sph;
	tmp[2][0]=sth*cph;			 
	tmp[2][1]=sph;
	tmp[2][2]=cth*cph;
	matmul(rot.arr,tmp);

	clearlists

	player[0].points=player_p[(short)(anim*jno)];
	oldobj=curobj;
	addobjy(&player[0]);
	if (oldobj!=curobj) sortobj(oldobj);

	ball.y=-player[0].points[1+23*3];
	ball.z=player[0].points[1+23*3+2];
	ball.x=-ball.y*player[0].crot+ball.z*player[0].srot+player[0].x;
	ball.z=ball.y*player[0].srot+ball.z*player[0].crot+player[0].z;
	ball.y=player[0].points[1+23*3+1]+player[0].y;
	addobjxy(&ball,cos(balla),sin(balla));

	if (anim<0.88)
		{
		balla+=balld;
		balld*=0.89;
		if (ball.y-ballly>0 && ballld<=0) balld+=0.16;
		ballld=ball.y-ballly;
		ballly=ball.y;
		}
	dispols();
	}

int initman(int team, int capture)
	{
	int i,nheadfile;
	word selector;
	BYTE *seg;
	BYTE *tptr;

	BlitterMode = REAL_640480;

	maps = (mappage *)MallocX( (mappages=4)*65536);
	textures = (texture *)MallocX(600*sizeof(texture));
//	filters=(filter *)MallocX(FILTERS*sizeof(filter));
	player_p=(capfrm *)MallocX(300*sizeof(capfrm));
//	plyrtwtb=(plyrtwdat *)MallocX(NPLAYERS*sizeof(plyrtwdat));
	objlist=(objs *)MallocX(5*sizeof(objs));
	pollist=(pols *)MallocX(60*sizeof(pols));
	ptslist=(scrpt *)MallocX(400*10*sizeof(scrpt));
	rotlist=(rotpt *)MallocX(200*4*sizeof(rotpt));
	
	if( !maps || !textures || !player_p || !objlist || !pollist || !ptslist || !rotlist)
	  goto man_error;

// 	if (readrawfile(dataoffs,(BYTE *)load_offsets)) goto err8;
//	if (opendatafile(datafile)<0) return(-1);
	UseResFile( dataResFile2); resType = RESTYPE_ACTR;
	
#ifndef BBS
	if (teamlist[team].headfile==BM_XCAUCASB || teamlist[team].headfile==BM_XNORDIC)
		nheadfile=BM_XCAUCASA;
	else
#endif
		nheadfile=teamlist[team].headfile;

	if(
		readfile(TMD_MANDATA,(BYTE *)&textures[1]) ||
		readfile(PAL_FOOTY,palette_buffer) ||
		readfile(nheadfile,maps[0]) ||
		readfile(teamlist[team].torsofile,maps[1]) ||
		readfile(teamlist[team].limbsfile,maps[3]) ||
		readfile(BM_FEET,maps[3]+158*256) ||
		readfile(teamlist[team].palfile,&palette_buffer[3*32]) ||
		readfile(teamlist[team].skinfile,&palette_buffer[3*80])
	  )
	goto man_error;
	
	switch(capture)
		{
		case 0:
			readfile(MPB_JUGGLE,(BYTE *)&player_p[0]);
			jno=300;
			break;
		case 1:
			readfile(MPB_CELEB,(BYTE *)&player_p[0]);
			jno=118;
			break;
		}
//	closedatafile();
	UseResFile( mainResFile);
	
	for(i=0;i<mappages;i++)
	  mapsel[i]=(char *)maps+(i*65536);
	  
	tptr=maps[0];
	for (i=0;i<4*65536;i++) ++(*tptr++);

/*
	if ((selector=getselectors(mappages)) < 0)
		{
		puts("Error preparing selectors");
		goto err8;
		}
	for (i=0;i<mappages;i++)
		{
		mapsel[i]=selector;
		seg=maps[i];
		if (setselector(selector,seg,65536) < 0)
			{
			puts("Error setting selectors");
			goto err9;
			}
		selector+=nextselectordisp(selector);
		}
*/

	initobj(&player[0],player_p[0],player_f1,0,0,0,1,0);
	player[0].faces[0]=12;
	initobj(&ball,footy_p,footy_f,0,0,0,1,0);

	balla=0;
	balld=0;
	ballly=0;
	ballld=0;

	angle = -PI/3;
	
//	PaletteChange = true;
//	scrmode=VESAmode(&modelist[0]);
	return (0);
man_error:
	endman();
	return -1;
	}


void endman()
{
	FreeX(rotlist);
	FreeX(ptslist);
	FreeX(pollist);
	FreeX(objlist);
//	FreeX(plyrtwtb);
	FreeX(player_p);
//	FreeX(filters);
	FreeX(textures);
	FreeX(maps);
	HeapPurge();

	BlitterMode = REAL_640480;
}

char *GetResString( int number)
{
Str255 theString;
static char newString[128];
int a;

	GetIndString( theString, textGame+Language, number+1);
	memcpy( newString, theString+1, (int)*theString);
	newString[*theString]=0;	

	if( newString[(*theString)-1]==32)
		newString[(*theString)-1]=0;
		
	return newString;
}
