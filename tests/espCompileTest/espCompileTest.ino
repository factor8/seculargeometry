
#include <SPI.h>
#include <Adafruit_WS2801.h>

#include "driver.h"

#define DATAPIN   13 // Data pin for serial communication to shift registers
#define CLOCKPIN  14 // Clock pin for serial communication to shift registers
uint8_t TOUCHPIN = 4;	// Touch sensitive switch

int pixelsTotal = 20;
boolean DEBUG = true;
boolean automatic = 0;
uint8_t _brightness = 20;
unsigned long effectDuration = 20000;

Adafruit_WS2801* strip = new Adafruit_WS2801(pixelsTotal, DATAPIN, CLOCKPIN);

void setup() {

}

void loop() {

}
void configure() {
	if (!loadConfigFile()) {

		// Setup
		uint8_t e = 0;
		// effects.push_back(new ColorWipe(pixelsTotal));
				
		// effects.push_back(new ColorWipe(pixelsTotal));
		// effects[e]->updatePrimary(SGEffect::color(0,0,0));		
		// effects[e]->updateFrequency(100);
		// e++;

		// effects.push_back(new RainbowCycle(pixelsTotal));
		// effects[e]->updatePrimary(SGEffect::color(0,255,0));		
		// effects[e]->updateFrequency(100);
		// e++;

		// effects.push_back(new Rainbow(pixelsTotal));
		// effects[e]->updatePrimary(SGEffect::color(0,255,0));		
		// effects[e]->updateFrequency(100);
		// e++;

		// effects.push_back(new RainbowCycle(pixelsTotal));
		// effects[e]->updatePrimary(SGEffect::color(0,255,0));
		// effects[e]->updateFrequency(800);
		// e++;

		// effects.push_back(new ColorCycle(pixelsTotal));
	 //    Vector<uint32_t> f = effects[e-1]->exportFrame();
	 //    effects[e]->importFrame(f);
	 //    effects[e]->updateFrequency(300);
	 //    e++;

		// effects.push_back(new Scanner(pixelsTotal));
		// effects[e]->updatePrimary(SGEffect::color(0,255,0));
		// effects[e]->updateFrequency(100);
		// e++;

		Serial.print(F("Effect Count: "));Serial.println(effects.size());
		
		// Effect Template
		// effects.push_back(new ColorWipe(pixelsTotal));
		// effects[e]->updatePrimary(SGEffect::color(0,255,0));		
		// effects[e]->updateFrequency(100);
		// e++;
	}
}

void init(int pixelsTotal) {
  Serial.begin(115200);
  Serial.println("Starting up...");
  
  configure();

  strip->begin();
  strip->show();
  
  effects[_selector]->activate();

  _futureSelector = 1;
}


boolean loadConfigFile() {
  return false;
}

void sSetSelector() {
  uint8_t s = atoi(messageBuffer); 
  setSelector(s);
}
void setSelector(uint8_t s) {
  // if (messageBuffer) /// do we need checking here? it's all going to change eventually.  

  _futureSelector = s;

  if (DEBUG) Serial.print(F("User updated selector: "));Serial.println(_futureSelector); /// Should this be future_selector?
  
}

void sSetFrequency() {
  uint32_t s = atoi(messageBuffer);
  effects[_selector]->updateFrequency(s);
  if (DEBUG) Serial.print(F("User updated effectMS: "));Serial.println(s);
}

void sSetBrightness() {
  uint16_t b = atoi(messageBuffer);  
  setBrightness(b);
  if (DEBUG) {Serial.print(F("User updated brightness to: "));Serial.println(b);}
}
void setBrightness(uint8_t b) {  
  // add some bounds checks here
  if (b <= 100 && b>=0) { 
    _brightness = b;
  }
}

// Serial Functions
void interceptSerial(char x) {
  
  if        ( x == '!' )    {   readMode  = 1;    }         //Set Selector
  else if   ( x == '@' )    {   readMode  = 2;    }         //Set Frequency
  else if   ( x == '#' )    {   readMode  = 3;    }         //Set 
  else if   ( x == '+' )    {   readMode  = 4;    }         //Shift Register IDs, separated by comma (no whitespace)
  else if   ( x == '-' )    {   readMode  = 5;    }         //Shift Register IDs, separated by comma (no whitespace)
  else if   ( x == '~' )    {   readMode  = 6;    }         //System Mode 
  else if   ( x == '*' )    {   readMode  = 7;    }         //System Mode   
  else if   ( x == '?' )    {   readMode  = 8;    }         //Status Update
  // else if    ( x == '/' )    {   getFiles();       }   
  // else if    ( x == '?' )    {   statusUpdate();   }     
  //Add custom flags here...
  
  //Finish up
  else if   (x == '.')    {   //...
    
    //This will update the global variables accordingly.
    switch(readMode){
      case 1:      sSetSelector();        break;
      case 2:      sSetFrequency();       break;
      case 3:      sSetBrightness();      break;
      case 4:      toggleAutomatic();     break;
      // case 5:      setValveOff();      break;
      // case 6:      setMode();          break;      
      case 7:      toggleDebug();         break;
      case 8:      statusUpdate();        break;
      default:                            break;  
    }
    
      // lastSerialCMD = now;         //Used for switching to autoPilot
      readMode = 0;                   //We're done reading. (until another.)
      bufferIndex = 0;
      resetMessageBuffer();         
  }
  else { messageBuffer[bufferIndex++] = x; }        //Magic.

}

void interceptTouch(int TOUCHPIN) {

  if (!touching) {
      if (readCapacitivePin(TOUCHPIN) < 3) {      
        return; 
      } else {
        // First Touch!
        if (DEBUG) Serial.println(F("First Touch!"));
        touching = true;
        firstTouch = now;
      }
  }  

  // How long have we been touching?
  touchDuration = now - firstTouch;

  // We've stopped touching now so...
  if (readCapacitivePin(TOUCHPIN) < 2) {
      // Are we overtime?
      if (overtimeTouch) {
        if (DEBUG) Serial.println(F("Ending overtime touch."));
        touching = false; overtimeTouch = false; return;
      }    
   
      // If it was not an accidental touch...
      if (touchDuration > 50) {
     
        // Debounce
        if (now - lastTouchAction > 300) {
         
            // Do touch action
            // if (automatic == 0) {
              // toggleAutomatic();
            // }
             
            // Change mode.
            _futureSelector = _selector+1; /// messy

            if (DEBUG) Serial.println(F("User activated mode change."));

            lastTouchAction = now;
        } else {
            if (DEBUG) Serial.println(F("Debounce!"));
        }
        
        touching = false;
       
        return;
      }  

      // Accidental touch, take no action.
      if (DEBUG) Serial.println(readCapacitivePin(TOUCHPIN));
      if (DEBUG) Serial.println(F("Short Touch - No Action"));
      touching = false;
      return;
   
    // Or we haven't so check some other stuff...
   } else {

      // Like if we are overtime on our touch
      if (touchDuration >= 1000) { 
       
        if (!overtimeTouch) {
            // Overtime triggered.
            if (DEBUG) Serial.println(F("Overtime Triggered"));
         
            // Do hold action
            
            // Tufrn automatic off.
            if (!automatic) {
              toggleAutomatic();

              // Notify that we are activated
              flash();
              delay(100);
              flash();          
           
            } else {          
              // Turn the lamp off.
              _futureSelector = 0;
              toggleAutomatic();
            }

            overtimeTouch = true;
        }   
      }
  }
}

void toggleAutomatic() {
  if (automatic) {
    automatic = false;
    if (DEBUG) {Serial.print(F("Automatic turning off. "));}
  } else {
    automatic = true;
    if (DEBUG) {Serial.print(F("Automatic turning on. "));}
  }
}
void toggleDebug() {if (DEBUG){DEBUG = 0;} else {DEBUG=1;}Serial.print(F("Toggling debug:"));Serial.println(DEBUG);}

// RCP for ESP:
// uint8_t readCapacitivePin(int pinToMeasure) {

//   uint8_t total = 0;
  
//   for(uint8_t i = 0; i < 6; i++) {

//     pinMode(pinToMeasure, OUTPUT);

//     digitalWrite(pinToMeasure, LOW);

//     //delay(1);

//     //noInterrupts();

//     pinMode(pinToMeasure, INPUT_PULLUP);

//     uint8_t cycles = 17;

//     if (digitalRead(pinToMeasure)) { cycles =  0;}
//     else if (digitalRead(pinToMeasure)) { cycles =  1;}
//     else if (digitalRead(pinToMeasure)) { cycles =  2;}
//     else if (digitalRead(pinToMeasure)) { cycles =  3;}
//     else if (digitalRead(pinToMeasure)) { cycles =  4;}
//     else if (digitalRead(pinToMeasure)) { cycles =  5;}
//     else if (digitalRead(pinToMeasure)) { cycles =  6;}
//     else if (digitalRead(pinToMeasure)) { cycles =  7;}
//     else if (digitalRead(pinToMeasure)) { cycles =  8;}
//     else if (digitalRead(pinToMeasure)) { cycles =  9;}
//     else if (digitalRead(pinToMeasure)) { cycles = 10;}
//     else if (digitalRead(pinToMeasure)) { cycles = 11;}
//     else if (digitalRead(pinToMeasure)) { cycles = 12;}
//     else if (digitalRead(pinToMeasure)) { cycles = 13;}
//     else if (digitalRead(pinToMeasure)) { cycles = 14;}
//     else if (digitalRead(pinToMeasure)) { cycles = 15;}
//     else if (digitalRead(pinToMeasure)) { cycles = 16;}

//     //interrupts();

//     digitalWrite(pinToMeasure, LOW);

//     pinMode(pinToMeasure, OUTPUT);

//     if(cycles < 1 || cycles > 11) { // outside threshold - resample
//       i--;
//     } else {
//        total = total + cycles;
//     }
//     //return cycles;
//   }
//   return((total/5));
// }

uint8_t readCapacitivePin(int pinToMeasure) {

  uint8_t total = 0;
  
  for(uint8_t i = 0; i < 6; i++) {

    pinMode(pinToMeasure, OUTPUT);

    digitalWrite(pinToMeasure, LOW);

    //delay(1);

    //noInterrupts();

    pinMode(pinToMeasure, INPUT_PULLUP);

    uint8_t cycles = 17;

    if (digitalRead(pinToMeasure)) { cycles =  0;}
    else if (digitalRead(pinToMeasure)) { cycles =  1;}
    else if (digitalRead(pinToMeasure)) { cycles =  2;}
    else if (digitalRead(pinToMeasure)) { cycles =  3;}
    else if (digitalRead(pinToMeasure)) { cycles =  4;}
    else if (digitalRead(pinToMeasure)) { cycles =  5;}
    else if (digitalRead(pinToMeasure)) { cycles =  6;}
    else if (digitalRead(pinToMeasure)) { cycles =  7;}
    else if (digitalRead(pinToMeasure)) { cycles =  8;}
    else if (digitalRead(pinToMeasure)) { cycles =  9;}
    else if (digitalRead(pinToMeasure)) { cycles = 10;}
    else if (digitalRead(pinToMeasure)) { cycles = 11;}
    else if (digitalRead(pinToMeasure)) { cycles = 12;}
    else if (digitalRead(pinToMeasure)) { cycles = 13;}
    else if (digitalRead(pinToMeasure)) { cycles = 14;}
    else if (digitalRead(pinToMeasure)) { cycles = 15;}
    else if (digitalRead(pinToMeasure)) { cycles = 16;}

    //interrupts();

    digitalWrite(pinToMeasure, LOW);

    pinMode(pinToMeasure, OUTPUT);

    if(cycles < 1 || cycles > 11) { // outside threshold - resample
      i--;
    } else {
       total = total + cycles;
    }
    //return cycles;
  }
  return((total/5));
}

// Just flash black for a short bit.
void flash( ) {
  
  uint32_t colors[pixelsTotal];

  for(int i=0; i<pixelsTotal; i++){
    colors[i] = strip->getPixelColor(i);
    strip->setPixelColor(i,0);

  }
  strip->show();
  delay(40);
  for(int i=0; i<pixelsTotal; i++){
    strip->setPixelColor(i,colors[i]);
  }
  strip->show();
}

void statusUpdate() {
  // Serial.print(F("Is Dave there?"));
  Serial.println(F("<=== Creamery Status Update ===>")); 
  Serial.print(F("Selector: "));
  Serial.println(_selector); 

  Serial.println("-------");
  Serial.print(F("panelsTotal: "));
  Serial.println(pixelsTotal);  

  // Serial.print(F("Phase: "));
  // Serial.println(phase);

  Serial.print(F("Free Ram: "));
  Serial.println(freeRam());


  Serial.print(F("DEBUG: "));
  Serial.println(DEBUG);
  // Serial.print(F("VERBOSE: "));
  // Serial.println(verbose);
  
  Serial.println(F("<===========>"));
  Serial.println(F(""));
  /// checking how many items are in the menu / effect list. 
  // for (int j=0;j<menu_count;j++) {
  //  Serial.println(j);  
  // }

}


boolean drive() {
    now = millis();   // This moment is beautiful.
    
    // Do Wifi

    // Do Touch
    interceptTouch(TOUCHPIN);

    // Do Serial // Can we consolidate to "Do Input"?
    byte y = 10;
    while(Serial.available() > 0 && (y-- > 0)) { // Listen on the serial connection.    
        char x = Serial.read();
        interceptSerial(x);
    }    


    // Do mode change
    if (_futureSelector != _selector) {
    	if (_futureSelector >= effects.size()) {

    		if (DEBUG) Serial.println(F("Resetting selector because of overbounds...")); 
    		if (automatic) {
        		_futureSelector = 1;
      		} else {
        		_futureSelector = 0;
      		}
    	}

    	if (DEBUG) {Serial.print(F("Changing selector to: "));Serial.println(_futureSelector);}    	
    	if (DEBUG) {Serial.print(F("Activating Effect: "));Serial.println(_futureSelector);}    	
    	
    	if (effects[_futureSelector]->willTransition()) {
      		transitioning = 1;
      		currentTransitionStep = 0;
          transitionSteps = transitionTime / frameRate;

      		// if (DEBUG) {Serial.print("Transition Steps: ");Serial.println(transitionSteps);}
    	} else {
          
          if (DEBUG) {Serial.print(F("Deactivating Effect: "));Serial.println(_selector);}
          effects[_selector]->deactivate();
          
      }    	
      
      effects[_futureSelector]->reset();
    	effects[_futureSelector]->activate();

      _pastSelector = _selector;
      _selector = _futureSelector;
    	
      // if (DEBUG) {Serial.print(F("Past Selector: "));Serial.println(_pastSelector);}
      // if (DEBUG) {Serial.print(F("Current Selector: "));Serial.println(_selector);}
      // if (DEBUG) {Serial.print(F("Future Selector: "));Serial.println(_selector);}
    }

    if (transitioning) {
    
      if (currentTransitionStep >= transitionSteps) {
        if (DEBUG) {Serial.println(F("Finished Transitioning."));}
        if (DEBUG) {Serial.print(F("Deactivating Effect: "));Serial.println(_pastSelector);}
        effects[_pastSelector]->deactivate();
        transitioning = false;        
      }      
    
    }



    if (now >= then+frameRate) {
    	then = now;
    	return true;
    }

    return false;

} 

// void setup() {

//   init(pixelsTotal);
    
//   Serial.println("Running...");
// }

// Free Ram Output via Adafruit ///Move this into a testing utilities library.
String freeRam () 
{
  return "";
  // return String(ESP.getFreeHeap());
}


