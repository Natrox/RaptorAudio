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

char buffer[ 10 * 1024 * 1024 ];

int main( void )
{
	SoundMixer::InitializeMixer( RATE, 4410, SoundMixerProfiles::SOUND_MIXER_SPEAKERS );

	FILE* file = fopen( "Space Faring.ogg", "rb" );

	int count = 0;

	while ( fread( &buffer[ count ], 1, 1, file ) ) count++;

	StreamingSoundObject* song = new StreamingSoundObject( buffer, AudioOrigins::AUDIO_ORIGIN_OPENMEMORY_POINT, count );
	SharedProperties sprop = CreateSharedProperties();
	SoundObjectProperties prop = SoundMixer::GetMixer()->CreateProperties( song, sprop );

	prop->SetLooping( true );

	SoundMixer::GetMixer()->PlaySoundObject( prop );

	while ( 1 )
	{
		Sleep( 250 );
	}
}