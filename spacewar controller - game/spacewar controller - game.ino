#include "TFT_eSPI.h"
#include "picture.h"
#include "ship.h"
#include "enemy.h"
#include "rocket.h"
#include "rocketH.h"
#include "start.h"
#include <Bluepad32.h>

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

TFT_eSPI tft= TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);
TFT_eSprite player = TFT_eSprite(&tft);
TFT_eSprite enemy = TFT_eSprite(&tft);
TFT_eSprite stats = TFT_eSprite(&tft);


#define gray 0xA514

int imageW=480;
int imageH=480;
int screenW=320;
int screenH=140;
int m=imageW;

int start=1;
unsigned short imageS[44800]={0}; // edit this screenW * screen H

int pos=0;
int x=0;
int y=30;
int changeX=1;
int changeY=1;

int ex=0;
int ey=0;
int ecX=1;
int ecY=1;

#define LED_PIN 16

#define left 21
#define up 17
#define down 18
#define right 44
#define a LED_PIN
#define b 43
#define color   0x0042
int px=30;
int py=80;
int w=63;
int h=28;

bool bullet[10]={0}; // my bulets
int bx[10]={0};    // position x of each bullet
int by[10]={0};    //position y of each bullet
int bullet_counter=0;
int bdamage=4;

bool rocket[3]={0};
int rx[3]={0};
int ry[3]={0};

int rdamage=20;

bool ebullet[8]={0};
int ebx[10]={0};    // position x of each Enemy bullet
int eby[10]={0};    //position y of each Enemy bullet
int ebullet_counter=0;
int edamage=10;

int debounceA=0;
int debounceB=0;

bool e=0;

int level=1;
int score=0;
int rocketN=2;
long tt=0;
int health=100;
int shield=100;
int enemyHealth=100;
int miss=0;
int hit=0;
int headshot=0;
int Min=0;
int Sec=0;
long Time=0;
bool gameOver=0;

String Minutes;
String Seconds;


void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
        "misc: 0x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d\n",
        ctl->index(),        // Controller Index
        ctl->dpad(),         // D-pad
        ctl->buttons(),      // bitmask of pressed buttons
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY(),       // (-511 - 512) right Y axis
        ctl->brake(),        // (0 - 1023): brake button
        ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
        ctl->miscButtons(),  // bitmask of pressed "misc" buttons
        ctl->gyroX(),        // Gyro X
        ctl->gyroY(),        // Gyro Y
        ctl->gyroZ(),        // Gyro Z
        ctl->accelX(),       // Accelerometer X
        ctl->accelY(),       // Accelerometer Y
        ctl->accelZ()        // Accelerometer Z
    );
}

void processGamepad(ControllerPtr ctl) {
    // Movimento da nave com DPAD
    if (ctl->dpad() & DPAD_UP && py > 0)
        py -= 3;
    if (ctl->dpad() & DPAD_DOWN && py < 140 - 28)
        py += 3;
    if (ctl->dpad() & DPAD_LEFT && px > 0)
        px -= 3;
    if (ctl->dpad() & DPAD_RIGHT && px < 290)
        px += 3;

    // Disparo de projétil com botão A
    if (ctl->buttons() & 0x0001) {
        digitalWrite(LED_PIN, HIGH);  // Liga LED (efeito visual)

        if (debounceA == 0) {
            debounceA = 1;
            bullet[bullet_counter] = true;
            bx[bullet_counter] = px + w;
            by[bullet_counter] = py + h / 2;
            bullet_counter++;
            if (bullet_counter == 10) bullet_counter = 0;
        }
    } else {
        digitalWrite(LED_PIN, LOW);   // Desliga LED
        debounceA = 0;
    }

    // Disparo de foguete com botão B
    if (ctl->buttons() & 0x0002 && debounceB == 0 && rocketN >= 0) {
        debounceB = 1;
        rocket[rocketN] = true;
        rx[rocketN] = px + w;
        ry[rocketN] = py + h / 2;
        rocketN--;
    } else if (!(ctl->buttons() & 0x0002)) {
        debounceB = 0;
    }

    // Apenas se quiser ver os dados no Serial
        dumpGamepad(ctl);
}

void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            } else {
                Serial.println("Unsupported controller");
            }
        }
    }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
  BP32.enableVirtualDevice(false);

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.setTextColor(TFT_SILVER, color);

  tft.drawString("T-DISPLAY S3", 5, 162);
  tft.drawLine(0, 19, 320, 19, TFT_WHITE);
  tft.drawLine(0, 160, 320, 160, TFT_WHITE);

  sprite.createSprite(320, 140);
  sprite.setSwapBytes(true);
  player.createSprite(63, 40);
  player.setTextColor(TFT_WHITE, TFT_BLACK);
  enemy.createSprite(aniWidth, aniHeigth);
  stats.createSprite(320, 19);
  stats.setSwapBytes(true);

  ey = random(20, 50);
  ex = random(190, 250);

  tft.pushImage(0, 0, 320, 170, startScreen);
  while (digitalRead(a) == 1) {}  // Manter botão A como "Start"
  tft.fillScreen(color);
}

void drawBackground()
{
    pos=x+imageW*y;
  start=pos;
  m=screenW+pos;
  for(int i=0;i<screenW*screenH;i++)
  {
  if(start%m==0)
  {
  start=start+(imageW-screenW);
  m=m+imageW;
  }
  imageS[i]=picture[start];
  start++;
  
  }
  x=x+changeX;
  if(x==imageW-screenW-1 || x<0)
  changeX=changeX*-1;

   y=y+changeY;
  if(y==imageH-screenH-1 || y<0)
  changeY=changeY*-1;

    ex=ex+ecX;
  if(ex==250 || ex<180)
  ecX=ecX*-1;

   ey=ey+ecY;
  if(ey==90 || ey<-20)
  ecY=ecY*-1;
  
  }

void showStats()
{
  stats.fillSprite(color);
   stats.setTextColor(gray,color);
  stats.drawString("SHIELD:",4,2,2);

  stats.drawRect(56,4,54,11,gray);
  stats.fillRect(58,6,map(health,0,100,0,50),7,TFT_GREEN);

  for(int i=0;i<rocketN+1;i++)
  stats.pushImage(120+(i*8)+(i*3),3,7,12,rrocket);

  
  stats.drawString("LVL:      SCORE:",164,2,2);

  stats.setTextColor(0x7BF,color);
  stats.drawString(String(level),194,2,2);
  stats.drawString(String(score),276,2,2);
  }

  void checkColision()
  {
    for(int i=0;i<8;i++)
    {
    if(ebullet[i]==true)
    if(ebx[i]>px && ebx[i]<px+58 && eby[i]>py && eby[i]<py+27)
        {
          health=health-edamage;
          ebullet[i]=false;
        }  
    }

     for(int i=0;i<10;i++)
    {
    if(bullet[i]==true)
    if(bx[i]>ex+10 && bx[i]<ex+62 && by[i]>ey && by[i]<ey+66)
        {  
           if(by[i]>ey+25 && by[i]<ey+25+18)
           {score=score+12; headshot++;
           enemyHealth=enemyHealth-3;}else
           {score=score+10;
           enemyHealth=enemyHealth-2;}
          hit++;
          
          bullet[i]=false;
        }  
    }

      for(int i=0;i<3;i++)
    {
    if(rocket[i]==true)
    if(rx[i]>ex+10 && rx[i]<ex+62 && ry[i]>ey && ry[i]<ey+66)
        {  
          
          score=score+16;
          
          hit++;
          enemyHealth=enemyHealth-12;
          rocket[i]=false;
        }  
    }
    
    
    }

int frameC=0;

long timeLast=0;
long timeNext=1000;

void reset()
{
 level=1;
 score=0;
 rocketN=2;
 tt=0;
 health=100;
 shield=100;
 enemyHealth=100;
 edamage=10;
 miss=0;
 hit=0;
 headshot=0;
 Min=0;
 Sec=0;
 Time=0;
 ey=random(20,50);
 ex=random(190,250);
  gameOver=0;
  }

void loop() {

bool controllerConnected = false;
for (auto ctl : myControllers) {
    if (ctl && ctl->isConnected()) {
        controllerConnected = true;
        break;
    }
}

sprite.setTextColor(controllerConnected ? TFT_GREEN : TFT_RED, TFT_BLACK);
sprite.drawString(controllerConnected ? "Controller OK" : "No Controller", 10, 10);

bool dataUpdated = BP32.update();
if (dataUpdated)
    processControllers();

delay(10);

  if(digitalRead(0)==0)
  reset();
 showStats();
 checkColision();
 
  //check time
  if(Time+1000<millis())
  {Time=millis();
  Sec++; 
  if(Sec>59)
  {Sec=0; Min++;}
  if(Sec<10) Seconds="0"+String(Sec); else Seconds=String(Sec);
  if(Min<10) Minutes="0"+String(Min); else Minutes=String(Min);
  
  }

  if(timeLast+timeNext<millis())
  {
      timeLast=millis();
      timeNext=(random(6,15)*100)-level*30;
      ebullet[ebullet_counter]=true;
      ebx[ebullet_counter]=ex+aniWidth/2;
      eby[ebullet_counter]=ey+aniHeigth/2;
      ebullet_counter++;
      if(ebullet_counter==8)
      ebullet_counter=0;
    }


  if(e==0){
  drawBackground();
  frameC++;
  }

  e=!e;
  sprite.pushImage(0,0,screenW,screenH,imageS);
 
  player.pushImage(0,0,w,h,ship);
  for(int i=0;i<8;i++)
  player.fillCircle(random(0,10),random(9,18),1,TFT_RED);
  player.drawString(String(health)+"%",16,32);
  enemy.pushImage(0,0,aniWidth,aniHeigth,logo2[frameC]);
  
  //
  

   player.pushToSprite(&sprite,px,py,TFT_BLACK);
  enemy.pushToSprite(&sprite,ex,ey,TFT_BLACK);
  
  for(int i=0;i<10;i++)
  if(bullet[i]==true){
     bx[i]=bx[i]+4;
  if(bx[i]<322)
  sprite.fillCircle(bx[i],by[i],2,0xEA5B); /// player bullets........................................
  else{
  miss++; 
  
  bullet[i]=false;}}

   for(int i=0;i<8;i++)
  if(ebullet[i]==true){
     ebx[i]=ebx[i]-4;
  if(ebx[i]>-4)
  sprite.fillCircle(ebx[i],eby[i],3,0x7BF);
  else
  ebullet[i]=false;}

    for(int i=0;i<3;i++)
  if(rocket[i]==true){
     rx[i]=rx[i]+2;
  if(rx[i]<320)
  sprite.pushImage(rx[i],ry[i]-6,12,7,rocketH);
  else
  rocket[i]=false;}
  

  sprite.drawRoundRect(268,8,44,14,3,gray);

  sprite.setTextColor(gray,TFT_BLACK);
  sprite.drawString(Minutes+":"+Seconds,276,11);
  sprite.drawString("HEADS:"+String(headshot),254,128);
  sprite.drawString("HIT :"+String(hit),254,104);
  sprite.drawString("MISS :"+String(miss),254,116);
  sprite.drawString("ENEMY HEALTH:",8,128);
  sprite.fillRect(92,129,map(enemyHealth,0,100,0,50),5,0xF4DE); //..............enemy healthBAr
  sprite.drawRect(90,127,54,9,gray); //..............enemy healthBAr
  sprite.drawLine(0,0,320,0,0x4C7A); 
  sprite.drawLine(0,139,320,139,0x4C7A);
 
  sprite.pushSprite(0,20);
  stats.pushSprite(0,0);

  if(health<=0)
  {
  tft.fillScreen(color);
  tft.drawString("GAME OVER",40,40,4);
  tft.drawString("SCORE: "+String(score),40,65,2);
  tft.drawString("LEVEL: "+String(level),40,85,2);
  tft.drawString("TIME: "+Minutes+":"+Seconds,40,105,2);
  tft.drawString("HEADSHOT: "+String(headshot),234,128);
  tft.drawString("HIT :"+String(hit),234,104);
  tft.drawString("MISS :"+String(miss),234,116);
  
  delay(5000);
  while(digitalRead(a)==1)
  {}  
  reset();
    
  }
  if(enemyHealth<=0)
  {
    level++;
    enemyHealth=100+(15*level);
    rocketN=2;
    edamage++;
    tft.fillScreen(color);
    tft.drawString("LEVEL "+String(level),80,80,4);
    delay(3000);
    }
   
   if(frameC==framesNumber-1)
   frameC=0;
}
