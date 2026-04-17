
#include "as_tpdisplay.h"

#include "tplinkconf.h"

#ifdef MIT_OLEDDISP
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void disp_init(){
  Wire.setClock(100000);
  if(display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();
    delay(2000); // Pause for 2 seconds
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    
    display.cp437(true);         // Use full 256 char 'Code Page 437' font

    disp_clear();
    disp_flush();

  }
}
void disp_clear(){
  display.clearDisplay();
  
}
void disp_flush(){
  display.display();
}
void disp_clearLine(int line){
  display.setCursor(0, line*8); 
  display.fillRect( 0, line*8, 128,8, 0);
}
void disp_writeLine(char * txt, int line){
  display.setCursor(0, line*8);  
  display.write(txt);
}


#endif
