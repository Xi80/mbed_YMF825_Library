# mbed YMF825 Library

Copyright(c) 2020 Xi80



## 概要

YMF825のmbed用のライブラリがないので自分用に作りました



## 使い方

下のサンプルコードを参考にしてください

## 対応機能

- ノートオン
- ノートオフ
- 音色データの送信
- ピッチベンド

## サンプルコード

```cpp
#include <mbed.h>
#include "midi/midi.h"
#include "ymf825/ymf825.h"

struct ymf825::channel channels[16];
uint8_t midiChannels[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t toneNumbers[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile unsigned short pitchBends[16]= {8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192};

midi MIDI(USBTX,USBRX);
ymf825 YMF825(D11,D12,D13,D10,D9);

void noteOff(int ch,int note);

void noteOn(int ch,int note,int vel){
  for(int i = 0;i < 16;i++){
    if(channels[i].isUsed == false){
      channels[i].isUsed = true;
      channels[i].noteNumber = note;
      midiChannels[i] = ch;
      YMF825.noteOn(i,note,vel,toneNumbers[i],pitchBends[ch]);
      return;
    }
  }
}

void noteOff(int ch,int note){
  for(int i = 0;i < 16;i++){
    if(channels[i].isUsed == true && channels[i].noteNumber == note && midiChannels[i] == ch){
      YMF825.noteOff(i);
      channels[i].isUsed = false;
      return;
    }
  }
}

void controlChange(int ch,int num,int val){
  if(num >= 51 && num <=66){
    
    YMF825.setToneListFromGM(num - 51,val);
  } else if(num >= 101 && num <= 116){
    toneNumbers[num-101] = val;
  } else if(num == 20){
    YMF825.sendTone();
  }
}

void programChange(int ch,int num){
  return;
}

void reset(void){
  return;
}

void pitchBend(int ch,unsigned short pb){
  pitchBends[ch] = pb;
  for(int i = 0;i < 16;i++){
    if(channels[i].isUsed == true && midiChannels[i] == ch){
      YMF825.pitchBend(i,pb);
    }
  }
}


int main() {
  MIDI.setCallbackNoteOn(noteOn);
  MIDI.setCallbackNoteOff(noteOff);
  MIDI.setCallbackControlChange(controlChange);
  MIDI.setCallbackProgramChange(programChange);
  MIDI.setCallbackPitchBend(pitchBend);
  MIDI.setCallbackReset(reset);
  YMF825.sendTone();
  while(1){
    MIDI.midiParse();
  }
}
```

拙作のMIDIライブラリと組み合わせて簡易音源にできます。