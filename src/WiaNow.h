#ifndef WIA_MEGA_NET_NOW
  #define WIA_MEGA_NET_NOW

  #ifdef LINKER_CONNECT
    // 
  #endif
  #ifdef NOW_TYPE
    // 
  #endif
  
  

/*
---------------------------------------------------------------------------------------
---------------------------------- HELPING MATERIAL ----------------------------------
---------------------------------------------------------------------------------------
*/

  #include "WiaNetworkBase.h"
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  #if defined(ESP32)
    #include <esp_now.h>
    #include <WiFi.h>
    #include <esp_wifi.h>
    void sniffer_callback(void *buf, wifi_promiscuous_pkt_type_t type);
    void OnDataRecv(const esp_now_recv_info_t* info, const uint8_t* incomingData, int len);
    void OnDataSent(const uint8_t *mac, esp_now_send_status_t status);
  #elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <espnow.h>
    struct sniffer_buf {
      struct rx_ctrl {
        signed rssi:8;
      } rx_ctrl;
      uint8_t data[36];
    };
    void ICACHE_FLASH_ATTR sniffer_callback(uint8_t *buffer, uint16_t length);
    void OnDataRecv(uint8_t * mac_address, uint8_t *incomingData, uint8_t len);
    void OnDataSent(uint8_t *mac, uint8_t status);
  #endif


/*
---------------------------------------------------------------------------------------
-------------------------------------- CLASS ----------------------------------
---------------------------------------------------------------------------------------
*/

  class WiaNow : public WiaNet{
    public:
      bool init() override;
      void loop() override;
      bool send(String data) override;
      bool send(String data, String mac) override;
      bool send(String data, uint8_t*  mac);

      void addDataCB(CB_String cb) override { cbs = cb; }
      void addDataCB(CB_Json cb)   override { cbj = cb; }   

      void data_sent(String mac, bool state);
      void data_recv(String mac, String data);

      void setRssi(uint8_t _rssi){ rssi = _rssi; }
      uint8_t RSSI() { return rssi; }
      String MAC() { return WiFi.macAddress(); }  

    protected:
      bool setSnifferCallback();
      bool addPeer(uint8_t* mac);
      bool addPeers();
    private:
      uint8_t rssi = 0;
      int peer_channel = 1;
      CB_String cbs = nullptr;
      CB_Json cbj = nullptr;
  };

  /*
---------------------------------------------------------------------------------------
---------------------------------- DETAIL FUNCTIONS -----------------------------
---------------------------------------------------------------------------------------
*/
  bool WiaNow::init(){
    Serial.println("wia_setup_net::init --> begin");
    #ifdef NOW_TYPE
      NetDB.init();
    #endif
    WiFi.mode(WIFI_STA);
    Serial.println("wia_setup_net::setSnifferCallback -->" + String(setSnifferCallback()?"success":"fail"));
    bool isInit = (esp_now_init()==0);
    Serial.println("wia_setup_net::now_init -->" + String(isInit?"success":"fail"));
    if(!isInit) return false;
    Serial.println("wia_setup_net::addPeers -->" + String(addPeers()?"success":"fail")); 
    Serial.println("wia_setup_net::now_cb_send -->" + String(esp_now_register_send_cb(OnDataSent)==0?"success":"fail"));
    Serial.println("wia_setup_net::now_cb_recv " + String(esp_now_register_recv_cb(OnDataRecv)==0?"success":"fail"));         
    Serial.println("wia_setup_net::init --> complete");
    delay(50);
  
    #ifdef LINKER_CONNECT
      Linker.init(WiFi.macAddress(),!CONNECTION_BUILDER);
      Linker.addPairCB([this](String data){ SerialHandler.serial_pass(data); wia_yield(); send(data); });    
      SerialHandler.addCB([](String data){ Linker.onRecvProtected(data); wia_yield(); });
    #endif
    return true;
    }
      

  void WiaNow::loop(){ 
    #ifdef LINKER_CONNECT
      Linker.periodicRequest();
    #endif
    }
  bool WiaNow::send(String data, String mac){
        if(mac.isEmpty()) return false;
        uint8_t cmac[6];
        MacConverter::toUIntMac(mac.c_str(),cmac);
        #ifdef LINKER_CONNECT
          data = WiaProtector::addProtectionLayer(data, WiFi.macAddress());
        #endif
        int error = esp_now_send(cmac, (uint8_t *) data.c_str(), data.length());
        Serial.println("WiaNow::send::"+  data);
        yield();
        return error == 0;
    }
  bool WiaNow::send(String data){
    #ifdef NOW_TYPE
      #if(CONN_TYP == CON_BROAD)
        String mac = NetDB.getLoadedBroads();
        String pcs;
        bool sent = false;
        while (mac.length() > 0) {
          int id = mac.indexOf('\n');
          if (id == -1) {
            pcs = mac;
            mac = "";
          } else {
            pcs = mac.substring(0, id);
            mac = mac.substring(id + 1);
          }
          sent = send(data, pcs);
        }
        return sent;
      #elif(CONN_TYP == CON_P2P)
        return send(data, NetDB.getPeer());
      #elif(CONN_TYP == CON_P2M)
        return send(data, NetDB.getPeer());
      #else
        return false;
      #endif
    #else
      return send(data, MacConverter::toStringMac(broadcastAddress));
    #endif
    }
  bool WiaNow::send(String data, uint8_t*  mac){ return send(data, MacConverter::toStringMac(mac)); }

  
  void WiaNow::data_sent(String mac, bool state){ Serial.println("wia_net_sent -->" + String(state ? "success |" : "fail | ")+ mac); }
  void WiaNow::data_recv(String mac, String data){
        if(cbs != nullptr) cbs(data); 
        if(cbj != nullptr) ;//cbj(data); 
        #ifdef LINKER_CONNECT
          Linker.onRecvProtected(data); 
        #endif
      }  
  bool WiaNow::setSnifferCallback(){
    #if defined(ESP8266) && (!defined(IS_ESP01) || (!IS_ESP01))
      wifi_set_opmode(STATION_MODE);
      wifi_promiscuous_enable(true);
      wifi_set_promiscuous_rx_cb(sniffer_callback);
      return true;
    #elif defined(ESP32) 
      esp_wifi_set_mode(WIFI_MODE_STA);
      esp_wifi_set_promiscuous(true);
      esp_wifi_set_promiscuous_rx_cb(sniffer_callback);
      return true;
    #endif
    return false;
    }
  bool WiaNow::addPeer(uint8_t* mac){
    bool added = false;
    #if defined(ESP32)
      esp_now_peer_info_t peer = {};
      memcpy(peer.peer_addr, mac, 6);
      peer.channel = peer_channel;
      peer.encrypt = false;
      peer.ifidx = WIFI_IF_STA;
      added = (esp_now_add_peer(&peer) == ESP_OK);
    #elif defined(ESP8266)
      int error = esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
      Serial.println("addPeer::esp_now_set_self_role:: " + error);
      error = esp_now_add_peer(mac, ESP_NOW_ROLE_COMBO, peer_channel, NULL, 0);
      Serial.println("addPeer::esp_now_add_peer:: " + error);
      added = (error == 0);     
    #endif     
    return added;
    }
  bool WiaNow::addPeers(){
    bool added = false;
    #ifdef NOW_TYPE
      #if(CONN_TYP == CON_BROAD)
        added = addPeer(broadcastAddress);
        Serial.println("addPeers::CON_BROAD -->" + String(added?"success":"fail"));
        NetDB.load();
      #endif
      #if(CONN_TYP == CON_P2P)
        String smac="";    
        bool allow = NetDB.getPeer(smac);
        Serial.println("addPeers::wia_ndb_get_peer -->" + String(allow?"success":"fail"));
        if(allow){
          uint8_t mac[6];
          if(MacConverter::toUIntMac(smac.c_str(),mac)) added = addPeer(mac);                
        }
        Serial.println("addPeers::"+String((CONN_TYP == CON_P2M)?"CON_P2M:-PEER -->":"CON_P2P -->")+String(added?"success":"fail"));
      #endif
      #if(CONN_TYP == CON_P2M)
        String smac="";   
        bool allow = NetDB.getPeer(smac);
        Serial.println("addPeers::wia_ndb_get_peer -->" + String(allow?"success":"fail"));
        if(allow){
          uint8_t mac[6];
          if(MacConverter::toUIntMac(smac.c_str(),mac)) added = addPeer(mac);                
        }
        Serial.println("addPeers::CON_P2M:-PEER -->" + String(added?"success":"fail"));
        allow = NetDB.getMaster(smac);
        Serial.println("addPeers::wia_ndb_get_master -->" + String(allow?"success":"fail"));
        if(allow){
          uint8_t mac[6];
          if(MacConverter::toUIntMac(smac.c_str(),mac))added = addPeer(mac);
        }
        Serial.println("addPeers::CON_P2M:-MASTER -->" + String(added?"success":"fail"));      
      #endif
    #endif
    return added;
    }
  

  WiaNow* _netNow = nullptr;
  void reg_net(WiaNow* netNow){ _netNow = netNow; }

  /////////////////////////////////////////////////////////////////

  #if defined(ESP32)
    void sniffer_callback(void *buf, wifi_promiscuous_pkt_type_t type) { if (type == WIFI_PKT_MGMT && _netNow != nullptr) _netNow->setRssi(((wifi_promiscuous_pkt_t *)buf)->rx_ctrl.rssi);}
    void OnDataRecv(const esp_now_recv_info_t* info, const uint8_t* incomingData, int len){ if(_netNow!=nullptr) _netNow->data_recv(MacConverter::toStringMac(info->src_addr), String((char*) incomingData));}
    void OnDataSent(const uint8_t *mac, esp_now_send_status_t status){ if(_netNow!=nullptr) _netNow->data_sent(MacConverter::toStringMac(mac),(status == ESP_NOW_SEND_SUCCESS)); }
  #elif defined(ESP8266)
    void ICACHE_FLASH_ATTR sniffer_callback(uint8_t *buffer, uint16_t length) { if(_netNow!=nullptr) _netNow->setRssi(((struct sniffer_buf*) buffer)->rx_ctrl.rssi); }
    void OnDataRecv(uint8_t * mac_address, uint8_t *incomingData, uint8_t len) { if(_netNow!=nullptr) _netNow->data_recv(MacConverter::toStringMac(mac_address),String((char*) incomingData)); }
    void OnDataSent(uint8_t *mac, uint8_t status) { if(_netNow!=nullptr) _netNow->data_sent(MacConverter::toStringMac(mac),status == 0); }
  #endif

#endif