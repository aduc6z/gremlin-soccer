#include "Frontend Vars.h"#include "Sos.h"#undef EXTERN#define EXTERN extern#include "Mallocx.h"#include "Play.h"int sosDIGIStartSample( int handle, _SOS_START_SAMPLE *theSample){int sndChannel = PLAYER_PlayExternalSound ( MallocGetHandle( theSample->lpSamplePtr), theSample->dwSamplePitchAdd, theSample->bLoop);	PLAYER_SetVolume( sndChannel, theSample->wVolume);	return sndChannel;}void sosDIGISetMasterVolume( int handle, int Volume){long newVolume;	Volume >>= 7;	newVolume = (Volume)<<16 | Volume;	SetDefaultOutputVolume( newVolume);}Boolean sosDIGISampleDone( int handle, int channel){	return externalPlaying[ channel];}void sosDIGIStopSample( int handle, int channel){	  PLAYER_StopSound( channel);}