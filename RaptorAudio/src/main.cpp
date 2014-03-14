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

	StreamingSoundObject* song = new StreamingSoundObject( "Space Faring.ogg", SoundMixer::GetMixer()->GetWaveOut() );
	SoundObjectProperties prop = SoundMixer::GetMixer()->CreateProperties( song );

	prop->SetLooping( true );

	SoundMixer::GetMixer()->PlaySoundObject( prop );

	while ( 1 )
	{
		Sleep( 250 );
	}
}