#pragma once

#include "tplinkconf.h"



#ifdef MIT_QNETHERNET
#include <QNEthernet.h>
using namespace qindesign::network;
#else
#include <NativeEthernet.h>
#endif

void as_eth_setup();
void as_eth_beat();
bool isB300Connected();