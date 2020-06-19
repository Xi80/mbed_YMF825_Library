#include "ymf825.h"


ymf825::ymf825(PinName mosi,PinName miso,PinName sck,PinName chipSelect,PinName reset) : _spi(mosi,miso,sck),_chipSelect(chipSelect),_reset(reset){
    _spi.frequency(10000000UL);
    init();
}

ymf825::ymf825(SPI spi,PinName chipSelect,PinName reset) : _spi(spi),_chipSelect(chipSelect),_reset(reset){
    _spi.frequency(10000000UL);
    init();
}

void ymf825::init(void){

    for(int i = 0;i < 16;i++){
        prevModulationVal[i] = 0;
        prevChannelVolumeVal[i] = 0;
        prevPitchBendVal[i] = 8192;
        prevVolumeVal[i] = 0;
    }

    _reset = 0;
    wait_us(100);
    _reset = 1;

   singleWrite( 0x1D, 0x01 );
   singleWrite( 0x02, 0x0E );
   wait_ms(1);
   singleWrite( 0x00, 0x01 );
   singleWrite( 0x01, 0x00 );
   singleWrite( 0x1A, 0xA3 );
   wait_ms(1);
   singleWrite( 0x1A, 0x00 );
   wait_ms(30);
   singleWrite( 0x02, 0x04 );
   wait_ms(1);
   singleWrite( 0x02, 0x00 );
   singleWrite( 0x19, 0xF0 );
   singleWrite( 0x1B, 0x3F );
   singleWrite( 0x14, 0x00 );
   singleWrite( 0x03, 0x01 );

   singleWrite( 0x08, 0xF6 );
   wait_ms(21);
   singleWrite( 0x08, 0x00 );
   singleWrite( 0x09, 0xF8 );
   singleWrite( 0x0A, 0x00 );
   
   singleWrite( 0x17, 0x40 );
   singleWrite( 0x18, 0x00 );

    for(int i = 0;i < 16;i++){
        singleWrite( 0x0B, i);
        singleWrite( 0x0F, 0x30 );
        singleWrite( 0x10, 0x71 );
        singleWrite( 0x11, 0x00 );
        singleWrite( 0x12, 0x08 );
        singleWrite( 0x13, 0x00 );
    }
}

void ymf825::singleWrite(uint8_t addr,uint8_t data){
    _chipSelect = 0;
    _spi.write(addr);
    _spi.write(data);
    _chipSelect = 1;
    wait_us(100);
}

void ymf825::burstWrite(uint8_t addr,uint8_t *data,int length){
    _chipSelect = 0;
    _spi.write(addr);
    for(int i = 0;i < length;i++){
        _spi.write(data[i]);
    }
    _chipSelect = 1;
    wait_us(100);
}

void ymf825::sendTone(void){
   singleWrite( 0x08, 0xF6 );
   wait_ms(1);
   singleWrite( 0x08, 0x00 );
    _chipSelect = 0;
    _spi.write(0x07);
    _spi.write(0x90);
    for(int i = 0;i < 16;i++){
        for(int j = 0;j <30;j++){
            _spi.write(gmTable[toneList[i]][j]);
        }
    }
    _chipSelect = 1;
    wait_us(100);
}

void ymf825::setToneListFromGM(uint8_t ch,uint8_t num){
    toneList[ch] = num;
}

void ymf825::noteOn(uint8_t channel,uint8_t note,uint8_t inst){
    singleWrite( 0x0B, channel );
    singleWrite( 0x0D, fNumberTableHigh[note] );
    singleWrite( 0x0E, fNumberTableLow[note] );
    singleWrite( 0x0F, 0x40 | (inst & 0x0F));
}

void ymf825::noteOff(uint8_t channel){
    singleWrite( 0x0B, channel );
    singleWrite( 0x0F, 0x00 );
}

void ymf825::pitchBend(uint8_t channel,unsigned short pb){
    unsigned char reg[2];
	unsigned char pit = pb >> 6;
    if(pit == prevPitchBendVal[channel])return;
    prevPitchBendVal[channel] = pit;
    reg[1] = (unsigned char)((tPitTbl[pit]<<1) & 0x007e);
	reg[0] = (unsigned char)(((tPitTbl[pit]<<2) & 0x1f00)>>8);
    singleWrite( 0x0B, channel);
	singleWrite( 0x12, reg[0] );
	singleWrite( 0x13, reg[1] );
}

void ymf825::setVolume(uint8_t channel,uint8_t vel){
    if(vel == prevVolumeVal[channel])return;
    prevVolumeVal[channel] = vel;
    singleWrite(0x0B,channel);
    singleWrite(0x0C,vel);
}

void ymf825::setChannelVolume(uint8_t channel,uint8_t vol){
    if(vol == prevChannelVolumeVal[channel])return;
    prevChannelVolumeVal[channel] = vol;
    singleWrite(0x0B,channel);
    singleWrite(0x10,vol);
}

void ymf825::setModulation(uint8_t channel,uint8_t m){
    m = m >> 4;
    if(m == prevModulationVal[channel])return;
    prevModulationVal[channel] = m;
    singleWrite(0x0B,channel);
    singleWrite(17,m);
}

void ymf825::allNotesOff(void){
    for(int i = 0;i < 16;i++){
        singleWrite(0x0B,i);
        singleWrite(15,0x30);
    }
}