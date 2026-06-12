#include <xc.h>

#pragma config FOSC = HS        
#pragma config WDTE = OFF       
#pragma config LVP = OFF        

#define _XTAL_FREQ 20000000 

#define IR_SENSOR PORTBbits.RB0
#define SERVO_PIN PORTDbits.RD0

#define I2C_ADDR 0x4E 

void I2C_Init() {
    TRISC3 = 1; TRISC4 = 1; 
    SSPCON = 0x28;          
    SSPADD = 49;            
    SSPSTAT = 0x00;
}

void I2C_Wait() { while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)); }
void I2C_Start() { I2C_Wait(); SSPCON2bits.SEN = 1; }
void I2C_Stop() { I2C_Wait(); SSPCON2bits.PEN = 1; }
void I2C_Write(unsigned char data) { I2C_Wait(); SSPBUF = data; }

void LCD_Write_Nibble(unsigned char val, unsigned char rs) {
    val <<= 4;
    I2C_Start();
    I2C_Write(I2C_ADDR);
    // 0x08 keeps backlight ON, 0x04 is EN Pulse
    I2C_Write(val | 0x08 | (rs ? 0x01 : 0x00) | 0x04); 
    __delay_ms(1); // Longer pulse for testing
    I2C_Write(val | 0x08 | (rs ? 0x01 : 0x00));        
    I2C_Stop();
    __delay_ms(1);
}

void LCD_Cmd(unsigned char cmd) {
    LCD_Write_Nibble(cmd >> 4, 0);
    LCD_Write_Nibble(cmd & 0x0F, 0);
}

void LCD_Data(unsigned char data) {
    LCD_Write_Nibble(data >> 4, 1);
    LCD_Write_Nibble(data & 0x0F, 1);
}

void LCD_Init() {
    __delay_ms(100); // Wait for LCD power to stabilize
    LCD_Write_Nibble(0x03, 0); __delay_ms(5);
    LCD_Write_Nibble(0x03, 0); __delay_ms(5);
    LCD_Write_Nibble(0x03, 0); __delay_ms(5);
    LCD_Write_Nibble(0x02, 0); 
    LCD_Cmd(0x28); 
    LCD_Cmd(0x0C); 
    LCD_Cmd(0x01); 
    __delay_ms(5);
}


void main(void) {
    TRISBbits.TRISB0 = 1;  // IR Sensor Pin 33
    TRISDbits.TRISD0 = 0;  // Servo Pin 19
    
    char A[7]={'W','E','L','C','O','M','E'};
    
    SERVO_PIN = 0;
    
    //I2C_Init();
    //LCD_Init();

    while(1) {
        if(IR_SENSOR == 0) {
                I2C_Init();
                LCD_Init();
            // OBSTACLE DETECTED: Move to 90 degrees and HOLD
            // We loop 250 times to create a 5-second hold period
            for(int i = 0; i < 250; i++) {
                SERVO_PIN = 1;
                __delay_us(2200);  // 90 degree pulse
                SERVO_PIN = 0;
                //__delay_us(18500);// Remainder of 20ms frame
            }
                LCD_Cmd(0x80); 
                
                for(char i=0; i<7; i++) { // Simple counter to see if it's alive
                LCD_Data(A[i]);
                __delay_ms(200);
            }
               __delay_ms(5000);
               LCD_Cmd(0x01);
        } else {
            // NO OBSTACLE: Stay at 0 degrees
            SERVO_PIN = 1;
            __delay_us(1000);  // 0 degree pulse
            SERVO_PIN = 0;
            __delay_us(19000); 
        }
    }
}
