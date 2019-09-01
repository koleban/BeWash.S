#include "../main.h"

static unsigned char newChar [] =
{
  0x04, 0x0a, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x00, // russian A		0
  0x1f, 0x10, 0x10, 0x1e, 0x11, 0x11, 0x1e, 0x00, // russian Be     1
  0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e, 0x0,  // russian Ve
  0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, // russian Ge     3
  0x0e, 0x0a, 0x0a, 0x0a, 0x0a, 0x1f, 0x11, 0x00, // russian De		4
  0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f, 0x00, // russian E
  0x15, 0x15, 0x15, 0x0e, 0x15, 0x15, 0x15, 0x00, // russian Zhe
  0x0e, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0e, 0x00, // russian Ze
  0x11, 0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x00, // russian I
  0x0e, 0x00, 0x11, 0x13, 0x15, 0x19, 0x11, 0x00, // russian Ij
  0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11, 0x00, // russian Ka
  0x07, 0x09, 0x09, 0x09, 0x09, 0x09, 0x11, 0x00, // russian L      11
  0x11, 0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x00, // russian M
  0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11, 0x00, // russian N      13
  0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00, // russian O
  0x1f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00, // russian Pe		15
  0x1e, 0x11, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x00, // russian Re		16
  0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e, 0x00, // russian Se     17
  0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, // russian Te
  0x11, 0x11, 0x11, 0x0f, 0x01, 0x11, 0x0e, 0x00, // russian U      19
  0x0e, 0x15, 0x15, 0x15, 0x0e, 0x04, 0x04, 0x00, // russian Fe
  0x11, 0x0a, 0x04, 0x04, 0x0a, 0x0a, 0x11, 0x00, // russian He
  0x12, 0x12, 0x12, 0x12, 0x12, 0x1f, 0x01, 0x00, // russian Ce
  0x11, 0x11, 0x11, 0x0f, 0x01, 0x01, 0x01, 0x00, // russian Che
  0x11, 0x15, 0x15, 0x15, 0x15, 0x15, 0x1f, 0x00, // russian She
  0x11, 0x15, 0x15, 0x15, 0x15, 0x1f, 0x01, 0x00, // russian Sche
  0x18, 0x08, 0x08, 0x0e, 0x09, 0x09, 0x0e, 0x00, // russian Tverduy znak
  0x11, 0x11, 0x19, 0x15, 0x15, 0x15, 0x19, 0x00, // russian bI     27
  0x10, 0x10, 0x1e, 0x11, 0x11, 0x11, 0x1e, 0x00, // russian Myagkiy znak
  0x1e, 0x01, 0x01, 0x0f, 0x01, 0x01, 0x1e, 0x00, // russian Ee
  0x17, 0x15, 0x15, 0x1d, 0x15, 0x15, 0x17, 0x00, // russian You
  0x0f, 0x11, 0x11, 0x0f, 0x05, 0x09, 0x11, 0x00 // russian Ya
};


// Global lcd handle:

static int lcdHandle ;

static const char *message =
  "                       "
  "beWash: Self service carwash technology. http://www.bewash.ru    email: sale@bewash.ru    tel.: +7 (928) 937-16-78"
  "                    ";

void scrollMessage (int line, int width)
{
  char buf [32] ;
  static int position = 0 ;
  static int timer = 0 ;

  if (millis () < timer)
    return ;

  timer = millis () + 200 ;

  strncpy (buf, &message [position], width) ;
  buf [width] = 0 ;
  lcdPosition (lcdHandle, 0, line) ;
  lcdPuts     (lcdHandle, buf) ;

  if (++position == (strlen (message) - width))
    position = 0 ;
}

PI_THREAD(Lcd20x4)
{
	///
	/// Если ПОТОК запрещен, то завершаемся
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.Lcd20x4Watch) return (void*)0;
	if (!settings->useMCP) return (void*)0;

	usleep(3000000);
	int i ;
	int lcd ;
	int bits, rows, cols ;

	struct tm *t ;
	time_t tim ;

	char buf [32] ;

	bits = 8 ;
	cols = 20 ;
	rows = 4 ;
	char tmp_dig[20];
	char tmp_buff_string[200];
	char bal_str[] = {0x01, 'A', 0x02, 'A', 'H', 'C', ':', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
	char rub_str[] = {0x20, 'P', 0x03, 0x01, '.', 0x00};
	char prg_str[] = {0x05, 'P', 'O', 0x04, 'P', 'A', 'M', 'M', 'A', ':', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
	char pil_str[] = {0x05, 0x06, 0x02, 'E', 'C', 'O', 'C', 0x20, 0x20, 0x00};
	char voz_str[] = {'B', 'O', '3', 0x07, 0x03, 'X', 0x20, 0x20, 0x20, 0x00};
	char stp_str[] = {'C', 'T', 'O', 0x05, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
	char lin_str[] = "--------------------";
	char lin_str_warn[] = "!!!!!!!!!!!!!!!!!!!!";

	setPinModeMy(1, 0);
	setPinModeMy(15, 0);
	setPinModeMy(16, 0);
	setPinModeMy(104, 0);
	setPinModeMy(105, 0);
	setPinModeMy(106, 0);
	setPinModeMy(107, 0);
	setPinModeMy(108, 0);
	setPinModeMy(109, 0);
	setPinModeMy(110, 0);
	setPinModeMy(111, 0);

	setGPIOState(1, 1);
	usleep(50000);
	setGPIOState(1, 0);
	setGPIOState(15, 0);
	setGPIOState(16, 0);

	lcdHandle = lcdInit(rows, cols, 8, 16, 15, 104,105, 106, 107, 108, 109, 110, 111) ;

	if (lcdHandle < 0)
	{
		printf ("lcd20x4 Init failed\n") ;
		return (void*)0 ;
	}

	lcdDisplay(lcdHandle, 0);
	delay_ms(50);
	lcdDisplay(lcdHandle, 1);

	sprintf(tmp_buff_string, "    beWash v 2.05   ");
	lcdPosition (lcdHandle, 0, settings->lcd24Param.balStrNum) ;
	lcdPuts( lcdHandle, tmp_buff_string);

	sprintf(tmp_buff_string, "Self services system");
	lcdPosition (lcdHandle, 0, settings->lcd24Param.prgStrNum) ;
	lcdPuts( lcdHandle, tmp_buff_string);

	sprintf(tmp_buff_string, "     3A_P_3KA       ");
	tmp_buff_string[7] = 0x04;
	tmp_buff_string[9] = 0x03;
	lcdPosition (lcdHandle, 0, settings->lcd24Param.adsStrNum) ;
	lcdPuts( lcdHandle, tmp_buff_string);

	sprintf(tmp_buff_string, "                    ");
	for(int ind=0; ind < 16; ind++)
	{
		tmp_buff_string[2+ind] = '#';
		lcdPosition (lcdHandle, 0, settings->lcd24Param.lineStrNum) ;
		lcdPuts( lcdHandle, tmp_buff_string);
		usleep(50000);
	}
	usleep(1000000);
	sprintf(tmp_buff_string, "         OK         ");
	lcdPosition (lcdHandle, 0, settings->lcd24Param.adsStrNum) ;
	lcdPuts( lcdHandle, tmp_buff_string);
	usleep(1000000);

	lcdClear    (lcdHandle) ;

	int counter = 0;

	int currentPrg = status.intDeviceInfo.program_currentProgram;

	int indexA = 0;
	while (settings->threadFlag.Lcd20x4Watch)
	{
		if (indexA++ > 20)
		{
			lcdReInit(lcdHandle);
			indexA = 0;
		}
		if ((currentPrg != status.intDeviceInfo.program_currentProgram) || (counter++ > 80))
		{
			counter = 0;
			currentPrg = status.intDeviceInfo.program_currentProgram;
			lcdReInit(lcdHandle);
			delay_ms(50);
		}
		setGPIOState(1, 0);
		lcdCharDef( lcdHandle, 1, &newChar[8]);		// Б
		lcdCharDef( lcdHandle, 2, &newChar[88]);	// Л
		lcdCharDef( lcdHandle, 3, &newChar[19*8]);	// У
		lcdCharDef( lcdHandle, 4, &newChar[24]);	// Г
		lcdCharDef( lcdHandle, 5, &newChar[15*8]);	// П
		lcdCharDef( lcdHandle, 6, &newChar[27*8]);	// Ы
		lcdCharDef( lcdHandle, 7, &newChar[32]);	// Д
		lcdCharDef( lcdHandle, 8, &newChar[64]);	// И

		scrollMessage (settings->lcd24Param.adsStrNum, cols) ;

		sprintf(tmp_dig, "%6d", status.intDeviceInfo.money_currentBalance);
		memcpy(tmp_buff_string, bal_str, sizeof(bal_str));
		memcpy(&tmp_buff_string[8], tmp_dig, 6);
		memcpy(&tmp_buff_string[14], rub_str, 5);

		if (status.extDeviceInfo.collectionButton)
		{
			lcdPosition (lcdHandle, 0, settings->lcd24Param.balStrNum) ;
			char warning_str[] = "   BH_MAH_E !!!     ";
			warning_str[5] = 0x08;
			warning_str[9] = 0x08;
			lcdPuts( lcdHandle, warning_str);

			sprintf(tmp_dig, "%7d", status.intDeviceInfo.allMoney);
			memcpy(tmp_buff_string, bal_str, sizeof(bal_str));
			memcpy(&tmp_buff_string[8], tmp_dig, 7);
			memcpy(&tmp_buff_string[15], rub_str, 5);
			lcdPosition (lcdHandle, 0, settings->lcd24Param.prgStrNum) ;
			lcdPuts( lcdHandle, tmp_buff_string);
			lcdPosition (lcdHandle, 0, settings->lcd24Param.lineStrNum) ;
			lcdPuts( lcdHandle, lin_str_warn);
		}
		else
		{
			lcdPosition (lcdHandle, 0, settings->lcd24Param.balStrNum) ;
			lcdPuts( lcdHandle, tmp_buff_string);

			memset(tmp_buff_string, 0, sizeof(tmp_buff_string));
			memcpy(tmp_buff_string, prg_str, sizeof(prg_str));
			if (status.intDeviceInfo.program_currentProgram < 2)
				memcpy(&tmp_buff_string[11], stp_str, 9);
			if (status.intDeviceInfo.program_currentProgram == 2)
				memcpy(&tmp_buff_string[11], pil_str, 9);
			if (status.intDeviceInfo.program_currentProgram > 2)
				memcpy(&tmp_buff_string[11], voz_str, 9);

			lcdPosition (lcdHandle, 0, settings->lcd24Param.prgStrNum) ;
			lcdPuts( lcdHandle, tmp_buff_string);
			lcdPosition (lcdHandle, 0, settings->lcd24Param.lineStrNum) ;
			lcdPuts( lcdHandle, lin_str);
		}

		usleep(250000);
	}

	return (void*)0;
}
