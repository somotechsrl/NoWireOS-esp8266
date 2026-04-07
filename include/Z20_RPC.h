#ifndef __PROTO_Z20_RPC_INO__
#define __PROTO_Z20_RPC_INO__
//Extracted Prototyes
// ****************************
// src/Z20_RPC.ino prototypes
// ****************************
void rpcWrongParams(String bitname, int res);
void rpcManage(String &payload, bool sync);
void rpcManage(const char *payload, bool sync);
#endif
