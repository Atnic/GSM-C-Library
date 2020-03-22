#ifndef URC_h
#define URC_h

#include <stddef.h>

#define FALSE 0
#define TRUE 1

struct CallReady
{
  unsigned char updated;
};

struct EnterPin
{
  unsigned char updated;
  char code[11];
};

struct HttpAction
{
  unsigned char updated;
  unsigned char method;
  unsigned int statusCode;
  unsigned int dataLength;
};

struct Psuttz
{
  unsigned char updated;
  unsigned int year;
  unsigned char month;
  unsigned char day;
  unsigned char hour;
  unsigned char minute;
  unsigned char second;
  unsigned char timezone;
  unsigned char dst;
};

struct NewMessageIndication
{
  unsigned char updated;
  char mem[3];
  unsigned char index;
};

struct Message
{
  unsigned char index;
  unsigned char status;
  char address[16];
  unsigned char typeOfAddress;
  char timestamp[21];
  unsigned char firstOctet;
  unsigned int pid;
  unsigned int dataCodingScheme;
  char serviceCenterAddress[16];
  unsigned char typeOfSeviceCenterAddress;
  unsigned char length;
  char data[163];
};

struct NewMessage
{
  unsigned char updated;
  struct Message *message;
  unsigned char waiting;
};

struct ServiceDataIndication
{
  unsigned char updated;
  unsigned char n;
  char str[200];
  unsigned char dcs;
};

typedef struct URC
{
  /** Get Local Timestamp Unsolicited Result Code */
  struct Psuttz psuttz;

  /** Call Ready Unsolicited Result Code */
  struct CallReady callReady;

  /** +CPIN Unsolicited Result Code */
  struct EnterPin enterPin;

  /** +HTTPACTION Unsolicited Result Code */
  struct HttpAction httpAction;

  /** +CMTI Unsolicited Result Code */
  struct NewMessageIndication newMessageIndication;

  /** +CMT Unsolicited Result Code */
  struct NewMessage newMessage;

  /** +CUSD Unsolicited Result Code */
  struct ServiceDataIndication serviceDataIndication;
} URC;

unsigned char URC_unsolicitedResultCode(URC *this, const char urcResponse[]);
void URC_resetUnsolicitedResultCode(URC *this);

extern URC Urc;

#endif
