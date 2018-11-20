#ifndef _RELAYS_H_
#define _RELAYS_H_

class Settings;

class RelaySwitch
{
  private:
    static RelaySwitch* p_instance;

    RelaySwitch(){}
    RelaySwitch( const RelaySwitch& );
    RelaySwitch& operator=( RelaySwitch& );
  public:
  	void Init(Settings* setting);
	void relayInit(int relayNum);
	void relayOn(int relayNum);
	void relayOff(int relayNum);
	void valveInit(int valveNum);
	void valveOn(int valveNum);
	void valveOff(int valveNum);

    static RelaySwitch* getInstance()
    {
        if(!p_instance)
            p_instance = new RelaySwitch();
        return p_instance;
    }
};

#endif