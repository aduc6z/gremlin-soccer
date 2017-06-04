#include "Frontend Vars.h"#include "Play.h"#undef EXTERN#define EXTERN extern#include <AIFF.h>//==============================================================  Functions//--------------------------------------------------------------  SetVolume//void PLAYER_StopSound( int theChannel, int theVolume)//{//	theVolume = theVolume/(0x8000/kFullVolume);//}//--------------------------------------------------------------  SetVolumevoid PLAYER_SetVolume( int theChannel, int Volume){SndCommand	theCommand;OSErr		theErr;	Volume >>= 7;	theCommand.param1 = 0;	theCommand.param2 = (Volume<<16) | Volume;	theCommand.cmd = volumeCmd;						// Send quietCmd to stop any current sound.	theErr = SndDoImmediate( externalChannel[theChannel], &theCommand);}//--------------------------------------------------------------  StopSoundvoid PLAYER_StopSound( int theChannel){SndCommand	theCommand;OSErr		theErr;	theCommand.param1 = 0;	theCommand.param2 = 0L;	theCommand.cmd = quietCmd;						// Send quietCmd to stop any current sound.	theErr = SndDoImmediate( externalChannel[theChannel], &theCommand);	theCommand.cmd = flushCmd;						// Send flushCmd to clear the sound queue.	theErr = SndDoImmediate( externalChannel[theChannel], &theCommand);	externalPlaying[ theChannel] = false;}ExtSoundHeader ExtSndHeaders[ MAXIMUM_CHANNELS];//--------------------------------------------------------------  PlaySoundvoid PLAYER_PlaySound( Handle theSound, int theChannel, Fixed fixed, Boolean bRepeatFlag){SndCommand	theCommand;OSErr		theErr;		PLAYER_StopSound( theChannel);		ExtSndHeaders[ theChannel].samplePtr = (Ptr)((char*)*theSound);	//point to sample buffer	ExtSndHeaders[ theChannel].numChannels = 1;						//mono	ExtSndHeaders[ theChannel].sampleRate = rate11025hz;				//11025 samples per second	ExtSndHeaders[ theChannel].loopStart = 0;						//start of sample repeat	ExtSndHeaders[ theChannel].loopEnd = 0;//	if( bRepeatFlag)//		ExtSndHeaders[ theChannel].loopEnd = GetHandleSize( theSound)-1;	//end of sample to repeat		ExtSndHeaders[ theChannel].encode = extSH;						//extended sound header//	ExtSndHeaders[ theChannel].baseFrequency = 60;					//unused here	ExtSndHeaders[ theChannel].numFrames = GetHandleSize( theSound);//	ExtSndHeaders[ theChannel].AIFFSampleRate = 0;			//	ldtox80(&AIFFRate, &(**hSounds[i]).AIFFSampleRate);	//unused here	ExtSndHeaders[ theChannel].markerChunk = nil;					//unused here	ExtSndHeaders[ theChannel].instrumentChunks = nil;				//unused here	ExtSndHeaders[ theChannel].AESRecording = nil;					//unused here	ExtSndHeaders[ theChannel].sampleSize = (short)0x0008;			//16 bits per sample//	if( bRepeatFlag)//		{//		theCommand.cmd = freqCmd;							// Then, send a bufferCmd to channel 1.//		theCommand.param1 = 0;								// The sound played will be soundID.//		theCommand.param2 = 64;//		theErr = SndDoImmediate( externalChannel[theChannel], &theCommand);//		externalPlaying[ theChannel] = true;//		return;//		}	theCommand.cmd = bufferCmd;							// Then, send a bufferCmd to channel 1.	theCommand.param1 = 0;								// The sound played will be soundID.	theCommand.param2 = (long)&ExtSndHeaders[ theChannel];	theErr = SndDoImmediate( externalChannel[theChannel], &theCommand);			if( theErr == noErr)		{		externalPlaying[ theChannel] = true;		theCommand.cmd = callBackCmd;					// Lastly, queue up a callBackCmd to notify us�		theCommand.param1 = SOUND_RANDOM+theChannel;	// when the sound has finished playing.		if( bRepeatFlag)			theCommand.param1 |= 0x8000;#ifndef __POWERPC__		theCommand.param2 = SetCurrentA5();#endif		theErr = SndDoCommand( externalChannel[theChannel], &theCommand, TRUE);		}}//--------------------------------------------------------  PlayExternalSoundint PLAYER_PlayExternalSound (Handle theSound, Fixed rate, Boolean bRepeatFlag){int		a;	for( a=0; a<MAXIMUM_CHANNELS; a++)		if( externalPlaying[a] == false)			{			PLAYER_PlaySound( theSound, a, rate, bRepeatFlag);			return a;			}	return -1;}//--------------------------------------------------------  ExternalCallBack// Callback routine.  If this looks ugly, blame Apple's Universal Headers.// The callback routine is called after a sound finishes playing.  The�// callback routine is extremely useful in that it enables us to know when�// to set the sound channels priority back to 0 (meaning no sound playing).// Keep in mind (by the way) that this funciton is called at interrupt time�// and thus may not cause memory to be moved.  Also, note that also because�// of the interupt situation, we need to handle setting A5 to point to our�// app's A5 and then set it back again.RoutineDescriptor ExternalCallBackRD = BUILD_ROUTINE_DESCRIPTOR(uppSndCallBackProcInfo, PLAYER_ExternalCallBack);pascal void PLAYER_ExternalCallBack (SndChannelPtr theChannel, SndCommand *theCommand){int 	a = (theCommand -> param1 & 0x7fff) - SOUND_RANDOM;OSErr	theErr;SndCommand newCommand;#ifndef __POWERPC__long	thisA5, gameA5;		gameA5 = theCommand->param2;			// Extract our A5 from sound command.	thisA5 = SetA5(gameA5);					// Point A5 to our app (save off current A5).#endif	if( a>=0 && a<MAXIMUM_CHANNELS)		{		if( theCommand->param1 & 0x8000)			{			newCommand.param1 = 0;			newCommand.param2 = (long)&ExtSndHeaders[a];			newCommand.cmd = bufferCmd;			theErr = SndDoCommand( externalChannel[ a], &newCommand, TRUE);			newCommand.param1 = (SOUND_RANDOM+a)|0x8000;	// when the sound has finished playing.#ifndef __POWERPC__			newCommand.param2 = SetCurrentA5();#endif			newCommand.cmd = callBackCmd;					// Lastly, queue up a callBackCmd to notify us�			theErr = SndDoCommand( externalChannel[ a], &newCommand, TRUE);			}		else 			externalPlaying[ a] = 0;		}#ifndef __POWERPC__	thisA5 = SetA5(thisA5);	#endif}//--------------------------------------------------------  InitSoundvoid PLAYER_InitSound (void){OSErr theErr;int   a;		#if USESROUTINEDESCRIPTORS		externalCallBackUPP = &ExternalCallBackRD;	// Handle Universal Header ugliness.	#else		externalCallBackUPP = (SndCallBackUPP) &ExternalCallBack;	#endif		theErr = noErr;									// Assume no errors.		for( a=0; a<MAXIMUM_CHANNELS; a++)		{		externalPlaying[a] = false;		theErr = SndNewChannel(&externalChannel[a], sampledSynth, initNoInterp + initMono, (SndCallBackUPP)externalCallBackUPP);		if (theErr != noErr)							// See if it worked.			externalChannel[a] = NULL;		}}//--------------------------------------------------------  KillSoundvoid PLAYER_KillSound (void){OSErr	theErr, thisErr;int		a;	theErr = noErr;	for( a=0; a<MAXIMUM_CHANNELS; a++)		{		PLAYER_StopSound( a);		thisErr = SndDisposeChannel(externalChannel[a], TRUE);	  	externalChannel[a] = 0;	    if( thisErr != noErr)			theErr = thisErr;		}}#if DEBUGGING==ONextern void DumpAudioStuff(void);void DebugSoundChannels(){	DumpLong( 300, externalPlaying[0]<<24 | externalPlaying[1]<<16 | externalPlaying[2]<<8 | externalPlaying[3]);	DumpLong( 310, externalPlaying[4]<<24 | externalPlaying[5]<<16 | externalPlaying[6]<<8 | externalPlaying[7]);	DumpLong( 320, externalPlaying[8]<<24 | externalPlaying[9]<<16 | externalPlaying[10]<<8 | externalPlaying[11]);	DumpLong( 330, externalPlaying[12]<<24 | externalPlaying[13]<<16 | externalPlaying[14]<<8 | externalPlaying[15]);	DumpAudioStuff();}#endif