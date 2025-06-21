#ifndef WIA_MEGA_DB
  #define WIA_MEGA_DB
/*
--------------------------------------------------------------------------------------- 
------------------------------------ Define Checks ------------------------------------ 
--------------------------------------------------------------------------------------- 
*/

  //

/*
--------------------------------------------------------------------------------------- 
----------------------------------- Class ---------------------------------- 
--------------------------------------------------------------------------------------- 
*/
  class WiaDB{
    public:
      bool init();

      template <typename T>
      bool put(const char* key, T val);
      
      template <typename T>
      T get(const char* key, T def);
    protected:
      bool wia_db_init();

      bool getBool(const char* key, bool val = false);
      int getInt(const char* key, int val = 0);
      long getLong(const char* key, long val = 0L);
      float getFloat(const char* key, float val = 0.0f);
      String getString(const char* key, String val = String());
      
      bool setBool(const char* key, bool val);
      bool setInt(const char* key, int val);
      bool setLong(const char* key, long val);
      bool setFloat(const char* key, float val);
      bool setString(const char* key, String val);

      #if defined(ESP8266)
        bool set_db_data(const char* key, String data, char typ, bool append = false);
        String get_db_data(const char* key, char typ);
      #endif
  };

  class WiaDbHelper : public WiaDB{
    protected:
      WiaDbHelper(String typ);
      bool has(String key = String());
      bool is(String data, String key = String());
      bool add(String data, String key = String());
      bool get(String& data, String key = String());
      String getStr(String key = String());
    private:
      String _typ;
  };



/*
--------------------------------------------------------------------------------------- 
------------------------------------ Libraries ------------------------------------ 
--------------------------------------------------------------------------------------- 
*/ 
  //#include "WiaDb.h" 
  #if defined(ESP32)
    #include <Preferences.h>
    Preferences _pref;
    #define DB "wia_db"
  #elif defined(ESP8266)
    #include <FS.h>
    #include <LittleFS.h>
  #endif
  #include <Arduino.h>

  bool db_init = false;

/*
--------------------------------------------------------------------------------------- 
------------------------------- WiaDB:: Definations ---------------------------------- 
--------------------------------------------------------------------------------------- 
*/

  
  bool WiaDB::init(){ if(db_init) return true;
    db_init = wia_db_init(); 
    return db_init; 
    }
  template <typename T>
  bool WiaDB::put(const char* key, T val){
    if constexpr (std::is_same<T, int>::value) return setInt(key, val);
    else if constexpr (std::is_same<T, float>::value) return setFloat(key, val);
    else if constexpr (std::is_same<T, long>::value) return setLong(key, val);
    else if constexpr (std::is_same<T, bool>::value) return setBool(key, val);
    else if constexpr (std::is_same<T, String>::value) return setString(key, val);
    else return false;  // unsupported type

  }

  template <typename T>
  T WiaDB::get(const char* key, T def){
    if constexpr (std::is_same<T, int>::value) return getInt(key, def);
    else if constexpr (std::is_same<T, float>::value) return getFloat(key, def);
    else if constexpr (std::is_same<T, long>::value) return getLong(key, def);
    else if constexpr (std::is_same<T, bool>::value) return getBool(key, def);
    else if constexpr (std::is_same<T, String>::value) return getString(key, def);
    else return def; 
  }
  
  #if defined(ESP32)
    bool WiaDB::wia_db_init(){return true;}
    bool WiaDB::getBool(const char* key, bool val){
      _pref.begin(DB, false);
      bool l = _pref.getBool(key, val);
      _pref.end();
      return l;}
    int WiaDB::getInt(const char* key, int val){ 
      _pref.begin(DB, false);
      int l = _pref.getInt(key, val);
      _pref.end();
      return l;}
    long WiaDB::getLong(const char* key, long val){ 
      _pref.begin(DB, false);
      long l = _pref.getLong(key, val);
      _pref.end();
      return l;}
    float WiaDB::getFloat(const char* key, float val){ 
      _pref.begin(DB, false);
      float l = _pref.getFloat(key, val);
      _pref.end();
      return l;}
    String WiaDB::getString(const char* key, String val){ 
      _pref.begin(DB, false);
      String l = _pref.getString(key, val);
      _pref.end();
      return l;}


    bool WiaDB::setBool(const char* key, bool val){
      _pref.begin(DB, false);
      size_t res = _pref.putBool(key, val);
      _pref.end();
      return res > 0;}
    bool WiaDB::setInt(const char* key, int val){
      _pref.begin(DB, false);
      size_t res = _pref.putInt(key, val);
      _pref.end();
      return res > 0;}
    bool WiaDB::setLong(const char* key, long val){
      _pref.begin(DB, false);
      size_t res = _pref.putLong(key, val);
      _pref.end();
      return res > 0;}
    bool WiaDB::setFloat(const char* key, float val){
      _pref.begin(DB, false);
      size_t res = _pref.putFloat(key, val);
      _pref.end();
      return res > 0;}
    bool WiaDB::setString(const char* key, String val){
      _pref.begin(DB, false);
      size_t res = _pref.putString(key, val);
      _pref.end();
      return res > 0;}


  #elif defined(ESP8266)
    bool WiaDB::wia_db_init(){ return LittleFS.begin();}

    bool WiaDB::set_db_data(const char* key, String data, char typ, bool append){
      String path = String('/')+String(key)+String(".txt");
      File file = LittleFS.open(path.c_str(), append?"a":"w");
      if (!file) return false;
      String _data = String(typ)+data;
      bool success = (file.print(_data));
      yield();
      file.close();
      return success;}
    String WiaDB::get_db_data(const char* key, char typ){
      String path = String('/')+String(key)+String(".txt");
      File file = LittleFS.open(path.c_str(), "r");
      if (!file) return String();
      String data = file.readString();
      file.close();
      if(data[0] == typ)
        return data.substring(1);
      else return String();
    }  

    bool WiaDB::getBool(const char* key, bool val){return (getInt(key, val?1:0)==1); }
    int WiaDB::getInt(const char* key, int val){ 
      String data = get_db_data(key, 'i');
      if(data.isEmpty()) return val;
      return data.toInt();}
    long WiaDB::getLong(const char* key, long val){ 
      String data = get_db_data(key, 'l');
      if(data.isEmpty()) return val;
      return (long)data.toInt();}
    float WiaDB::getFloat(const char* key, float val){ 
      String data = get_db_data(key, 'f');
      if(data.isEmpty()) return val;
      return data.toFloat();}
    String WiaDB::getString(const char* key, String val){ 
    String data = get_db_data(key, 's');
      if(data.isEmpty()) return val;
      return data;}   
    bool WiaDB::setBool(const char* key, bool val){ return setInt(key, val?1:0);}
    bool WiaDB::setInt(const char* key, int val){ return set_db_data(key, String(val),'i');}
    bool WiaDB::setLong(const char* key, long val){ return set_db_data(key, String(val),'l');}
    bool WiaDB::setFloat(const char* key, float val){ return set_db_data(key, String(val),'f');}
    bool WiaDB::setString(const char* key, String val){ return set_db_data(key, val,'s');}


  #endif
/*
--------------------------------------------------------------------------------------- 
--------------------------- WiaDbHelper:: Definations -------------------------------- 
--------------------------------------------------------------------------------------- 
*/
    WiaDbHelper::WiaDbHelper(String typ):_typ(typ){ }
    bool WiaDbHelper::has(String key){ 
      return !getStr(key).isEmpty(); 
      }
    bool WiaDbHelper::is(String data, String key) {
      String id = getStr(key);
      return !id.isEmpty() && id == data;
      }
    bool WiaDbHelper::add(String data, String key){ 
      return setString(String(String(_typ) + key).c_str(), data);
      }
    bool WiaDbHelper::get(String& data, String key){ 
      data = getStr(key); 
      return !data.isEmpty();
      }
    String WiaDbHelper::getStr(String key){ 
      return getString(String(String(_typ) + key).c_str());
      }
//
#endif