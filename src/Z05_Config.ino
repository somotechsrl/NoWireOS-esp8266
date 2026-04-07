/*
   Configuration Subsystem
   Configuration is fully managed by Cloud and should be pushed at connection.
*/

static uint32_t configuration;
static const char  *bits[] = {"GPIO", "DHT","MODBUS", "OPENTHERM", "STATUS", "PULSE","XSERIAL",(const char *)NULL};

uint16_t configGetBit(const char *bitname) {
  for (int i = 0; bits[i]; i++)
    if (!strcmp(bitname, bits[i])) return configuration & (1 << i);
  return 0;
}

static int configGetBitPos(const char *bitname) {
  for (int i = 0; bits[i]; i++)
    if (!strcmp(bitname, bits[i])) return i;
  return -1;
}

void configEnableBit(const char *bitname) {
  int cbit = configGetBitPos(bitname);
  if (cbit < 0) return jsonErrorNoConfig();
  configuration |= (1 << cbit);
  configGetBitsStatus();
}

void configDisableBit(const char *bitname) {
  int cbit = configGetBitPos(bitname);
  if (cbit < 0) return jsonErrorNoConfig();;
  configuration &= ~(1 << cbit);
  configGetBitsStatus();
}

void configGetBitsStatus() {
  jsonAddArray("Bits");
  for(uint8_t i=0;i<16;i++) 
    if(configuration & (1 << i)) jsonAddValue(bits[i]);
  jsonClose();
} 

void configGetBitsList() {
  jsonAddArray("CFG.Bits.List");
  for (int i = 0; bits[i]; i++) jsonAddValue(bits[i]);
  jsonClose();
}

void configSetBitsStatus(uint32_t bitmask) {
  configuration =  bitmask;
  configGetBitsStatus();
}
