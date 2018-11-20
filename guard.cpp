#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include "iniparser/iniparser.h"
#include "iniparser/dictionary.h"

char userName[7] = {0xC3, 0xC6, 0xD2, 0xD7, 0xC2, 0xD3, 0x00};
char userPass[7] = {0x9E, 0x9B, 0x8F, 0x8A, 0x9F, 0x8E, 0x00};

pid_t proc_find(const char* name)
{
    DIR* dir;
    struct dirent* ent;
    char* endptr;
    char buf[512];

    if (!(dir = opendir("/proc"))) {
        perror("can't open /proc");
        return -1;
    }

    while((ent = readdir(dir)) != NULL) {
        /* if endptr is not a null character, the directory is not
         * entirely numeric, so ignore it */
        long lpid = strtol(ent->d_name, &endptr, 10);
        if (*endptr != '\0') {
            continue;
        }

        /* try to open the cmdline file */
        snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);
        FILE* fp = fopen(buf, "r");

        if (fp) {
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                /* check the first token in the file, the program name */
                char* first = strtok(buf, " ");
                if (!strcmp(first, name)) {
                    fclose(fp);
                    closedir(dir);
                    return (pid_t)lpid;
                }
            }
            fclose(fp);
        }

    }

    closedir(dir);
    return -1;
}


void convert()
{
	for (int index=0; index < 6; index++)
		userName[index] ^= 0xB6;
	for (int index=0; index < 6; index++)
		userPass[index] ^= 0xEB;
}

int main()
{
	dictionary*   		ini;
	int useGuard = 0;
	int DeviceID;
	int GuardCheckTimeSec = 2;
	char paramName[100];
	char process_name[1024];
	char command_name[1024];
	char devaddr[256];

	char settingFileName[] = "/home/pi/bewash/bewash.cfg\x0";
	ini = iniparser_load(settingFileName);
	if (ini == NULL)
	{
		printf("Error: cannot parse config file: %s\n", settingFileName);
		return -1;
	}

	convert();
	convert();

	memset(paramName, 0, sizeof(paramName));

	sprintf(process_name, "%s", iniparser_getstring(ini, "Guard:GuardProcessName", "bewashs"));
	sprintf(command_name, "%s", iniparser_getstring(ini, "Guard:GuardCommandName", "sudo /home/pi/bewash/bewashs"));
	useGuard							= iniparser_getint(ini, "Guard:Guard", 	0);
	GuardCheckTimeSec					= iniparser_getint(ini, "Guard:GuardCheckTimeSec", 	2);
	DeviceID							= iniparser_getint(ini, "Common:DeviceID", 	109);

	sprintf(devaddr, "sudo ifconfig eth0 add 192.168.254.%d netmask 255.255.255.0", DeviceID);
	printf("Add new IP: %s\n", devaddr);
	system(devaddr);

	int status = 1;
	while ((useGuard))
	{
		pid_t pid = proc_find(process_name);
		if (pid == -1)
		{
			printf("\n\n\n[GUARD]: Start BeWash Application ...\n\n");
			status = system(command_name);
			printf("[GUARD]: beWash application closed: STATUS %d\n", status);
			if (status == 2816) break;
		}
		sleep(GuardCheckTimeSec);
	}
    return 0;
}
