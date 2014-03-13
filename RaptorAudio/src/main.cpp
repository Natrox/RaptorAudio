////////////////////////////////////
// RaptorAudio Testing file       //
////////////////////////////////////

#include "RaptorAudio.h"

#include <iomanip> 
#include <string>
#include <sstream>

#include "SharedPointer.h"

using namespace Raptor::Audio;
using namespace Raptor::Utility;

#define RATE 44100

int main( void )
{
	SoundMixer::InitializeMixer( RATE, 4410, SoundMixerBufferingModes::BUFFERING_BLOCKS, SoundMixerProfiles::SOUND_MIXER_SPEAKERS );

	while ( 1 )
	{
		Sleep( 250 );
	}
}