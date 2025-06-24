#ifndef WIA_MEGA_NET_BASE
  #define WIA_MEGA_NET_BASE
/*
--------------------------------------------------------------------------------------- 
------------------------------------ Define Checks ------------------------------------ 
--------------------------------------------------------------------------------------- 
*/
  //
/*
--------------------------------------------------------------------------------------- 
------------------------------------ Libraries ------------------------------------ 
--------------------------------------------------------------------------------------- 
*/
  #include <ArduinoJson.h>
/*
--------------------------------------------------------------------------------------- 
------------------------------------ Helping Material ------------------------------------ 
--------------------------------------------------------------------------------------- 
*/

  typedef std::function<void()> CB_Void;
  typedef std::function<void(String data)> CB_String;
  typedef std::function<void(JsonObject data)> CB_Json;

  typedef std::function<String()>                 RCB_Void;
  typedef std::function<String(bool)>             RCB_Bool;
  typedef std::function<String(int)>              RCB_Int;
  typedef std::function<String(float)>            RCB_Float;
  typedef std::function<String(String)>           RCB_String;
  typedef std::function<String(JsonObject data)>  RCB_Json;


/*
--------------------------------------------------------------------------------------- 
--------------------------------------- Class ------------------------------------ 
--------------------------------------------------------------------------------------- 
*/
  class WiaNet {
    public:
      virtual bool init() = 0;
      virtual void loop() = 0;

      virtual bool send(String data) = 0;
      virtual bool send(String data, String mac) = 0;

      virtual void addDataCB(CB_String cb) { (void)cb; }
      virtual void addDataCB(CB_Json cb)   { (void)cb; }     
  };

  class MacConverter{
    public:
      static String toStringMac(const uint8_t* mac) {
        char buf[18];
        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return String(buf);}
      static bool toStringMac(const uint8_t* mac, String& _str){
        char buf[18];
        bool res= sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        _str = String(buf);
        return res;}
      static bool toCharArrayMac(const uint8_t* mac, char* _charArray) { return sprintf(_charArray, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); }
      static bool toUIntMac(const char* mac, uint8_t* _uint) { return sscanf(mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &_uint[0], &_uint[1], &_uint[2], &_uint[3], &_uint[4], &_uint[5]); }
      static bool equalMakes(const uint8_t* _m1, const uint8_t* _m2) { for (int i = 0; i < 6; i++) { if (_m1[i] != _m2[i]) { return false; } } return true; }
  };

#endif