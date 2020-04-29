
CL	= clean
CC      = g++
CC49      = g++-4.9
CFLAGS  =
# RELEASE
# -O2 -s -DNDEBUG
MFLAGS  = -x c++ -std=c++11 -lstdc++ -lwiringPi -Wsizeof-pointer-memaccess -fdiagnostics-color=auto -fsanitize=undefined -Wno-psabi
LFLAGS  = -x c++ -std=c++11 -lstdc++ -L/usr/local/lib  -L/usr/lib -lwiringPi -lfbclient -lm -ldl -lpthread -Wsizeof-pointer-memaccess -Wno-psabi -fdiagnostics-color=auto -fsanitize=undefined
SRCLIST = main.cpp debug.cpp db.cpp netserver.cpp netclient.cpp settings.cpp common.cpp tlv/key_list.c tlv/tlv_box.c drvfr/DrvFR.cpp drvfr/DrvFR_Conn.cpp devices/visa.cpp devices/crt288.cpp devices/cp-z.cpp devices/eeprom.cpp devices/lcd.c devices/engine.cpp devices/engine_E8400.cpp devices/engine_E9300.cpp devices/engine_E8100.cpp devices/engine_Emu.cpp devices/relays.cpp devices/monitor.cpp devices/rfid.cpp devices/coin.cpp devices/coinpulse.cpp devices/cctalkbill.cpp devices/ccbill.cpp protocols/serial.c protocols/max7219.cpp protocols/ccnet.cpp protocols/cctalk.cpp protocols/rs232.cpp protocols/rs485.cpp threads/th_remoteSender.cpp threads/th_buttons_terminal.cpp threads/th_osmos.cpp threads/th_voice.cpp threads/th_visa_device.cpp threads/th_kkm.cpp threads/th_buttons_master.cpp threads/th_remoteCtrl.cpp threads/th_remoteCounterCtrl.cpp threads/th_alien_device.cpp threads/th_service.cpp threads/th_7seg.cpp threads/th_debug.cpp threads/th_bill.cpp threads/th_billcctalk.cpp threads/th_buttons.cpp threads/th_coin.cpp threads/th_coin_pulse.cpp threads/th_data_exchange.cpp threads/th_engine.cpp threads/th_ext_common.cpp threads/th_int_common.cpp threads/th_net_client.cpp threads/th_rfid.cpp threads/th_eeprom.cpp threads/th_time.cpp threads/th_console.cpp threads/th_thermal.cpp threads/th_security.cpp threads/th_rfid_exchange.cpp threads/th_diag.cpp threads/th_lcd24x4.cpp iniparser/dictionary.c iniparser/iniparser.c ibpp/all_in_one.cpp other/queue.cpp other/md5.c

AR	= ar
ARFLAGS = rcv
RM      = rm -f

default: bewash

all: bewash-crt bewash-shs bewash-cp-z

bewash:
	$(CC49) $(CFLAGS) -o bewashs.guard guard.cpp qrcode/qrcode.c iniparser/dictionary.c iniparser/iniparser.c other/queue.cpp other/md5.c $(MFLAGS)
	$(CC49) $(CFLAGS) -o bewashs $(SRCLIST) $(LFLAGS) -D_RFID_DEVICE_CP_Z_ 

bewash-crt:
	mkdir ./bin/gcc4.9/bewashs/ -p
	mkdir ./bin/gcc6.0/bewashs/ -p
	$(CC49) $(CFLAGS) -o ./bin/gcc4.9/bewashs/bewashs.guard guard.cpp qrcode/qrcode.c iniparser/dictionary.c iniparser/iniparser.c other/queue.cpp other/md5.c $(MFLAGS)
	$(CC49) $(CFLAGS) -o ./bin/gcc4.9/bewashs/bewashs $(SRCLIST) $(LFLAGS) -D_RFID_DEVICE_CRT288K_ 
	$(CC) $(CFLAGS) -o ./bin/gcc6.0/bewashs/bewashs.guard guard.cpp qrcode/qrcode.c iniparser/dictionary.c iniparser/iniparser.c other/queue.cpp other/md5.c $(MFLAGS)
	$(CC) $(CFLAGS) -o ./bin/gcc6.0/bewashs/bewashs $(SRCLIST) $(LFLAGS) -D_RFID_DEVICE_CRT288K_ 

bewash-shs:
	mkdir ./bin/gcc4.9/bewashs-shs/ -p
	mkdir ./bin/gcc6.0/bewashs-shs/ -p
	$(CC49) $(CFLAGS) -o ./bin/gcc4.9/bewashs-shs/bewashs.guard guard.cpp qrcode/qrcode.c iniparser/dictionary.c iniparser/iniparser.c other/queue.cpp other/md5.c $(MFLAGS)
	$(CC49) $(CFLAGS) -o ./bin/gcc4.9/bewashs-shs/bewashs $(SRCLIST) $(LFLAGS) -D_RFID_DEVICE_SHS_ 
	$(CC) $(CFLAGS) -o ./bin/gcc6.0/bewashs-shs/bewashs.guard guard.cpp qrcode/qrcode.c iniparser/dictionary.c iniparser/iniparser.c other/queue.cpp other/md5.c $(MFLAGS)
	$(CC) $(CFLAGS) -o ./bin/gcc6.0/bewashs-shs/bewashs $(SRCLIST) $(LFLAGS) -D_RFID_DEVICE_SHS_ 

bewash-cp-z:
	mkdir ./bin/gcc4.9/bewashs-cp-z/ -p
	mkdir ./bin/gcc6.0/bewashs-cp-z/ -p
	$(CC49) $(CFLAGS) -o ./bin/gcc4.9/bewashs-cp-z/bewashs.guard guard.cpp qrcode/qrcode.c iniparser/dictionary.c iniparser/iniparser.c other/queue.cpp other/md5.c $(MFLAGS)
	$(CC49) $(CFLAGS) -o ./bin/gcc4.9/bewashs-cp-z/bewashs $(SRCLIST) $(LFLAGS) -D_RFID_DEVICE_CP_Z_ 
	$(CC) $(CFLAGS) -o ./bin/gcc6.0/bewashs-cp-z/bewashs.guard guard.cpp qrcode/qrcode.c iniparser/dictionary.c iniparser/iniparser.c other/queue.cpp other/md5.c $(MFLAGS)
	$(CC) $(CFLAGS) -o ./bin/gcc6.0/bewashs-cp-z/bewashs $(SRCLIST) $(LFLAGS) -D_RFID_DEVICE_CP_Z_ 

guard:
	$(CC) $(CFLAGS) -o bewashs.guard guard.cpp qrcode/qrcode.c iniparser/dictionary.c iniparser/iniparser.c other/queue.cpp other/md5.c $(MFLAGS)
