#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "DTE.h"
#include "URC.h"

void __attribute__((weak)) DTE_debugPrint(DTE *this, const char message[], unsigned char returnChar)
{
  return this->debugPrint(message, returnChar);
}

unsigned long __attribute__((weak)) DTE_millis(DTE *this)
{
  return this->millis();
}

unsigned char __attribute__((weak)) DTE_delay(DTE *this, unsigned long value)
{
  return this->delay(value);
}

unsigned char __attribute__((weak)) DTE_isListening(DTE *this)
{
  return this->isListening();
}

unsigned char __attribute__((weak)) DTE_listen(DTE *this)
{
  return this->listen();
}

int __attribute__((weak)) DTE_available(DTE *this)
{
  return this->available();
}

void __attribute__((weak)) DTE_flush(DTE *this)
{
  return this->flush();
}

int __attribute__((weak)) DTE_read(DTE *this)
{
  return this->read();
}

size_t __attribute__((weak)) DTE_write(DTE *this, const char str[])
{
  return this->write(str);
}

size_t __attribute__((weak)) DTE_readBytes(DTE *this, char buffer[], size_t length)
{
  return this->readBytes(buffer, length);
}

void __attribute__((weak)) DTE_setPowerPin(DTE *this, unsigned char state)
{
  return this->setPowerPin(state);
}

/* DTE */
void DTE_init(DTE *this, int powerPin, unsigned char debug)
{
  this->powerPin = powerPin;
  this->debug = debug;
  this->response[0] = '\0';
  this->echo = TRUE;
  this->flowControl = (struct FlowControl){0, TRUE, 0, TRUE};
  this->baudrate = -1;
  this->powerDown = TRUE;

  DTE_setPowerPin(this, 1);
}

unsigned char DTE_atReIssueLastCommand(DTE *this)
{
  const char *command = "A/\r";

  DTE_clearReceivedBuffer(this);
  if (!DTE_ATCommand(this, command))
    return FALSE;
  if (!DTE_ATResponseOk(this))
    return FALSE;
  return TRUE;
}

unsigned char DTE_atSetCommandEchoMode(DTE *this, unsigned char echo)
{
  const char *command = "ATE%d&W\r";
  char buffer[8]; // "ATEX&W\r"

  sprintf(buffer, (const char *)command, echo ? 1 : 0);

  DTE_clearReceivedBuffer(this);
  if (!DTE_ATCommand(this, buffer))
    return FALSE;
  if (!DTE_ATResponseOk(this))
    return FALSE;
  this->echo = echo;
  return TRUE;
}

unsigned char DTE_atRequestProductSerialNumberIdentification(DTE *this)
{
  const char *command = "AT+GSN\r";
  char productSerialNumberIdentification[17];

  DTE_clearReceivedBuffer(this);
  if (!DTE_ATCommand(this, command))
    return FALSE;
  if (!DTE_ATResponse(this))
    return FALSE;
  while (!DTE_isResponseOk(this) && !isdigit(*DTE_getResponse(this)))
  {
    if (!DTE_ATResponse(this))
      return FALSE;
  }
  if (!DTE_isResponseOk(this))
  {
    strcpy(productSerialNumberIdentification, DTE_getResponse(this));
  }
  if (!DTE_ATResponseOk(this))
    return FALSE;
  strcpy(this->productSerialNumberIdentification, productSerialNumberIdentification);
  return TRUE;
}

unsigned char DTE_atSetLocalDataFlowControlQuery(DTE *this)
{
  const char *command = "AT+IFC?\r";
  const char *response = "+IFC: ";
  struct FlowControl flowControl;

  flowControl = this->flowControl;
  DTE_clearReceivedBuffer(this);
  if (!DTE_ATCommand(this, command))
    return FALSE;
  if (!DTE_ATResponseContain(this, response))
    return FALSE;
  char *pointer = strstr(DTE_getResponse(this), (const char *)response) + strlen((const char *)response);
  char *str = strtok(pointer, ",");
  for (size_t i = 0; i < 2 && str != NULL; i++)
  {
    if (i == 0)
      flowControl.dce = str[0] - '0';
    if (i == 1)
      flowControl.dte = str[0] - '0';
    str = strtok(NULL, ",");
  }
  if (!DTE_ATResponseOk(this))
    return FALSE;
  this->flowControl = flowControl;
  return TRUE;
}

unsigned char DTE_atSetLocalDataFlowControl(DTE *this, unsigned char dce, unsigned char dte)
{
  char buffer[16]; // "AT+IFC=X,X;&W\r"

  if (dte == 1)
    sprintf(buffer, (const char *)"AT+IFC=%d,%d;&W\r", dce, dte);
  else
    sprintf(buffer, (const char *)"AT+IFC=%d;&W\r", dce);

  DTE_clearReceivedBuffer(this);
  if (!DTE_ATCommand(this, buffer))
    return FALSE;
  if (!DTE_ATResponseOk(this))
    return FALSE;
  this->flowControl.dce = dce;
  this->flowControl.dte = dte;
  return TRUE;
}

unsigned char DTE_atSetFixedLocalRateQuery(DTE *this)
{
  const char *command = "AT+IPR?\r";
  const char *response = "+IPR: ";
  long baudrate = 0;

  DTE_clearReceivedBuffer(this);
  if (!DTE_ATCommand(this, command))
    return FALSE;
  if (!DTE_ATResponseContain(this, response))
    return FALSE;
  char *str = strstr(DTE_getResponse(this), (const char *)response) + strlen((const char *)response);
  baudrate = atoi(str);
  if (!DTE_ATResponseOk(this))
    return FALSE;
  this->baudrate = baudrate;
  return TRUE;
}

unsigned char DTE_atSetFixedLocalRate(DTE *this, long baudrate)
{
  const char *command = "AT+IPR=%ld\r";
  char buffer[15]; // "AT+IPR=XXXXXX\r"

  sprintf(buffer, (const char *)command, baudrate);

  DTE_clearReceivedBuffer(this);
  if (!DTE_ATCommand(this, buffer))
    return FALSE;
  if (!DTE_ATResponseOk(this))
    return FALSE;
  this->baudrate = baudrate;
  return TRUE;
}

void DTE_togglePower(DTE *this)
{
  DTE_debugPrint(this, "Toggle Power", TRUE);

  DTE_setPowerPin(this, 1);
  DTE_delay(this, 1200);
  DTE_setPowerPin(this, 0);
  this->powerDown = FALSE;
  this->flowControl = (struct FlowControl){0, TRUE, 0, TRUE};
  while (DTE_ATResponse(this, 3000))
  {
    if (DTE_isResponseEqual(this, "RDY"))
    {
      this->powerDown = FALSE;
      URC_resetUnsolicitedResultCode(&Urc);
      if (DTE_AT(this))
        return;
    }
    if (DTE_isResponseEqual(this, "NORMAL POWER DOWN"))
    {
      this->powerDown = TRUE;
      DTE_delay(this, 1000);
      return;
    }
    else
    {
      DTE_unsolicitedResultCode(this);
    }
  }
  if (DTE_AT(this))
  {
    this->powerDown = FALSE;
    URC_resetUnsolicitedResultCode(&Urc);
    if (DTE_isEcho(this))
      DTE_setEcho(this, FALSE);
    if (DTE_getFlowControl(this).dce == 0)
      DTE_setFlowControl(this, 1);
    DTE_setFlowControlStatusDce(this, FALSE);
    return;
  }
  else
  {
    this->powerDown = TRUE;
    DTE_delay(this, 1000);
    return;
  }
}

void DTE_clearReceivedBuffer(DTE *this)
{
  DTE_debugPrint(this, "DTE_clearReceivedBuffer", TRUE);

  if (!DTE_isListening(this))
    DTE_listen(this);
  DTE_setFlowControlStatusDce(this, TRUE);
  unsigned long t = DTE_millis(this);

  while (DTE_available(this) == 0 && DTE_millis(this) - t < 50)
    DTE_delay(this, 1);
  if (DTE_available(this) == 0)
  {
    DTE_setFlowControlStatusDce(this, FALSE);
    return;
  }

  while (DTE_ATResponse(this, 50))
  {
    if (DTE_isResponseEqual(this, "RDY"))
      this->echo = TRUE;
    else
      DTE_unsolicitedResultCode(this);
  }
  DTE_setFlowControlStatusDce(this, FALSE);
}

unsigned char DTE_AT(DTE *this)
{
  const char *command = "AT\r";

  this->powerDown = FALSE;
  this->echo = FALSE;
  DTE_ATCommand(this, command);
  while (TRUE)
  {
    if (!DTE_ATResponse(this))
    {
      this->powerDown = TRUE;
      break;
    }
    if (DTE_isResponseEqual(this, command))
      this->echo = TRUE;
    else if (DTE_isResponseOk(this))
      return TRUE;
    else
      DTE_unsolicitedResultCode(this);
  }
  this->powerDown = TRUE;
  return FALSE;
}

unsigned char DTE_ATCommand(DTE *this, const char at[])
{
  if (this->powerDown)
  {
    DTE_debugPrint(this, "Power Down", TRUE);
    return FALSE;
  }

  DTE_debugPrint(this, "Command: ");
  DTE_debugPrint(this, at, TRUE);

  if (!DTE_isListening(this))
    DTE_listen(this);
  DTE_setFlowControlStatusDce(this, FALSE);
  DTE_write(this, at);
  if (this->echo)
  {
    if (strlen(at) > (sizeof(this->response) - 2))
    {
      char atEcho[strlen(at) + 3];
      while (TRUE)
      {
        if (!DTE_ATResponse(this, atEcho, sizeof(atEcho)))
          return FALSE;
        if (strstr(atEcho, "ERROR") != NULL)
          return FALSE;
        if (strcmp(atEcho, at) == 0)
          break;
        else
          URC_unsolicitedResultCode(&Urc, atEcho);
      }
    }
    else if (!DTE_ATResponseEqual(this, at))
      return FALSE;
  }
  return TRUE;
}

unsigned char DTE_ATResponse(DTE *this, char buffer[], size_t bufferSize, unsigned long timeout)
{
  if (this->powerDown)
  {
    DTE_debugPrint(this, "Power Down", TRUE);
    return FALSE;
  }

  DTE_setFlowControlStatusDce(this, TRUE);

  unsigned long t = DTE_millis(this);
  while (TRUE)
  {
    if (DTE_available(this) > 0)
      break;
    if (DTE_millis(this) - t > timeout)
    {
      DTE_debugPrint(this, "No response", TRUE);
      return FALSE;
    }
    DTE_delay(this, 1);
  }

  t = DTE_millis(this);
  unsigned int i = 0;
  while (TRUE)
  {
    while (DTE_available(this) > 0)
    {
      buffer[i++] = DTE_read(this);
      t = DTE_millis(this);
      if (i >= 2)
      {
        if (buffer[i - 2] == '\r' && buffer[i - 1] == '\n')
          break;
      }
    }

    if (i >= 2)
    {
      if (buffer[i - 2] == '\r' && buffer[i - 1] == '\n')
      {
        buffer[i - 2] = '\0';
        if (strlen(buffer) > 0)
          break;
        else
          i = 0;
      }
    }
    if (DTE_millis(this) - t > 50 || i >= (bufferSize - 1))
    {
      buffer[i] = '\0';
      break;
    }
  }
  DTE_debugPrint(this, "Response: ", TRUE);
  DTE_debugPrint(this, buffer, TRUE);
  return TRUE;
}

unsigned char DTE_ATResponseEqual(DTE *this, const char expected[], unsigned long timeout)
{
  while (TRUE)
  {
    if (!DTE_ATResponse(this, timeout))
      return FALSE;
    if (DTE_isResponseContain(this, "ERROR"))
      return FALSE;
    if (DTE_isResponseEqual(this, expected))
      break;
    if (DTE_isResponseEqual(this, "RDY"))
      this->echo = TRUE;
    else
      DTE_unsolicitedResultCode(this);
  }
  return TRUE;
}

unsigned char DTE_ATResponseContain(DTE *this, const char expected[], unsigned long timeout)
{
  while (TRUE)
  {
    if (!DTE_ATResponse(this, timeout))
      return FALSE;
    if (DTE_isResponseContain(this, "ERROR"))
      return FALSE;
    if (DTE_isResponseContain(this, expected))
      break;
    if (DTE_isResponseEqual(this, "RDY"))
      this->echo = TRUE;
    else
      DTE_unsolicitedResultCode(this);
  }
  return TRUE;
}

unsigned char DTE_ATResponseOk(DTE *this, unsigned long timeout)
{
  DTE_ATResponse(this, timeout);
  return DTE_isResponseOk(this);
}

unsigned char DTE_isResponseEqual(DTE *this, const char expected[])
{
  if (strcmp(this->response, expected) != 0)
    return FALSE;
  return TRUE;
}

unsigned char DTE_isResponseContain(DTE *this, const char expected[])
{
  if (strstr(this->response, expected) == NULL)
    return FALSE;
  return TRUE;
}

unsigned char DTE_isResponseOk(DTE *this)
{
  if (!(DTE_isResponseEqual(this, "OK")) || DTE_isResponseEqual(this, "OK\r"))
    return FALSE;
  if (DTE_available(this))
    DTE_clearReceivedBuffer(this);
  DTE_setFlowControlStatusDce(this, FALSE);
  return TRUE;
}

unsigned char DTE_unsolicitedResultCode(DTE *this)
{
  DTE_debugPrint(this, "URC", TRUE);
  return URC_unsolicitedResultCode(&Urc, this->response);
}

unsigned char DTE_setEcho(DTE *this, unsigned char echo)
{
  if (this->echo == echo)
    return TRUE;
  if (!DTE_atSetCommandEchoMode(this, echo))
    return FALSE;
  return TRUE;
}

const char *DTE_getProductSerialNumberIdentification(DTE *this)
{
  if (strlen(this->productSerialNumberIdentification) == 0)
    DTE_atRequestProductSerialNumberIdentification(this);
  return this->productSerialNumberIdentification;
}

struct FlowControl DTE_getFlowControl(DTE *this)
{
  if (!DTE_atSetLocalDataFlowControl(this))
    return (struct FlowControl){0, TRUE, 0, TRUE};
  return this->flowControl;
}

unsigned char DTE_setFlowControl(DTE *this, unsigned char dce, unsigned char dte)
{
  if (!DTE_atSetLocalDataFlowControl(this, dce, dte))
    return FALSE;
  if (this->flowControl.dce == 1)
    DTE_setFlowControlStatusDce(this, FALSE);
  return TRUE;
}

unsigned char DTE_setFlowControlStatusDce(DTE *this, unsigned char on)
{
  if (this->flowControl.dce != 1)
    return FALSE;
  if (this->flowControl.dceOn == on)
    return TRUE;
  if (on)
    DTE_write(this, "\17");
  else
    DTE_write(this, "\19");
  this->flowControl.dceOn = on;
  return TRUE;
}

long DTE_getBaudrate(DTE *this)
{
  if (!DTE_atSetLocalDataFlowControl(this))
    return 0;
  return this->baudrate;
}

unsigned char DTE_powerReset(DTE *this)
{
  DTE_togglePower(this);
  while (this->powerDown)
  {
    DTE_togglePower(this);
  }
  return TRUE;
}
