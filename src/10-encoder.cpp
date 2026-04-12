#include "main.h"
#include "10-encoder.h"
#include "rBase64.h"

// Json formatting utilities
#define TAG "JSON"
#define JLEVELS 10
static char r[BUFSIZE], *rp;
static char s[BUFSIZE];
static char jclose[JLEVELS];
uint16_t comma[JLEVELS], level;

// Parallel binary structure. Emulates json
// and compresses data. keep only brackets but removes
// all info names (only values)
// every info is prefixed by a char wich defines type
// Receiver MUST know the structure sent....
// Implements a very simple packet compressione method, based on values degrading
// uint32_t => uint16_t => uint8_t where value is zero 
/*

   a: array key\0
   o: obiect key\0
   e: object or array end
   s: string key\0value\0
   b: unsigned byte key\0value
   w: unsigned word key\0value
   l: unsigned long key\0value
   B: signed byte key\0value
   W: signed word key\0value
   L: signed long key\0value 
   f: float
   d: double
   z: zero-value (no trailing data)
   n: zero string (no trailing data)  

   Finally, buffer is xor-encryptesd with special key

   example: "{u0x0000b0x00i0x0000{u0x000}}"

*/

// Encryption Xor key
// decoder must use it
#define XKEY 0xf1

void jsonInit() {

  // clears all buffers
  rp=r;
  memset(s, 0, sizeof(s));
  memset(r, 0, sizeof(r));
  memset(comma, 0, sizeof(comma));
  memset(jclose, 0, sizeof(jclose));

  // opens Json
  level=1;
  *rp++='{';
  strcpy(s, "{");

}

// print comma for level (autoicrement)
static const char *jsonComma() {
  return comma[level]++ ? "," : "";
}

// gets jsonBuffer
const char *jsonGetBuffer() {
  return s;
}
// gets jsonBuffer
uint16_t jsonGetBufferSize() {
  return strlen(s);
}

static char eb[BUFSIZE];
static char b64[BUFSIZE];
const char *jsonGetBase64() {

  int bsize=rp-r;

  // cleanup
  memset(eb,0,sizeof(eb));
  memset(b64,0,sizeof(b64));

  // simple encrypt before send and then 64 encode
  for(int i=0;i<bsize;i++) eb[i]=r[i]^XKEY;
  rbase64_encode(b64, eb, bsize);
  
  return (char *)b64;
}

// binary buffer add function
static void bpAddValue(const char vtype, const void *vvalue, const uint8_t vsize) {

  *rp++ = vtype;
  // checks special types (z=zero value, n= empty string)
  // in this way we can save some bytes in xmission
  if (vsize && vtype!='z' && vtype!='n') {    
    memcpy(rp, vvalue, vsize);
    rp += vsize;
  }
}

static void jsonOpen(const char *oname, char bracket, char cbracket) {
  *rp++ = bracket;
  sprintf(s + strlen(s), "%s\"%s\":%c", jsonComma(), oname, bracket);
  jclose[++level] = cbracket;
  comma[level] = 0;
}

void jsonClose() {
  if (!level) return;
  *rp++ = jclose[level];
  sprintf(s + strlen(s), "%c", jclose[level--]);
}

void jsonCloseAll() {
  if(!level) return;
  while (level > 0) jsonClose();
  *rp++='}';
  strcat(s, "}");
}

// adds a integer object
void jsonAddValue(int8_t value) {
  bpAddValue(value==0 ? 'z' : 'B', &value, sizeof(value));
  sprintf(s + strlen(s), "%s%d", jsonComma(), value);
}
// adds an unsigned integer object
void jsonAddValue(int16_t value) {
  bpAddValue(value==0 ? 'z' : 'W', &value, sizeof(value));
  sprintf(s + strlen(s), "%s%d", jsonComma(), value);
}// adds an unsigned integer object
void jsonAddValue(int32_t value) {
  bpAddValue(value==0 ? 'z' : 'L', &value, sizeof(value));
  sprintf(s + strlen(s), "%s%d", jsonComma(), value);
}
// adds a char (char)
void jsonAddValue(char value) {
  bpAddValue(value==0 ? 'z' : 'b', &value, sizeof(value));
  sprintf(s + strlen(s), "%s%c", jsonComma(), value);
}
// adds a uiigned short
void jsonAddValue(uint8_t value) {
  bpAddValue(value==0 ? 'z' : 'b', &value, sizeof(value));
  sprintf(s + strlen(s), "%s%u", jsonComma(), value);
}
// adds a unsigned
void jsonAddValue(uint16_t value) {
  //if(value<=USHRT_MAX) return jsonAddValue((uint8_t)value);
  bpAddValue(value==0 ? 'z' : 'w', &value, sizeof(value));
  sprintf(s + strlen(s), "%s%u", jsonComma(), value);
}
// adds a unsigned long
void jsonAddValue(uint32_t value) {
  //if(value<=UINT_MAX) return jsonAddValue((uint16_t)value);
  bpAddValue(value==0 ? 'z' : 'L', &value, sizeof(value));
  sprintf(s + strlen(s), "%s%u", jsonComma(), value);
}
// adds a float
void jsonAddValue(float value) {
  bpAddValue(value==0 ? 'z' : 'f', &value, sizeof(value));
  sprintf(s + strlen(s), "%s%f", jsonComma(), value);
}
// adds a double
void jsonAddValue(double value) {
  bpAddValue(value==0 ? 'z' : 'f', &value, sizeof(value));
  sprintf(s + strlen(s), "%s%f", jsonComma(), value);
}
// adds a string with formatter (or only a string)
void jsonAddValue(const char *format, ...) {
 
  char out[BUFTINY];
  va_list args;
  va_start(args, format);
  vsprintf(out, format, args);
  va_end(args);
 
  sprintf(s + strlen(s),"%s\"%s\"", jsonComma(),out);
  bpAddValue('s', out, strlen(out) + 1);
}

// adds an array header
void jsonAddArray(const char *oname) {
  bpAddValue('a', oname, strlen(oname) + 1);
  jsonOpen(oname, '[', ']');
}

// adds an object header
void jsonAddObject(const char *oname) {
  bpAddValue('o', oname, strlen(oname) + 1);
  jsonOpen(oname, '{', '}');
}

// adds a string data object
void jsonAddObject(const char *oname, const char *format, ...) {

  char out[BUFTINY];
  va_list args;
  va_start(args, format);
  vsprintf(out, format, args);
  va_end(args);

  bpAddValue('o', oname, strlen(oname) + 1);
  bpAddValue('s', out, strlen(out) + 1);
  sprintf(s + strlen(s), "%s\"%s\":\"%s\"", jsonComma(), oname, out);

}

// adds an unsigned short object
void jsonAddObject(const char *oname, uint8_t value) {
  bpAddValue('o', oname, strlen(oname) + 1);
  bpAddValue('b', &value, sizeof(value));
  sprintf(s + strlen(s), "%s\"%s\":%u", jsonComma(), oname, value);
}

// adds a bool object
void jsonAddObject(const char *oname, bool value) {
  bpAddValue('o', oname, strlen(oname) + 1);
  bpAddValue('b', &value, sizeof(value));
  sprintf(s + strlen(s), "%s\"%s\":%u", jsonComma(), oname, value);
}

// adds a unsigned object
void jsonAddObject(const char *oname, uint16_t value) {
  bpAddValue('o', oname, strlen(oname) + 1);
  bpAddValue('w', &value, sizeof(value));
  sprintf(s + strlen(s), "%s\"%s\":%u", jsonComma(), oname, value);
}

// adds an unsigned long object
void jsonAddObject(const char *oname, uint32_t value) {
  bpAddValue('o', oname, strlen(oname) + 1);
  bpAddValue('l', &value, sizeof(value));
  sprintf(s + strlen(s), "%s\"%s\":%u", jsonComma(), oname, value);
}

// adds a float
void jsonAddObject(const char *oname, float value) {
  bpAddValue('o', oname, strlen(oname) + 1);
  bpAddValue('f', &value, sizeof(value));
  sprintf(s + strlen(s), "%s\"%s\":%f", jsonComma(), oname, value);
}

// adds a double
void jsonAddObject(const char *oname, double value) {
  bpAddValue('o', oname, strlen(oname) + 1);
  bpAddValue('d', &value, sizeof(value));
  sprintf(s + strlen(s), "%s\"%s\":%f", jsonComma(), oname, value);
}