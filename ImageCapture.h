#ifndef IMAGE_CAPTURE_H
#define IMAGE_CAPTURE_H

#include <stdint.h>

//Prototypes
void imageCaptureSetup();
void captureImage();
void uploadImage(uint8_t *imgData, uint32_t imgLen);
void checkMemory();

#endif
