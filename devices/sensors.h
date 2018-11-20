#ifndef _SENSORS_H_
#define _SENSORS_H_

class Settings;

class Sensors
{
  private:
    static Sensors* p_instance;

    Sensors(){}
    Sensors( const Sensors& );
    Sensors& operator=( Sensors& );
  public:
  	void Init(Settings* setting);

    static Sensors* getInstance()
    {
        if(!p_instance)
            p_instance = new Sensors();
        return p_instance;
    }
};

#endif