#include "../main.h"
#include "visa.h"

static void startTransactionAction(const char *transactionId)
{
	if (transactionId)
	{
		mTransactionId = transactionId;
		fprintf(stderr, "TransactionID:%s\n", transactionId);
		sprintf(sTransactionId, "%s", transactionId);
	}
	else
	{
		sprintf(sTransactionId, "%s", "");
	}
}

void VisaDevice::Init(Settings* setting)
{
	sprintf(user_name, setting->visaParam.login);
	sprintf(user_password, setting->visaParam.password);
	debug_enabled = setting->visaParam.debug;

	errorCode = 0;
}

bool VisaDevice::IsOpened()
{
	return 1;
}

bool VisaDevice::OpenDevice()
{
	return 1;
}

bool VisaDevice::CloseDevice()
{
	return 1;
}

bool VisaDevice::Update()
{
	if (payInfo.inUse == 1)
	{
		if (DoPayment(&payInfo) == 0)
		{
			payInfo.inUse = 0;
			return 0;
		}
		else
		{
			payInfo.inUse = 0;
			return 1;
		}
	}
	return 1;
}

bool VisaDevice::DoPayment(PayInfo* payInfo)
{
	payInfo->result = 1;
	char command[4096] = {0};
	sprintf(command, "./bwpay \"%s\" \"%s\" %d %4.2f \"%s\" \"%s\"", user_name, user_password, 1, payInfo->summ, "Услуги автомойки", payInfo->note);
	printf("Run: %s\n", command);
	int res = system(command);
	if (res == 0)
	{
		payInfo->result = 0;
		return 1;
	}
	return 0;
}


VisaDevice::VisaDevice()
{
}

VisaDevice::~VisaDevice()
{
}