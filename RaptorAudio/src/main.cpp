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

char data[ 10 * 1024 * 1024 ];

int main( void )
{
	SoundMixer::InitializeMixer( RATE, 4410, SoundMixerBufferingModes::BUFFERING_BLOCKS, SoundMixerProfiles::SOUND_MIXER_SPEAKERS );

	FILE* file = fopen( "Space Faring.ogg", "rb" );

	int i = 0;
	while ( fread( data + i, 1, 1, file ) ) i++;

	fclose( file );

	StreamingSoundObject* song = new StreamingSoundObject( data, AudioOrigins::AUDIO_ORIGIN_OPENMEMORY_POINT, i );
	SoundObjectProperties prop = SoundMixer::GetMixer()->CreateProperties( song );

	prop->SetLooping( true );

	SoundMixer::GetMixer()->PlaySoundObject( prop );

	while ( 1 )
	{
		Sleep( 250 );
	}
}