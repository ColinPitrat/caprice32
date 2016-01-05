/* Caprice32 - Amstrad CPC Emulator
   (c) Copyright 1997-2004 Ulrich Doewich

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* General Instruments AY-3-8912 Programable Sound Generator

   converted from the AY Emulator project by Sergey Vladimirovich Bulba
   based on Version 2.7 beta 3

   Jul 07, 2001 - 16:07 basic functionality of sound_fill_buffer, converted from asm code
   Jul 20, 2001 - 18:49 sound emulation works properly for the first time; tone and noise implemented
   Jul 28, 2001 - 15:23 switched to the AY emulation from MAME
   Jul 28, 2001 - 16:09 fixed the envelope problem: the amp_table was still being referenced as byte!
   Aug 01, 2001 - 17:28 added code to support mono output
   Aug 12, 2001 - 21:42 moved psg_reg_write() from cap32.cpp
   Dec 08, 2002 - 10:43 properly integrated the tape noise into the mixing buffer
   Apr 29, 2003 - 21:48 started to convert AyEmul source code from Pascal to C
   Apr 30, 2003 - 23:39 continued with conversion
   May 04, 2003 - 00:13 modified Synthesizer_Stereo16 for Caprice32 - new AY code works for the first time!
   May 04, 2003 - 16:09 fixed the problem with the envelope period: the struct in TRegisterAY wasn't accessed properly
   May 05, 2003 - 18:46 added alternate synthesizers for mono/stereo + 8/16 bits; some code cleanup
   May 06, 2003 - 15:18 enabled 'tape noise' again
   May 13, 2003 - 15:57 moved counter init to InitAYCounterVars; corrected the 'divide by zero' possibility in the
                        synthesizer routines: LoopCountInit calculation needs to take CPC speed into account
   May 15, 2003 - 00:08 replaced the rand() call with Sergey's noise algorithm
   May 23, 2003 - 14:58 added code to supress sound if the tone frequency is set to a value below 5, and no
                        envelope assigned to the channel
   Jun 04, 2003 - 19:37 added support for Digiblaster/Soundplayer on the printer port
*/

#include <math.h>

#include "cap32.h"
#include "z80.h"

extern t_CPC CPC;
extern t_PSG PSG;
extern dword freq_table[];

extern byte *pbSndBuffer;
extern byte *pbSndBufferEnd;
extern byte bTapeLevel;

#define TAPE_VOLUME 32



// Amplitude table (c)Hacker KAY
word Amplitudes_AY[16] = {
   0, 836, 1212, 1773, 2619, 3875, 5397, 8823,
   10392, 16706, 23339, 29292, 36969, 46421, 55195, 65535
};

int Level_PP[256];

union {
   struct {
      dword Lo;
      dword Hi;
   };
   int64_t Re;
} LoopCount;
int64_t LoopCountInit;

bool Ton_EnA, Ton_EnB, Ton_EnC, Noise_EnA, Noise_EnB, Noise_EnC;
bool Envelope_EnA, Envelope_EnB, Envelope_EnC;
void (*Case_EnvType)(void);

union TCounter {
   struct {
      word Lo;
      word Hi;
   };
   dword Re;
};
TCounter Ton_Counter_A, Ton_Counter_B, Ton_Counter_C, Noise_Counter;
union {
   struct {
      word Low;
      word Val;
   };
   dword Seed;
} Noise;
union {
   struct {
      dword Lo;
      dword Hi;
   };
   int64_t Re;
} Envelope_Counter;
byte Ton_A, Ton_B, Ton_C;

int Level_AR[32], Level_AL[32], Level_BR[32], Level_BL[32], Level_CR[32], Level_CL[32];
int LevelTape;
byte Index_AL, Index_AR, Index_BL, Index_BR, Index_CL, Index_CR;
int PreAmp, PreAmpMax;
int Left_Chan, Right_Chan;



inline void SetMixerRegister(byte Value)
{
   PSG.RegisterAY.Mixer = Value;
   Ton_EnA = Value & 1 ? false : true;
   Noise_EnA = Value & 8 ? false : true;
   Ton_EnB = Value & 2 ? false : true;
   Noise_EnB = Value & 16 ? false : true;
   Ton_EnC = Value & 4 ? false : true;
   Noise_EnC = Value & 32 ? false : true;
}



inline void SetAmplA(byte Value)
{
   PSG.RegisterAY.AmplitudeA = Value;
   Envelope_EnA = Value & 16 ? false : true;
}



inline void SetAmplB(byte Value)
{
   PSG.RegisterAY.AmplitudeB = Value;
   Envelope_EnB = Value & 16 ? false : true;
}



inline void SetAmplC(byte Value)
{
   PSG.RegisterAY.AmplitudeC = Value;
   Envelope_EnC = Value & 16 ? false : true;
}



void Case_EnvType_0_3__9(void)
{
   if (PSG.FirstPeriod) {
      PSG.AmplitudeEnv--;
      if (!PSG.AmplitudeEnv) {
         PSG.FirstPeriod = false;
      }
   }
}



void Case_EnvType_4_7__15(void)
{
   if (PSG.FirstPeriod) {
      PSG.AmplitudeEnv++;
      if (PSG.AmplitudeEnv == 32) {
         PSG.FirstPeriod = false;
         PSG.AmplitudeEnv = 0;
      }
   }
}



void Case_EnvType_8(void)
{
   PSG.AmplitudeEnv = (PSG.AmplitudeEnv - 1) & 31;
}



void Case_EnvType_10(void)
{
   if (PSG.FirstPeriod) {
      PSG.AmplitudeEnv--;
      if (PSG.AmplitudeEnv == -1) {
         PSG.FirstPeriod = false;
         PSG.AmplitudeEnv = 0;
      }
   }
   else {
      PSG.AmplitudeEnv++;
      if (PSG.AmplitudeEnv == 32) {
         PSG.FirstPeriod = true;
         PSG.AmplitudeEnv = 31;
      }
   }
}



void Case_EnvType_11(void)
{
   if (PSG.FirstPeriod) {
      PSG.AmplitudeEnv--;
      if (PSG.AmplitudeEnv == -1) {
         PSG.FirstPeriod = false;
         PSG.AmplitudeEnv = 31;
      }
   }
}



void Case_EnvType_12(void)
{
   PSG.AmplitudeEnv = (PSG.AmplitudeEnv + 1) & 31;
}



void Case_EnvType_13(void)
{
   if (PSG.FirstPeriod) {
      PSG.AmplitudeEnv++;
      if (PSG.AmplitudeEnv == 32) {
         PSG.FirstPeriod = false;
         PSG.AmplitudeEnv = 31;
      }
   }
}



void Case_EnvType_14(void)
{
   if (!PSG.FirstPeriod) {
      PSG.AmplitudeEnv--;
      if (PSG.AmplitudeEnv == -1) {
         PSG.FirstPeriod = true;
         PSG.AmplitudeEnv = 0;
      }
   }
   else {
      PSG.AmplitudeEnv++;
      if (PSG.AmplitudeEnv == 32) {
         PSG.FirstPeriod = false;
         PSG.AmplitudeEnv = 31;
      }
   }
}



inline void SetEnvelopeRegister(byte Value)
{
   Envelope_Counter.Hi = 0;
   PSG.FirstPeriod = true;
   if (!(Value & 4)) {
      PSG.AmplitudeEnv = 32;
   }
   else {
      PSG.AmplitudeEnv = -1;
   }
   PSG.RegisterAY.EnvType = Value;
   switch (Value)
   {
      case 0:
      case 1:
      case 2:
      case 3:
      case 9:
         Case_EnvType = Case_EnvType_0_3__9;
         break;
      case 4:
      case 5:
      case 6:
      case 7:
      case 15:
         Case_EnvType = Case_EnvType_4_7__15;
         break;
      case 8:
         Case_EnvType = Case_EnvType_8;
         break;
      case 10:
         Case_EnvType = Case_EnvType_10;
         break;
      case 11:
         Case_EnvType = Case_EnvType_11;
         break;
      case 12:
         Case_EnvType = Case_EnvType_12;
         break;
      case 13:
         Case_EnvType = Case_EnvType_13;
         break;
      case 14:
         Case_EnvType = Case_EnvType_14;
         break;
   }
}



void SetAYRegister(int Num, byte Value)
{
   switch(Num)
   {
      case 13:
         SetEnvelopeRegister(Value & 15);
         break;
      case 1:
      case 3:
      case 5:
         PSG.RegisterAY.Index[Num] = Value & 15;
         break;
      case 6:
         PSG.RegisterAY.Noise = Value & 31;
         break;
      case 7:
         SetMixerRegister(Value & 63);
         break;
      case 8:
         SetAmplA(Value & 31);
         break;
      case 9:
         SetAmplB(Value & 31);
         break;
      case 10:
         SetAmplC(Value & 31);
         break;
      case 0:
      case 2:
      case 4:
      case 11:
      case 12:
         PSG.RegisterAY.Index[Num] = Value;
         break;
   }
}



inline void Synthesizer_Logic_Q(void)
{
   Ton_Counter_A.Hi++;
   if (Ton_Counter_A.Hi >= *(word *)&PSG.RegisterAY.TonALo) {
      Ton_Counter_A.Hi = 0;
      Ton_A ^= 1;
   }
   Ton_Counter_B.Hi++;
   if (Ton_Counter_B.Hi >= *(word *)&PSG.RegisterAY.TonBLo) {
      Ton_Counter_B.Hi = 0;
      Ton_B ^= 1;
   }
   Ton_Counter_C.Hi++;
   if (Ton_Counter_C.Hi >= *(word *)&PSG.RegisterAY.TonCLo) {
      Ton_Counter_C.Hi = 0;
      Ton_C ^= 1;
   }
   Noise_Counter.Hi++;
   if ((!(Noise_Counter.Hi & 1)) && (Noise_Counter.Hi >= (PSG.RegisterAY.Noise << 1))) {
      Noise_Counter.Hi = 0;
      Noise.Seed = (((((Noise.Seed >> 13) ^ (Noise.Seed >> 16)) & 1) ^ 1) | Noise.Seed << 1) & 0x1ffff;
   }
   if (!Envelope_Counter.Hi) {
      Case_EnvType();
   }
   Envelope_Counter.Hi++;
   if (Envelope_Counter.Hi >= *(word *)&PSG.RegisterAY.EnvelopeLo) {
      Envelope_Counter.Hi = 0;
   }
}



inline void Synthesizer_Mixer_Q(void)
{
   int LevL, LevR, k;

   LevL = bTapeLevel ? LevelTape : 0; // start with the tape signal
   if (CPC.snd_pp_device) {
      LevL += Level_PP[CPC.printer_port];
   }

   LevR = LevL;
   if (Ton_EnA) {
      if ((!Envelope_EnA) || (*(word *)&PSG.RegisterAY.TonALo > 4)) {
         k = Ton_A;
      }
      else {
         k = 1;
      }
   }
   else {
      k = 1;
   }
   if (Noise_EnA) {
      k &= Noise.Val;
   }
   if (k) {
      if (Envelope_EnA) {
         LevL += Level_AL[PSG.RegisterAY.AmplitudeA * 2 + 1];
         LevR += Level_AR[PSG.RegisterAY.AmplitudeA * 2 + 1];
      }
      else {
         LevL += Level_AL[PSG.AmplitudeEnv];
         LevR += Level_AR[PSG.AmplitudeEnv];
      }
   }

   if (Ton_EnB) {
      if ((!Envelope_EnB) || (*(word *)&PSG.RegisterAY.TonBLo > 4)) {
         k = Ton_B;
      }
      else {
         k = 1;
      }
   }
   else {
      k = 1;
   }
   if (Noise_EnB) {
      k &= Noise.Val;
   }
   if (k) {
      if (Envelope_EnB) {
         LevL += Level_BL[PSG.RegisterAY.AmplitudeB * 2 + 1];
         LevR += Level_BR[PSG.RegisterAY.AmplitudeB * 2 + 1];
      }
      else {
         LevL += Level_BL[PSG.AmplitudeEnv];
         LevR += Level_BR[PSG.AmplitudeEnv];
      }
   }

   if (Ton_EnC) {
      if ((!Envelope_EnC) || (*(word *)&PSG.RegisterAY.TonCLo > 4)) {
         k = Ton_C;
      }
      else {
         k = 1;
      }
   }
   else {
      k = 1;
   }
   if (Noise_EnC) {
      k &= Noise.Val;
   }
   if (k) {
      if (Envelope_EnC) {
         LevL += Level_CL[PSG.RegisterAY.AmplitudeC * 2 + 1];
         LevR += Level_CR[PSG.RegisterAY.AmplitudeC * 2 + 1];
      }
      else {
         LevL += Level_CL[PSG.AmplitudeEnv];
         LevR += Level_CR[PSG.AmplitudeEnv];
      }
   }

   Left_Chan += LevL;
   Right_Chan += LevR;
}



void Synthesizer_Stereo16(void)
{
   int Tick_Counter = 0;
   while (LoopCount.Hi) {
      Synthesizer_Logic_Q();
      Synthesizer_Mixer_Q();
      Tick_Counter++;
      LoopCount.Hi--;
   }
   LoopCount.Re += LoopCountInit;
   reg_pair val;
   val.w.l = Left_Chan / Tick_Counter;
   val.w.h = Right_Chan / Tick_Counter;
   *(dword *)CPC.snd_bufferptr = val.d; // write to mixing buffer
   CPC.snd_bufferptr += 4;
   Left_Chan = 0;
   Right_Chan = Left_Chan;
   if (CPC.snd_bufferptr >= pbSndBufferEnd) {
      CPC.snd_bufferptr = pbSndBuffer;
      PSG.buffer_full = 1;
   }
}



void Synthesizer_Stereo8(void)
{
   int Tick_Counter = 0;
   while (LoopCount.Hi) {
      Synthesizer_Logic_Q();
      Synthesizer_Mixer_Q();
      Tick_Counter++;
      LoopCount.Hi--;
   }
   LoopCount.Re += LoopCountInit;
   reg_pair val;
   val.b.l = 128 + Left_Chan / Tick_Counter;
   val.b.h = 128 + Right_Chan / Tick_Counter;
   *(word *)CPC.snd_bufferptr = val.w.l; // write to mixing buffer
   CPC.snd_bufferptr += 2;
   Left_Chan = 0;
   Right_Chan = Left_Chan;
   if (CPC.snd_bufferptr >= pbSndBufferEnd) {
      CPC.snd_bufferptr = pbSndBuffer;
      PSG.buffer_full = 1;
   }
}



inline void Synthesizer_Mixer_Q_Mono(void)
{
   int Lev, k;

   Lev = bTapeLevel ? LevelTape : 0; // start with the tape signal
   if (CPC.snd_pp_device) {
      Lev += Level_PP[CPC.printer_port];
   }

   if (Ton_EnA) {
      if ((!Envelope_EnA) || (*(word *)&PSG.RegisterAY.TonALo > 4)) {
         k = Ton_A;
      }
      else {
         k = 1;
      }
   }
   else {
      k = 1;
   }
   if (Noise_EnA) {
      k &= Noise.Val;
   }
   if (k) {
      if (Envelope_EnA) {
         Lev += Level_AL[PSG.RegisterAY.AmplitudeA * 2 + 1];
      }
      else {
         Lev += Level_AL[PSG.AmplitudeEnv];
      }
   }

   if (Ton_EnB) {
      if ((!Envelope_EnB) || (*(word *)&PSG.RegisterAY.TonBLo > 4)) {
         k = Ton_B;
      }
      else {
         k = 1;
      }
   }
   else {
      k = 1;
   }
   if (Noise_EnB) {
      k &= Noise.Val;
   }
   if (k) {
      if (Envelope_EnB) {
         Lev += Level_BL[PSG.RegisterAY.AmplitudeB * 2 + 1];
      }
      else {
         Lev += Level_BL[PSG.AmplitudeEnv];
      }
   }

   if (Ton_EnC) {
      if ((!Envelope_EnC) || (*(word *)&PSG.RegisterAY.TonCLo > 4)) {
         k = Ton_C;
      }
      else {
         k = 1;
      }
   }
   else {
      k = 1;
   }
   if (Noise_EnC) {
      k &= Noise.Val;
   }
   if (k) {
      if (Envelope_EnC) {
         Lev += Level_CL[PSG.RegisterAY.AmplitudeC * 2 + 1];
      }
      else {
         Lev += Level_CL[PSG.AmplitudeEnv];
      }
   }

   Left_Chan += Lev;
}



void Synthesizer_Mono16(void)
{
   int Tick_Counter = 0;
   while (LoopCount.Hi) {
      Synthesizer_Logic_Q();
      Synthesizer_Mixer_Q_Mono();
      Tick_Counter++;
      LoopCount.Hi--;
   }
   LoopCount.Re += LoopCountInit;
   *(word *)CPC.snd_bufferptr = Left_Chan / Tick_Counter; // write to mixing buffer
   CPC.snd_bufferptr += 2;
   Left_Chan = 0;
   if (CPC.snd_bufferptr >= pbSndBufferEnd) {
      CPC.snd_bufferptr = pbSndBuffer;
      PSG.buffer_full = 1;
   }
}



void Synthesizer_Mono8(void)
{
   int Tick_Counter = 0;
   while (LoopCount.Hi) {
      Synthesizer_Logic_Q();
      Synthesizer_Mixer_Q_Mono();
      Tick_Counter++;
      LoopCount.Hi--;
   }
   LoopCount.Re += LoopCountInit;
   *(byte *)CPC.snd_bufferptr = 128 + Left_Chan / Tick_Counter; // write to mixing buffer
   CPC.snd_bufferptr++;
   Left_Chan = 0;
   if (CPC.snd_bufferptr >= pbSndBufferEnd) {
      CPC.snd_bufferptr = pbSndBuffer;
      PSG.buffer_full = 1;
   }
}



void Calculate_Level_Tables(void)
{
   int i, b, l, r;
   int Index_A, Index_B, Index_C;
   double k;

   Index_A = Index_AL;
   Index_B = Index_BL;
   Index_C = Index_CL;
   l = Index_A + Index_B + Index_C;
   r = Index_AR + Index_BR + Index_CR;
   if (CPC.snd_stereo) {
      if (l < r) {
         l = r;
      }
   }
   else {
      l += r;
      Index_A += Index_AR;
      Index_B += Index_BR;
      Index_C += Index_CR;
   }
   if (l == 0) {
      l++;
   }
   if (!CPC.snd_bits) { // 8 bits per sample?
      r = 127;
   }
   else {
      r = 32767;
   }
   l = 255 * r / l;
   for (i = 0; i < 16; i++) {
      b = (int)rint(Index_A / 255.0 * Amplitudes_AY[i]);
      b = (int)rint(b / 65535.0 * l);
      Level_AL[i * 2] = b;
      Level_AL[i * 2 + 1] = b;
      b = (int)rint(Index_AR / 255.0 * Amplitudes_AY[i]);
      b = (int)rint(b / 65535.0 * l);
      Level_AR[i * 2] = b;
      Level_AR[i * 2 + 1] = b;
      b = (int)rint(Index_B / 255.0 * Amplitudes_AY[i]);
      b = (int)rint(b / 65535.0 * l);
      Level_BL[i * 2] = b;
      Level_BL[i * 2 + 1] = b;
      b = (int)rint(Index_BR / 255.0 * Amplitudes_AY[i]);
      b = (int)rint(b / 65535.0 * l);
      Level_BR[i * 2] = b;
      Level_BR[i * 2 + 1] = b;
      b = (int)rint(Index_C / 255.0 * Amplitudes_AY[i]);
      b = (int)rint(b / 65535.0 * l);
      Level_CL[i * 2] = b;
      Level_CL[i * 2 + 1] = b;
      b = (int)rint(Index_CR / 255.0 * Amplitudes_AY[i]);
      b = (int)rint(b / 65535.0 * l);
      Level_CR[i * 2] = b;
      Level_CR[i * 2 + 1] = b;
   }
   k = exp(CPC.snd_volume * log(2) / PreAmpMax) - 1;
   for (i = 0; i < 32; i++) {
      Level_AL[i] = (int)rint(Level_AL[i] * k);
      Level_AR[i] = (int)rint(Level_AR[i] * k);
      Level_BL[i] = (int)rint(Level_BL[i] * k);
      Level_BR[i] = (int)rint(Level_BR[i] * k);
      Level_CL[i] = (int)rint(Level_CL[i] * k);
      Level_CR[i] = (int)rint(Level_CR[i] * k);
   }
   if (!CPC.snd_bits) { // 8 bits per sample?
      LevelTape = -(int)rint((TAPE_VOLUME / 2) * k);
   }
   else {
      LevelTape = -(int)rint((TAPE_VOLUME * 128) * k);
   }
   for (i = 0, b = 255; i < 256; i++) { // calculate the 256 levels of the Digiblaster/Soundplayer
      Level_PP[i] = -(int)rint(((b << 8) / 65535.0 * l) * k);
      b--;
   }
}



void ResetAYChipEmulation(void)
{
   Ton_Counter_A.Re = 0;
   Ton_Counter_B.Re = 0;
   Ton_Counter_C.Re = 0;
   Noise_Counter.Re = 0;
   Envelope_Counter.Re = 0;
   Ton_A = 0;
   Ton_B = 0;
   Ton_C = 0;
   Left_Chan = 0;
   Right_Chan = 0;
   Noise.Seed = 0xffff;
}



void InitAYCounterVars(void)
{
   CPC.snd_cycle_count_init.both = (int64_t)rint((4000000 * ((CPC.speed * 25) / 100.0)) /
      freq_table[CPC.snd_playback_rate] * 4294967296.0); // number of Z80 cycles per sample
   LoopCountInit = (int64_t)rint(1000000.0 / (4000000.0 * ((CPC.speed * 25) / 100.0)) / 8.0 *
      CPC.snd_cycle_count_init.both); // number of AY counter increments per sample
   LoopCount.Re = LoopCountInit;
}



void InitAY(void)
{
   Index_AL = 255;
   Index_AR = 13;
   Index_BL = 170;
   Index_BR = 170;
   Index_CL = 13;
   Index_CR = 255;
   PreAmpMax = 100;
   Calculate_Level_Tables();
   InitAYCounterVars();
   ResetAYChipEmulation();

   if (CPC.snd_stereo) { // stereo mode?
      if (CPC.snd_bits) { // 16 bits per sample?
         PSG.Synthesizer = Synthesizer_Stereo16;
      }
      else { // 8 bits
         PSG.Synthesizer = Synthesizer_Stereo8;
      }
   }
   else { // mono
      if (CPC.snd_bits) { // 16 bits per sample?
         PSG.Synthesizer = Synthesizer_Mono16;
      }
      else { // 8 bits
         PSG.Synthesizer = Synthesizer_Mono8;
      }
   }
}
