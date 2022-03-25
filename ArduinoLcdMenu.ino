
//MULTIPLE ANALOG INPUT 4X-RELAY CONTROLLER WITH ROTARY ENCODER LCD MENU SYSTEM AND RTC MODULE
//23-03-2022
//V.0.1
//AUTHOR: JP
//Licence: Nonfree
//---------------------------------------------------------------

#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <EEPROM.h>

//----------EEPROM VARIABLES DECLARATION----------
float set0 = 0.00f;
float set1 = 0.00f;
float set2 = 0.00f;
float set3 = 0.00f;
float set4 = 0.00f;
float set5 = 0.00f;
float set6 = 0.00f;
float set7 = 0.00f;
float set8 = 0.00f;
float set9 = 0.00f;

//DEFINE ROTARY ENCODER INPUTS
#define          re_input_CLK  6
#define          re_input_DT   7 //USE INTERRUPT PIN
#define          re_input_BTN  8 //USE INTERRUPT PIN
volatile int     re_counter =  0;
volatile int     re_current_CLK = LOW;
volatile int     re_prev_CLK = re_current_CLK;
volatile int     re_btn = HIGH;

//RTC settings
ThreeWire myWire(4, 5, 2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

//LCD settings
// Set the LCD address to 0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);// Initialize LCD Display at address 0x27
bool firstloop = true;

//GLOBAL MENU VARIABLES
float menu_page = 0;
int   menu_row  = 1;
bool  menu_page_change_enabled = true;
bool  menu_row_change_enabled = true;
bool  POWERSAVE = false;
int   pwr_cycles = 0;     //for counting cycles foe powersave when nothing is happening
int   sec;                //for counting seconds
float valuesar[9];      //ARRAY FOR STORED EEPROM VALUES

//----ANALOG SENSOR INPUT PINS----
int SENS_0 = A0; //SENSOR 0 ANALOG PIN
int SENS_1 = A1; //SENSOR 1 ANALOG PIN
int SENS_2 = A2; //SENSOR 2 ANALOG PIN
int SENS_3 = A3; //SENSOR 3 ANALOG PIN

float SENS_0_VALUE = 0;
float SENS_1_VALUE = 0;
float SENS_2_VALUE = 0;
float SENS_3_VALUE = 0;

//----RELAY OUTPUTS---------------
#define REL_1_PIN 9   //digital pin 9
#define REL_2_PIN 10  //digital pin 10
#define REL_3_PIN 11  //digital pin 11
#define REL_4_PIN 12  //digital pin 12

bool REL_1_ON = false;
bool REL_2_ON = false;
bool REL_3_ON = false;
bool REL_4_ON = false;


//----------------SETUP START------------------------------
void setup ()
{
  Serial.begin(9600);

  //----------EEPROM VARIABLES SETUP----------
  EEPROM.get(0, set0);
  EEPROM.get(10,set1);
  EEPROM.get(20,set2);
  EEPROM.get(30,set3);
  EEPROM.get(40,set4);
  EEPROM.get(50,set5);
  EEPROM.get(60,set6);
  EEPROM.get(70,set7);
  EEPROM.get(80,set8);
  EEPROM.get(90,set9);

  //VALUES TO ARRAY
  valuesar[1]  = set0;
  valuesar[2]  = set1;
  valuesar[3]  = set2;
  valuesar[4]  = set3;
  valuesar[5]  = set4;
  valuesar[6]  = set5;
  valuesar[7]  = set6;
  valuesar[8]  = set7;
  valuesar[9]  = set8;
  valuesar[10] = set9;

  //----------RELAY SETUP------------------
  pinMode (REL_1_PIN, OUTPUT);
  pinMode (REL_2_PIN, OUTPUT);
  pinMode (REL_3_PIN, OUTPUT);
  pinMode (REL_4_PIN, OUTPUT);
   

  //----------ROTARY ENCODER SETUP---------
  pinMode (re_input_CLK,INPUT);
  pinMode (re_input_DT,INPUT);
  pinMode (re_input_BTN,INPUT_PULLUP);  //INVERT ACTION

  //---------ROTARY ENCODER INTERRUPTS -----------------------
  //we run re_update function if DT change is detected on a rotary encoder
  attachInterrupt(digitalPinToInterrupt(re_input_DT), re_update, CHANGE);
  
  //we run button_update if change is detected
  attachInterrupt(digitalPinToInterrupt(re_input_BTN), btn_update, FALLING);

  
  //----------RTC SETUP---------
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid())
  {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    //UNCOMMENT THIS TO WRITE NEW TIME TO RTC MODULE
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  //----------LCD SETUP---------
  // activate LCD module
  lcd.begin ();
  lcd.backlight();
  lcd.print("--STARTING SYSTEM!--");

} // 
//----------END OF SETUP-----------------------------------------------------------------------------

//-------BUTTON INTERRUPT CALLS THIS FUNCTION------------------------------------------
volatile bool button_pressed = false;
void btn_update(){
  button_pressed = true;
  Serial.println("Button pressed");
}

//-------ROTARY ENCODER INTERRUPT CALLS THIS FUNCTION WHEN IT HAS BEEN TURNED----------
void re_update(){
  Serial.print("Rotation ");
  //ROTATION DIRECTION
  re_current_CLK = digitalRead(re_input_CLK);   //read current state of clk

  //if there is a minimal movement of 1 step
  if ((re_prev_CLK == LOW) && (re_current_CLK == HIGH)){

    if (digitalRead(re_input_DT) == HIGH){
      Serial.println("Right ");
      re_counter++;
    } else {
      Serial.println("Left ");
      re_counter--;
    }
  } else {
    if (digitalRead(re_input_DT) == HIGH){
      Serial.println("Left ");
      re_counter--;
    } else {
      Serial.println("Right ");
      re_counter++;
    }
  }
  if (re_counter>999 || re_counter<-999){re_counter=0;}
}



//--------------------------------------------------------------------------------------------
//----------MENUS-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

//MENUS AND LOOP
const String sel = ">";
const String ad_ = " ";

String datetime, timehhmm, start_date, re_msg, line1, line2, line3, line4;
String menu1_name = "Menu 1 ";
String menu2_name = "Menu 2 ";
String menu3_name = "Menu 3 ";
String menu4_name = "REAL TIME CLOCK ";
const String re_msge=" ";
int count, re_cnt_prev;


//------------------0.MAIN MENU----------------
void menu0(){
  //Menu item names
  menu1_name = "1.Value Ctrl";
  menu2_name = "2.Sensors";
  menu3_name = "3.Relays";
  menu4_name = "4.REAL TIME CLOCK ";

  //Menu item selection view
  if (menu_row == 1){
    line1 = sel + menu1_name;
    line2 = ad_+ menu2_name;
    line3 = ad_+ menu3_name;
    line4 = ad_+ menu4_name;
  }
  else if (menu_row == 2){
    line1 = ad_+ menu1_name;
    line2 = sel + menu2_name;
    line3 = ad_+ menu3_name;
    line4 = ad_+ menu4_name;
  }
  else if (menu_row == 3){
    line1 = ad_+ menu1_name;
    line2 = ad_+ menu2_name;
    line3 = sel + menu3_name;
    line4 = ad_+ menu4_name;
  }
  else if (menu_row == 4){
    line1 = ad_+ menu1_name;
    line2 = ad_+ menu2_name;
    line3 = ad_+ menu3_name;
    line4 = sel + menu4_name;
  }
  
  lcd.home();          //set cursor to 0,0
  lcd.print(line1);
  
  lcd.setCursor(0, 1); //go to start of 2nd line
  lcd.print(line2);

  lcd.setCursor(0, 2); //go to start of 3rd line
  lcd.print(line3);

  lcd.setCursor(0, 3); //go to start of 4th line
  lcd.print(line4);

  //print time to top right corner
  //blink : every second
  sec = datetime.substring(17).toInt();
  if (sec % 2){         //if number is even
    lcd.setCursor(15,0);
    lcd.print(timehhmm);
    }else{
    lcd.setCursor(17,0);
    lcd.print(" ");    //if number is odd print space instead of :
    }
}

//-----------1.VALUE CTRL MENU-----------------
void menu1(){

  //Menu item names
  menu1_name = "View stored values "; // menu 1.1
  menu2_name = "Zero all values ";    // menu 1.2 
  menu3_name = "Change values ";      // menu 1.3
  menu4_name = "Exit ";
  
   if (menu_row == 1){
    line1 = sel + menu1_name;
    line2 = ad_+ menu2_name;
    line3 = ad_+ menu3_name;
    line4 = ad_+ menu4_name;
  }
  else if (menu_row == 2){
    line1 = ad_+ menu1_name;
    line2 = sel + menu2_name;
    line3 = ad_+ menu3_name;
    line4 = ad_+ menu4_name;
  }
  else if (menu_row == 3){
    line1 = ad_+ menu1_name;
    line2 = ad_+ menu2_name;
    line3 = sel + menu3_name;
    line4 = ad_+ menu4_name;
  }
  else if (menu_row == 4){
    line1 = ad_+ menu1_name;
    line2 = ad_+ menu2_name;
    line3 = ad_+ menu3_name;
    line4 = sel + menu4_name;
  }
  
  lcd.home();          //set cursor to 0,0
  lcd.print(line1);
  
  lcd.setCursor(0, 1); //go to start of 2nd line
  lcd.print(line2);

  lcd.setCursor(0, 2); //go to start of 3rd line
  lcd.print(line3);

  lcd.setCursor(0, 3); //go to start of 4th line
  lcd.print(line4);
}

//------- 1.1 VIEW STORED VALUES MENU ---------------
int startline = 0;
bool refreshed = false;
void menu1_1(){
  
   //Scroll value list with rotary encoder
   if(re_cnt_prev != re_counter && refreshed == true){
     if (re_cnt_prev < re_counter){
       startline++;
       refreshed = false;
     }else{
       startline--;
       refreshed = false;
     }
     if(startline<0){startline=0;} //startline min is 0
     if(startline>6){startline=6;} //startline max is 6
   }
    
   //VALUE NAMES:
   if (refreshed == false){
     lcd.setCursor(0,0);
     lcd.print(String(startline)+": ");
     lcd.setCursor(0,1);
     lcd.print(String(startline+1)+": ");
     lcd.setCursor(0,2);
     lcd.print(String(startline+2)+": ");
     lcd.setCursor(0,3);
     lcd.print(String(startline+3)+": ");
  
     //PRINT VALUES FROM ARRAY
     lcd.setCursor(2,0);
     lcd.print(valuesar[startline+1],2);
     lcd.print("  ");
     lcd.setCursor(2,1);
     lcd.print(valuesar[startline+2],2);
     lcd.print("  ");
     lcd.setCursor(2,2);
     lcd.print(valuesar[startline+3],2);
     lcd.print("  ");
     lcd.setCursor(2,3);
     lcd.print(valuesar[startline+4],2);
     lcd.print("  ");

     refreshed = true;
   }
}

//---------- 1.3 EDIT VALUES VALUES MENU------------
int m_1_3_valNo = 0;
int m_1_3_Value;
int m_1_3_editrow = 0;
bool m_1_3_init = false;

void menu1_3(){
  
  //If rotation when editing valNo then add or substract from valNo
  if (m_1_3_editrow == 1 && re_cnt_prev != re_counter){
    m_1_3_init = false;
    if (re_cnt_prev < re_counter){
      m_1_3_valNo++;
    }else{
      m_1_3_valNo--;
    }
    if (m_1_3_valNo < 0){m_1_3_valNo = 9;} //loop number back to 9 if under 0
    if (m_1_3_valNo > 9){m_1_3_valNo = 0;} //loop number back to 0 if over 9
  }

  //If rotation when editing Value add or substract from Value
  if (m_1_3_editrow == 2 && re_cnt_prev != re_counter){
    if (re_cnt_prev < re_counter){
      m_1_3_Value = m_1_3_Value + (re_counter - re_cnt_prev);
    }else{
      m_1_3_Value = m_1_3_Value - (re_cnt_prev - re_counter);
    }
    if (m_1_3_Value < -100){m_1_3_Value = -50;} //loop back to -50 if less than -100
    if (m_1_3_Value > 100){m_1_3_Value = 50;}   //loop back to 50 if over 100
  }

  //Read Value from EEPROM values based on selected valNo
  if (m_1_3_init == false){
    if (m_1_3_valNo==0){m_1_3_Value=set0;}
    if (m_1_3_valNo==1){m_1_3_Value=set1;}
    if (m_1_3_valNo==2){m_1_3_Value=set2;}
    if (m_1_3_valNo==3){m_1_3_Value=set3;}
    if (m_1_3_valNo==4){m_1_3_Value=set4;}
    if (m_1_3_valNo==5){m_1_3_Value=set5;}
    if (m_1_3_valNo==6){m_1_3_Value=set6;}
    if (m_1_3_valNo==7){m_1_3_Value=set7;}
    if (m_1_3_valNo==8){m_1_3_Value=set8;}
    if (m_1_3_valNo==9){m_1_3_Value=set9;}
    m_1_3_init = true;
  }
    
  //Menu item names
  menu1_name = "ValNo ";    
  menu2_name = "Value ";    
  menu3_name = "SAVE";     
  menu4_name = "Exit ";
  
   if (menu_row == 1){
    if (m_1_3_editrow == 1){line1 = ad_ + menu1_name + m_1_3_valNo + "<  ";} //WHEN EDITING ROW 1
    else{line1 = sel + menu1_name + m_1_3_valNo + "  ";}                     //WHEN NOT EDITING ROW 1
    line2 = ad_+ menu2_name + m_1_3_Value + "  ";
    line3 = ad_+ menu3_name;
    line4 = ad_+ menu4_name;
  }
  else if (menu_row == 2){
    line1 = ad_+ menu1_name + m_1_3_valNo;
    if (m_1_3_editrow == 2){line2 = ad_ + menu2_name + m_1_3_Value + "<  ";}      //WHEN EDITING ROW 2
    else{line2 = sel + menu2_name + m_1_3_Value + "  ";}                          //WHEN NOT EDITING ROW 1
    line3 = ad_+ menu3_name;
    line4 = ad_+ menu4_name;
  }
  else if (menu_row == 3){
    line1 = ad_+ menu1_name + m_1_3_valNo;
    line2 = ad_+ menu2_name + m_1_3_Value;
    line3 = sel + menu3_name;
    line4 = ad_+ menu4_name;
  }
  else if (menu_row == 4){
    line1 = ad_+ menu1_name + m_1_3_valNo;
    line2 = ad_+ menu2_name + m_1_3_Value;
    line3 = ad_+ menu3_name;
    line4 = sel + menu4_name;
  }
  
  lcd.home();          //set cursor to 0,0
  lcd.print(line1);
  
  lcd.setCursor(0, 1); //go to start of 2nd line
  lcd.print(line2);

  lcd.setCursor(0, 2); //go to start of 3rd line
  lcd.print(line3);

  lcd.setCursor(0, 3); //go to start of 4th line
  lcd.print(line4);

  
}

//-------------MENU 2---------------
void menu2(){
  line1 = "SENS_0: " + String(SENS_0_VALUE) + "   ";
  line2 = "SENS_1: " + String(SENS_1_VALUE) + "   ";
  line3 = "SENS_2: " + String(SENS_2_VALUE) + "   ";
  line4 = "SENS_3: " + String(SENS_3_VALUE) + "   ";
  
  lcd.setCursor(0, 0);
  lcd.print(line1);

  lcd.setCursor(0, 1);
  lcd.print(line2);

  lcd.setCursor(0, 2);
  lcd.print(line3);

  lcd.setCursor(0, 3);
  lcd.print(line4);

}

//-------------MENU 3 RELAY STATUS---------------
void menu3(){
  line1 = "RELAY 1: " + String(REL_1_ON) + "   ";
  line2 = "RELAY 2: " + String(REL_2_ON) + "   ";
  line3 = "RELAY 3: " + String(REL_3_ON) + "   ";
  line4 = "RELAY 4: " + String(REL_4_ON) + "   ";
  
  lcd.setCursor(0, 0);
  lcd.print(line1);

  lcd.setCursor(0, 1);
  lcd.print(line2);

  lcd.setCursor(0, 2);
  lcd.print(line3);

  lcd.setCursor(0, 3);
  lcd.print(line4);
}

//------REALTIME CLOCK MENU--------
void menu4(){
  if (re_cnt_prev != re_counter) {
    
    lcd.setCursor(16, 2); //go to 3rd line and position 16
    re_msg = re_counter + re_msge;
    lcd.print(re_msg);
    Serial.println(re_msg);
  }
  
  //----------LCD STUFF---------
  //LINE 1
  line1 = "--RT CLOCK DISPLAY--";
  lcd.home(); //set cursor to 0,0
  lcd.print(line1);
  //LINE 2
  lcd.setCursor(0, 1); //go to start of 2nd line
  line2 = datetime;
  lcd.print(line2);
  //LINE 3
  lcd.setCursor(0, 2); //go to start of 3rd line
  line3 = "Nano temp: ";
  int temp = getTemperature();
  line3 = line3 + temp;
  lcd.print(line3);
  
  //LINE 4
  if (count == 0){clearLCDLine(3);}
  
  lcd.setCursor(0, 3); //go to start of 4th line
  if (count <= 2){
    line4 = "Power on since:";  
  }
  else if (2 < count && count <= 6){
    line4 = start_date;
    //line4 = "PAIVAMAARA 2022/02/02";
  }
  else{
    line4 = start_date;
    count = -1;
    }
  count++;
  lcd.print(line4);
  delay(800);
}

//SAVE VALUES TO EEPROM PERSISTENT MEMORY
void save_value(int addr, float value){
  String msg1, msg2;
  lcd.clear();
  lcd.home();
  msg1 = String("Saving value ") + value;
  msg2 = String("to EEPROM address ") + addr;
  lcd.print(msg1);
  lcd.setCursor(0, 1);
  lcd.print(msg2);
  EEPROM.put(addr, value);
  delay(2000);
  //read updated EEPROM values back to set variables
  EEPROM.get(0, set0);
  EEPROM.get(10,set1);
  EEPROM.get(20,set2);
  EEPROM.get(30,set3);
  EEPROM.get(40,set4);
  EEPROM.get(50,set5);
  EEPROM.get(60,set6);
  EEPROM.get(70,set7);
  EEPROM.get(80,set8);
  EEPROM.get(90,set9);
  //VALUES TO ARRAY
  valuesar[1]  = set0;
  valuesar[2]  = set1;
  valuesar[3]  = set2;
  valuesar[4]  = set3;
  valuesar[5]  = set4;
  valuesar[6]  = set5;
  valuesar[7]  = set6;
  valuesar[8]  = set7;
  valuesar[9]  = set8;
  valuesar[10] = set9;
  lcd.clear();
}

void RTC_update(){
  //----------RTC STUFF---------
  RtcDateTime now = Rtc.GetDateTime();

  printDateTime(now);
  printhhmm(now);
  
  if (!now.IsValid())
  {
    // Common Causes:
    //    1) the battery on the device is low or even missing and the power line was disconnected
    Serial.println("RTC lost confidence in the DateTime!");
  }
}

//----------LOOP FUNCTION-----------------------------------------------------------------------------

void loop ()
{ 
  //READ ALL ANALOG SENSOR VALUES ON START OF EVERY LOOP 
  READ_ANALOG_SENSORS();
  
  //IF BUTTON PRESSED DELAY TO AVOID DOUPLE PRESSES
  if (button_pressed){delay(50);}
  
  //POWERSAVE ON WHEN NO ACTION (LCD BACKLIGHT OFF)
  powersave();
  
  //UPDATE CLOCK
  RTC_update();
    
  //SAVE START DATETIME
  if (firstloop == true)
  {
    delay(2000);
    lcd.clear();
    start_date = datetime;
    firstloop = false;
  };

  //+++++++ BUTTON PRESS MENU ACTIONS +++++++
  
  //IF BUTTON PRESSED ON MENU 1.3 ROW 1
  if (button_pressed == true && menu_page == 1.3 && menu_row == 1 && m_1_3_editrow == 0){
    menu_row_change_enabled = false;
    m_1_3_editrow = 1;
    button_pressed = false;
  }
  
  //IF BUTTON PRESSED ON MENU 1.3 ROW 1 while m_1_3_editrow = 1
  if (button_pressed == true && menu_page == 1.3 && menu_row == 1 && m_1_3_editrow == 1){
    menu_row_change_enabled = true;
    m_1_3_editrow = 0;
    button_pressed = false;
    clearLCDLine(0);
  }

  //IF BUTTON PRESSED ON MENU 1.3 ROW 2
  if (button_pressed == true && menu_page == 1.3 && menu_row == 2 && m_1_3_editrow == 0){
    menu_row_change_enabled = false;
    m_1_3_editrow = 2;
    button_pressed = false;
  }
  
  //IF BUTTON PRESSED ON MENU 1.3 ROW 2 while m_1_3_editrow = 2
  if (button_pressed == true && menu_page == 1.3 && menu_row == 2 && m_1_3_editrow == 2){
    menu_row_change_enabled = true;
    m_1_3_editrow = 0;
    button_pressed = false;
    clearLCDLine(0);
  }

  //IF BUTTON PRESSED ON MENU 1.3 ROW 3 WRITE VALUE TO EEPROM TO ADDRESS ValNo*10
  if (button_pressed == true && menu_page == 1.3 && menu_row == 3){
    save_value(m_1_3_valNo*10 , m_1_3_Value);
    button_pressed = false;
  }
  
  
  //LOOP SELECTED MENU ROW POSITION IF ON MAIN OR MENU 1 IF ROTATION DETECTED
  if (menu_page < 2 && re_cnt_prev != re_counter && menu_row_change_enabled == true) {
    if (re_cnt_prev < re_counter){
      if (menu_row < 4){
        menu_row++;
      }
      else if (menu_row == 4){
        menu_row = 1;
      }
    } else {
      if (menu_row > 1){
        menu_row--;
      }
      else if (menu_row == 1){
        menu_row = 4;
      }
    }
  }

  //PRINT DEBUG INFO TO SERIAL MONITOR
  if (re_cnt_prev != re_counter) {
    serial_prints();
  }

  //RETURN FROM MENU 1 WITH BUTTON PRESS ON EXIT SELECTED
  if (menu_page == 1 && menu_row == 4 && button_pressed == true) {
    lcd.clear();
    menu_page = 0;
    menu_row = 1;
    button_pressed=false;
  }
  //RETURN FROM MENU 1.1 TO MENU 1 WITH BUTTON PRESS
  if (menu_page == 1.1 && button_pressed == true) {
    lcd.clear();
    menu_page = 1;
    menu_row = 1;
    button_pressed=false;
  }
  //RETURN FROM MENU 1.3 WITH BUTTON PRESS ON EXIT SELECTED
  if (menu_page == 1.3 && menu_row == 4 && button_pressed == true) {
    lcd.clear();
    menu_page = 0;
    menu_row = 1;
    button_pressed=false;
  }
  //RETURN FROM MENU 2 WITH BUTTON PRESS
  if (menu_page == 2 && button_pressed) {
    lcd.clear();
    menu_page = 0;
    menu_row = 1;
    button_pressed=false;
  }
  //RETURN FROM MENU 3 WITH BUTTON PRESS
  if (menu_page == 3 && button_pressed) {
    lcd.clear();
    menu_page = 0;
    menu_row = 1;
    button_pressed=false;
  }

  //RETURN FROM RTC MENU WITH BUTTON PRESS
  if (menu_page == 4 && button_pressed) {
    lcd.clear();
    menu_page = 0;
    menu_row = 1;
    button_pressed=false;
  }

  //SELECT MENU PAGE 1
  if (menu_page == 0 && menu_row == 1 && button_pressed){
    lcd.clear();
    menu_page = 1;
    menu_row = 1;
    button_pressed=false;
  }
  if (menu_page == 1 && menu_row == 1 && button_pressed){
    lcd.clear();
    menu_page = 1.1;
    menu_row = 1;
    button_pressed=false;
    refreshed = false;
  }
  if (menu_page == 1 && menu_row == 3 && button_pressed){
    lcd.clear();
    menu_page = 1.3;
    menu_row = 1;
    button_pressed=false;
  }
  
  if (menu_page == 0 && menu_row == 2 && button_pressed){
    lcd.clear();
    menu_page = 2;
    menu_row = 1;
    button_pressed=false;
  }
  if (menu_page == 0 && menu_row == 3 && button_pressed){
    lcd.clear();
    menu_page = 3;
    menu_row = 1;
    button_pressed=false;
  }
  
  //SELECT RTC MENU PAGE
  if (menu_page == 0 && menu_row == 4 && button_pressed){
    lcd.clear();
    menu_page = 4;
    menu_row = 1;
    button_pressed=false;
  }
  
  //+++++++ PAGE DISPLAY +++++++
  
  //MAIN MENU
  if (menu_page == 0){
    menu0();
  }

  //MENU PAGE 1
  if (menu_page == 1){
    menu1();
  }
  if (menu_page == 1.1){
    menu1_1();
  }
   if (menu_page == 1.3){
    menu1_3();
  }

  //MENU PAGE 2
  if (menu_page == 2){
    menu2();
  }

  //MENU PAGE 3
  if (menu_page == 3){
    menu3();
  }

  //RT CLOCK DISPLAY MENU
  if (menu_page == 4){
    menu4();
  }

  //CHANGE RELAY STATUSES IF NECESSARY
  RELAY_1();
  RELAY_2();
  RELAY_3();
  RELAY_4();
  relay_ctrl();

  //SAVE ROTARY ENCODER POSITION
  re_cnt_prev = re_counter;

  //LOOP DELAY
  delay(200);
}
//----------------------------LOOP END-------------------------------

//-------READ ANALOG INPUT SENSORS-------------------------
void READ_ANALOG_SENSORS(){
  //SCALE HERE IF NECESSARY
  SENS_0_VALUE = analogRead(SENS_0);
  SENS_1_VALUE = analogRead(SENS_1);
  SENS_2_VALUE = analogRead(SENS_2);
  SENS_3_VALUE = analogRead(SENS_3);
}


//-------RELAY CTRL----------------------------------------
void RELAY_1(){
  if(SENS_0_VALUE > set0){REL_1_ON=true;}else{REL_1_ON=false;}
}
void RELAY_2(){
  if(SENS_1_VALUE > set1){REL_2_ON=true;}else{REL_2_ON=false;}
}
void RELAY_3(){
  if(SENS_2_VALUE > set2){REL_3_ON=true;}else{REL_3_ON=false;}
}
void RELAY_4(){
  if(SENS_3_VALUE > set3){REL_4_ON=true;}else{REL_4_ON=false;}
}

void relay_ctrl(){
  if(REL_1_ON){digitalWrite(REL_1_PIN, HIGH);}
  else{digitalWrite(REL_1_PIN, LOW);}

  if(REL_2_ON){digitalWrite(REL_2_PIN, HIGH);}
  else{digitalWrite(REL_2_PIN, LOW);}

  if(REL_3_ON){digitalWrite(REL_3_PIN, HIGH);}
  else{digitalWrite(REL_3_PIN, LOW);}

  if(REL_4_ON){digitalWrite(REL_4_PIN, HIGH);}
  else{digitalWrite(REL_4_PIN, LOW);}
}


//-------POWER SAVE CTRL-----------------------------------
void powersave(){
  if (!POWERSAVE){pwr_cycles++;}
  if(!button_pressed && re_cnt_prev == re_counter && pwr_cycles > 500){
    POWERSAVE = true;
  } else {
    POWERSAVE = false;
  }
  if(button_pressed || re_cnt_prev != re_counter){pwr_cycles = 0;}
  
  //LCD BACKLIGHT ON / OFF
  if(POWERSAVE){lcd.noBacklight();}else{lcd.backlight();}
}

//-------SERIAL PRINTS FOR DEBUGGING-----------------------
void serial_prints(){
  Serial.print("menu_page=");
  Serial.print(menu_page);
  Serial.print(" menu_row=");
  Serial.println(menu_row);
  Serial.println(re_counter);
}

//--------------Clear selected line in LCD screen-----------------------
void clearLCDLine(int line)
{               
        lcd.setCursor(0,line);
        for(int n = 0; n < 20; n++) // 20 indicates symbols in line. For 4x20 LCD write - 20
        {
                lcd.print(" ");
        }
}

//--------DATETIME CTRL FOR REAL TIME CLOCK-----------------------------
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  //Serial.print(datestring);
  datetime = datestring;
}

void printhhmm(const RtcDateTime& dt)
{
  char timestring[6];

  snprintf_P(timestring,
             countof(timestring),
             PSTR("%02u:%02u"),
             dt.Hour(),
             dt.Minute());
  //Serial.print(datestring);
  timehhmm = timestring;
}

//------------read temperature from arduino internal chip sensor (arduino nano every. Ei toimi megalla!)---------------------
int getTemperature(void) {
VREF.CTRLA = VREF_ADC0REFSEL_1V1_gc;          // 1.1 volt reference
ADC0.CTRLC = ADC_SAMPCAP_bm + 3;              // VREF reference, correct clock divisor
ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc;        // Select temperature sensor
ADC0.CTRLD = 2 * (1 << ADC_INITDLY_gp);       // Initial delay of 32us 
ADC0.SAMPCTRL = 31;                           // Maximum length sample time (32us is desired) 
ADC0.COMMAND = 1;                             // Start the conversion 
while ((ADC0.INTFLAGS & ADC_RESRDY_bm) == 0) ;// wait for completion 

// The following code is based on the ATmega4809 data sheet
int8_t sigrow_offset = SIGROW.TEMPSENSE1;     // Read signed value from signature row 
uint8_t sigrow_gain = SIGROW.TEMPSENSE0;      // Read unsigned value from signature row 
uint16_t adc_reading = ADC0.RES;              // ADC conversion result with 1.1 V internal reference 
uint32_t temp = adc_reading - sigrow_offset; 
temp *= sigrow_gain;                          // Result might overflow 16 bit variable (10bit+8bit) 
temp += 0x80;                                 // Add 1/2 to get correct rounding on division below 
temp >>= 8;                                   // Divide result to get Kelvin
uint16_t temperature_in_K = temp;
return temperature_in_K - 273;                // Return Celsius temperature
}
