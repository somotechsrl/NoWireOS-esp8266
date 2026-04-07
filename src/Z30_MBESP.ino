#ifdef __MODBUS_TCP__

// Modbus ESP Includes
#include <ModbusTCP.h>


// Single block query modbus -- returns iobu compile as hex string
void getModbusEMU(const char *tag, uint8_t se, const char *ad, uint8_t fn, uint16_t rs, uint16_t rn, uint8_t retries = 2) {

  uint8_t qresult = 0;

  char rseqn[10];
  sprintf(rseqn,"r%04x",rs);
  jsonAddArray(rseqn);
  jsonAddValue(fn);
  jsonAddValue(qresult);
  jsonAddValue(rs);

  // gets emulated values
  for (int i = 0; i < rn; i++) {
      jsonAddValue((uint16_t)random(65535L));
      //jsonAddValue((uint16_t)0);
    }
    // closes dataset
    jsonClose();
    return;
  }


// Single block query modbus -- returns iobu compile as hex string
static ModbusTCP mb;

void getModbusTCP(const char *tag, uint8_t se, const char *ad, uint8_t fn, uint16_t rs, uint16_t rn, uint8_t retries = 2) {

  // Sets modbus parmeters for RTU
  //memset(&mb,0,sizeof(mb));
  int8_t qresult = 0;
  String regseq;


  // Work variables
  uint16_t mbuffer[rn];
  memset(mbuffer, 0, sizeof(mbuffer));

  // Sets modbus parmeters for TCP. f already connected ok
  IPAddress remote(192, 168, 43, 169);  // Address of Modbus Slave device
  if(mb.isConnected(remote)) {
    mb.connect(remote);
    if(mb.isConnected(remote)) {
      debug(DBG_MODBUS, String("Not Connected: ") + ad + ":" + fn + ":" + rs + ":" + rn);
      jsonClose();
      return;
      }
    }
 
  // Simulates 'standard' key
  debug(DBG_MODBUS, String("Querying: ") + ad + ":" + fn + ":" + rs + ":" + rn);

  for (uint8_t r = 0;r < retries; r++) {

    delay(200);

    // Dsipatches function
    switch (fn) {
      case 1:
        qresult=mb.readCoil(remote, rs, (bool *)mbuffer, rn, NULL);
        break;
      case 2:
        //qresult=mb->read(remote, rs, (uint16_t *)mbuffer, rn, NULL);
        break;
      case 3:
        qresult=mb.readHreg(remote, rs, (uint16_t *)mbuffer, rn, NULL);
        break;
      case 4:
        qresult=mb.readIreg(remote, rs, (uint16_t *)mbuffer, rn, NULL);
        break;
      case 5:
        // forces r=regn so we exit loop...
        //qresult = rtunode.writeSingleCoil(rs, rn);
        break;
      case 6:
        // forces r=regn so we exit loop...
        //qresult = rtunode.writeSingleRegister(rs, rn);
        break;
      default:
        qresult = -1;
    }

    mb.task();
    delay(500);

    break;
  }

  // array 'tag' is sequence,address,function,register,value,value,value...  
  debug(DBG_MODBUS, String("Finish: ") + ad + ":" + fn + ":" + rs + ":" + rn);

  char rseqn[10];
  sprintf(rseqn,"r%04x",rs);
  jsonAddArray(rseqn);
  jsonAddValue(fn);
  jsonAddValue(qresult);
  jsonAddValue(rs);

  // gets registers or emulated values
  for (int i = 0; i < rn; i++) {
    jsonAddValue((uint16_t)mbuffer[i]);
  }

  // array 'tag' is sequence,address,function,register,value,value,value...  
  debug(DBG_MODBUS, String("Finish: ") + rs + ":" + regseq);

  // closes dataset
  jsonClose();

}

#endif
