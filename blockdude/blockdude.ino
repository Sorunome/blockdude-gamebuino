#include <SPI.h>
#include <Gamebuino.h>
#include <EEPROM.h>
Gamebuino gb;
/*
 * 0 - nothing
 * 1 - block
 * 2 - start position
 * 3 - end position
 * 4 - wall
 */
extern const byte font3x5[];
const byte wall[] PROGMEM = {8,8,0xFB,0xFB,0x00,0xFE,0xFE,0x00,0xFB,0xFB};
const byte door[] PROGMEM = {8,8,0x7E,0x42,0x42,0x42,0x46,0x42,0x42,0x7E};
const byte block[] PROGMEM = {8,8,0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0xFF};
const byte dude_left[] PROGMEM = {8,8,0x1C,0x7E,0x12,0x22,0x14,0x2A,0x08,0x36};
const byte dude_right[] PROGMEM = {8,8,0x38,0x7E,0x48,0x44,0x28,0x54,0x10,0x6C};
const byte ok[] PROGMEM = {8,7,0x2,0x4,0x88,0x48,0x50,0x30,0x20}; // from bub
const byte ko[] PROGMEM = {8,7,0x82,0x44,0x28,0x10,0x28,0x44,0x82}; // from bub
const byte PROGMEM logo[] = {64,35,0xFF,0x80,0xF8,0x00,0x00,0x00,0x00,0x00,0xFF,0x80,0xF8,0x00,0x00,0x00,0x00,0x00,0xDD,0x80,0xD8,0x00,0x00,0x00,0x00,0x00,0xFF,0x80,0xF8,0x00,0x00,0x00,0x00,0x00,0xFF,0xF8,0xF8,0xFF,0xF8,0xFF,0x9F,0x1F,0xF8,0xF8,0xF8,0xFF,0xF8,0xFF,0x9F,0x1F,0xD8,0xD8,0xD8,0xDD,0xD8,0xDB,0x9B,0x1B,0xF8,0xF8,0xF8,0xFF,0xF8,0xFF,0x9F,0x1F,0xFF,0xF8,0xF8,0xFF,0xF8,0xFF,0x9F,0xFF,0xFF,0x80,0xF8,0xF8,0xF8,0xF8,0x1F,0xF0,0xDD,0x80,0xD8,0xD8,0xD8,0xD8,0x1B,0xB0,0xFF,0x80,0xF8,0xF8,0xF8,0xF8,0x1F,0xF0,0xFF,0xF8,0xF8,0xFF,0xF8,0xFF,0x9F,0xFF,0xF8,0xF8,0xF8,0xFF,0xF8,0xFF,0x9F,0x1F,0xD8,0xD8,0xD8,0xDD,0xD8,0xDD,0x9B,0x1B,0xF8,0xF8,0xF8,0xFF,0xF8,0xFF,0x9F,0x1F,0xFF,0xF8,0xF8,0xFF,0xF8,0xFF,0x9F,0x1F,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0xDD,0x80,0x00,0x00,0xF0,0x00,0x00,0x00,0xFF,0x80,0x00,0x01,0xD8,0x00,0x00,0x00,0xFF,0x80,0x00,0x00,0x4C,0x22,0x0C,0x00,0x00,0x00,0x00,0x00,0x44,0x13,0x14,0x00,0x00,0x0F,0xF1,0xC0,0x25,0x09,0x98,0x3F,0x00,0x08,0x13,0xF0,0x25,0x8A,0x51,0x21,0x00,0x08,0x12,0x40,0x2C,0xCA,0x6E,0x21,0x00,0x08,0x12,0x20,0x18,0x71,0x80,0x21,0x00,0x08,0x11,0x40,0x30,0x00,0x00,0x23,0x00,0x08,0x12,0xA0,0x40,0x00,0x00,0x21,0x00,0x08,0x10,0x80,0x00,0x00,0x00,0x21,0x00,0x0F,0xF3,0x60,0x00,0x00,0x00,0x3F,0xDF,0xDF,0xDF,0xDC,0x03,0x7F,0x7F,0x7F,0xDF,0xDF,0xDF,0xDC,0x03,0x7F,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0xFE,0xFE,0xFC,0x03,0xFB,0xFB,0xFB,0xFE,0xFE,0xFE,0xFC,0x03,0xFB,0xFB,0xFB};
extern const byte *gamemaps[];
const byte *gamemap;
byte curlevel = 0;
int mapX;
int mapY;
byte mapHeight;
byte mapWidth;
byte numBlocks = 0;
byte playerX;
byte playerY;
byte liftBlock;
bool doLift = false;
bool lookLeft = true;

byte getTile(byte x,byte y){
  return pgm_read_byte(gamemap+y*mapWidth+x);
}
class Block {
  byte x,y;
  bool islift;
  public:
    Block(byte bx,byte by){
      x = bx;
      y = by;
      islift = false;
    }
    void draw(){
      if(islift){ // we let other stuff handle this as we will be on player
        return;
      }
      int drawX = x*8-mapX;
      int drawY = y*8-mapY;
      if(drawX > 86 || drawY > 68 || drawX < -8 || drawY < -8){
        return;
      }
      gb.display.drawBitmap(drawX,drawY,(const byte*)block);
    }
    bool is(byte ix,byte iy){
      return !islift && ix==x && iy==y;
    }
    void lift(){
      islift = true;
    }
    void put(byte nx,byte ny){
      for(;canMove(nx,ny+1);ny++);
      x = nx;
      y = ny;
      islift = false;
    }
};
Block *blocks[70];
void loadLevel(){
  gb.display.persistence = false;
  gamemap = gamemaps[curlevel];
  mapWidth = pgm_read_byte(gamemap);
  mapHeight = pgm_read_byte(gamemap+1);
  gamemap += 2;
  for(byte i = 0;i < numBlocks;i++){
    delete blocks[i];
  }
  numBlocks = 0;
  for(byte y = 0;y < mapHeight;y++){
    for(byte x = 0;x < mapWidth;x++){
      byte tile = getTile(x,y);
      if(tile == 2){
        mapX = 8*(x-5) + 2; // it works, OK!?!?!?!
        mapY = 8*(y-2) - 4;
        playerX = x;
        playerY = y;
      }else if(tile == 1){
        blocks[numBlocks++] = new Block(x,y);
      }
    }
  }
}
void drawWorld(){
  for(byte y = 0;y < mapHeight;y++){
    for(byte x = 0;x < mapWidth;x++){
      int drawX = x*8-mapX;
      int drawY = y*8-mapY;
      if(drawX > 86 || drawY > 68 || drawX < -8 || drawY < -8){
        continue;
      }
      byte tile = getTile(x,y);
      if(tile == 3){
        gb.display.drawBitmap(drawX,drawY,(const byte*)door);
      }else if(tile == 4){
        gb.display.drawBitmap(drawX,drawY,(const byte*)wall);
      }
    }
  }
  for(byte i = 0;i < numBlocks;i++){
    blocks[i]->draw();
  }
  int drawX = playerX*8-mapX;
  int drawY = playerY*8-mapY;
  if(!(drawX > 86 || drawY > 68 || drawX < -8 || drawY < -8)){
    gb.display.drawBitmap(drawX,drawY,(lookLeft?(const byte*)dude_left:(const byte*)dude_right));
  }
  drawY -= 8;
  if(doLift && !(drawX > 86 || drawY > 68 || drawX < -8 || drawY < -8)){
    gb.display.drawBitmap(drawX,drawY,(const byte*)block);
  }
}
bool canMove(byte cx,byte cy){
  if(getTile(cx,cy) == 4){
    return false;
  }
  for(byte i = 0;i < numBlocks;i++){
    if(blocks[i]->is(cx,cy)){
      return false;
    }
  }
  return true;
}
void moveWorld(){
  drawWorld();
  while(1){
    if(gb.update()){
      if(gb.buttons.released(BTN_A)){
        return;
      }
      if(gb.buttons.pressed(BTN_LEFT)){
        mapX -= 8;
      }
      if(gb.buttons.pressed(BTN_RIGHT)){
        mapX += 8;
      }
      if(gb.buttons.pressed(BTN_UP)){
        mapY -= 8;
      }
      if(gb.buttons.pressed(BTN_DOWN)){
        mapY += 8;
      }
      drawWorld();
    }
  }
}
unsigned int moves;
void climb(){
  byte tmpX = playerX+(lookLeft?-1:1);
  if(!canMove(tmpX,playerY) && canMove(tmpX,playerY-1) && canMove(playerX,playerY-1)  && (!doLift || (canMove(tmpX,playerY-2) && canMove(playerX,playerY-2)))){
    playerY--;
    playerX = tmpX;
    mapY -= 8;
    mapX = mapX+(lookLeft?-8:8);
    gb.sound.playTick();
    moves++;
  }
}
int playMap(){
  moves = 0;
  while(1){
    if(gb.update()){
      if(gb.buttons.pressed(BTN_LEFT)){
        if(canMove(playerX-1,playerY)){
          if(doLift && !canMove(playerX-1,playerY-1)){
            blocks[liftBlock]->put(playerX,playerY-1);
            doLift = false;
          }
          playerX--;
          mapX -= 8;
          gb.sound.playTick();
          moves++;
        }else if(lookLeft){
          climb();
        }
        lookLeft = true;
      }
      if(gb.buttons.pressed(BTN_RIGHT)){
        if(canMove(playerX+1,playerY)){
          if(doLift && !canMove(playerX+1,playerY-1)){
            blocks[liftBlock]->put(playerX,playerY-1);
            doLift = false;
          }
          playerX++;
          mapX += 8;
          gb.sound.playTick();
          moves++;
        }else if(!lookLeft){
          climb();
        }
        lookLeft = false;
      }
      if(gb.buttons.pressed(BTN_DOWN)){
        if(doLift){
          byte tmpX = playerX+(lookLeft?-1:1);
          if(canMove(tmpX,playerY-1)){
            blocks[liftBlock]->put(tmpX,playerY-1);
            doLift = false;
            gb.sound.playOK();
            moves++;
          }
        }else{
          byte tmpX = playerX+(lookLeft?-1:1);
          if(canMove(tmpX,playerY-1) && canMove(playerX,playerY-1)){
            for(byte i = 0;i < numBlocks;i++){
              if(blocks[i]->is(lookLeft?playerX-1:playerX+1,playerY)){
                doLift = true;
                blocks[i]->lift();
                liftBlock = i;
                gb.sound.playOK();
                moves++;
                break;
              }
            }
          }
        }
      }
      if(gb.buttons.pressed(BTN_UP)){
        climb();
      }
      if(gb.buttons.pressed(BTN_A)){
        int oldMapX = mapX;
        int oldMapY = mapY;
        moveWorld();
        mapX = oldMapX;
        mapY = oldMapY;
      }
      if(gb.buttons.held(BTN_B,10)){
        loadLevel();
        gb.sound.playCancel();
        doLift = false;
        moves = 0;
      }
      if(gb.buttons.pressed(BTN_C)){
        gb.sound.playCancel();
        return 0;
      }
      if(getTile(playerX,playerY) == 3){
        return moves;
      }
      for(;canMove(playerX,playerY+1);){
        mapY += 8;
        playerY++;
        if(getTile(playerX,playerY) == 3){
          return moves;
        }
      }
      drawWorld();
    }
  }
}
void drawLevelMenu(){
  gb.display.persistence = true;
  gb.display.clear();
  gb.display.cursorX = 0;
  gb.display.cursorY = 0;
  gb.display.setFont(font3x5);
  gb.display.print("Level Menu");
  gb.display.print("\n\nLevel  \x11  \x10");
  refreshLevelMenu(curlevel+1);
}
void refreshLevelMenu(byte curPick){
  gb.display.cursorX = 32;
  gb.display.cursorY = 12;
  if(curPick < 10){
    gb.display.print(" ");
  }
  gb.display.print(curPick);
  unsigned int moves = EEPROM.read((curPick-1)*2)*256 + EEPROM.read((curPick-1)*2 + 1);
  gb.display.setColor(WHITE);
  gb.display.fillRect(48,11,10,10); // erase old icon
  gb.display.fillRect(0,24,48,5); // erase old moves display
  gb.display.setColor(BLACK);
  gb.display.drawBitmap(48,11,moves>0?ok:ko);
  if(moves > 0){
    gb.display.cursorX = 0;
    gb.display.cursorY = 24;
    gb.display.print("Moves  ");
    gb.display.print(moves);
  }
}
void chooseLevel(){
  byte curPick = curlevel+1;
  drawLevelMenu();
  while(1){
    if(gb.update()){
      if(gb.buttons.pressed(BTN_C)){
        gb.titleScreen(logo);
        while(1){ // wait until the A key is released
          if(gb.update()){
            if(!gb.buttons.repeat(BTN_A,0)){
              break;
            }
          }
        }
        drawLevelMenu();
      }
      if(gb.buttons.pressed(BTN_RIGHT)){
        curPick++;
        if(curPick > 11){
          curPick = 11;
        }
        refreshLevelMenu(curPick);
      }
      if(gb.buttons.pressed(BTN_LEFT)){
        curPick--;
        if(curPick < 1){
          curPick = 1;
        }
        refreshLevelMenu(curPick);
      }
      if(gb.buttons.pressed(BTN_A)){
        break;
      }
    }
  }
  curlevel = curPick-1;
}
void setup() {
  gb.begin();
  gb.titleScreen(logo);
}
void loop() {
  chooseLevel();
  while(1){
    loadLevel();
    unsigned int moves = playMap();
    if(moves > 0){
      unsigned int oldMoves = EEPROM.read(curlevel*2)*256 + EEPROM.read(curlevel*2 + 1);
      if(moves < oldMoves || oldMoves == 0){
        EEPROM.write(curlevel*2,moves>>8);
        EEPROM.write(curlevel*2 + 1,moves&0xFF);
      }
      if(curlevel < 11){
        curlevel++;
        continue;
      }
    }
    break;
  }
}
