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
	if (payInfo.inUse == 0) return 1;
	return DoPayment(&payInfo);
}

bool VisaDevice::DoPayment(PayInfo* payInfo)
{
	payInfo->result = PAY_RESULT_ERROR;
	char command[1024];
	memset(command, 0, sizeof(command));
	sprintf(command, "./bwpay \"%s\" \"%s\" %d %4.2f \"%s\" \"%s\" \"%s\" \"%s\"", user_name, user_password, 1, payInfo->summ, "Услуги автомойки", payInfo->note, payInfo->r_phone, payInfo->r_email);
//	printf("Run: %s\n", command);
	int res = system(command);
//	printf("payCmd result: %d %d\n", res, WEXITSTATUS(res));
	payInfo->inUse = 0;
	if (res == -1 || WEXITSTATUS(res) != 0)
		payInfo->result = PAY_RESULT_OK;
	return payInfo->result;
}


VisaDevice::VisaDevice()
{
}

VisaDevice::~VisaDevice()
{
}