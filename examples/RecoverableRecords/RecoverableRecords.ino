// SPDX-License-Identifier: MIT
// RAM protocol demonstration only. No persistence or physical validation claimed.
#include <Arduino.h>
#include <SuperJupiter/EmbeddedReliability/RecordJournal.h>
struct Memory:superjupiter::record_journal::Storage{
 uint8_t bytes[64];Memory(){for(unsigned i=0;i<64;++i)bytes[i]=255;}
 bool read(size_t o,uint8_t*p,size_t n){if(o>64||n>64-o)return false;for(size_t i=0;i<n;++i)p[i]=bytes[o+i];return true;}
 bool write(size_t o,const uint8_t*p,size_t n){if(o>64||n>64-o)return false;for(size_t i=0;i<n;++i)bytes[o+i]=p[i];return true;}
 bool sync(){return true;} // RAM only. A real adapter must implement real durability.
};
Memory memory;
void setup(){
 Serial.begin(115200);
 using namespace superjupiter::record_journal;
 RecordJournal<4,2> journal(memory);uint8_t payload[4]={1,2,3,4};
 if(journal.recover()!=Status::Ok){Serial.println("Recovery failed");return;}
 if(journal.append(payload)!=Status::Ok){Serial.println("Append failed");return;}
 RecordJournal<4,2> reopened(memory);uint8_t loaded[4]={};
 if(reopened.recover()!=Status::Ok||reopened.get(0,loaded)!=Status::Ok){Serial.println("Read failed");return;}
 Serial.println(loaded[0]==1?"PASS RAM protocol":"FAIL payload");
}
void loop(){}
