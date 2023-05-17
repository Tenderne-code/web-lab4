#include "switch.h"
#include <map>
#include <stdio.h>
#include <arpa/inet.h>
#include <math.h>

using namespace std;
#define PAYLOAD 1516; //最大幀

uint64_t change(mac_addr_t loc){
  uint64_t result = 0;
  for(int i=0; i<sizeof(mac_addr_t); i++){
    result = result << 8;
    result += loc[i];
  }
  return result;
}

class EthernetSwitch : public SwitchBase {
  
  int ports_num = 0;
  map<uint64_t, int> Map;
  map<uint64_t, int> time_map;  //aging
  map<uint64_t, int>::iterator it_src;
  map<uint64_t, int>::iterator it_dest;
  map<uint64_t, int>::iterator time_src;
  map<uint64_t, int>::iterator time_dest;

  public:
    void InitSwitch(int numPorts) override {
      this->ports_num = numPorts;
      return;
    }
    int ProcessFrame(int inPort, char* framePtr) override { 
        /* your implementation ... */ 
        //從幀中提取信息
        ether_header_t new_head;
        memcpy(&new_head, framePtr, sizeof(ether_header_t));

        uint8_t dest[6];
        memset(dest, 0, 6*sizeof(uint8_t));
        for(int i=0;i<6;i++){
          dest[i] = framePtr[i];
        }
        //strncpy(dest, framePtr, 6);
        //printf("dest: %s\n", dest);

        uint8_t src[6];
        memset(src, 0, 6*sizeof(uint8_t));
        for(int i=0;i<6;i++){
          src[i] = framePtr[i+6];
        }
        //strncpy(src, framePtr+6, 6);
        //printf("src: %s\n", src);

        //printf("12: %d, 13: %d\n", framePtr[12], framePtr[13]);
        
        // uint16_t type = 0;
        // for(int i=0; i<2; i++){
        //   type = type << 8;
        //   type += framePtr[i+12];
        // }
        // printf("type_tmp: %s\n", type_tmp);

        char length[2];
        memset(length, 0, sizeof(char)*2);
        strncpy(length, framePtr+14, 2);
        //printf("length: %s\n", length);

        //uint16_t type = atoi((char*)type_tmp);
        uint16_t len = atoi(length);
        //printf("轉化前type: %d\n", atoi((char*)type_tmp));
        //printf("type: %s\n", type_tmp);
        //printf("len: %d\n", len);
        uint64_t new_src = change(src);
        uint64_t new_dest = change(dest);
        //printf("new_src: %d, new_dest: %d\n", new_src, new_dest);

        if(new_head.ether_type == ETHER_DATA_TYPE){  //數據
          //printf("data pkg:\n");
          this->it_dest = this->Map.find(new_dest);
          this->it_src = this->Map.find(new_src);
          
          if(this->it_src == this->Map.end()){
            this->Map.insert(pair<uint64_t, int>(new_src, inPort));
            this->time_map.insert(pair<uint64_t, int>(new_src, 10));
            //printf("map_f: %d, map_s: %d\n", 
             //       this->it_src->first, this->it_src->second);
          }
          else{
            printf("update %d\n", new_src);
            this->time_src = this->time_map.find(new_src);
            this->time_src->second = 10;
          }
          if(this->it_dest != this->Map.end()){
            printf("update %d\n", new_dest);
            this->time_dest = this->time_map.find(new_dest);
            this->time_dest->second = 10;
            if(this->it_dest->second==inPort){
              return -1;
            }
            return this->it_dest->second;
          }
          else{
            return 0;
          }
        }
        else if(new_head.ether_type == ETHER_CONTROL_TYPE){  //控制
            //printf("contrl pkg:\n");
            map<uint64_t, int>::iterator it;
            it = this->time_map.begin();
            while(it != this->time_map.end()){
              printf("page: %d->%d\n", it->first, it->second);
              it->second--;
              if(!it->second){
                uint64_t tmp = it->first;
                it++;
                this->time_map.erase(tmp);
                this->Map.erase(tmp);
                continue;
              }
              it++;
            }
            return -1;
          }
        }
};


SwitchBase* CreateSwitchObject() {
  // TODO : Your code.
  return new EthernetSwitch();
}

