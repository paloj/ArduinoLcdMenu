# ArduinoLcdMenu
 LCD menu system for rotary encoder and relay control. All menus, functions and settings are accessible with rotating the encoder and pushing a button.
 
 - Main menu
 - 1.Value Ctrl menu -> persistent values to guide relay ctrl limits
   - 1.1 View stored values menu
   - 1.2 Zero all values (empty menu)
   - 1.3 Change values menu
 - 2.Sensors menu -> values from four analog inputs (A0 - A3).
 - 3.Relays menu -> Relay ctrl status of four relays (D9 - D12).
 - 4.Real Time Clock menu -> Date and time display
   - 4.1 Change date and time menu  
 
 Development setup:
 - Arduino nano every
 - 4x20 LCD + i2c board
 - DS1302 RTC
 - HW-040 Rotary encoder with pushbutton

Libraries:
- #include <LiquidCrystal_I2C.h>
- #include <ThreeWire.h>
- #include <RtcDS1302.h>
- #include <EEPROM.h>


https://user-images.githubusercontent.com/73587747/160590976-a1e4b383-2817-4338-9233-c5fb3b8100ef.mp4
