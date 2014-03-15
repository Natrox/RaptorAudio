RaptorAudio
===========

RaptorAudio is a simple software audio engine with support for WAV and OGG files. It currently only runs on Windows. 

Current Issues
--------------

WAV files with metadata are not supported at this point in time. Please use OGG if you want to retain metadata.

You cannot delete DSP variables, when they are initialized, they remain until the program is closed.

Acknowledgements
--------------
I would like to thank the following people;

Nils Desle, my teacher at NHTV IGAD, for teaching me about audio programming in his audio course and providing feedback.

<a href="http://www.nothings.org/">Sean Barrett</a>, for providing stb_vorbis, which is used for OGG decompression in RaptorAudio.

Features
--------

RaptorAudio has basic mono or stereo sound playback, in both 2D and 3D. Sounds can be played from memory, or they can be streamed from disk. The engine also features a way to easily change sound parameters on-the-fly, using garbage-collected sound property objects. The engine has support for common WAV bitrates; 8-bit unsigned, 16-bit signed, 24-bit signed, 32-bit float.

Included as well is a DSP system; this system allows practically any operation on the playing sound data. DSP effects such as a simple LowPass/HighPass, Stereo Enhancer and Echo Enhancer are included, but users may create their own effects.

Mixing in RaptorAudio is done using a form of brick-wall compression. There are two possible outputs for the mixer. One is a block-buffering system, which is the canonical way to send data to the audio device. Additionally, there is also support for a ringbuffer, which has a bit more latency, but is a lot more stable on slow devices.

Feature sheet:
- Support for WAV (8bu/16bs/24bs/32bf) and OGG.
- Fully multithreaded.
- Support for the streaming of sounds.
- 3D sound and stereo sound in 3D (using a mono to stereo interpolation).
- Full DSP chaining pipeline with user customizable variables.
- Recyclable and shareable properties for 3D and DSP.
- Mixer profiles for both speakers and headphones setups.
- Multiple types of output buffers (block buffer, ring buffer).

Streaming or not?
-------
OGG is only support with StreamingSoundObject. StreamingSoundObject does not necessarily mean that the whole file is streamed from disk, it merely means that not all of the sound data will be available at all times. Note that it is possible to use StreamingSoundObject with pre-loaded memory. As such, **MemorySoundObject does not support OGG!**

Recommendations
-----------

I recommend that you use WAV for sound effects and OGG for music. 

Usage
=======
Using the basic functionality of RaptorAudio is very simple. First, the libraries must be linked and the headers must be included. Then, the audio engine has to be initialized. 

```cpp
#include "RaptorAudio.h"

using namespace Raptor::Audio;

...
// Example
SoundMixer::InitializeMixer( sampleRate, bufferSize, SoundMixerBufferingModes::BUFFERING_BLOCKS, SoundMixerProfiles::SOUND_MIXER_SPEAKERS );
```

From here on, the user may create sounds;

```cpp
// For streaming audio (from file).
StreamingSoundObject* sound = new StreamingSoundObject( "Space Faring.ogg" );

// For loaded audio (from file). MemorySoundObject does not support OGG!
MemorySoundObject* sound = new StreamingSoundObject( "Space Faring.wav" );

// For audio from memory.
StreamingSoundObject* sound = new StreamingSoundObject( ptr, AudioOrigins::AUDIO_ORIGIN_OPENMEMORY, length );
MemorySoundObject* sound = new MemorySoundObject( ptr, AudioOrigins::AUDIO_ORIGIN_OPENMEMORY_POINT, length );
```

If the user now wishes to just play the sound (fire and forget);

```cpp
SoundMixer::GetMixer()->PlaySoundObject( sound );
```

However, often at times, you may want to control the sound as it is playing;

```cpp
SoundObjectProperties props = SoundMixer::GetMixer()->CreateProperties( sound );

// Set sound to loop
props->SetLooping( true );

SoundMixer::GetMixer()->PlaySoundObject( props );

....

// Somewhere else
SoundMixer::GetMixer()->Stop( props );
```

Note that it is perfectly safe to use the 'props' object anywhere. It is garbage-collected, so it exists for as long as you keep it, and the sound mixer will safely ignore it if it is not relevant anymore.

Sound loading
-------------

Sounds may be loaded from files or from memory;

Type | Explanation
--- | ---
AUDIO_ORIGIN_FILE | Load sound from file.
AUDIO_ORIGIN_OPENMEMORY | Copy memory, and load sound from copied memory.
AUDIO_ORIGIN_OPENMEMORY_POINT | Load sound directly from memory. Memory should not be freed before the sound is stopped.

It is possible to stream a WAV file from memory, however impractical that may be. It is recommended that you load from memory whenever you can (even better using AUDIO_ORIGIN_OPENMEMORY_POINT), to avoid using I/O too much. Note that, if you stream from memory and use AUDIO_ORIGIN_OPENMEMORY, the data will be copied for each instance of the sound.

Shared properties
-----------------

RaptorAudio provides a 'SharedProperties' object. This object can be shared between sounds (or used individually), and contains variables that govern the pitch, volume and DSP chain of sounds;

```cpp
SharedProperties sprops = CreateSharedProperties();
sprops->sp_Volume = 0.5;

// Link to a SoundObjectProperties
SoundObjectProperties props = SoundMixer::GetMixer()->CreateProperties( sound, sprops );
```

These shared properties are also used for DSP effects.

DSP Chain
---------

RaptorAudio has a DSP system that works by chaining a variety of effects. These effects can be used with different output semantics;

Ouptput semantic | Effect
--- | ---
SEMANTIC_CARRY_SIGNAL | Carries the output samples over to the next effect in the chain. Does not change the DSP chain global samples.
SEMANTIC_SUBSTITUTE_SIGNAL | Replaces the DSP chain global samples to the output samples.
SEMANTIC_ADDITIVE_SIGNAL | Adds the output samples to the DSP chain global samples.
SEMANTIC_SUBTRACTIVE_SIGNAL | Subtracts the output samples from the DSP chain global samples.
SEMANTIC_NO_SIGNAL | Does not output any samples.
SEMNATIC_PERFORM_PER_PLAY | Only runs the effect once per play (should be used for “PitchShift”)

Here is an example on how to use the DSP system to create a HighPass;

```cpp
DSPChain* chain = new DSPChain();

// Each effect has a variable set containing a number of variables.
DSPVariableSet set1;

// Create a variable object by name
DSPVariable* var1 = DSPVariables::CreateVariableObject( "LowPassLevel", DSPVariableSemantics::SEMANTIC_USER_VARIABLE, 90 );
// ..or
DSPVariable* var1 = DSPVariables::CreateVariableObject( "LowPassLevel", DSPVariableSemantics::SEMANTIC_RANDOM_RANGED_PLAY, 40, 90 );

set1.dvs_Val1 = var;

chain->AddToChain( "HighPass", DSPFunctionSemantics::SEMANTIC_SUBTRACTIVE_SIGNAL, BetterLowPass, set1 );

...

SharedProperties sprops = CreateSharedProperties();
sprops->sp_DSPChain = chain;

...

// To disable the effect
const DSPChainEntry* entry = chain->GetEffectEntry( "HighPass" );
if ( entry != 0 ) entry->dce_Enabled = false;
```

It is up to the creator of the DSP effect to declare what meaning a value (dvs_Val1/dvs_Val2/dvs_Val3) has.

Here is another example of a DSP chain;

```cpp
chain->AddToChain( "HighPass", DSPFunctionSemantics::SEMANTIC_CARRY_SIGNAL, BetterLowPass, set1 );
chain->AddToChain( "Amplify", DSPFunctionSemantics::SEMANTIC_CARRY_SIGNAL, Amplify, set2 );
chain->AddToChain( "Stereo Expand", DSPFunctionSemantics::SEMANTIC_SUBSTITUTE_SIGNAL, StereoExpand, set3 );
```

Preloaded DSP effects can be found in 'DSPFunctions.h'.

3D Sound
-----------

Every sound can be played back in 3D;

```cpp
Sound3DDescription* desc3d = CreateSound3DDescription();

desc3d->s3d_AttenuationMax = 300; // Set max attenuation (sound has 0.0 volume at x units)
desc3d->s3d_AttenuationMin = 15; // Set max attenuation (sound has 1.0 volume at x units)
desc3d->s3d_StereoInnerDistance = 5; // Sound is fully stereo from this point
desc3d->s3d_StereoOuterDistance = 20; // Sound is fully mono from this point
desc3d->s3d_Position = glm::vec3( 1, 5, 16 ); // Set a position in 3D space. May be changed whenever!
```

The stereo distances are used to interpolate between a mono and stereo version of a sound, which is useful for soundscapes. The position can be changed at any point in time.

Of course, to perform 3D calculations, RaptorAudio will need to have camera data;

```cpp
// Set this each frame (usually)
SoundMixer::GetMixer()->SetListenerAttributes( cameraPos, cameraForward, cameraUp );
```
Sound profiles
----------

The sound mixer may be set to a headphones or speaker profile at any time. The difference between these profiles is that the headphones profile disables hard panning by duplicating audio from one channel into another at a lower volume.

```cpp
SoundMixer::GetMixer()->SetProfile( SoundMixerProfiles::SOUND_MIXER_HEADPHONES );
SoundMixer::GetMixer()->SetProfile( SoundMixerProfiles::SOUND_MIXER_SPEAKERS );
```

This volume of duplication is currently not tweakable.

History buffers (echoes)
------------

RaptorAudio has support for generic echoes. This is done using the HistoryBufferObject, a special kind of SoundObject. The sound objects are fully compatible with the shared properties and DSP systems. Usage is as follows;

```cpp
SoundObjectProperties histObjProp = CreateSoundObjectProperties();
HistoryBufferObject* histObj = new HistoryBufferObject( ECHO_PARAMETERS_TO_SAMPLES( mixerSampleRate, amountOfEchoes, delayInBetween ) );

// This factor is used to decrease the levels (level^volumeFactor).
double volumeFactor = 2.0;

histObj->InitializeEchoes( histObjProp, amountOfEchoes, delayInBetween, volumeFactor );
SoundMixer::GetMixer()->AddGroup( histObj );

...

// For a sound
SoundObjectProperties props = SoundMixer::GetMixer()->CreateProperties( sound );
props->SetHistoryBufferObject( histObj );
```

All sounds connected to this history buffer will produce an echo.

Safety of SoundObjectProperties and SharedProperties
-----

These two classes are garbage-collected, because in the past, they could be used after expiration (because the sound stopped playing or else). It was really hard to actually figure out if the objects were safe to use, so I decided to use thread-safe shared pointers to manage the lifetime. Now, using these classes is safe, even if the sound has stopped playing and doesn't exist anymore in the mixer.

Future
------

These are things that should be done at some point;
- Improve the DSP Chain and HistoryBufferObject.
- Improve documentation.
- Decrease verbosity of the advanced functionality.
- Simplify sound loading.
