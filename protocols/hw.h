/*
 * hw.h
 *
 *  Created on: Feb 5, 2018
 *      Author: mac
 */

#ifndef HW_H_
#define HW_H_

#include "../main.h"
#ifdef _VISA_PAY_DEVICE__

#include <curl/curl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>

#include "serial.h"

extern "C"{
#include "../SDK/PaymentController.h"
}

Ibox_MemoryStruct *sendWebRequest(const char *request, const char *url);
Ibox_MemoryStruct *sendReaderRequest(char *request, int length);
#endif

#endif /* HW_H_ */
