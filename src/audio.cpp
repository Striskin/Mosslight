#include "audio.hpp"
namespace moss {
Audio::Audio() {
    InitAudioDevice(); ready=IsAudioDeviceReady(); if(!ready) return;
    SetMasterVolume(.45f);
    const float frequencies[]={0,250,510,125,784,523,659,880,220};
    for(int i=1;i<9;++i) {
        float duration=i==8?3.6f:(i>=5?.65f:.14f); unsigned frames=static_cast<unsigned>(duration*22050);
        std::vector<float> data(frames);
        for(unsigned j=0;j<frames;++j) {
            float t=j/22050.0f,progress=t/duration;
            float envelope=std::min(1.0f,t*60)*std::pow(1-progress,i==8?1.8f:2.5f);
            float f=frequencies[i]; if(i==1) f*=1-progress*.6f;
            float sample=std::sin(2*Pi*f*t)*.55f+std::sin(2*Pi*f*1.5f*t)*.18f;
            if(i==8) sample=std::sin(2*Pi*220*t)*.12f+std::sin(2*Pi*330*t)*.1f+std::sin(2*Pi*440*t)*.055f;
            data[j]=sample*envelope*.32f;
        }
        Wave wave{frames,22050,32,1,data.data()}; sounds[i]=LoadSoundFromWave(wave);
    }
}
Audio::~Audio() { if(ready) { for(int i=1;i<9;++i) UnloadSound(sounds[i]); CloseAudioDevice(); } }
void Audio::toggleMute() { muted=!muted; if(ready) SetMasterVolume(muted?0:.45f); }
void Audio::update(Cue cue,float dt) {
    if(!ready) return;
    int index=static_cast<int>(cue); if(index>0&&index<8) PlaySound(sounds[index]);
    ambience-=dt; if(ambience<=0) { PlaySound(sounds[8]); ambience=7.5f; }
}
}
