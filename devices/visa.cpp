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

	Ibox_PaymentController_SetSendWebRequestAction(&sendWebRequest);
	Ibox_PaymentController_SetSendReaderRequestAction(&sendReaderRequest);
	Ibox_PaymentController_SetStartTransactionAction(&startTransactionAction);
	Ibox_PaymentController_SetCredentials(user_name, user_password);
	Ibox_PaymentController_SetDebugEnabled(debug_enabled);
	Ibox_PaymentController_ReaderSoundEnabled(1);

	mTransactionId = NULL;

	readerIdResult = Ibox_PaymentController_ReaderId();
	if (!readerIdResult->errorCode)
		printf("ReaderId: %s\n", readerIdResult->id);
	else
		{errorCode = 1; return;}

	readerInfoResult = Ibox_PaymentController_ReaderInfo();
	if (!readerInfoResult->errorCode)
	{
		printf("Version:           %s\n", Ibox_PaymentController_Version());
		printf("BootloaderVersion: %s\n", readerInfoResult->bootloaderVersion);
		printf("FirmwareVersion:   %s\n", readerInfoResult->firmwareVersion);
		printf("HardwareVersion:   %s\n", readerInfoResult->hardwareVersion);
	}
	else
		{errorCode = 2; return;}

	authResult = Ibox_PaymentController_Authentication();
	if (!authResult->errorCode)
	{
		Ibox_Account *account = authResult->account;
		if (account)
		{
			printf("Account data:\n%s\n", account->name);
			printf("%s\n", account->clientLegalAddress);
			printf("%s\n", account->branchPhone);
			if (account->paymentOptionsCount)
			{
				Ibox_PaymentOption *paymentOption = account->paymentOptions[0];
				printf("First acquirer name: %s\n", paymentOption->acquirer->name);
				printf("First acquirer code: %s\n", paymentOption->acquirer->code);
			}
			errorCode = 0;
			return;
		}
	}
	else
	{
		printf("Authentication error:\n%s\n", authResult->errorMessage);
		errorCode = 3;
		return;
	}
	errorCode = 4;
}

bool VisaDevice::IsOpened()
{
	readerIdResult = Ibox_PaymentController_ReaderId();
	if (readerIdResult->errorCode)
		return 0;
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
		if (IsOpened() == 0)
			return 0;
		else
			if (readerIdResult != NULL)
				if (!readerIdResult->errorCode)
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
	if (payInfo == NULL) return 0;
	Ibox_PaymentContext* paymentContext = (Ibox_PaymentContext*)calloc(1, sizeof(Ibox_PaymentContext));
	paymentContext->inputType = Ibox_PaymentController_InputType_CARD;
	paymentContext->currencyType = Ibox_PaymentController_CurrencyType_RUB;
	paymentContext->amount = payInfo->summ;
	paymentContext->description = payInfo->note;
	paymentContext->singleStepAuth = 1;
	paymentContext->receiptEmail = payInfo->r_email;
	paymentContext->receiptPhone = payInfo->r_phone;
	paymentContext->acquirerCode = NULL;

	payInfo->id = transactionCount++;
    char ware_cp1251[1024] = {0};
    char ware[1024] = {0};
	sprintf(ware_cp1251, "{\"Purchases\": [{\"Title\": \"Услуга автомойки (Операция %02d)\", \"Price\": %4.2f, \"Quantity\": 1, \"TaxCode\": []}]}", payInfo->deviceNum, payInfo->summ);
	cp2utf(ware_cp1251, ware);
	printf("%s\n", ware);
	if (paymentContext)
		paymentContext->purchasesJson = (const char*)ware;
	else
		return 0;
	Ibox_Result_Submit *submitResult;
	printf("[VISA] Start payment\n");
	try
	{
   		submitResult = Ibox_PaymentController_StartPayment(paymentContext);
   	} catch(...) {free(paymentContext); return 0;}
   	if (submitResult == NULL) { free(paymentContext); return 0;}
	printf("[VISA] Check result\n");
	if (!submitResult->errorCode)
	{
		sprintf(payInfo->note, "");
		payInfo->result = submitResult->errorCode;
		sprintf(payInfo->note, "Payment done: %s\n", submitResult->errorMessage);
		sprintf(payInfo->transactionId, "%s", sTransactionId);
	}
	else
	{
		sprintf(payInfo->note, "Payment error: %s\n", submitResult->errorMessage);
		payInfo->result = submitResult->errorCode;
		sprintf(payInfo->transactionId, "%s", "");

		free(submitResult);
		free(paymentContext);
		fprintf(stderr, "\n");
		printf("%s\n", payInfo->note);
		return 0;
	}

	free(submitResult);
	free(paymentContext);
	fprintf(stderr, "\n");
	printf("%s\n", payInfo->note);
	return 1;
}


VisaDevice::VisaDevice()
{
}

VisaDevice::~VisaDevice()
{
}