#include "USB.h"
#include "USBMSC.h"

USBMSC msc;
bool mscConfirm = false;
uint32_t sectorCount = 0;
uint32_t sectorSize = 512;

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);
void startMSC();

