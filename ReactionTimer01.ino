// Clay pigeon release reaction timer game
// simulates a manual clay pigeon trap release button,
// displays reaction time and statistics on the previous 10 button presses
////
/// Uses Adafruit OLED 1.3" 128x64 SPI display and simple momentary push button
// and a generic Arduino Nano v3 ATMega328@16Mhz
//////////////
// To create your own audio sample for 'pull', record your sample in e.g. Audacity at 8Khz mono
// in Audacity, to go Analyse->Sample Data Export-->
// limit output to [insert length of your recording - aroun 3-5000 samples]
// measurement scale - linear
//data (csv)
//no header
//L channel first
//chose filename as required
// then open the CSV in e.g. Excel
// you need to transpose the values to integers between 0 and 255
// save from excel as CSV, then open in a text editor e.g. notepad++, and do a search and replace \r\n for ','
// this will give you a comma separated list of samples, integer values between 0 and 255 to copy into PROGMEM below
//////////////////////
//// PINS ///
// n.b. some pins unavailable for other uses as they are used for interrupt-driven async audio output by ATMega328
// SPI Data   D8
// SPI Clock  D9
// SPI DC     D10
// SPI CS     D12
// SPI  REST  D13
//
// BUTTON     D2 and ground
/////////////////
// uses Asynchronous PWM audio playback
// http://highlowtech.org/?p=1963
// and Adafruit SSD1306 / GFX libraries
// https://github.com/adafruit/Adafruit_SSD1306
// respective licenses respected
////////////////////
// libs for OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// libs for async audio
#include <PCM.h>

// audio clip
const unsigned char sample[] PROGMEM = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 95, 95, 95, 95, 95, 95, 96, 96, 96, 95, 95, 95, 95, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 97, 96, 95, 96, 97, 96, 96, 97, 96, 95, 95, 96, 96, 96, 96, 96, 96, 95, 95, 95, 96, 96, 95, 95, 94, 95, 95, 95, 95, 94, 95, 95, 95, 95, 94, 95, 95, 95, 95, 96, 95, 95, 95, 94, 94, 94, 94, 95, 94, 94, 94, 93, 93, 93, 93, 94, 94, 93, 93, 93, 93, 93, 93, 93, 93, 92, 92, 93, 93, 92, 92, 93, 93, 93, 92, 93, 93, 93, 93, 93, 93, 92, 95, 88, 77, 83, 82, 82, 72, 60, 72, 73, 72, 74, 72, 80, 82, 82, 91, 91, 91, 94, 92, 93, 91, 88, 84, 84, 79, 74, 75, 74, 75, 74, 74, 75, 74, 76, 78, 80, 80, 81, 84, 84, 85, 89, 91, 89, 89, 92, 93, 91, 91, 93, 95, 95, 91, 91, 93, 94, 95, 91, 87, 87, 89, 89, 90, 91, 89, 88, 91, 96, 98, 97, 93, 97, 100, 100, 101, 96, 94, 97, 95, 94, 95, 94, 92, 93, 92, 92, 97, 96, 93, 95, 98, 99, 99, 98, 98, 99, 98, 97, 100, 101, 99, 98, 98, 98, 95, 96, 96, 95, 96, 95, 98, 101, 100, 99, 99, 102, 103, 105, 105, 105, 107, 109, 108, 108, 104, 102, 103, 101, 100, 103, 103, 101, 100, 98, 97, 98, 97, 97, 96, 95, 94, 95, 94, 91, 88, 85, 85, 85, 84, 81, 80, 79, 77, 78, 79, 79, 80, 81, 83, 86, 88, 87, 88, 90, 92, 92, 94, 97, 99, 102, 104, 104, 104, 106, 109, 113, 113, 115, 118, 121, 123, 124, 121, 119, 117, 113, 107, 105, 94, 85, 78, 73, 72, 68, 65, 66, 66, 63, 63, 65, 66, 71, 77, 80, 83, 86, 85, 86, 88, 88, 90, 96, 100, 107, 116, 118, 118, 118, 120, 127, 132, 136, 135, 138, 132, 127, 123, 113, 109, 103, 96, 92, 87, 83, 80, 80, 81, 83, 81, 77, 72, 69, 72, 79, 81, 82, 80, 79, 79, 77, 76, 77, 84, 93, 103, 111, 115, 119, 124, 127, 130, 130, 135, 142, 147, 154, 155, 150, 142, 129, 113, 105, 99, 93, 87, 79, 73, 71, 72, 72, 76, 78, 81, 83, 75, 71, 65, 66, 76, 79, 77, 73, 64, 63, 66, 71, 80, 93, 104, 117, 128, 133, 135, 138, 142, 144, 145, 153, 151, 156, 158, 134, 116, 98, 88, 90, 88, 81, 79, 75, 68, 75, 81, 85, 93, 99, 91, 83, 71, 61, 57, 58, 57, 57, 62, 62, 64, 72, 76, 81, 95, 107, 121, 138, 145, 143, 146, 141, 138, 143, 143, 144, 132, 107, 91, 78, 78, 79, 77, 79, 73, 70, 71, 80, 88, 101, 107, 108, 102, 86, 64, 51, 50, 50, 54, 62, 63, 62, 74, 80, 87, 105, 120, 136, 158, 161, 155, 155, 146, 151, 151, 152, 141, 101, 89, 70, 66, 81, 72, 75, 75, 68, 66, 81, 87, 95, 116, 110, 110, 106, 80, 60, 53, 47, 48, 55, 64, 62, 75, 82, 87, 99, 114, 133, 158, 176, 171, 162, 153, 146, 145, 158, 149, 113, 97, 72, 60, 79, 65, 71, 78, 66, 67, 83, 87, 91, 117, 108, 103, 105, 77, 50, 56, 50, 50, 60, 60, 54, 70, 82, 87, 108, 121, 137, 162, 178, 171, 174, 164, 149, 147, 157, 110, 88, 85, 49, 79, 76, 62, 69, 60, 53, 67, 90, 89, 112, 117, 97, 102, 97, 60, 55, 46, 35, 48, 59, 57, 64, 82, 78, 97, 109, 117, 143, 165, 169, 170, 169, 152, 147, 162, 144, 97, 103, 64, 61, 84, 64, 70, 70, 54, 52, 77, 80, 95, 121, 105, 95, 107, 70, 45, 52, 34, 45, 63, 56, 53, 75, 74, 88, 117, 117, 140, 167, 165, 164, 174, 150, 142, 162, 136, 88, 104, 61, 61, 94, 65, 78, 73, 56, 53, 79, 80, 95, 124, 109, 98, 108, 77, 48, 55, 39, 48, 67, 62, 54, 78, 77, 87, 119, 118, 135, 170, 164, 159, 173, 152, 141, 163, 145, 91, 105, 70, 59, 103, 73, 80, 81, 56, 52, 80, 82, 89, 124, 105, 97, 108, 74, 40, 44, 30, 40, 70, 64, 56, 77, 73, 82, 122, 121, 140, 179, 174, 160, 171, 154, 134, 170, 128, 81, 95, 50, 69, 98, 91, 94, 85, 63, 52, 79, 86, 107, 130, 114, 100, 86, 50, 34, 45, 52, 64, 71, 57, 44, 63, 68, 96, 134, 146, 168, 187, 168, 153, 156, 138, 152, 138, 84, 81, 49, 58, 81, 81, 106, 88, 72, 62, 64, 74, 102, 123, 119, 114, 88, 44, 35, 42, 48, 87, 77, 61, 52, 51, 59, 92, 124, 146, 186, 187, 173, 165, 143, 134, 158, 119, 85, 77, 29, 50, 69, 71, 106, 100, 81, 76, 77, 68, 100, 119, 111, 113, 72, 42, 42, 62, 71, 96, 85, 60, 52, 58, 69, 100, 135, 160, 185, 188, 169, 156, 149, 141, 148, 106, 81, 55, 28, 52, 51, 74, 93, 81, 83, 84, 80, 90, 115, 110, 114, 88, 58, 41, 51, 70, 86, 107, 86, 66, 61, 65, 83, 124, 147, 174, 187, 179, 166, 159, 158, 150, 128, 85, 65, 33, 37, 44, 53, 75, 68, 73, 76, 78, 93, 117, 118, 99, 85, 52, 32, 57, 63, 77, 114, 92, 74, 76, 64, 76, 127, 145, 169, 198, 176, 161, 162, 150, 145, 129, 93, 62, 37, 41, 31, 57, 64, 52, 68, 62, 63, 89, 117, 107, 112, 88, 42, 37, 44, 50, 86, 101, 85, 88, 79, 78, 110, 141, 157, 184, 189, 169, 163, 155, 143, 127, 109, 87, 58, 60, 50, 43, 68, 49, 41, 53, 43, 58, 88, 99, 94, 109, 76, 51, 58, 49, 55, 84, 75, 75, 86, 82, 107, 152, 170, 195, 210, 184, 162, 155, 135, 109, 98, 85, 55, 78, 66, 60, 88, 67, 58, 60, 46, 38, 75, 73, 79, 102, 67, 66, 74, 65, 66, 85, 61, 66, 76, 80, 113, 158, 178, 203, 219, 187, 175, 157, 122, 82, 80, 52, 45, 76, 59, 80, 87, 71, 67, 71, 48, 58, 73, 57, 76, 68, 58, 63, 77, 74, 88, 83, 64, 66, 70, 94, 134, 179, 204, 226, 218, 190, 162, 123, 72, 63, 46, 51, 64, 62, 81, 71, 77, 68, 61, 64, 66, 73, 72, 70, 54, 56, 54, 70, 82, 88, 86, 80, 72, 79, 102, 139, 185, 224, 241, 229, 213, 141, 79, 55, 18, 44, 69, 66, 93, 79, 63, 64, 62, 59, 74, 83, 65, 81, 64, 56, 75, 76, 66, 88, 70, 62, 80, 84, 107, 147, 168, 188, 225, 212, 208, 180, 102, 68, 27, 25, 47, 60, 93, 89, 84, 70, 58, 52, 60, 76, 76, 88, 74, 63, 71, 79, 78, 99, 84, 72, 72, 71, 97, 141, 178, 206, 239, 219, 203, 153, 82, 63, 27, 46, 58, 61, 86, 73, 73, 69, 62, 59, 70, 69, 65, 80, 57, 61, 74, 73, 82, 97, 77, 80, 81, 82, 114, 150, 184, 224, 250, 231, 201, 120, 68, 23, 18, 43, 49, 91, 82, 75, 71, 53, 50, 73, 86, 85, 96, 69, 51, 60, 58, 73, 96, 87, 85, 82, 70, 95, 130, 169, 222, 253, 255, 217, 134, 84, 18, 21, 39, 46, 90, 76, 73, 67, 50, 42, 67, 76, 85, 103, 74, 66, 69, 58, 71, 94, 79, 91, 90, 74, 98, 122, 143, 200, 238, 242, 216, 141, 85, 23, 28, 31, 62, 102, 85, 91, 61, 32, 28, 53, 62, 97, 107, 79, 79, 64, 54, 81, 97, 93, 114, 92, 81, 101, 114, 152, 215, 238, 237, 180, 109, 44, 6, 33, 37, 97, 112, 100, 96, 54, 32, 33, 54, 65, 99, 88, 72, 71, 64, 68, 96, 99, 104, 111, 93, 97, 112, 133, 170, 229, 235, 219, 151, 84, 17, 5, 18, 42, 104, 98, 115, 96, 55, 39, 37, 47, 74, 100, 82, 86, 69, 59, 73, 88, 89, 117, 109, 105, 119, 121, 152, 204, 240, 239, 188, 120, 39, 0, 4, 16, 84, 99, 107, 111, 63, 43, 36, 40, 58, 95, 78, 79, 74, 58, 73, 96, 90, 109, 106, 93, 113, 119, 146, 196, 239, 239, 195, 147, 48, 17, 6, 0, 76, 76, 96, 109, 74, 54, 53, 46, 50, 92, 68, 76, 76, 57, 65, 95, 79, 109, 112, 96, 123, 120, 142, 184, 230, 226, 203, 159, 60, 41, 4, 1, 74, 67, 98, 110, 69, 54, 52, 41, 59, 97, 75, 86, 80, 52, 65, 89, 70, 113, 106, 98, 121, 115, 134, 180, 221, 214, 207, 154, 70, 55, 7, 16, 76, 63, 104, 104, 65, 52, 47, 36, 65, 94, 73, 98, 79, 62, 83, 89, 80, 120, 99, 105, 124, 113, 145, 181, 204, 192, 194, 121, 74, 67, 4, 55, 74, 68, 113, 91, 57, 59, 41, 29, 81, 73, 81, 99, 75, 69, 97, 87, 97, 128, 94, 126, 127, 129, 159, 188, 172, 169, 157, 74, 90, 40, 28, 81, 59, 88, 103, 76, 60, 64, 32, 49, 77, 59, 89, 84, 70, 88, 100, 87, 123, 110, 110, 133, 133, 148, 181, 179, 147, 166, 85, 80, 68, 23, 76, 64, 75, 101, 85, 71, 74, 53, 45, 73, 50, 74, 84, 69, 81, 97, 84, 115, 122, 115, 142, 137, 150, 167, 176, 137, 154, 99, 75, 85, 30, 73, 68, 64, 96, 83, 70, 78, 64, 49, 77, 50, 58, 76, 63, 77, 99, 86, 106, 123, 107, 145, 141, 157, 176, 187, 146, 150, 103, 62, 91, 35, 73, 83, 61, 92, 86, 66, 81, 74, 53, 80, 57, 47, 70, 56, 66, 99, 87, 100, 119, 96, 128, 138, 148, 175, 192, 158, 149, 125, 57, 90, 48, 59, 96, 64, 86, 91, 65, 74, 82, 57, 81, 78, 51, 73, 59, 55, 85, 88, 89, 123, 95, 116, 130, 132, 161, 179, 176, 148, 149, 77, 84, 70, 45, 96, 71, 81, 97, 72, 70, 77, 60, 64, 83, 58, 75, 77, 63, 76, 91, 75, 109, 101, 95, 126, 119, 147, 167, 182, 152, 158, 109, 75, 93, 41, 78, 87, 62, 96, 76, 67, 77, 69, 58, 81, 66, 66, 85, 69, 78, 92, 85, 90, 109, 80, 111, 110, 123, 152, 170, 170, 153, 149, 86, 98, 72, 51, 96, 62, 80, 86, 63, 72, 74, 61, 71, 82, 58, 87, 75, 80, 92, 95, 83, 104, 84, 86, 108, 97, 135, 144, 171, 169, 156, 141, 95, 102, 66, 69, 88, 61, 88, 72, 67, 68, 70, 58, 72, 73, 66, 92, 83, 93, 102, 95, 83, 100, 70, 89, 95, 95, 126, 134, 156, 168, 155, 150, 120, 106, 88, 70, 79, 66, 72, 76, 69, 69, 65, 62, 54, 68, 67, 88, 101, 104, 112, 103, 87, 85, 75, 76, 90, 92, 112, 123, 134, 159, 160, 153, 158, 118, 115, 91, 66, 76, 59, 69, 76, 71, 69, 70, 50, 58, 69, 73, 105, 115, 118, 118, 98, 79, 75, 62, 73, 84, 92, 103, 114, 122, 142, 160, 153, 168, 134, 116, 99, 54, 66, 55, 62, 78, 77, 67, 66, 50, 46, 70, 81, 113, 135, 129, 116, 100, 66, 63, 61, 62, 86, 86, 93, 96, 97, 108, 130, 155, 177, 173, 162, 126, 93, 66, 44, 59, 60, 81, 84, 76, 65, 50, 47, 58, 88, 110, 140, 144, 131, 106, 80, 57, 53, 58, 70, 87, 91, 93, 91, 93, 98, 117, 142, 164, 175, 162, 143, 100, 76, 54, 47, 63, 68, 83, 82, 72, 61, 55, 58, 81, 103, 123, 147, 135, 129, 98, 74, 62, 60, 65, 80, 87, 85, 94, 86, 94, 106, 119, 146, 160, 163, 153, 135, 101, 84, 63, 54, 66, 65, 79, 80, 75, 68, 69, 68, 86, 106, 119, 140, 135, 127, 105, 83, 63, 57, 55, 66, 81, 87, 102, 98, 101, 103, 110, 126, 141, 156, 153, 145, 118, 92, 71, 56, 58, 68, 80, 91, 93, 84, 80, 73, 79, 92, 108, 121, 128, 121, 106, 88, 68, 65, 64, 72, 83, 88, 94, 92, 92, 94, 100, 108, 118, 123, 129, 129, 119, 113, 98, 94, 90, 86, 89, 89, 84, 85, 77, 75, 77, 79, 89, 99, 105, 111, 111, 102, 102, 91, 88, 86, 83, 84, 86, 85, 85, 87, 85, 92, 97, 103, 110, 114, 117, 121, 124, 120, 122, 112, 103, 94, 80, 76, 74, 74, 81, 89, 90, 96, 94, 91, 91, 91, 96, 103, 108, 111, 110, 97, 90, 78, 73, 78, 81, 92, 99, 104, 105, 105, 100, 99, 98, 100, 107, 112, 116, 114, 110, 102, 96, 89, 91, 93, 94, 97, 92, 89, 84, 81, 83, 87, 92, 100, 104, 108, 108, 104, 102, 98, 96, 97, 95, 93, 88, 86, 81, 83, 84, 90, 98, 104, 110, 112, 114, 113, 111, 110, 109, 109, 107, 104, 99, 91, 86, 81, 82, 85, 92, 99, 104, 104, 103, 98, 92, 93, 92, 97, 102, 105, 104, 101, 90, 84, 78, 79, 87, 95, 105, 109, 108, 101, 95, 87, 86, 88, 92, 98, 99, 98, 96, 95, 95, 99, 103, 107, 107, 105, 101, 95, 90, 88, 91, 94, 98, 100, 99, 97, 91, 90, 88, 91, 96, 99, 104, 102, 99, 93, 87, 84, 85, 89, 96, 105, 109, 113, 113, 110, 107, 105, 103, 105, 105, 102, 98, 90, 83, 77, 76, 77, 82, 87, 93, 98, 99, 101, 102, 103, 105, 107, 105, 106, 102, 99, 96, 95, 95, 95, 96, 93, 94, 92, 93, 92, 92, 93, 93, 95, 94, 96, 95, 96, 97, 96, 97, 96, 96, 95, 97, 97, 101, 103, 104, 104, 102, 102, 99, 98, 97, 96, 94, 95, 94, 93, 93, 92, 93, 93, 93, 93, 95, 95, 96, 96, 95, 94, 95, 96, 96, 97, 97, 96, 95, 96, 94, 94, 94, 96, 98, 99, 101, 102, 102, 100, 99, 96, 94, 92, 93, 93, 93, 93, 92, 91, 90, 90, 92, 95, 96, 97, 97, 95, 95, 94, 94, 95, 95, 96, 98, 98, 99, 99, 100, 101, 102, 102, 104, 103, 101, 100, 97, 96, 93, 91, 91, 89, 89, 89, 91, 91, 93, 94, 96, 98, 100, 100, 100, 98, 97, 96, 94, 94, 92, 92, 93, 94, 95, 96, 99, 100, 101, 103, 104, 104, 103, 103, 100, 98, 96, 95, 94, 92, 93, 92, 93, 92, 92, 92, 92, 93, 94, 97, 99, 101, 102, 102, 102, 100, 99, 98, 98, 98, 99, 98, 98, 97, 96, 95, 95, 95, 95, 96, 96, 95, 96, 95, 94, 94, 95, 95, 95, 94, 94, 95, 94, 95, 95, 95, 95, 96, 96, 97, 96, 96, 96, 95, 95, 94, 94, 93, 93, 92, 93, 93, 94, 94, 95, 95, 97, 97, 98, 99, 98, 98, 97, 97, 96, 96, 96, 97, 97, 96, 96, 94, 93, 92, 93, 93, 94, 96, 96, 97, 96, 96, 94, 94, 94, 94, 94, 94, 94, 95, 95, 94, 94, 94, 95, 94, 94, 94, 96, 96, 96, 96, 96, 95, 95, 96, 95, 95, 95, 95, 95, 94, 95, 96, 95, 95, 95, 95, 94, 95, 95, 95, 94, 94, 95, 95, 95, 94, 94, 95, 96, 97, 97, 97, 96, 95, 94, 94, 94, 95, 96, 96, 95, 94, 94, 94, 95, 96, 97, 99, 99, 97, 97, 97, 97, 97, 98, 98, 97, 97, 98, 98, 97, 97, 97, 96, 96, 95, 95, 95, 95, 95, 94, 94, 94, 94, 94, 94, 94, 94, 95, 94, 94, 94, 94, 94, 94, 94, 95, 95, 95, 94, 95, 95, 95, 96, 97, 97, 96, 96, 96, 96, 96, 96, 96, 96, 95, 94, 94, 93, 94, 95, 95, 96, 95, 94, 94, 93, 93, 93, 94, 95, 95, 95, 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 96, 96, 95, 94, 94, 93, 93, 93, 93, 93, 93, 92, 92, 93, 92, 92, 92,
};

// If using software SPI (the default case):
#define OLED_MOSI   8
#define OLED_CLK   9
#define OLED_DC    10
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//// CONSTANTS
int button = 2;           // button pin

long starttime = 0;       //start time of 'pull'
long endtime = 0;         //end time of 'pull'
long mypause = 5000;      // delay between 'pull cause' -
long myrandomtime = 0;    //variable for randomising 'pull' calls by a certain degree

long mydelay = 0;         // single press reaction time
long results[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // array to hold past 10 results
long myaverage = 0;       //average of past 10 calls
long mymin = 0;           // minimum reaction time of past 10 calls
long mymax = 0;           // maximum "" ""
long mystdev = 0;         // Standard Deviation of reaction times
int pointer = 0;          // 'pointer' to non-zero values in an array of previous button pressses
long mytotal = 0;         // helper for calculating averagage
int count = 0;            //helper for calculatin g average - number of legitimate button presses
long mysquares = 0;       // helper for calculating standard deviation
int line1start = 30;      // pixel of dividing line for display grid
int line2start = 84;      // "" "" ""

void setup()   {
  //start serial for debug
  Serial.begin(9600);
  //set button to input pullup, so we can just wire it to ground to trigger
  pinMode(button, INPUT_PULLUP);
  //init SSD as per sample Adafruit code
  display.begin(SSD1306_SWITCHCAPVCC);
  // init done

  // clear display buffer
  display.clearDisplay();
  // portrait rotation
  display.setRotation(1);
  // set text and home
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  // intro notice
  display.println("Pull!");
  display.println();
  display.println("Reaction");
  display.println("Timer");
  display.println("(C) Steve Wilson");
  display.println("Feb 2016");
  display.println("Ver 0.1");
  display.display();
  delay(2000);

  // now clear display and set up a grid pattern for displaying results neatly
  // this is done in a function we can re-use, as each time we draw text we need to clear the background
  // probably a more efficient way of doing this, but hey, it works.
  redraw();
}


void loop() {

  //debug to serial
  Serial.println ("Pull!");

  //start playback of audio sample
  startPlayback(sample, sizeof(sample));

  //start our timer
  starttime = millis();

  //now wait for a button press - super tight loop here (could be done with interrupts, but this works fine
  while (digitalRead(button) == HIGH) {
  }

  // button has been pressed - record time
  endtime = millis();

  //calculate reaction time
  mydelay = endtime - starttime;
  Serial.print("Delay: ");
  Serial.println(mydelay);


  //if the reaction time is a reasonable figure (less than 2 seconds), then record it as a valid result
  // if the device is left unused, then delay is large and result is discarded
  // this effectively puts the box into an inefficient standby until the button is pressed to restart the sequence
  if (mydelay < 2000)
  {
    //we have a valid button press, store the result in our array of 10 previous valid responses
    // rolling buffer array of previous 10 arrays. Oldest result gets overwritten
    results[pointer] = mydelay;
    pointer++;
    if (pointer == 10) {
      pointer = 0;
    }

    //reset stats
    myaverage = 0;
    mytotal = 0;
    mymin = 0;
    mymax = 0;
    count = 0;
    mystdev = 0;
    mysquares = 0;


    // calculate stats based on previous valid button presses
    for (int i = 0; i < 10; i++)
    {

      // result in array is a valid button press, calculate stats
      if (results[i] > 0)
      {
        //count how many valid presses we have
        count++;

        //sum values for working average
        mytotal = mytotal + results[i];

        //get the minimum value in the array
        if (mymin == 0) {
          mymin = results[i];
        }
        if (results[i] < mymin) {
          mymin = results[i];
        }

        //max value in array
        if (results[i] > mymax) {
          mymax = results[i];
        }

      }//end if array unit greater than zero
    }//end loop over results array

    //having looped over all valid results, now calculate the average reaction time and print
    myaverage = mytotal / count;
    Serial.print("Average: ");
    Serial.println (myaverage);


    //calculate standard deviation
    for (int i = 0; i < 10; i++)
    {
      if (results[i] > 0) {
        long var = results[i] - myaverage;
        mysquares = mysquares + (var * var);
      }
    }
    long sqmean = mysquares / count;
    mystdev = sqrt(sqmean);



  } //end if delay less than 2000

  //debug - output the array
  for (int i = 0; i < 10; i++) {
    Serial.println(results[i]);
  }
  //debug output raw values used in calcs
  Serial.println(myaverage);
  Serial.println(count);
  Serial.println(mymin);
  Serial.println(mymax);
  Serial.println(mystdev);

  //display results on OLED

  //clear display
  display.clearDisplay();

  //redraw grid
  redraw();

  // put last reaction time at top in big fint
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print(mydelay);
  display.setTextSize(1);
  display.setCursor(50, 0);
  display.print (" m");
  display.setCursor(50, 10);
  display.print (" s");

  //minimum - top right of grid
  display.setCursor(5, line1start + 30);
  display.print(mymin);
  //max - top  left
  display.setCursor(37, line1start + 30);
  display.print(mymax);
  // average
  display.setCursor(5, line2start + 30);
  display.print(myaverage);
  //std deviation
  display.setCursor(37, line2start + 30);
  display.print(mystdev);
  //draw
  display.display();

  //now wait for our given pause (e.g. 6 seconds +-1 second) between 'pull' calls to mimic a real DTL/ABT layout
  delay(mypause + random(0, 2000));

}//end loop



//function to clear display and draw simple grid
void redraw() {

  display.clearDisplay();
  display.setCursor(0, 0);
  //fast horiz line at 32

  display.drawFastHLine(0, line1start, 64, 1);
  display.drawFastHLine(0, line2start, 64, 1);
  display.drawFastVLine(32, line1start, 128, 1);
  display.display();
  display.setCursor(0, 0);
  display.setTextSize(2);
  // display.println("123 ms");
  // display.display();
  //vast vert line at 64
  display.setTextSize(1);
  display.setCursor(0, (line1start + 2));
  display.print("Best");

  display.setCursor(32, (line1start + 2));
  display.print("Worst");
  display.setCursor(5, (line2start + 2));

  display.print("Avg:");
  display.setCursor(35, (line2start + 2));

  display.print("Dev:");
  display.display();

}



