#include <stdlib.h>
#include <string.h>
#include "URC.h"

URC Urc;

void URC_init(URC *this)
{
  this->newMessage.message = NULL;
}

unsigned char URC_unsolicitedResultCode(URC *this, const char urcResponse[])
{
  const char *urcCallReady = "Call Ready";
  const char *urcHttpAction = "+HTTPACTION:";
  const char *urcEnterPin = "+CPIN: ";
  const char *urcNewMessageIndication = "+CMTI: ";
  const char *urcNewMessage = "+CMT: ";
  const char *urcServiceDataIndication = "+CUSD: ";
  const char *urcGetLocalTimestamp = "*PSUTTZ: ";

  char *pointer;
  char urc[strlen(urcResponse) + 1];

  strcpy(urc, urcResponse);
  if ((pointer = strstr(urc, (const char *)urcCallReady)) != NULL)
  {
    this->callReady.updated = TRUE;
    return TRUE;
  }
  else if ((pointer = strstr(urc, (const char *)urcHttpAction)) != NULL)
  {
    pointer += strlen((const char *)urcHttpAction);
    char *str = strtok_r(pointer, ",", &pointer);
    unsigned char i = 0;
    for (i = 0; i < 3 && str != NULL; i++)
    {
      if (i == 0)
        this->httpAction.method = str[0] - '0';
      if (i == 1)
        this->httpAction.statusCode = atoi(str);
      if (i == 2)
        this->httpAction.dataLength = atoi(str);
      str = strtok_r(pointer, ",", &pointer);
    }
    if (i >= 3)
      this->httpAction.updated = TRUE;
    return TRUE;
  }
  else if ((pointer = strstr(urc, (const char *)urcEnterPin)) != NULL)
  {
    pointer += strlen((const char *)urcEnterPin);
    char *str = strtok_r(pointer, "\"", &pointer);
    strcpy(this->enterPin.code, str);
    this->enterPin.updated = TRUE;
    return TRUE;
  }
  else if ((pointer = strstr(urc, (const char *)urcGetLocalTimestamp)) != NULL)
  {
    pointer += strlen((const char *)urcGetLocalTimestamp);
    char *str = strtok_r(pointer, ",\" +", &pointer);
    for (unsigned char i = 0; i < 8 && str != NULL; i++)
    {
      if (i == 0)
        this->psuttz.year = atoi(str);
      if (i == 1)
        this->psuttz.month = atoi(str);
      if (i == 2)
        this->psuttz.day = atoi(str);
      if (i == 3)
        this->psuttz.hour = atoi(str);
      if (i == 4)
        this->psuttz.minute = atoi(str);
      if (i == 5)
        this->psuttz.second = atoi(str);
      if (i == 6)
        this->psuttz.timezone = atoi(str) / 4;
      str = strtok_r(pointer, ",\" +", &pointer);
    }
    this->psuttz.updated = TRUE;
    return TRUE;
  }
  else if ((pointer = strstr(urc, (const char *)urcNewMessageIndication)) != NULL)
  {
    pointer += strlen((const char *)urcNewMessageIndication);
    char *str = strtok_r(pointer, "\",", &pointer);
    strcpy(this->newMessageIndication.mem, str);
    str = strtok_r(pointer, "\",", &pointer);
    this->newMessageIndication.index = atoi(str);
    this->newMessageIndication.updated = TRUE;
    return TRUE;
  }
  else if ((pointer = strstr(urc, (const char *)urcNewMessage)) != NULL)
  {
    if (this->newMessage.message == NULL)
      return FALSE;
    strcpy(this->newMessage.message->data, urc);
    this->newMessage.waiting = TRUE;
    return TRUE;
  }
  else if ((pointer = strstr(urc, (const char *)urcServiceDataIndication)) != NULL)
  {
    pointer += strlen((const char *)urcServiceDataIndication);
    char *str = strtok_r(pointer, ",", &pointer);
    this->serviceDataIndication.n = atoi(str);
    str = strtok_r(pointer, "\"", &pointer);
    strcpy(this->serviceDataIndication.str, str);
    str = strtok_r(pointer, "\",", &pointer);
    this->serviceDataIndication.dcs = atoi(str);
    this->serviceDataIndication.updated = TRUE;
    return TRUE;
  }
  else if (this->newMessage.waiting)
  {
    if (this->newMessage.message == NULL)
      return FALSE;
    pointer = strstr(this->newMessage.message->data, (const char *)urcNewMessage) + strlen((const char *)urcNewMessage);
    char *str = strtok_r(pointer, ",", &pointer);
    for (size_t i = 0; str != NULL; i++)
    {
      if (i == 0)
      {
        if (str[0] == '\"')
        {
          strncpy(this->newMessage.message->address, str + 1, strlen(str + 1) - 1);
          this->newMessage.message->address[strlen(str + 1) - 1] = '\0';
        }
        else
          this->newMessage.message->firstOctet = atoi(str);
      }
      if ((this->newMessage.message->firstOctet & 0x03) == 0x00 ||
          (this->newMessage.message->firstOctet & 0x03) == 0x02)
      {
        if (i == 2)
        {
          if (strlen(str) == 20)
          {
            strcpy(this->newMessage.message->timestamp, str);
            str = strtok_r(pointer, ",", &pointer);
          }
          this->newMessage.message->typeOfAddress = atoi(str);
        }
        if (i == 3)
          this->newMessage.message->firstOctet = atoi(str);
        if (i == 4)
          this->newMessage.message->pid = atoi(str);
        if (i == 5)
          this->newMessage.message->dataCodingScheme = atoi(str);
        if (i == 6)
        {
          if ((this->newMessage.message->firstOctet & 0x03) == 0x02)
            str = strtok_r(pointer, ",", &pointer);
          strncpy(this->newMessage.message->serviceCenterAddress, str + 1, strlen(str + 1) - 1);
          this->newMessage.message->serviceCenterAddress[strlen(str + 1) - 1] = '\0';
        }
        if (i == 7)
          this->newMessage.message->typeOfSeviceCenterAddress = atoi(str);
        if (i == 8)
          this->newMessage.message->length = atoi(str);
        if (i == 1)
          str = strtok_r(pointer, "\"", &pointer);
        else
          str = strtok_r(pointer, ",", &pointer);
      }
      else
      {
        if (i == 1)
        {
          // if(strlen(this->newMessage.message->address) == 0) this->newMessage.message->mr = atoi(str);
        }
        str = strtok_r(pointer, ",", &pointer);
      }
    }
    strcpy(this->newMessage.message->data, urc);
    this->newMessage.waiting = FALSE;
    this->newMessage.updated = TRUE;
  }
  return FALSE;
}

void URC_resetUnsolicitedResultCode(URC *this)
{
  this->callReady.updated = FALSE;
  this->enterPin.updated = FALSE;
  this->httpAction.updated = FALSE;
  this->psuttz.updated = FALSE;
  this->newMessageIndication.updated = FALSE;
  this->newMessage.updated = FALSE;
  this->serviceDataIndication.updated = FALSE;
}
