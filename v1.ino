
//Settings for YF-S201
//const int input = A5; // Pin the sensor is connected to
//float pipe_diameter = 12.7; // internal pipe diameter in mm
//int sample_time = 5; // number of seconds to average for the reading
//float unit_frequency = 7.5; //frequency from sensor data sheet

//Settings for YF-DN80
const int input = A5; // Pin the sensor is connected to
float pipe_diameter = 62.5; // internal pipe diameter in mm80 = default
int sample_time = 10; // number of seconds to average for the reading
float unit_frequency = .55; //frequency from sensor data sheet

// touch screen settings
// copy-paste results from TouchScreen_Calibr_native.ino
#define MINPRESSURE 100 //min press down pressure
#define MAXPRESSURE 1000 // max press down pressure

const int XP = 8, XM = A2, YP = A3, YM = 9; //ID=0x9341
const int TS_LEFT = 100, TS_RT = 903, TS_TOP = 78, TS_BOT = 888;


//global variables
unsigned long  X; //used for measuring the signal from sensor
unsigned long  Y; //used for measuring the signmal from sensor
float WATER = 0; //L/M
float V = 0; // KM/h
unsigned long startTime = 0; // when did the current reading start
unsigned long samples_taken = 0; // how many samples have been taken so far during the current time span
int pulse = 0;  
int lastPulse = 0;  
int rotations = 0;
unsigned long  readTime = 0;

//libraries
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>

// Touchscreen global variables
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_GFX_Button settings_btn, main_btn, pipe_diameter_p_btn,pipe_diameter_n_btn, sample_time_p_btn, sample_time_n_btn, sensor_f_p_btn, sensor_f_n_btn;
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


int pixel_x, pixel_y;     //Touch_getXY() updates global vars
int program_mode = 0; //0 is main screen without initial things drawn //is the main screen once initial stuff is drawn
bool initial = true;


bool Touch_getXY(void)
{

    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }

    return pressed;
}



void setup(void)
{
    uint16_t ID = tft.readID();
    tft.begin(ID);
    tft.setRotation(0);            //PORTRAIT
    tft.fillScreen(BLACK);
    pinMode(input, INPUT);
}


void loop(void)
{

    //use analog pin A5 to count rotations
    unsigned long startReadTime = millis();
    pulse = digitalRead(input);
    while(startReadTime - millis() <= 1000)
    {
      if (pulse != lastPulse) {
       lastPulse = pulse;
       if (pulse == HIGH) rotations++;
      } 
    }
    readTime = readTime + (millis()-startReadTime); // keep track of how long we have been taking 1 second readings for final calculations
    
   // test();
 
   bool down = Touch_getXY();
   settings_btn.press(down && settings_btn.contains(pixel_x, pixel_y));
   main_btn.press(down && main_btn.contains(pixel_x, pixel_y));
 
    if (settings_btn.justPressed()) 
    {
      program_mode = 1;
      initial=true;   
      settings_btn.drawButton(true);
    }
    if (main_btn.justPressed()) 
    {
      program_mode = 0;
      initial=true;
      main_btn.drawButton(true);
    }   
    switch (program_mode)
    {
      case 0:
          drawMainScreen();
      break;
      case 1:
          drawSettingsScreen();
      break;      
      default:
        drawMainScreen();
      break;
    }
    
}


void drawMainScreen(void)
{
    if(initial)
    {
      tft.fillScreen(BLACK);
      showmsgXY(20, 10, 3, NULL, "Velocity:");
      showmsgXY(20, 160, 3, NULL, "Flow Rate:");
      settings_btn.initButton(&tft,  60, 275, 70, 40, WHITE, CYAN, BLACK, "CFG_HOLD", 1);
      settings_btn.drawButton(false);
      main_btn.initButton(&tft,  185, 275, 70, 40, WHITE, CYAN, BLACK, "Main", 1);
      main_btn.drawButton(false);      
      rotations = 0;
    }
    if(startTime == 0)
      startTime = millis();

    unsigned long currentTime = millis();

    if( (currentTime - startTime)/1000 > sample_time-1)//sample time -1 due to the fact that the main loop waits a full second to take readings
    {

      WATER = (rotations/(readTime/1000))/unit_frequency; //L/M
      float conversion =  (1000000 / ((pipe_diameter/2)*(pipe_diameter/2) * 3.14) * 60)/1000000;
      V = conversion*WATER;
      String val = String(V);
      val  = val  + " KP/h";
      tft.fillRect(0,40,400,40,BLACK);
      showmsgXY(20,40,3,NULL,val);

      float conversion2 =  (1000000 / ((pipe_diameter/2)*(pipe_diameter/2) * 3.14) * 60)/1000000 *0.27778;
      V = conversion2*WATER;
      val = String(V);
      val  = val  + " MP/s";
      tft.fillRect(0,80,400,40,BLACK);
      showmsgXY(20,80,3,NULL,val);
 
      val = String(WATER);
      val  = val  + " L/m";
      tft.fillRect(0,200,400,40,BLACK);
      showmsgXY(20,200,3,NULL,val);
      startTime = millis();
      
      
      readTime = 0;
      rotations = 0;
    } 
   
    initial = false;
}

void drawSettingsScreen(void)
{
    String val;


    if(initial == true)
    {
      tft.fillScreen(BLACK);
      showmsgXY(20, 10, 2, NULL, "Pipe Dia (mm):");
      showmsgXY(20, 80, 2, NULL, "Sample Time:");      
      showmsgXY(20, 155, 2, NULL, "Sensor Frequency:");  
      
      settings_btn.initButton(&tft,  60, 275, 70, 40, WHITE, CYAN, BLACK, "CFG_HOLD", 1);
      settings_btn.drawButton(false);
      main_btn.initButton(&tft,  185, 275, 70, 40, WHITE, CYAN, BLACK, "Main", 1);
      main_btn.drawButton(false);    

      //pipe_diameter buttons
      pipe_diameter_p_btn.initButton(&tft, 30, 50, 30, 40, WHITE, CYAN, BLACK, "+", 4);
      pipe_diameter_p_btn.drawButton(false);
      pipe_diameter_n_btn.initButton(&tft,  200, 50, 30, 40, WHITE, CYAN, BLACK, "-", 4);
      pipe_diameter_n_btn.drawButton(false);    
      
      //sample_time buttons
      sample_time_p_btn.initButton(&tft,  30, 130, 30, 40, WHITE, CYAN, BLACK, "+", 4);
      sample_time_p_btn.drawButton(false);
      sample_time_n_btn.initButton(&tft,  200, 130, 30, 40, WHITE, CYAN, BLACK, "-", 4);
      sample_time_n_btn.drawButton(false);    

      //sensor frequency buttons
      sensor_f_p_btn.initButton(&tft,  30, 200, 30, 40, WHITE, CYAN, BLACK, "+", 4);
      sensor_f_p_btn.drawButton(false);
      sensor_f_n_btn.initButton(&tft,  200, 200, 30, 40, WHITE, CYAN, BLACK, "-", 4);
      sensor_f_n_btn.drawButton(false);    
    
      val = String(pipe_diameter,1);
      tft.fillRect(60,50,100,30,BLACK);
      showmsgXY(60,40,3,NULL,val);
      val = String(sample_time);
      tft.fillRect(60,120,100,30,BLACK);
      showmsgXY(60,120,3,NULL,val);
      val = String(unit_frequency);
      tft.fillRect(60,190,100,30,BLACK);
      showmsgXY(60,190,3,NULL,val);
      
      initial = false;
      
  }
  bool down = Touch_getXY();
  pipe_diameter_p_btn.press(down && pipe_diameter_p_btn.contains(pixel_x, pixel_y));
  pipe_diameter_n_btn.press(down && pipe_diameter_n_btn.contains(pixel_x, pixel_y));
  sample_time_p_btn.press(down && sample_time_p_btn.contains(pixel_x, pixel_y));
  sample_time_n_btn.press(down && sample_time_n_btn.contains(pixel_x, pixel_y));    
  sensor_f_p_btn.press(down && sensor_f_p_btn.contains(pixel_x, pixel_y));
  sensor_f_n_btn.press(down && sensor_f_n_btn.contains(pixel_x, pixel_y));   
       
  if (pipe_diameter_p_btn.justPressed()) 
  {
  
    pipe_diameter = pipe_diameter + .1;
    val = String(pipe_diameter,1);
      tft.fillRect(60,40,100,30,BLACK);
      showmsgXY(60,40,3,NULL,val);
      delay(50);
  
  }
  if (pipe_diameter_n_btn.justPressed()) 
  {
    if(pipe_diameter > 0 )
      pipe_diameter = pipe_diameter - .1;
      val = String(pipe_diameter,1);
      tft.fillRect(60,40,100,30,BLACK);
      showmsgXY(60,40,3,NULL,val);
      delay(50);
  }  
  
  if (sample_time_p_btn.justPressed()) 
  {
    sample_time++;
    val = String(sample_time);
    tft.fillRect(60,120,100,30,BLACK);
    showmsgXY(60,120,3,NULL,val);
     delay(50);
  }
  if (sample_time_n_btn.justPressed()) 
  {
    if(sample_time > 1)
      sample_time--;
    val = String(sample_time);
     tft.fillRect(60,120,100,30,BLACK);
    showmsgXY(60,120,3,NULL,val);
     delay(50);
  }    
  if (sensor_f_p_btn.justPressed()) 
  {
    unit_frequency = unit_frequency + .01;
    val = String(unit_frequency);
    tft.fillRect(60,190,100,30,BLACK);
    showmsgXY(60,190,3,NULL,val);
     delay(50);
  }
  if (sensor_f_n_btn.justPressed()) 
  {
    if(unit_frequency > 1)
      unit_frequency = unit_frequency -.01;
    val = String(unit_frequency);
     tft.fillRect(60,190,100,30,BLACK);
    showmsgXY(60,190,3,NULL,val);
    delay(50);
  }   
  
}



void showmsgXY(int x, int y, int sz, const GFXfont *f, float msg)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  // tft.drawFastHLine(0, y, tft.width(), WHITE);
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(GREEN);
  tft.setTextSize(sz);
  tft.print(msg);
}

void showmsgXY(int x, int y, int sz, const GFXfont *f, int msg)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  // tft.drawFastHLine(0, y, tft.width(), WHITE);
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(GREEN);
  tft.setTextSize(sz);
  tft.print(msg);
}

void showmsgXY(int x, int y, int sz, const GFXfont *f, String msg)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  // tft.drawFastHLine(0, y, tft.width(), WHITE);
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(GREEN);
  tft.setTextSize(sz);
  tft.print(msg);
}

void showmsgXY(int x, int y, int sz, const GFXfont *f, unsigned long msg)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  // tft.drawFastHLine(0, y, tft.width(), WHITE);
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(GREEN);
  tft.setTextSize(sz);
  tft.print(msg);
}
