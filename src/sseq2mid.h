/**
 * sseq2mid.h: convert sseq into standard midi
 * presented by loveemu, feel free to redistribute
 */

#ifndef SSEQ2MID_H
#define SSEQ2MID_H


#include <stddef.h>
#include <stdint.h>
#include "libsmfc.h"
#include "libsmfcx.h"

// new code start
enum sseqParameterType {
	NOPARAM,
	BOOLPARAM,
	S8PARAM,
	U8PARAM,
	HEXU8PARAM, // used to display commandByte parameters as hexadecimal.
	S16PARAM,
	U16PARAM,
	HEXU24PARAM, // used for offset parameters for the commands Jump and Call
	VARLENPARAM
};
enum midiEventType {
	CC = 1,
	TEXTMARKER,
	NOTE,
	PROGRAMCHANGE,
	MASTERVOLSYSEX,
	PITCHBEND,
	TEMPOSET,
	REST,
	NEWTRACK,
	JUMP,
	CALL,
	RPNTRANSPOSE,
	RPNPITCHBENDRANGE,
	MONOPOLY,
	LOOPSTART,
	LOOPEND,
	RETURN
};

typedef struct sseqComStruct {
	char commandName[50];
	uint8_t commandByte, param1, param2, param3;
	uint8_t convToMidiEvType;
	uint8_t CCnum; // Decides what midi CC number the sseq event will be converted to. Only read if convToMidiEvType == CC.
} sseqCom;

//struct sseqCom rest = {"Rest", 0x80, VARLENPARAM, NULL, NULL, REST, 0};
// don't rely entirely on this table for conversion. Most sseq commands will have specific cases in a switch ladder to handle them. This table is to handle the many commands that just convert to a CC or Text Marker.
sseqCom sseqComList[] = {
	{"Rest", 0x80, VARLENPARAM, NOPARAM, NOPARAM, REST, 0},
	{"ProgramChange", 0x81, VARLENPARAM, NOPARAM, NOPARAM, PROGRAMCHANGE, 0},
	{"OpenTrack", 0x93, U8PARAM, HEXU24PARAM, NOPARAM, NEWTRACK, 0},
	{"Jump", 0x94, HEXU24PARAM, NOPARAM, NOPARAM, JUMP, 0},
	{"Call", 0x95, HEXU24PARAM, NOPARAM, NOPARAM, CALL, 0},
	{"Random", 0xA0, HEXU8PARAM, S16PARAM, S16PARAM, TEXTMARKER, 0},
	{"UseVar", 0xA1, HEXU8PARAM, U8PARAM, NOPARAM, TEXTMARKER, 0},
	{"If", 0xA2, HEXU8PARAM, NOPARAM, NOPARAM, TEXTMARKER, 0},
	{"Pan", 0xC0, U8PARAM, NOPARAM, NOPARAM, CC, SMF_CONTROL_PANPOT},
	{"TrackVolume", 0xC1, U8PARAM, NOPARAM, NOPARAM, CC, SMF_CONTROL_VOLUME},
	{"MasterVolume", 0xC2, U8PARAM, NOPARAM, NOPARAM, MASTERVOLSYSEX, 0},
	{"Transpose", 0xC3, S8PARAM, NOPARAM, NOPARAM, RPNTRANSPOSE, 0},
	{"PitchBend", 0xC4, S8PARAM, NOPARAM, NOPARAM, PITCHBEND, 0},
	{"PitchBendRange", 0xC5, U8PARAM, NOPARAM, NOPARAM, RPNPITCHBENDRANGE, 0},
	{"Priority", 0xC6, U8PARAM, NOPARAM, NOPARAM, CC, 14},
	{"NoteWait", 0xC7, BOOLPARAM, NOPARAM, NOPARAM, MONOPOLY, 0},
	{"Tie", 0xC8, BOOLPARAM, NOPARAM, NOPARAM, TEXTMARKER, 0},
	{"PortamentoControl", 0xC9, U8PARAM, NOPARAM, NOPARAM, CC, SMF_CONTROL_PORTAMENTOCTRL},
	{"ModDepth", 0xCA, U8PARAM, NOPARAM, NOPARAM, CC, SMF_CONTROL_MODULATION},
	{"ModSpeed", 0xCB, U8PARAM, NOPARAM, NOPARAM, CC, 21},
	{"ModType", 0xCC, U8PARAM, NOPARAM, NOPARAM, CC, 22},
	{"ModRange", 0xCD, U8PARAM, NOPARAM, NOPARAM, CC, 3},
	{"Portamento", 0xCE, BOOLPARAM, NOPARAM, NOPARAM, CC, SMF_CONTROL_PORTAMENTO},
	{"PortamentoTime", 0xCF, U8PARAM, NOPARAM, NOPARAM, CC, SMF_CONTROL_PORTAMENTOTIME},
	{"AttackRate", 0xD0, U8PARAM, NOPARAM, NOPARAM, CC, SMF_CONTROL_ATTACKTIME},
	{"DecayRate", 0xD1, U8PARAM, NOPARAM, NOPARAM, CC, SMF_CONTROL_DECAYTIME},
	{"SustainRate", 0xD2, U8PARAM, NOPARAM, NOPARAM, CC, 76},
	{"ReleaseRate", 0xD3, U8PARAM, NOPARAM, NOPARAM, CC, SMF_CONTROL_RELEASETIME},
	{"LoopStart", 0xD4, U8PARAM, NOPARAM, NOPARAM, LOOPSTART, 0},
	{"Expression", 0xD5, U8PARAM, NOPARAM, NOPARAM, CC, SMF_CONTROL_EXPRESSION},
	{"PrintVar", 0xD6, U8PARAM, NOPARAM, NOPARAM, TEXTMARKER, 0},
	{"ModDelay", 0xE0, S16PARAM, NOPARAM, NOPARAM, TEXTMARKER, 0},
	{"Tempo", 0xE1, U16PARAM, NOPARAM, NOPARAM, TEMPOSET, 0},
	{"SweepPitch", 0xE3, S16PARAM, NOPARAM, NOPARAM, TEXTMARKER, 0},
	{"LoopEnd", 0xFC, NOPARAM, NOPARAM, NOPARAM, LOOPEND, 0},
	{"Return", 0xFD, NOPARAM, NOPARAM, NOPARAM, RETURN, 0},
	{"SignifyMultiTrack", 0xFE, U16PARAM, NOPARAM, NOPARAM, 0, 0},
	{"EndOfTrack", 0xFF, NOPARAM, NOPARAM, NOPARAM, 0, 0},
};

const size_t sseqComListLen=38;

// new code end

#define SSEQ_INVALID_OFFSET     -1

typedef struct TagSseq2midTrackState
{
  int loopCount;
  int absTime;
  bool noteWait;
  size_t curOffset;
  size_t offsetToTop;
  size_t offsetToReturn;
  int offsetToAbsTime[ 262144 ]; // XXX
} Sseq2midTrackState;


#define SSEQ_MAX_TRACK          16

typedef void (Sseq2midLogProc)(const char*);

typedef struct TagSseq2mid
{
  byte* sseq;
  size_t sseqSize;
  Smf* smf;
  Sseq2midTrackState track[SSEQ_MAX_TRACK];
  Sseq2midLogProc* logProc;
  int chOrder[SSEQ_MAX_TRACK];
  bool modifyChOrder;
  bool noReverb;
  int loopCount;
} Sseq2mid;

Sseq2mid* sseq2midCreate(const byte* sseq, size_t sseqSize, bool modifyChOrder);
Sseq2mid* sseq2midCreateFromFile(const char* filename, bool modifyChOrder);
void sseq2midDelete(Sseq2mid* sseq2mid);
Sseq2mid* sseq2midCopy(Sseq2mid* sseq2mid);
bool sseq2midConvert(Sseq2mid* sseq2mid);
size_t sseq2midWriteMidi(Sseq2mid* sseq2mid, byte* buffer, size_t bufferSize);
size_t sseq2midWriteMidiFile(Sseq2mid* sseq2mid, const char* filename);
void sseq2midSetLogProc(Sseq2mid* sseq2mid, Sseq2midLogProc* logProc);
bool sseq2midNoReverb(Sseq2mid* sseq2mid, bool noReverb);
int sseq2midSetLoopCount(Sseq2mid* sseq2mid, int loopCount);


#endif /* !SSEQ2MID_H */
