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

#endif