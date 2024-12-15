/**
 * sseq2mid.c: convert sseq into standard midi
 * presented by loveemu, feel free to redistribute
 * see sseq2midConvert to know conversion detail :)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sseq2mid.h"
#include <stdint.h>

#ifndef countof
#define countof(a)  (sizeof(a) / sizeof(a[0]))
#endif

#define SSEQ2MID_NAME "sseq2mid"
#define SSEQ2MID_VER "20070314"

bool g_log = false;
bool g_modifyChOrder = false;
bool g_noReverb = false;
int g_loopCount = 1; 
int g_loopStyle = 0;
bool g_spacer = false;

void dispatchLogMsg(const char* logMsg);
bool dispatchOptionChar(const char optChar);
bool dispatchOptionStr(const char* optString);
void showUsage(void);
int main(int argc, char* argv[]);


void sseq2midPutLog(Sseq2mid* sseq2mid, const char* logMessage);
void sseq2midPutLogLine(Sseq2mid* sseq2mid, size_t offset, size_t size, 
	const char* description, const char* comment);
int sseq2midSseqChToMidiCh(Sseq2mid* sseq2mid, int sseqChannel);

int getS1From(byte* data);
int getS2LitFrom(byte* data);
int getS3LitFrom(byte* data);
int getS4LitFrom(byte* data);
unsigned int getU1From(byte* data);
unsigned int getU2LitFrom(byte* data);
unsigned int getU3LitFrom(byte* data);
unsigned int getU4LitFrom(byte* data);


/* dispatch log message */
void dispatchLogMsg(const char* logMsg)
{
	printf(logMsg);	 /* output to stdout */
}

/* dispatch option character */
bool dispatchOptionChar(const char optChar)
{
	switch(optChar)
	{
	case '0':
		g_noReverb = true;
		break;

	case '1':
		g_loopCount = 1;
		break;

	case '2':
		g_loopCount = 2;
		break;

	case '3':
		g_loopCount = 3;
		break;

	case '7':
		g_loopStyle = 2;
		break;

	case 'd':
		g_loopStyle = 1;
		break;

	case 'l':
		g_log = true;
		break;

	case 'm':
		g_modifyChOrder = true;
		break;
	
	case 's':
		g_spacer = true;
		break;

	default:
		return false;
	}
	return true;
}

/* dispatch option string */
bool dispatchOptionStr(const char* optString)
{
	if(strcmp(optString, "help") == 0)
	{
		showUsage();
	}
	else if(strcmp(optString, "log") == 0)
	{
		g_log = true;
	}
	else if(strcmp(optString, "modify-ch") == 0)
	{
		g_modifyChOrder = true;
	}
	else if(strcmp(optString, "reverb0") == 0)
	{
		g_noReverb = true;
	}
	else if(strcmp(optString, "1loop") == 0)
	{
		g_loopCount = 1;
	}
	else if(strcmp(optString, "2loop") == 0)
	{
		g_loopCount = 2;
	}
	else if(strcmp(optString, "3loop") == 0)
	{
		g_loopCount = 3;
	}
	else if(strcmp(optString, "loopstyle1") == 0)
	{
			g_loopStyle = 1;
	}
	else if(strcmp(optString, "loopstyle2") == 0)
	{
			g_loopStyle = 2;
	}
	else if(strcmp(optString, "spacer") == 0)
	{
			g_spacer = true;
	}
	else
	{
		return false;
	}
	return true;
}

/* show sseq2mid usage */
void showUsage(void)
{
	const char* options[] = {
		"", "--help", "show this usage", 
		"-0", "--noreverb", "set 0 to reverb send", 
		"-1", "--1loop", "convert to 1 loop (no loop)", 
		"-2", "--2loop", "convert to 2 loop", 
		"-d", "--loopstyle1", "Duke nukem style loop points (Event 0x74/0x75)",
		"-7", "--loopstyle2", "FF7 PC style loop points (Meta text \"loop(start/end)\"",
		"-l", "--log", "put conversion log", 
		"-m", "--modify-ch", "modify midi channel to avoid rhythm channel"
		"-s", "--spacer", "(EXPERIMENTAL) insert a short rest in between simultaneous events"
	};
	int optIndex;

	puts("usage	: sseq2mid (options) [input-files]");
	puts("options:");
	for(optIndex = 0; optIndex < countof(options); optIndex += 3)
	{
		printf("	%-2s	%-16s	%s\n", options[optIndex], options[optIndex + 1], options[optIndex + 2]);
	}
	puts("____");
	puts(SSEQ2MID_NAME" ["SSEQ2MID_VER"] by loveemu");
}

/* sseq2mid application main */
int main(int argc, char* argv[])
{
	int argi = 1;
	int argci;

	if(argc == 1) /* no arguments */
	{
		showUsage();
	}
	else
	{
		/* options */
		while((argi < argc) && (argv[argi][0] == '-'))
		{
			if(argv[argi][1] == '-') /* --string */
			{
				dispatchOptionStr(&argv[argi][2]);
			}
			else /* -letters (alphanumeric only) */
			{
				argci = 1;
				while(argv[argi][argci] != '\0')
				{
					dispatchOptionChar(argv[argi][argci]);
					argci++;
				}
			}
			argi++;
		}

		/* input files */
		for(; argi < argc; argi++)
		{
			Sseq2mid* sseq2mid = sseq2midCreateFromFile(argv[argi], g_modifyChOrder);
			bool convResult;

			if(sseq2mid)
			{
				char* midFilename;

				midFilename = (char*) malloc((strlen(argv[argi]) + 5) * sizeof(char));
				if(midFilename)
				{
					sprintf(midFilename, "%s.mid", argv[argi]);

					sseq2midSetLoopCount(sseq2mid, g_loopCount);
					sseq2midNoReverb(sseq2mid, g_noReverb);
					if(g_log)
					{
						sseq2midSetLogProc(sseq2mid, dispatchLogMsg);
					}
					sseq2midPutLog(sseq2mid, argv[argi]);
					sseq2midPutLog(sseq2mid, ":\n");
					fprintf(stderr, "%s:\n", argv[argi]);
					convResult = sseq2midConvert(sseq2mid);
					if(!convResult)
					{
						fprintf(stderr, "error: conversion failed\n", argv[argi]);
					}
					sseq2midWriteMidiFile(sseq2mid, midFilename);
					sseq2midDelete(sseq2mid);

					free(midFilename);
				}
				else
				{
					fprintf(stderr, "error: memory allocation failed\n", argv[argi]);
				}
			}
			else
			{
				fprintf(stderr, "error: I/O initialize error\n", argv[argi]);
			}
		}
	}
	return 0;
}


/* call the fuction to put log message */
void sseq2midPutLog(Sseq2mid* sseq2mid, const char* logMessage)
{
	if(sseq2mid && sseq2mid->logProc)
	{
		sseq2mid->logProc(logMessage);
	}
}

#define SSEQ2MID_MAX_DUMP	 5

/* put log message in prescribed form */
void sseq2midPutLogLine(Sseq2mid* sseq2mid, size_t offset, size_t size, 
	const char* description, const char* comment)
{
	if(sseq2mid)
	{
		char logMsg[96];
		char hexDump[24];
		char hexDumpPart[8];
		size_t sizeToTransfer;
		size_t transferedSize;
		byte* sseq = sseq2mid->sseq;
		size_t sseqSize = sseq2mid->sseqSize;

		sizeToTransfer = (offset + size <= sseqSize) ? size : sseqSize - offset;
		sizeToTransfer = (sizeToTransfer < SSEQ2MID_MAX_DUMP) ? sizeToTransfer : SSEQ2MID_MAX_DUMP;

		strcpy(hexDump, "");
		for(transferedSize = 0; transferedSize < sizeToTransfer; transferedSize++)
		{
			if(transferedSize != 0)
			{
				strcat(hexDump, " ");
			}
			sprintf(hexDumpPart, "%02X", sseq[offset + transferedSize]);
			strcat(hexDump, hexDumpPart);
		}

		sprintf(logMsg, "%08X: %-14s | %-20s | %s\n", offset, hexDump, 
			description ? description : "", comment ? comment : "");
		sseq2midPutLog(sseq2mid, logMsg);
	}
}

/* filter: sseq channel number to midi track number */
int sseq2midSseqChToMidiCh(Sseq2mid* sseq2mid, int sseqChannel)
{
	return ((sseqChannel >= 0) && (sseqChannel <= SSEQ_MAX_TRACK)) ? sseq2mid->chOrder[sseqChannel] : sseqChannel;
}

/* create sseq2mid object */
Sseq2mid* sseq2midCreate(const byte* sseq, size_t sseqSize, bool modifyChOrder)
{
	Sseq2mid* newSseq2mid = (Sseq2mid*) calloc(1, sizeof(Sseq2mid));

	if(newSseq2mid)
	{
		newSseq2mid->sseq = (byte*) malloc(sseqSize);
		if(newSseq2mid->sseq)
		{
			newSseq2mid->smf = smfCreate();
			if(newSseq2mid->smf)
			{
				memcpy(newSseq2mid->sseq, sseq, sseqSize);
				newSseq2mid->sseqSize = sseqSize;

				smfSetTimebase(newSseq2mid->smf, 48);
				newSseq2mid->loopCount = 1;
				newSseq2mid->noReverb = false;
				newSseq2mid->modifyChOrder = modifyChOrder;
			}
			else
			{
				free(newSseq2mid->sseq);
				free(newSseq2mid);
				newSseq2mid = NULL;
			}
		}
		else
		{
			free(newSseq2mid);
			newSseq2mid = NULL;
		}
	}
	return newSseq2mid;
}

/* create sseq2mid object from file */
Sseq2mid* sseq2midCreateFromFile(const char* filename, bool modifyChOrder)
{
	Sseq2mid* newSseq2mid = NULL;
	FILE* sseqFile = fopen(filename, "rb");

	if(sseqFile)
	{
		size_t sseqSize;
		byte* sseq;

		fseek(sseqFile, 0, SEEK_END);
		sseqSize = (size_t) ftell(sseqFile);
		rewind(sseqFile);

		sseq = (byte*) malloc(sseqSize);
		if(sseq)
		{
			fread(sseq, sseqSize, 1, sseqFile);
			newSseq2mid = sseq2midCreate(sseq, sseqSize, modifyChOrder);
			free(sseq);
		}

		fclose(sseqFile);
	}
	return newSseq2mid;
}

/* delete sseq2mid object */
void sseq2midDelete(Sseq2mid* sseq2mid)
{
	if(sseq2mid)
	{
		smfDelete(sseq2mid->smf);
		free(sseq2mid->sseq);
		free(sseq2mid);
	}
}

/* copy sseq2mid object */
Sseq2mid* sseq2midCopy(Sseq2mid* sseq2mid)
{
	Sseq2mid* newSseq2mid = NULL;

	if(sseq2mid)
	{
		newSseq2mid = sseq2midCreate(sseq2mid->sseq, sseq2mid->sseqSize, sseq2mid->modifyChOrder);
		if(newSseq2mid)
		{
			smfDelete(newSseq2mid->smf);
			newSseq2mid->smf = smfCopy(sseq2mid->smf);
			if(newSseq2mid->smf)
			{
				sseq2midSetLogProc(newSseq2mid, sseq2mid->logProc);
				sseq2midSetLoopCount(newSseq2mid, sseq2mid->loopCount);
			}
			else
			{
				sseq2midDelete(newSseq2mid);
				newSseq2mid = NULL;
			}
		}
	}
	return newSseq2mid;
}

#define SSEQ_MIN_SIZE	 0x1d

/* sseq2mid conversion main, enjoy my dirty code :P */
bool sseq2midConvert(Sseq2mid* sseq2mid)
{
	bool result = false;
	char strForLog[64];
	int loopStartCount;
	size_t loopStartOffset;
	bool loopPointUsed = false;
	bool loopStartPointUsed = false;
	bool loopEndPointUsed = false;

	if(sseq2mid)
	{
		byte* sseq = sseq2mid->sseq;
		size_t sseqSize = sseq2mid->sseqSize;
		Smf* smf = sseq2mid->smf;

		if((sseqSize >= SSEQ_MIN_SIZE) && 
				(sseq[0x00] == 'S') && (sseq[0x01] == 'S') && (sseq[0x02] == 'E') && (sseq[0x03] == 'Q') && 
				(sseq[0x10] == 'D') && (sseq[0x11] == 'A') && (sseq[0x12] == 'T') && (sseq[0x13] == 'A'))
		{
			int trackIndex;
			int midiCh;
			size_t sseqOffsetBase;
			int midiChOrder[SSEQ_MAX_TRACK] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 9 };

			/* put SSEQ header info */
			sseq2midPutLogLine(sseq2mid, 0x00, 4, "Signature", "SSEQ");
			sseq2midPutLogLine(sseq2mid, 0x04, 2, "", "Unknown");
			sseq2midPutLogLine(sseq2mid, 0x06, 2, "", "Unknown");
			sprintf(strForLog, "%u", getU4LitFrom(&sseq[0x08]));
			sseq2midPutLogLine(sseq2mid, 0x08, 4, "SSEQ file size", strForLog);
			sseq2midPutLogLine(sseq2mid, 0x0c, 2, "", "Unknown");
			sseq2midPutLogLine(sseq2mid, 0x0e, 2, "", "Unknown");
			sseq2midPutLog(sseq2mid, "\n");

			/* put DATA chunk header */
			sseq2midPutLogLine(sseq2mid, 0x10, 4, "Signature", "DATA");
			sprintf(strForLog, "%u", getU4LitFrom(&sseq[0x14]));
			sseq2midPutLogLine(sseq2mid, 0x14, 4, "DATA chunk size", strForLog);
			sseqOffsetBase = (size_t) getU4LitFrom(&sseq[0x18]);
			sprintf(strForLog, "%08X", sseqOffsetBase);
			sseq2midPutLogLine(sseq2mid, 0x18, 4, "Offset Base", strForLog);
			sseq2midPutLog(sseq2mid, "\n");

			/* initialize channel order */
			for(midiCh = 0; midiCh < SSEQ_MAX_TRACK; midiCh++)
			{
				sseq2mid->chOrder[midiCh] = sseq2mid->modifyChOrder ? midiChOrder[midiCh] : midiCh;
			}

			/* initialize track settings */
			sseq2mid->track[0].loopCount = sseq2mid->loopCount;
			sseq2mid->track[0].absTime = 0;
			sseq2mid->track[0].noteWait = false;
			sseq2mid->track[0].offsetToTop = 0x1c;
			sseq2mid->track[0].offsetToReturn = SSEQ_INVALID_OFFSET;
			sseq2mid->track[0].curOffset = sseq2mid->track[0].offsetToTop;
			for(trackIndex = 1; trackIndex < SSEQ_MAX_TRACK; trackIndex++)
			{
				sseq2mid->track[trackIndex].loopCount = 0;	/* inactive */
				sseq2mid->track[trackIndex].noteWait = false;
			}

			/* initialize midi */
#if 0
			smfInsertGM1SystemOn(smf, 0, 0, 0);
#endif

			/* convert each track */
			result = true;
			for(trackIndex = 0; trackIndex < SSEQ_MAX_TRACK; trackIndex++)
			{
				int loopCount = sseq2mid->track[trackIndex].loopCount;
				
				int stackedEventTimeSpacer = 0; // used to space out midi events that, in the original sseq, happen at the same time (but in a specific order). The spacer ensures that the order of events is the same as in the sseq.
				
				byte prevStatusByte=0x00;

				if(loopCount > 0)
				{
					do
					{
						int absTime = sseq2mid->track[trackIndex].absTime; // absTime does not persist through each loop, but sseq2mid->track[trackIndex].absTime does. All assignments to absTime go to sseq2mid->track[trackIndex].absTime
						size_t curOffset = sseq2mid->track[trackIndex].curOffset;
						size_t eventOffset = curOffset;
						char eventName[64];
						char eventDesc[64];
						bool eventException;
						size_t eventExceptionOffset = curOffset;
						size_t offsetToJump = SSEQ_INVALID_OFFSET;

						midiCh = sseq2midSseqChToMidiCh(sseq2mid, trackIndex);
						sprintf(eventName, "Access Violation");
						sprintf(eventDesc, "End of File at %08X", sseqSize);
						eventException = true;

						if(curOffset < sseqSize)
						{
							byte statusByte;
					
							sseq2mid->track[trackIndex].offsetToAbsTime[curOffset] = absTime;

							statusByte = getU1From(&sseq[curOffset]);
							curOffset++;

							sprintf(eventName, "Unknown Event %02X", statusByte);
							sprintf(eventDesc, "");
							eventException = false;

							if(statusByte < 0x80)
							{
								if (g_spacer) {
									if (prevStatusByte < 0x80) stackedEventTimeSpacer--;
								}
								
								int velocity;
								int duration;
								const char* noteName[] = {
									"C ", "C#", "D ", "D#", "E ", "F ", 
									"F#", "G ", "G#", "A ", "A#", "B "
								};

								velocity = getU1From(&sseq[curOffset]);
								curOffset++;
								duration = smfReadVarLength(&sseq[curOffset], sseqSize - curOffset);
								curOffset += smfGetVarLengthSize(duration);

								smfInsertNote(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, statusByte, velocity, duration);
								if(sseq2mid->track[trackIndex].noteWait)
								{
									absTime += duration;
									stackedEventTimeSpacer=0;
								}

								sprintf(eventName, "Note with Duration");
								sprintf(eventDesc, "%s %d [%d]	vel:%-3d dur:%-3d", noteName[statusByte % 12], 
									(statusByte / 12) - 1, statusByte, velocity, duration);
							}
							else
							{
								switch(statusByte)
								{
								case 0x80: // Match sseq commands with equivalent midi commands, and if no equivalent midi commands exist, match them with undefined midi CCs. This program is compatible with my fork of midi2sseq, and it is possible to convert back and forth between sseq and midi (mostly) losslessly.
								{
									int tick;

									tick = smfReadVarLength(&sseq[curOffset], sseqSize - curOffset);
									curOffset += smfGetVarLengthSize(tick);

									absTime += tick;
									
									stackedEventTimeSpacer=0;

									sprintf(eventName, "Rest");
									sprintf(eventDesc, "%d", tick);
									break;
								}

								case 0x81:
								{
									int realProgram;
									int bankMsb;
									int bankLsb;
									int program;

									realProgram = smfReadVarLength(&sseq[curOffset], sseqSize - curOffset);
									curOffset += smfGetVarLengthSize(realProgram);

									program = realProgram % 128;
									bankLsb = (realProgram / 128) % 128;
									bankMsb = (realProgram / 128 / 128) % 128;
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_BANKSELM, bankMsb);
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_BANKSELL, bankLsb);
									smfInsertProgram(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, program);

									sprintf(eventName, "Program Change");
									sprintf(eventDesc, "%d", realProgram);
									break;
								}

								case 0x93: /* ('A`) */
								{
									int newTrackIndex;
									int offset;

									newTrackIndex = getU1From(&sseq[curOffset]);
									curOffset += 1;
									offset = getU3LitFrom(&sseq[curOffset]) + sseqOffsetBase;
									curOffset += 3;

									sseq2mid->track[newTrackIndex].loopCount = loopCount;
									sseq2mid->track[newTrackIndex].absTime = absTime;
									sseq2mid->track[newTrackIndex].offsetToTop = offset;
									sseq2mid->track[newTrackIndex].offsetToReturn = SSEQ_INVALID_OFFSET;
									sseq2mid->track[newTrackIndex].curOffset = offset;

									sprintf(eventName, "Open Track");
									sprintf(eventDesc, "Track %02d at %08Xh", newTrackIndex + 1, offset);
									break;
								}

								case 0x94:
								{
									int newOffset;

									newOffset = getU3LitFrom(&sseq[curOffset]) + sseqOffsetBase;
									curOffset += 3;

									offsetToJump = newOffset;

									if(offsetToJump >= sseq2mid->track[trackIndex].offsetToTop)
									{
										if(offsetToJump < curOffset)
										{
											switch(g_loopStyle)
											{
											case 0:
												loopCount--;
												break;
											
											case 1: 
												if(!loopPointUsed)
												{
														smfInsertControl(smf, sseq2mid->track[trackIndex].offsetToAbsTime[offsetToJump], midiCh, midiCh, 0x74, 0);
														smfInsertControl(smf, absTime, midiCh, midiCh, 0x75, 0);
														loopPointUsed = true;
												}
												loopCount = 0;
												break;

											case 2:
												if(!loopPointUsed)
												{
														smfInsertMetaEvent(smf, sseq2mid->track[trackIndex].offsetToAbsTime[offsetToJump], midiCh, 6, "loopStart", 9);
														smfInsertMetaEvent(smf, absTime+stackedEventTimeSpacer, midiCh, 6, "loopEnd", 7);
														loopPointUsed = true;
												}
												loopCount = 0;
												break;
											}
										}
										else
										{
											/* jump to forward */
										}
									}
									else
									{
										/* redirect */
									}

									sprintf(eventName, "Jump");
									sprintf(eventDesc, "%08X", newOffset);
									break;
								}

								case 0x95:
								{
									// TODO idea: Put a marker in the midi everytime there's a call command in the sseq; also put a marker for the return. Then, in midi2sseq, read these markers and recreate the call.
									int newOffset;

									newOffset = getU3LitFrom(&sseq[curOffset]) + sseqOffsetBase;
									curOffset += 3;

									sseq2mid->track[trackIndex].offsetToReturn = curOffset;
									offsetToJump = newOffset;

									sprintf(eventName, "Call");
									sprintf(eventDesc, "%08X", newOffset);
									break;
								}

								case 0xa0: /* Hanjuku Hero DS: NSE_45, New Mario Bros: BGM_AMB_CHIKA, Slime Morimori Dragon Quest 2: SE_187, SE_210, Advance Wars */
								{
									byte subStatusByte;
									int16_t randMin;
									int16_t randMax;

									subStatusByte = getU1From(&sseq[curOffset]);
									curOffset++;
									randMin = getU2LitFrom(&sseq[curOffset]);
									curOffset += 2;
									randMax = getU2LitFrom(&sseq[curOffset]);
									curOffset += 2;

									char markerText[26]; // "Random:0xFF,-32767,-32767"
									snprintf(markerText, 26, "Random:0x%02X,%d,%d", subStatusByte, randMin, randMax);
									smfInsertMetaEvent(smf, absTime+stackedEventTimeSpacer, midiCh, 6, markerText, 25);

									sprintf(eventName, "Random (%02X)", subStatusByte);
									sprintf(eventDesc, "Min:%d Max:%d", randMin, randMax);
									break;
								}

								case 0xa1: /* New Mario Bros: BGM_AMB_SABAKU */
								{
									// 0xA1. params: Sequence Command, u8. Replaces the last parameter of P1 with the value of variable P2
									byte subStatusByte;
									int varNumber;

									subStatusByte = getU1From(&sseq[curOffset]);
									curOffset++;
									if(subStatusByte >= 0xb0 && subStatusByte <= 0xbd) /* var */
									{
										/* loveemu is a lazy person :P */
										curOffset++;
										varNumber = getU1From(&sseq[curOffset]);
										curOffset++;
									}
									else
									{
										varNumber = getU1From(&sseq[curOffset]);
										curOffset++;
									}

									char markerText[16]; // "UseVar:0xFF,255"
									snprintf(markerText, 16, "UseVar:0x%02X,%u", subStatusByte, (uint8_t)varNumber);
									smfInsertMetaEvent(smf, absTime+stackedEventTimeSpacer, midiCh, 6, markerText, 15);

									sprintf(eventName, "From Var (%02X)", subStatusByte);
									sprintf(eventDesc, "var %d", varNumber);
									break;
								}

								case 0xa2:
								{
									// params: Sequence Command. Executes P1 if the track's conditional flag is set
									byte subStatusByte;
									subStatusByte = getU1From(&sseq[curOffset]);
									
									char markerText[8]; // If:0xFF
									snprintf(markerText, 8, "If:0x%02X", subStatusByte);
									smfInsertMetaEvent(smf, absTime+stackedEventTimeSpacer, midiCh, 6, markerText, 7);

									sprintf(eventName, "If");
									sprintf(eventDesc, "");
									break;
								}

								case 0xb0: /* Children of Mana: SEQ_BGM001 */
								case 0xb1: /* Advance Wars - Dual Strike: SE_TAGPT_COUNT01 */
								case 0xb2:
								case 0xb3:
								case 0xb4:
								case 0xb5:
								case 0xb6: /* Mario Kart DS: 76th sequence */
								case 0xb8: /* Tottoko Hamutaro: MUS_ENDROOL, Nintendogs */
								case 0xb9:
								case 0xba:
								case 0xbb:
								case 0xbc:
								case 0xbd:
								{
									uint8_t varNumber;
									int16_t val;
									const char* varMethodName[] = {
										"=", "+=", "-=", "*=", "/=", "[Shift]", "[Rand]", "", 
										"==", ">=", ">", "<=", "<", "!="
									};

									varNumber = getU1From(&sseq[curOffset]);
									curOffset++;
									val = getU2LitFrom(&sseq[curOffset]);
									curOffset += 2;

									char markerText[23]; // "Var:255,[Shift],-32767"
									snprintf(markerText, 23, "Var:%u,%s,%d", varNumber, varMethodName[statusByte - 0xb0], val);
									smfInsertMetaEvent(smf, absTime+stackedEventTimeSpacer, midiCh, 6, markerText, 22);

									sprintf(eventName, "Variable %s", varMethodName[statusByte - 0xb0]);
									sprintf(eventDesc, "var %u : %d", varNumber, val);
									break;
								}

								case 0xc0:
								{
									int pan;

									pan = getU1From(&sseq[curOffset]);
									curOffset++;

									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_PANPOT, pan);

									sprintf(eventName, "Pan");
									//sprintf(eventDesc, "%d", pan - 64);
									sprintf(eventDesc, "%d", pan); // printing it like this more closely matches midi and sseq
									break;
								}

								case 0xc1:
								{
									int vol;

									vol = getU1From(&sseq[curOffset]);
									curOffset++;

									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_VOLUME, vol);

									// sprintf(eventName, "Volume");
									sprintf(eventName, "Track Volume"); // according to Gota7's sequence.md
									sprintf(eventDesc, "%d", vol);
									break;
								}

								case 0xc2: /* Dawn of Sorrow: SDL_BGM_BOSS1_ */
								{
									int vol;

									vol = getU1From(&sseq[curOffset]);
									curOffset++;

									smfInsertMasterVolume(smf, absTime+stackedEventTimeSpacer, 0, midiCh, vol);

									// sprintf(eventName, "Master Volume");
									sprintf(eventName, "Player Volume"); // according to Gota7's sequence.md. Likely changes the volume of the master track
									sprintf(eventDesc, "%d", vol);
									break;
								}

								case 0xc3: /* Puyo Pop Fever 2: BGM00 */
								{
									int transpose;

									transpose = getS1From(&sseq[curOffset]); // I think this is equal to the number of semitones moved. in ex song in vgmtrans: C30C is read as "12", 12 semitones (an octave) makes sense. -12 is probably down 12 semitones.
									curOffset++;

									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_RPNM, 0);
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_RPNL, 2);
									//smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_DATAENTRYM, 64 + transpose);
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_DATAENTRYM, transpose + 32/*0x20*/);
									// "Coarse tuning: The coarse tuning RPNs use only the coarse data entry message to tune, with 0x20 representing central tuning of A = 440 Hz and with increments of whole semitones (e.g., 0x21 would be a whole semitone displacement up)."
									// https://www.recordingblogs.com/wiki/midi-registered-parameter-number-rpn

									sprintf(eventName, "Transpose");
									sprintf(eventDesc, "%d", transpose);
									break;
								}

								case 0xc4:
								{
									// signed value. C400 is normal pitch (midi pitch 8192 / +0)
									// WARNING: midi pitch bend is a 14-bit integer, while sseq pitch bend is a 16-bit signed integer. Conversion cannot be completely lossless.
									int bend;

									bend = getS1From(&sseq[curOffset]) * 64;
									curOffset++;

									smfInsertPitchBend(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, bend);

									sprintf(eventName, "Pitch Bend");
									sprintf(eventDesc, "%d", bend);
									break;
								}

								case 0xc5:
								{
									int range;

									range = getU1From(&sseq[curOffset]); // number of semitones. TODO: find out if negative values are valid
									curOffset++;

									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_RPNM, 0);
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_RPNL, 0);
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_DATAENTRYM, range);

									sprintf(eventName, "Pitch Bend Range");
									sprintf(eventDesc, "%d", range);
									break;
								}

								case 0xc6: /* Children of Mana: SEQ_BGM000 */
								{
									int priority;

									priority = getU1From(&sseq[curOffset]);
									curOffset++;
									
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, 14, priority);

									sprintf(eventName, "Priority");
									sprintf(eventDesc, "%d", priority);
									break;
								}

								case 0xc7: /* Dawn of Sorrow: SDL_BGM_ARR1_ */
								{
									// sequence.md describes this as "note wait mode": "Is off by default, but if on waits for a note to finish before continuing". "waiting for a note to finish" effectively means that the music/channel is monophonic (only one note can play at a time) instead of polyphonic (multiple notes can play at the same time)
									int flg;

									flg = getU1From(&sseq[curOffset]);
									curOffset++;

									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, flg ? SMF_CONTROL_MONO : SMF_CONTROL_POLY, 0);
									sseq2mid->track[trackIndex].noteWait = flg ? true : false;

									sprintf(eventName, "Mono/Poly");
									sprintf(eventDesc, "%s (%d)", flg ? "Mono" : "Poly", flg);
									break;
								}

								case 0xc8: /* Hanjuku Hero DS: NSE_42 */
								{
									int flg;

									flg = getU1From(&sseq[curOffset]);
									curOffset++;

									// "If on, notes don't end and new notes just change the pitch and velocity of the playing note"
									char markerText[8]; // Tie:Off
									snprintf(markerText, 8, "Tie:%s", flg ? "On" : "Off");
									smfInsertMetaEvent(smf, absTime+stackedEventTimeSpacer, midiCh, 6, markerText, 7);

									sprintf(eventName, "Tie");
									sprintf(eventDesc, "%s (%d)", flg ? "On" : "Off", flg);
									break;
								}

								case 0xc9: /* Hanjuku Hero DS: NSE_50 */
								{
									int key;

									key = getU1From(&sseq[curOffset]);
									curOffset++;

									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_PORTAMENTOCTRL, key);

									sprintf(eventName, "Portamento Control");
									sprintf(eventDesc, "%d", key);
									break;
								}

								case 0xca: /* Dawn of Sorrow: SDL_BGM_ARR1_ */
								{
									int amount;

									amount = getU1From(&sseq[curOffset]);
									curOffset++;

									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_MODULATION, amount);

									sprintf(eventName, "Modulation Depth");
									sprintf(eventDesc, "%d", amount);
									break;
								}

								case 0xcb: /* Dawn of Sorrow: SDL_BGM_ARR1_ */
								{
									int amount;

									amount = getU1From(&sseq[curOffset]);
									curOffset++;

									//smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_VIBRATORATE, 64 + amount / 2);
									// SMF_CONTROL_VIBRATORATE is cc76, which is Sound Controller 7: "Generic – Some manufacturers may use to further shave their sounds."
									// https://nickfever.com/music/midi-cc-list
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, /*cc*/21 /*same cc as gba_mus_ripper*/, amount); // other uint8 values, like 0xCA mod depth, seem to stay in between 0 and 127. This is also less lossy.

									sprintf(eventName, "Modulation Speed");
									sprintf(eventDesc, "%d", amount);
									break;
								}

								case 0xcc: /* Children of Mana: SEQ_BGM001 */
								{
									int type;
									char* typeStr[] = { "Pitch", "Volume", "Pan" };

									type = getU1From(&sseq[curOffset]);
									curOffset++;
									
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, /*cc*/22, type); // In the future, I may use cc110 and cc111 like gba_mus_ripper, but that would require writing/forking an nds sound bank ripper to add modulators to the sf2.

									sprintf(eventName, "Modulation Type");
									sprintf(eventDesc, "%s", typeStr[type]);
									break;
								}

								case 0xcd: /* Phoenix Wright: BGM021 */
								{
									int amount;

									amount = getU1From(&sseq[curOffset]);
									curOffset++;

									//smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_VIBRATODEPTH, 64 + amount / 2); // SMF_CONTROL_VIBRATODEPTH is also a generic sound controller.
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, /*cc*/3, amount);

									sprintf(eventName, "Modulation Range"); // TODO: how is this different from mod depth?
									sprintf(eventDesc, "%d", amount);
									break;
								}

								case 0xce: /* Dawn of Sorrow: SDL_BGM_ARR1_ */
								{
									// "Portamento Switch: Enters or cancels portamento mode"
									int flg; // TODO: research possible values of flg for midi2sseq conversion.

									flg = getU1From(&sseq[curOffset]);
									curOffset++;

									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_PORTAMENTO /*TODO: change name to PORTAMENTOSWITCH*/, !flg ? 0 : 127);

									sprintf(eventName, "Portamento");
									sprintf(eventDesc, "%s (%d)", flg ? "On" : "Off", flg);
									break;
								}

								case 0xcf: /* Bomberman: SEQ_AREA04 */
								{
									int time;

									time = getU1From(&sseq[curOffset]);
									curOffset++;

									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_PORTAMENTOTIME, time);

									sprintf(eventName, "Portamento Time");
									sprintf(eventDesc, "%d", time);
									break;
								}

								case 0xd0: /* Dawn of Sorrow: SDL_BGM_WIND_ */
								{
									int amount;

									amount = getU1From(&sseq[curOffset]); 
									curOffset++;
#if 0
									smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_ATTACKTIME, 64 + amount / 2);
#endif
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_ATTACKTIME, amount); // This may also require a modulator, but the semantic meaning of the midicc matches up.
									sprintf(eventName, "Attack Rate"); 
									sprintf(eventDesc, "%d", amount);
									break;
								}

								case 0xd1: /* Dawn of Sorrow: SDL_BGM_WIND_ */
								{
									int amount;

									amount = getU1From(&sseq[curOffset]); 
									curOffset++;
#if 0
									smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_DECAYTIME, 64 + amount / 2);
#endif
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_DECAYTIME, amount); // This is a generic sound controller. I'll leave it as a generic sound controller since attack and release are also sound controllers.
									sprintf(eventName, "Decay Rate");
									sprintf(eventDesc, "%d", amount);
									break;
								}

								case 0xd2: /* Dawn of Sorrow: SDL_BGM_WIND_ */
								{
									int amount;

									amount = getU1From(&sseq[curOffset]); 
									curOffset++;
									
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, /*cc*/76, amount); // This is a generic sound controller

									sprintf(eventName, "Sustain Rate");
									sprintf(eventDesc, "%d", amount);
									break;
								}

								case 0xd3: /* Dawn of Sorrow: SDL_BGM_WIND_ */
								{
									int amount;

									amount = getU1From(&sseq[curOffset]);
									curOffset++;
#if 0
									smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_RELEASETIME, 64 + amount / 2);
#endif
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_RELEASETIME, amount);
									sprintf(eventName, "Release Rate");
									sprintf(eventDesc, "%d", amount);
									break;
								}

								case 0xd4: /* Dawn of Sorrow: SDL_BGM_WIND_ */
								{

									loopStartCount = getU1From(&sseq[curOffset]);
									curOffset++;

									loopStartOffset = curOffset;
									if(loopStartCount == 0)
									{
											loopStartCount = -1;
											if(!loopStartPointUsed)
											{
													switch(g_loopStyle)
													{
													case 1:
															smfInsertControl(smf, absTime, midiCh, midiCh, 0x74, 0);
															break;
													case 2:
															smfInsertMetaEvent(smf, absTime, midiCh, 6, "loopStart", 9);
															break;
													}
													loopStartPointUsed = true;
											}
									}

									sprintf(eventName, "Loop Start");
									sprintf(eventDesc, "%d", loopStartCount);
									break;
								}

								case 0xd5:
								{
									// "Volume 2: Sets track volume 2 to P1"
									int expression;

									expression = getU1From(&sseq[curOffset]);
									curOffset++;

									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, SMF_CONTROL_EXPRESSION, expression);

									sprintf(eventName, "Expression");
									sprintf(eventDesc, "%d", expression);
									break;
								}

								case 0xd6:
								{
									int varNumber;

									varNumber = getU1From(&sseq[curOffset]);
									curOffset++;

									/* TEST */
									char markerText[13];
									snprintf(markerText, 13, "PrintVar:%u", (uint8_t)varNumber);
									smfInsertMetaEvent(smf, absTime+stackedEventTimeSpacer, midiCh, 6, markerText, 12);

									sprintf(eventName, "Print Variable");
									sprintf(eventDesc, "%d", varNumber);
									break;
								}

								case 0xe0: /* Children of Mana: SEQ_BGM001 */
								{
									// ex song has E04600 (little endian) which should be 70 in decimal.
									int amount;

									amount = getU2LitFrom(&sseq[curOffset]);
									curOffset += 2;

									//smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_VIBRATODELAY, 64 + amount / 2);
									// (amount / 0xffff) * 0x7f /*clamp uint16 range to the max possible value of a midi cc*/
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, /*cc*/26, amount); // same as gba_mus_ripper
									// TODO: research possible values for amount.
									
									sprintf(eventName, "Modulation Delay");
									sprintf(eventDesc, "%d", amount);
									break;
								}

								case 0xe1:
								{
									int bpm;

									bpm = getU2LitFrom(&sseq[curOffset]);
									curOffset += 2;

									smfInsertTempoBPM(smf, absTime, midiCh, bpm);

									sprintf(eventName, "Tempo");
									sprintf(eventDesc, "%d", bpm);
									break;
								}

								case 0xe3: /* Hippatte! Puzzle Bobble: SEQ_1pbgm03 */
								{
									// Gota's sequence.md lists this as 0xE2, which seems to be a typo.
									int amount;

									amount = getS2LitFrom(&sseq[curOffset]); // TODO: research possible values for amount. negative values likely set the sweep pitch below the default; or maybe negative values are invalid?
									// ex song contains "E3 C0 FF" and "E3 00 FA". Both of these are far outside the valid midi cc range of 0-127. I have implemented per-track text markers containing the original value so events like these can be converted losslessly.
									curOffset += 2;

									//smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_VIBRATODELAY, amount);
									smfInsertControl(smf, absTime+stackedEventTimeSpacer, midiCh, midiCh, 9, (((int32_t)amount + 0x7FFF) / (float)0xFFFE) * (int16_t)127 ); // If I ever make an NDS sound bank ripper that converts sound banks to sf2 files with modulators, this CC will be used as input for a modulator that controls vibrato. For now, it does nothing; only the below marker has any effect, and only when the midi is run through midi2sseq.
									char markerText[18]; // "?SweepPitch=-32767"
									snprintf(markerText, 18, "SweepPitch:%d", amount);
									smfInsertMetaEvent(smf, absTime+stackedEventTimeSpacer, midiCh, 6, markerText, 17);

									sprintf(eventName, "Sweep Pitch");
									sprintf(eventDesc, "%d", amount);
									break;
								}

								case 0xfc: /* Dawn of Sorrow: SDL_BGM_WIND_ */
								{
									if(loopStartCount > 0)
									{
											loopStartCount--;
											curOffset = loopStartOffset;
									}
									if(loopStartCount == -1)
									{
											switch(g_loopStyle)
											{
											case 0:
													loopCount--;
													curOffset = loopStartOffset;
													break;
											case 1:
													if(!loopEndPointUsed)
													{
															smfInsertControl(smf, absTime, midiCh, midiCh, 0x75, 0);
															loopCount = 0;
															loopEndPointUsed = true;
													}
													break;
											case 2:
													if(!loopEndPointUsed)
													{
															smfInsertMetaEvent(smf, absTime, midiCh, 6, "loopEnd", 7);
															loopCount = 0;
															loopEndPointUsed = true;
													}
													break;
											}
									}

									sprintf(eventName, "Loop End");
									sprintf(eventDesc, "");
									break;
								}

								case 0xfd:
								{
									offsetToJump = sseq2mid->track[trackIndex].offsetToReturn;
									sseq2mid->track[trackIndex].offsetToReturn = SSEQ_INVALID_OFFSET; /* to avoid eternal loop */

									if(offsetToJump == SSEQ_INVALID_OFFSET)
									{
										loopCount = 0;
										eventException = true;
										result = false;
									}

									sprintf(eventName, "Return");
									sprintf(eventDesc, "%08X", offsetToJump);
									break;
								}

								case 0xfe:
								{
									// "Allocate Tracks: Bitflag P1 for how to allocate tracks"
									int flag;
									unsigned int bit;

									flag = getU2LitFrom(&sseq[curOffset]);
									curOffset += 2;

									if(sseq2mid->modifyChOrder)
									{
										int sseqCh;
										int midiCh = 0;

										/* padding tracks, if necessary */
										bit = 1;
										for(sseqCh = 0; sseqCh < SSEQ_MAX_TRACK; sseqCh++)
										{
											if(flag & bit)
											{
												sseq2mid->chOrder[sseqCh] = midiChOrder[midiCh];
												midiCh++;
											}
											bit = bit << 1;
										}
										bit = 1;
										for(sseqCh = 0; sseqCh < SSEQ_MAX_TRACK; sseqCh++)
										{
											if(!(flag & bit))
											{
												sseq2mid->chOrder[sseqCh] = midiChOrder[midiCh];
												midiCh++;
											}
											bit = bit << 1;
										}
									}

									sprintf(eventName, "Signify Multi Track");
									strcpy(eventDesc, "");
									for(bit = 1; bit < 0x10000; bit = bit << 1)
									{
										strcat(eventDesc, (flag & bit) ? "*" : "-");
									}
									break;
								}

								case 0xff:
								{
									loopCount = 0;
									sprintf(eventName, "End of Track");
									sprintf(eventDesc, "");
									break;
								}

#if 0
								case 0xfa: /* WarioWare Touched! */ // TODO: research. This is not in Gota's SSEQ docs.
#endif

								default:
									loopCount = 0;
									eventException = true;
									eventExceptionOffset = curOffset;
									result = false;
									break;
								}
							}
							if (g_spacer) {
								uint8_t spacerIncBlacklist[] = {0x80, 0x93, 0x95, 0xd4, 0xe1, 0xfc, 0xfd, 0xfe}; // list of commands that should not increment stackedEventTimeSpacer.
								bool eventInBlacklist=false;
								int spacerIncBlacklistLength = sizeof(spacerIncBlacklist) / sizeof(spacerIncBlacklist[0]);
								for (int i=0; i<spacerIncBlacklistLength; i++) {
									if (statusByte == spacerIncBlacklist[i]) {
										eventInBlacklist=true;
										break;
									}
								}
								//printf("eventInBlacklist: %d\n", eventInBlacklist);
								if (!(eventException==true || eventInBlacklist==true)) stackedEventTimeSpacer++;
								//printf("stackedEventTimeSpacer: %d\n", stackedEventTimeSpacer);
								//printf("absTime: %d\n", absTime);
							}
							prevStatusByte = statusByte;
						}
						else
						{
							loopCount = 0;
						}

						if(eventException)
						{
							fprintf(stderr, "warning: exception [%s - %s]. Offset: 0x%lX.\n", eventName, eventDesc, eventExceptionOffset);
							strcat(eventDesc, " (!)");
						}

						sseq2midPutLogLine(sseq2mid, eventOffset, curOffset - eventOffset, eventName, eventDesc);
						if(offsetToJump != SSEQ_INVALID_OFFSET)
						{
							curOffset = offsetToJump;
						}
						sseq2mid->track[trackIndex].absTime = absTime;
						sseq2mid->track[trackIndex].curOffset = curOffset;
						sseq2mid->track[trackIndex].loopCount = loopCount;
					} while(loopCount > 0);

					if(sseq2mid->noReverb)
					{
						smfInsertControl(smf, 0, midiCh, midiCh, SMF_CONTROL_REVERB, 0);
					}
					smfSetEndTimingOfTrack(smf, midiCh, sseq2mid->track[midiCh].absTime); // on new super mario bros, BGM_AMB_CHIKA, with stackedEventTimeSpacer, the note gets cut off.
					sseq2midPutLog(sseq2mid, "\n");
				}
			}
		}
	}
	else
	{
		sseq2midPutLog(sseq2mid, "is not valid SSEQ\n");
	}
	return result;
}

/* output standard midi to memory from sseq2mid object */
size_t sseq2midWriteMidi(Sseq2mid* sseq2mid, byte* buffer, size_t bufferSize)
{
	return smfWrite(sseq2mid->smf, buffer, bufferSize);
}

/* output standard midi file from sseq2mid object */
size_t sseq2midWriteMidiFile(Sseq2mid* sseq2mid, const char* filename)
{
	return smfWriteFile(sseq2mid->smf, filename);
}

/* set log message procedure */
void sseq2midSetLogProc(Sseq2mid* sseq2mid, Sseq2midLogProc* logProc)
{
	if(sseq2mid && logProc)
	{
		sseq2mid->logProc = logProc;
	}
}

/* set reverb mode */
bool sseq2midNoReverb(Sseq2mid* sseq2mid, bool noReverb)
{
	bool oldNoReverb = false;

	if(sseq2mid)
	{
		oldNoReverb = sseq2mid->noReverb;
		sseq2mid->noReverb = noReverb;
	}
	return oldNoReverb;
}

/* set sequence loop count */
int sseq2midSetLoopCount(Sseq2mid* sseq2mid, int loopCount)
{
	int oldLoopCount = 1;

	if(sseq2mid)
	{
		oldLoopCount = sseq2mid->loopCount;
		if(loopCount >= 0)
		{
			sseq2mid->loopCount = (loopCount != 0) ? loopCount : 1;
		}
	}
	return oldLoopCount;
}


/* get signed byte */
int getS1From(byte* data)
{
	int val = data[0];
	return (val & 0x80) ? -(signed) (0xFF-val+1) : val;
}

/* get signed 2 bytes as little endian */
int getS2LitFrom(byte* data)
{
	int val = data[0] | (data[1] << 8);
	return (val & 0x8000) ? -(signed) (0xFFFF-val+1) : val;
}

/* get signed 3 bytes as little endian */
int getS3LitFrom(byte* data)
{
	int val = data[0] | (data[1] << 8) | (data[2] << 16);
	return (val & 0x800000) ? -(signed) (0xFFFFFF-val+1) : val;
}

/* get signed 4 bytes as little endian */
int getS4LitFrom(byte* data)
{
	int val = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	return (val & 0x80000000) ? -(signed) (0xFFFFFFFF-val+1) : val;
}

/* get unsigned byte */
unsigned int getU1From(byte* data)
{
	return (unsigned int) data[0];
}

/* get unsigned 2 bytes as little endian */
unsigned int getU2LitFrom(byte* data)
{
	return (unsigned int) (data[0] | (data[1] << 8));
}

/* get unsigned 3 bytes as little endian */
unsigned int getU3LitFrom(byte* data)
{
	return (unsigned int) (data[0] | (data[1] << 8) | (data[2] << 16));
}

/* get unsigned 4 bytes as little endian */
unsigned int getU4LitFrom(byte* data)
{
	return (unsigned int) (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}
