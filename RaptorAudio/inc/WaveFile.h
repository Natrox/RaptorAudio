/*
	Copyright (c) 2014 Sam Hardeman

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#pragma once

#define SIZE_OF_WAV_HEADER ( sizeof( char ) * 16 + sizeof( __int32 ) * 5 + sizeof( short ) * 4 )

namespace Raptor
{
	namespace Audio
	{
		typedef struct 
		{
			char rh_ChunkID[4];
			unsigned __int32 rh_ChunkSize;
			char rh_Format[4];
		} RiffHeader;

		typedef struct 
		{
			char fh_Subchunk1ID[4];
			unsigned __int32 fh_Subchunk1Size;
			unsigned short fh_AudioFormat;
			unsigned short fh_NumChannels;
			unsigned __int32 fh_SampleRate;
			unsigned __int32 fh_ByteRate;
			unsigned short fh_BlockAlign;
			unsigned short fh_BitsPerSample;
		} FmtHeader;

		typedef struct 
		{
			char dh_Subchunk2ID[4];
			unsigned __int32 dh_Subchunk2Size;
		} DataHeader;

		typedef struct
		{
			RiffHeader wh_RiffHeader;
			FmtHeader wh_FmtHeader;
			DataHeader wh_DataHeader;

			union
			{
				char dh_Data[1]; // QQQ
				short dh_SData[1];
				float dh_FData[1];
			};
		} WaveHeaders;

		typedef union
		{
			WaveHeaders* wf_WaveHeaders;
			char* wf_Data;
		} WaveFile;
	};
};