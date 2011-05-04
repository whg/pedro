
#include "line.h"

void lineTo(uint16_t x, uint16_t y) {
	line(pos.x, pos.y, x, y);
	
	//now make sure that we got to destination, ie force it.
	pos.x = x;
	pos.y = y;
	
//	sendUSART(121);
//	sendUSART16(x);
//	sendUSART16(y);
//	sendUSART16(pos.x);
//	sendUSART16(pos.y);
	
	delay_ms(1);
}

void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	
	int8_t xstep = 1;
	int8_t ystep = 1;
	
	if (x0 > x1) xstep = -1;
	if (y0 > y1) ystep = -1;
	
	int16_t dx = abs(x1 - x0);
	int16_t dy = abs(y1 - y0);
	
	int16_t x = x0;
	int16_t y = y0;
	
	wakeMotors();
	
	if (dy <= dx) {
		int16_t error = dy - dx;
		
		while (x != x1) {
			if (error >= 0) {
				if (error || (xstep > 0)) {
					moveHalfStep(2);
					stepCount2+= ystep;
					y+= ystep;
					error-= dx;
				}
			}
			
			moveHalfStep(1);
			stepCount1+= xstep;
			
			x+= xstep;
			error+= dy;
			
			//delay once...
			delay_ms(delayTime);
			
			//update pos
			pos.x = x;
			pos.y = y;
			
		}// end while
		
		//a little readjustment, as algorithm isn't quite right...
		if (x0 > x1) {
			y+= ystep;
			pos.y = y;
		}		
	}
	
	//if dy > dx
	else {
		int16_t error = dx - dy;
		
		while (y != y1) {
			
			if (error >= 0) {
				if (error || (ystep > 0)) {
					moveHalfStep(1);
					stepCount1+= xstep;
					x+= xstep;
					error-= dy;
				}
			}
			
			moveHalfStep(2);
			stepCount2+= ystep;
			
			y+= ystep;
			error+= dx;
			
			//delay once...
			delay_ms(delayTime);
			
			
			//update pos
			pos.x = x;
			pos.y = y;
			
		}// end while
		
		if (y1 < y0) {
			x+= xstep;
			pos.x = x;
		}
	}
	
	sleepMotors();
	
	
}
