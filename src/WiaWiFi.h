#ifndef WIA_MEGA_NET_WIFI
  #define WIA_MEGA_NET_WIFI
/*
--------------------------------------------------------------------------------------- 
------------------------------------ Define Checks ------------------------------------ 
--------------------------------------------------------------------------------------- 
*/

  #ifndef WIA_ENABLE_OTA
    #define WIA_ENABLE_OTA false
  #else 
    #ifndef WIA_OTA_PASSWORD_HASH
      #define WIA_OTA_PASSWORD_HASH "ff8d406cad04c4deb42da66b9989a5e8"
    #endif
  #endif
  #ifndef WIA_ENABLE_NET_AYSNC
    #define WIA_ENABLE_NET_AYSNC false
  #endif
  #ifndef DEFAULT_HOSTNAME
    #define DEFAULT_HOSTNAME "WiaWiFi"
  #endif
  #ifndef DEFAULT_GATEWAY
    #define DEFAULT_GATEWAY "192.168.10.1"
  #endif
  #ifndef DEFAULT_SUBNET
    #define DEFAULT_SUBNET "255.255.255.0"
  #endif
  #ifndef DEFAULT_IP
    #define DEFAULT_IP "192.168.10.90"
  #endif
  #ifndef DEFAULT_DNS
    #define DEFAULT_DNS "8.8.8.8"
  #endif

/*
--------------------------------------------------------------------------------------- 
------------------------------------ Libraries ------------------------------------ 
--------------------------------------------------------------------------------------- 
*/
  // Network 
    #include "WiaNetworkBases.h"
  // WiFiManager
    #if WIA_ENABLE_NET_AYSNC
      #include <ESPAsyncWiFiManager.h>
    #else
      #include <WiFiManager.h>
    #endif
  // OTA
    #if WIA_ENABLE_OTA
      #include <ArduinoOTA.h>
    #endif
  // DB
    #include "WiaDb.h"
  //
/*
--------------------------------------------------------------------------------------- 
---------------------------------------Helping Class ------------------------------------ 
--------------------------------------------------------------------------------------- 
*/
  #define DNS "dns"
  #define HOST_NAME "hn"
  #define GATE_WAY "gw"
  #define LOCAL_IP "lip"
  #define SUB_NET "sbn"

  class WiaNetCustomParams  : public WiaDB{
    public:
      bool setDNS(String dns){ return setString(DNS, dns); }
      bool setHostname(String hn){ return setString(HOST_NAME, hn); }
      bool setGateWay(String gw){ return setString(GATE_WAY, gw); }
      bool setSubnet(String sbn){ return setString(SUB_NET, sbn); }
      bool setIP(String ip){ return setString(LOCAL_IP, ip); }

      String getDNS(){  return getString(DNS, DEFAULT_DNS); }
      String getHostname(){  return getString(HOST_NAME, DEFAULT_HOSTNAME); }
      String getGateWay(){  return getString(GATE_WAY, DEFAULT_GATEWAY); }
      String getSubnet(){  return getString(SUB_NET, DEFAULT_SUBNET); }
      String getIP(){  return getString(LOCAL_IP, DEFAULT_IP); } 
  };
/*
--------------------------------------------------------------------------------------- 
--------------------------------------- Class ------------------------------------ 
--------------------------------------------------------------------------------------- 
*/

  class WiaWiFi : public WiaNet{
    public:
      bool init() override{ 
        Serial.print("WiaNetwork::init"); Serial.println("start");

        if(reset_pin!= -1){
          pinMode(reset_pin, INPUT);
        }

        ndb.init();
    
        bool saveConfig = false;
        
        wm.setSaveConfigCallback([&](){ saveConfig = true; });

        String hostname = ndb.getHostname();

        #if !WIA_ENABLE_NET_AYSNC
          bool saveParam = false;

          wm.setSaveParamsCallback([&](){ saveParam = true; });

          wm.setHostname(hostname);

          // set dark theme
          wm.setClass("invert");
          // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
          // const char* menu[] = {"wifi","info","param","sep","restart","exit"}; 
          // wm.setMenu(menu,6);
          std::vector<const char *> menu = {"wifi","param","sep","restart","exit"};
          wm.setMenu(menu);

          wm.setShowStaticFields(true); // force show static ip fields
          wm.setShowDnsFields(true);    // force show dns field always
          wm.setAPClientCheck(true); // avoid timeout if client connected to softap
        #endif

        #if WIA_ENABLE_NET_AYSNC
          AsyncWiFiManagerParameter 
        #else
          WiFiManagerParameter
        #endif
          customHost("host", "Host Name", hostname.c_str() , hostname.length());
          
        wm.addParameter(&customHost);
        IPAddress _ip, _gw, _sn, _dns;
        _ip.fromString(ndb.getIP().c_str());
        _gw.fromString(ndb.getGateWay().c_str());
        _sn.fromString(ndb.getSubnet().c_str());
        _dns.fromString(ndb.getDNS().c_str());
        
        yield();

        wm.setSTAStaticIPConfig(_ip, _gw, _sn);
        
        wm.setConnectTimeout(20); // how long to try to connect for before continuing
        wm.setConfigPortalTimeout(30); // auto close configportal after n seconds
        //wm.setCaptivePortalEnable(false); // disable captive portal redirection
        

        // wifi scan settings
        wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
        wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%

        wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails

        auto isConnected =  (wm.autoConnect("WIA_DEV_128","12345678"));

        if(isConnected){
          Serial.print("WiaNetwork::init"); Serial.println( "WiFi Connected.");
        }else{
          Serial.print("WiaNetwork::init"); Serial.println("WiFi Connection Failed.");
          delay(3000);
          ESP.restart();
          return false;
        }
        if(saveConfig){
          ndb.setIP(WiFi.localIP().toString());
          ndb.setGateWay(WiFi.gatewayIP().toString());
          ndb.setSubnet(WiFi.subnetMask().toString()); 
          ndb.setDNS(WiFi.dnsIP().toString()); 
        }
        #if !WIA_ENABLE_NET_AYSNC
          if(saveParam)
            ndb.setHostname(customHost.getValue());   
        #endif

        yield();
        setupOTA();
      
        Serial.print("WiaNetwork::init"); Serial.println("complete");        
        
        return true; }
      void loop() override{
        #if WIA_ENABLE_OTA
          ArduinoOTA.handle();
          yield();
        #endif
        resetLoop();
        
        }
      // send
      bool send(String data)             override{ (void)data; return false; }
      bool send(String data, String mac) override{ (void)data; (void)mac; return false; }
      
      bool setResetPin(int pin){ reset_pin = pin;}

    protected:
      void setupOTA(){
        #if (WIA_ENABLE_OTA)
            ArduinoOTA.onStart([]() { Serial.print("ArduinoOTA.onStart"); Serial.println(String((ArduinoOTA.getCommand() == U_FLASH)?"sketch":"filesystem")); });

            ArduinoOTA.onEnd([]() { Serial.print("ArduinoOTA.onEnd"); Serial.println("End"); });

            ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { Serial.print("ArduinoOTA.onProgress"); Serial.println( String (progress / (total / 100), 2)); });

            ArduinoOTA.onError([](ota_error_t error) {
              Serial.printf("Error[%u]: ", error);
              String val;
              if (error == OTA_AUTH_ERROR)          val = "OTA_AUTH_ERROR";
              else if (error == OTA_BEGIN_ERROR)    val = "OTA_BEGIN_ERROR";
              else if (error == OTA_CONNECT_ERROR)  val = "OTA_CONNECT_ERROR";
              else if (error == OTA_RECEIVE_ERROR)  val = "OTA_RECEIVE_ERROR";
              else if (error == OTA_END_ERROR)      val = "OTA_END_ERROR";
              else                                  val = "Unknown Error";
              Serial.print("ArduinoOTA.onError"); Serial.println( val);
            });

            ArduinoOTA.setPasswordHash(WIA_OTA_PASSWORD_HASH);

            ArduinoOTA.begin();
            Serial.println("OTA Ready");
            yield();
        #endif
      }  
      void resetLoop(){
        if(reset_pin == -1) return;
        auto state = digitalRead(reset_pin);
        if(digitalRead(reset_pin) == HIGH) return;
        delay(50); // debounce check
        if(digitalRead(reset_pin) == HIGH) return;
        delay(3000); // 3 sec low
        if(digitalRead(reset_pin) == HIGH) return;
        // reset creditional of WiFiManager
        wm.resetSettings();
        ESP.restart();
      }          
  
    private:
      WiaNetCustomParams ndb;
      int reset_pin = -1;
      #if WIA_ENABLE_NET_AYSNC
        AsyncWebServer server{80};
        DNSServer dns;
        AsyncWiFiManager wm{ &server, &dns};
      #else
        WiFiManager wm;
      #endif
  };
#endif