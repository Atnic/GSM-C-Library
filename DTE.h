#ifndef DTE_h
#define DTE_h

#include <stddef.h>

#define NUM_ARGS_(_1, _2, _3, _4, _5, _6, _7, _8, TOTAL, ...) TOTAL
#define NUM_ARGS(...) NUM_ARGS_(__VA_ARGS__, 6, 5, 4, 3, 2, 1, 0)
#define CONCATE_(X, Y) X##Y
#define CONCATE(MACRO, NUMBER) CONCATE_(MACRO, NUMBER)
#define VA_MACRO(MACRO, ...) CONCATE(MACRO, NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define FALSE 0
#define TRUE 1

#define DEFAULT_TIMEOUT 500
#define DEFAULT_FLOW_CONTROL_DTE 0

struct FlowControl
{
  unsigned char dce;
  unsigned char dceOn;
  unsigned char dte;
  unsigned char dteOn;
};

typedef struct DTE
{
  int powerPin;
  unsigned char debug;
  char response[203];
  unsigned char echo;
  char productSerialNumberIdentification[17];
  struct FlowControl flowControl;
  long baudrate;
  unsigned char powerDown;

  unsigned long (*millis)(void);
  void (*debugPrint)(const char message[], unsigned char returnChar);
  unsigned char (*delay)(unsigned long value);
  unsigned char (*isListening)(void);
  unsigned char (*listen)(void);
  int (*available)(void);
  void (*flush)(void);
  int (*read)(void);
  size_t (*write)(const char str[]);
  size_t (*readBytes)(char buffer[], size_t length);
  void (*setPowerPin)(unsigned char state);
} DTE;

void DTE_init(DTE *this, int pinPower, unsigned char debug);

/**
 * Sent debugging message via Serial.
 * @param message    Debug message
 * @param returnChar true: add new line, false: no new line (default)
 */
void DTE_debugPrint(DTE *this, const char message[], unsigned char returnChar);
#define DTE_debugPrint_2(s, t, a, b) DTE_debugPrint(a, b, FALSE)
#define DTE_debugPrint_3(s, t, a, b, c) DTE_debugPrint(a, b, c)
#define DTE_debugPrint(...) VA_MACRO(DTE_debugPrint_, void, void, __VA_ARGS__)

/**
 * Command A/
 * @return  true: If command successful, false: Otherwise
 */
unsigned char DTE_atReIssueLastCommand(DTE *this);

/**
 * Command ATE
 * @param  echo true: To echo command, false: Otherwise
 * @return      true: If command successful, false: Otherwise
 */
unsigned char DTE_atSetCommandEchoMode(DTE *this, unsigned char echo);

/**
 * Command AT+IFC?
 * @return  true: If command successful, false: Otherwise
 */
unsigned char DTE_atSetLocalDataFlowControlQuery(DTE *this);

/**
 * Command AT+IFC=
 * @param  dce Method used by TE receiving from TA
 *             0: No Flow Control
 *             1: Software Flow Control
 *             2: Hardware FLow Control
 * @param  dte Method used by TA receiving from TE,
 *             same as dceByDte
 * @return     true: If command successful, false: Otherwise
 */
unsigned char DTE_atSetLocalDataFlowControl(DTE *this, unsigned char dce, unsigned char dte);
#define DTE_atSetLocalDataFlowControl_1(s, t, a) DTE_atSetLocalDataFlowControlQuery(a)
#define DTE_atSetLocalDataFlowControl_2(s, t, a, b) DTE_atSetLocalDataFlowControl(a, b, DEFAULT_FLOW_CONTROL_DTE)
#define DTE_atSetLocalDataFlowControl_3(s, t, a, b, c) DTE_atSetLocalDataFlowControl(a, b, c)
#define DTE_atSetLocalDataFlowControl(...) VA_MACRO(DTE_atSetLocalDataFlowControl_, void, void, __VA_ARGS__)

/**
 * Command AT+GSN
 * @return  true: If command successful, false: Otherwise
 */
unsigned char DTE_atRequestProductSerialNumberIdentification(DTE *this);

/**
 * Command AT+IPR?
 * @return  true: If command successful, false: Otherwise
 */
unsigned char DTE_atSetFixedLocalRateQuery(DTE *this);

/**
 * Command AT+IPR=
 * @param  baudrate Baudrate
 * @return          true: If command successful, false: Otherwise
 */
unsigned char DTE_atSetFixedLocalRate(DTE *this, long baudrate);
#define DTE_atSetFixedLocalRate_1(s, t, a) DTE_atSetFixedLocalRateQuery(a)
#define DTE_atSetFixedLocalRate_2(s, t, a, b) DTE_atSetFixedLocalRate(a, b)
#define DTE_atSetFixedLocalRate(...) VA_MACRO(DTE_atSetFixedLocalRate_, void, void, __VA_ARGS__)

/**
 * Check received buffer
 * @return  Total received char available
 */
int DTE_available(DTE *this);

/**
 * If using SoftwareSerial, this method is to check whether
 * the serial is listening (active)
 * @return  true: listening, false: otherwise
 */
unsigned char DTE_isListening(DTE *this);

/**
 * Listen (active) SoftwareSerial for this instance
 * @return  true: success, false: failed
 */
unsigned char DTE_listen(DTE *this);

/**
 * Flush TX Buffer
 */
void DTE_flush(DTE *this);

/**
 * Read char
 * @return     Total successfully sent char
 */
int DTE_read(DTE *this);

/**
 * Send string char array
 * @param  str String to be sent
 * @return     Total successfully sent char
 */
size_t DTE_write(DTE *this, const char str[]);

/**
 * Read bytes received from Serial buffer, and save it on buffer.
 * Warning: buffer size must be equal or smaller then length.
 * @param  buffer Buffer to be updated
 * @param  length Length of received byte that want to be get
 * @return        Total successfully read char
 */
size_t DTE_readBytes(DTE *this, char buffer[], size_t length);

/** Toggle SIM Power, On become Off, and otherwise */
void DTE_togglePower(DTE *this);

/** Clear received Serial buffer */
void DTE_clearReceivedBuffer(DTE *this);

/**
 * Command: AT
 * @return  true: if nothing is wrong, false: otherwise
 */
unsigned char DTE_AT(DTE *this);

/**
 * Send AT Command
 * @param  at Command
 * @return    true: if nothing is wrong, false: otherwise
 */
unsigned char DTE_ATCommand(DTE *this, const char at[]);

/**
 * Get AT Response, this function is block call until timeout.
 * If response is received then, get it until "\\r\\n" chars
 * @param   buffer      Buffer to store string from SIM Module
 * @param   bufferSize Specified buffer size
 * @param  timeout    Timeout in millis, default: 500
 * @return          true: Response received, false: Timeout is reached
 */
unsigned char DTE_ATResponse(DTE *this, char buffer[], size_t bufferSize, unsigned long timeout);
#define DTE_ATResponse_1(s, t, a) DTE_ATResponse(a, a->response, sizeof(a->response), DEFAULT_TIMEOUT)
#define DTE_ATResponse_2(s, t, a, b) DTE_ATResponse(a, a->response, sizeof(a->response), b)
#define DTE_ATResponse_3(s, t, a, b, c) DTE_ATResponse(a, b, c, DEFAULT_TIMEOUT)
#define DTE_ATResponse_4(s, t, a, b, c, d) DTE_ATResponse(a, b, c, d)
#define DTE_ATResponse(...) VA_MACRO(DTE_ATResponse_, void, void, __VA_ARGS__)

/**
 * Get AT Response, and check if response is equal with expected
 * @param  expected     Expected response
 * @param  timeout      Timeout in millis, default: 500
 * @return              true: If response as expected, false: Otherwise or timeout
 * @see   ATResponse()
 */
unsigned char DTE_ATResponseEqual(DTE *this, const char expected[], unsigned long timeout);
#define DTE_ATResponseEqual_2(s, t, a, b) DTE_ATResponseEqual(a, b, DEFAULT_TIMEOUT)
#define DTE_ATResponseEqual_3(s, t, a, b, c) DTE_ATResponseEqual(a, b, c)
#define DTE_ATResponseEqual(...) VA_MACRO(DTE_ATResponseEqual_, void, void, __VA_ARGS__)

/**
 * Get AT Response, and check if response is contain with expected
 * @param  expected     Expected response
 * @param  timeout      Timeout in millis, default: 500
 * @return              true: If response contain expected, false: Otherwise or timeout
 * @see   ATResponse()
 */
unsigned char DTE_ATResponseContain(DTE *this, const char expected[], unsigned long timeout);
#define DTE_ATResponseContain_2(s, t, a, b) DTE_ATResponseContain(a, b, DEFAULT_TIMEOUT)
#define DTE_ATResponseContain_3(s, t, a, b, c) DTE_ATResponseContain(a, b, c)
#define DTE_ATResponseContain(...) VA_MACRO(DTE_ATResponseContain_, void, void, __VA_ARGS__)

/**
 * Get AT Response, and check if response is "OK"
 * @param  expected     Expected response
 * @param  timeout      Timeout in millis, default: 500
 * @return              true: If response "OK", false: Otherwise or timeout
 * @see   ATResponse()
 */
unsigned char DTE_ATResponseOk(DTE *this, unsigned long timeout);
#define DTE_ATResponseOk_1(s, t, a) DTE_ATResponseOk(a, DEFAULT_TIMEOUT)
#define DTE_ATResponseOk_2(s, t, a, b) DTE_ATResponseOk(a, b)
#define DTE_ATResponseOk(...) VA_MACRO(DTE_ATResponseOk_, void, void, __VA_ARGS__)

/**
 * Check that last response is equal as expected
 * @param  expected Expected response
 * @return          true: If last response as expected, false: Otherwise or timeout
 */
unsigned char DTE_isResponseEqual(DTE *this, const char expected[]);

/**
 * Check that last response is contain expected
 * @param  expected Expected response
 * @return          true: If last response contain expected, false: Otherwise or timeout
 */
unsigned char DTE_isResponseContain(DTE *this, const char expected[]);

/**
 * Check that last response is "OK"
 * @param  expected Expected response
 * @return          true: If last response is "OK", false: Otherwise or timeout
 */
unsigned char DTE_isResponseOk(DTE *this);

/**
 * Unsolicited Result Code (URC) check, if it URC,
 * then update URC Object member value
 * @return     true: If it is URC, false: If it is not
 */
unsigned char DTE_unsolicitedResultCode(DTE *this);

/**
 * Get last response
 * @return  Response
 */
const char *DTE_getResponse(DTE *this);

/**
 * Is Echo is enable
 * @return  true: If enable, false: Otherwise
 */
unsigned char DTE_isEcho(DTE *this);

/**
 * Set Echo
 * @param  echo true: Enable, false: Disable
 * @return      true: If success, false: Otherwise
 */
unsigned char DTE_setEcho(DTE *this, unsigned char echo);

/**
 * Get Product Serial Number Identification (IMEI)
 * @return  IMEI string
 */
const char *DTE_getProductSerialNumberIdentification(DTE *this);

/**
 * Get Flow Control
 * @return  FlowControl Struct
 */
struct FlowControl DTE_getFlowControl(DTE *this);

/**
 * Set Flow Control
 * @param  dce DCE by DTE
 * @param  dte DTE by DCE
 * @return     true: If command successful, false: Otherwise
 */
unsigned char DTE_setFlowControl(DTE *this, unsigned char dce, unsigned char dte);
#define DTE_setFlowControl_2(s, t, a, b) DTE_setFlowControl(a, b, DEFAULT_FLOW_CONTROL_DTE)
#define DTE_setFlowControl_3(s, t, a, b, c) DTE_setFlowControl(a, b, c)
#define DTE_setFlowControl(...) VA_MACRO(DTE_setFlowControl_, void, void, __VA_ARGS__)

/**
 * Set Flow Control Status on DCE
 * @param  on true: Send XON, false: Send XOFF
 * @return    true: If command successful, false: Otherwise
 */
unsigned char DTE_setFlowControlStatusDce(DTE *this, unsigned char on);

/**
 * Get current baudrate
 * @return  Baudrate
 */
long DTE_getBaudrate(DTE *this);

/**
 * Is SIM Module power is down
 * @return  true: If power is down, false: Otherwise
 */
unsigned char DTE_isPowerDown(DTE *this);

/**
 * Reset Power SIM Module
 * @return  true
 */
unsigned char DTE_powerReset(DTE *this);

#endif
