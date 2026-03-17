#pragma once

// Select the camera model
// =================================================================================
// IMPORTANT: Only one camera model should be selected
//            by uncommenting the corresponding line.
// =================================================================================

// #define CAMERA_MODEL_WROVER_KIT
// #define CAMERA_MODEL_ESP_EYE
// #define CAMERA_MODEL_M5STACK_PSRAM
// #define CAMERA_MODEL_M5STACK_V2_PSRAM
// #define CAMERA_MODEL_M5STACK_WIDE
// #define CAMERA_MODEL_M5STACK_ESP32CAM
// #define CAMERA_MODEL_M5STACK_UNITCAM
#define CAMERA_MODEL_AI_THINKER  // Most common model
// #define CAMERA_MODEL_TTGO_T_JOURNAL
// #define CAMERA_MODEL_XIAO_ESP32S3
// #define CAMERA_MODEL_FREENOVE_ESP32S3_WROVER
// #define CAMERA_MODEL_ESP32S3_EYE

// Include the corresponding pin definition file
#if defined(CAMERA_MODEL_WROVER_KIT)
#include "camera_pins.h"
#elif defined(CAMERA_MODEL_ESP_EYE)
#include "camera_pins.h"
#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
#include "camera_pins.h"
#elif defined(CAMERA_MODEL_M5STACK_V2_PSRAM)
#include "camera_pins.h"
#elif defined(CAMERA_MODEL_M5STACK_WIDE)
#include "camera_pins.h"
#elif defined(CAMERA_MODEL_M5STACK_ESP32CAM)
#include "camera_pins.h"
#elif defined(CAMERA_MODEL_M5STACK_UNITCAM)
#include "camera_pins.h"
#elif defined(CAMERA_MODEL_AI_THINKER)
#include "camera_pins.h"
#elif defined(CAMERA_MODEL_TTGO_T_JOURNAL)
#include "camera_pins.h"
#elif defined(CAMERA_MODEL_XIAO_ESP32S3)
#include "camera_pins.h"
#elif defined(CAMERA_MODEL_FREENOVE_ESP32S3_WROVER)
#include "camera_pins.h"
#elif defined(CAMERA_MODEL_ESP32S3_EYE)
#include "camera_pins.h"
#else
#error "Camera model not selected"
#endif