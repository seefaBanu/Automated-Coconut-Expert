#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LCD PORTC
#define EN 6
#define RS 0
#define RW 1 //defining display pins
#define ir PIND & (1<<5)
#define color PIND & (1<<6)
//function prototypes

unsigned char keypad();
void i2c_init();
void i2c_start();
void i2c_write(char x);
char i2c_read();
void HCSR04();
void uart_transmit (unsigned char data);
void anticlockstepper();
void clockstepper();
void servo1();
void servo2();
void DC1();
void DC2();
void DC3time1();
void DC3time2();

void lcddata(unsigned char data);

int main(void){
	//display function call
	i2c_init();
	i2c_start();
	i2c_write(0x70);
	lcd_init();
	unsigned char x;
	// make PA0 to PA3 = output and PA4 to PA7 = Input
	DDRC |=0X0f; //DC MOTOR CUTDISK
	DDRC |= 0XF0; //making keypad row pin as output
	DDRB = 0XFF;//making stepper pin output
	DDRD &= ~(1<<0) ;
	DDRD &= ~(1<<1) ;
	DDRD &= ~(1<<2) ; // keypad coloumn pin as input
	DDRD &= ~(1<<5) ;// IR AS INPUT
	DDRD &= ~(1<<6) ;// white AS INPUT
	DDRD &= ~(1<<7) ;// brown AS INPUT
	DDRA = 0XFF;

	
	while(1){
		PORTC &=~ 0xf0 ; //PIN4-7 of port c is low
		PORTD |= ( 1<<0) ; //PIN0 of port D is high
		PORTD |= ( 1<<1) ;
		PORTD |= ( 1<<2) ;
	
		//ir detect
			
		if(ir)
		{
			
			cutdiskstop(); // cut disk stop
			anticlockstepper();//stepper motor rotate anticlockwise
			lcd_cmd(0x01);
			lcd_msg("scrap or not");
			_delay_ms(200);
			lcd_cmd(0xC0);
			lcd_msg("yes-1 | No-2");
			_delay_ms(2000);
			lcd_cmd(0x01);
			x=keypad();
			_delay_ms(300);
			lcd_dwr(x);
			_delay_ms(1000);
			
			if(x =='1') //scaping yes
			{
				lcd_cmd(0x01);
				lcd_msg("How many parts?");
				_delay_ms(300);
				lcd_cmd(0xC0);
				lcd_msg("One-1 | Two-2");
				_delay_ms(2000);
				lcd_cmd(0x01);
				x=keypad();// user gives input
				lcd_dwr(x);
				_delay_ms(1000);
				lcd_cmd(0x01);
				if(x =='1')
				{
					servo1();//One handle rotate
					_delay_ms(1000);
					DC1();//Only one scraper work
					_delay_ms(2000);
					
			//color sensor detects brown
						if(color){
						stopDC(); //scraper off
						lcd_msg("-scraping done!-");
						_delay_ms(2000);
						lcd_cmd(0x01);
						lcd_msg("-Grind or not?-");
						lcd_cmd(0xC0);
						_delay_ms(100);
						lcd_msg("YES-1 | NO-2");
						_delay_ms(2000);
						lcd_cmd(0x01);
						x=keypad();// user gives input
						lcd_dwr(x);
						_delay_ms(1000);
						
						//KEYPAD
						
						if(x=='1') // scraped coconut grinding
						{
							lcd_cmd(0x01);
							lcd_msg("  Add water  ");
							lcd_cmd(0xC0);
							lcd_msg("  to grinder  ");
							_delay_ms(2000);							
							lcd_cmd(0x01);
							for (int i = 0; i<6; i++)
							{
								_delay_ms(500);
								lcd_msg("***Waiting***");
								_delay_ms(1000);
								lcd_cmd(0x01);
							}
							lcd_cmd(0x01);
							DC3time1();//grinder work
							_delay_ms(2000);
							lcd_msg("Grind finished");
							lcd_cmd(0x01);
							HCSR04();// ultrasonic sensor measure the coconut water level
							_delay_ms(2000);
						}
						else if(x=='2')
						{
							lcd_cmd(0x01);
							HCSR04();
							_delay_ms(1000);
							//machine off
						}
					}
				}
				
				
				else if(x == '2')
				{
					
					servo2();//two handles rotate
					_delay_ms(1000);
					DC2();//two scrapers work
					_delay_ms(2000);
					
					if(color)
					{
						stopDC();
						_delay_ms(1000); //scrapers off
						lcd_msg("-scraping done!-");
						_delay_ms(2000);
						lcd_cmd(0x01);
						lcd_msg("-Grind or not?-");
						lcd_cmd(0xC0);
						_delay_ms(100);
						lcd_msg("YES-1 | NO-2");
						_delay_ms(2000);
						lcd_cmd(0x01);
						x=keypad();// user gives input
						lcd_dwr(x);
						_delay_ms(1000);
						//KEYPAD
						if(x =='1')//scraped coconut grinding
						{
								lcd_cmd(0x01);
								lcd_msg("  Add water  ");
								lcd_cmd(0xC0);
								lcd_msg("  to Grinder  ");
								_delay_ms(2000);
								lcd_cmd(0x01);
								    for (int i = 0; i<6; i++)
								    {
										_delay_ms(500);
									  lcd_msg("***Waiting***");// waiting to add water
										_delay_ms(1000);
										lcd_cmd(0x01);
								    }
								lcd_cmd(0x01);	
							DC3time2();//grind
							_delay_ms(2000);
							lcd_msg("Grind finished");
							lcd_cmd(0x01);	
							_delay_ms(1000);
							HCSR04();
							_delay_ms(1000);
							//machine off
						}
						if(x=='2')// no grinding
						{
							HCSR04(); // ultrasonic sensor measure the coconut water level
							_delay_ms(1000);
							//machine off
						}
					}
				}
			}
			else if(x=='2')// no scraping only break
			{
				HCSR04();// ultrasonic sensor measure the coconut water level
			}
			//if not ir detected	
		}
		else{
			lcd_cmd(0x80);
			lcd_msg("----Welcome----");
			_delay_ms(1000);
			lcd_cmd(0x01);
			lcd_msg("---Group-15---");
			_delay_ms(100);
			lcd_cmd(0xC0);
			lcd_msg("--COCO MASTER--");
			_delay_ms(1000);
			lcd_cmd(0x01);
			
			//stepper rotate clockwise
			clockstepper();
			cutdisk();//cutdisk rotates
			
			_delay_ms(3000);
		}
	
	}
	return 0;
}

//stepper motor clockwise rotation
void clockstepper()
{   
	
	for(int i=0;i<10;i++)
	{
		if(!(ir)){
		PORTB &= ~ 0B00001111;
		PORTB = 0B00001000;
		_delay_ms(50);
		PORTB = 0B00000100;
		_delay_ms(50);
		PORTB = 0B00000010;
		_delay_ms(50);
		PORTB = 0B00000001;
		_delay_ms(50);
		}
	}
}

//stepper motor anti-clockwise rotation
void anticlockstepper()
{
	
	for(int i=0;i<10;i++){
		PORTB &= ~ 0B00001111;
		PORTB = 0B00000001;
		_delay_ms(50);
		PORTB = 0B00000010;
		_delay_ms(50);
		PORTB = 0B00000100;
		_delay_ms(50);
		PORTB = 0B00001000;
		_delay_ms(50);
	}
	
}
//cut disk(dc rotation)
void cutdisk()
{
	for(int i=0;i<20;i++)
	{
		if(!(ir)){
		PORTC |= (1<<3);
		}
	}
}

//cut disk stop(dc stop)
void cutdiskstop()
{
	PORTC &=~ (0x0C);
}
void lcd_Print (char*str){
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		lcddata(str[i]);
	}
}

unsigned char keypad(){
	// make 1st row 0
	PORTC |=   0b11100000;
	PORTC &= ~ 0b00010000;
	if((PIND & (1<<PIND0))==0){
		_delay_ms(10);
		return '1';
		}else if((PIND & (1<<PIND1))==0){
		_delay_ms(10);
		return '2';
		}else if((PIND & (1<<PIND2))==0){
		_delay_ms(10);
		return '3';
	}

	PORTC &= ~ 0b00100000;

	if((PIND & (1<<PIND0))==0){
		_delay_ms(30);
		return '4';
		}else if((PIND & (1<<PIND1))==0){
		_delay_ms(30);
		return '5';
		}else if((PIND & (1<<PIND2))==0){
		_delay_ms(30);
		return '6';
	}

	PORTC &= ~ 0b01000000;

	if((PIND & (1<<PIND0))==0){
		_delay_ms(30);
		return '7';
		}else if((PIND & (1<<PIND1))==0){
		_delay_ms(30);
		return '8';
		}else if((PIND & (1<<PIND2))==0){
		_delay_ms(30);
		return '9';
	}
	
	PORTC &= ~ 0b10000000;

	if((PIND & (1<<PIND0))==0){
		_delay_ms(30);
		return 'C';
		}else if((PIND & (1<<PIND1))==0){
		_delay_ms(30);
		return '0';
		}else if((PIND & (1<<PIND2))==0){
		_delay_ms(30);
		return '=';
	}
}

void i2c_init(){
	TWBR = 0x62;		//	Baud rate is set by calculating
	TWCR = (1<<TWEN);	//Enable I2C
	TWSR = 0x00;		//Prescaler set to 1

}
//Start condition
void i2c_start(){
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);	//start condition
	while (!(TWCR & (1<<TWINT)));				//check for start condition

}
//I2C stop condition
void i2c_write(char x){				//Cpn esta funcion se escribe en el bus de TWDR
	TWDR = x;						//Move value to I2C
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
}

char i2c_read(){
	TWCR  = (1<<TWEN) | (1<<TWINT);	//Enable I2C and clear interrupt
	while (!(TWCR & (1<<TWINT)));	//Read successful with all data received in TWDR
	return TWDR;
}


//-----Proto-Type-------//
void lcd_init();			//----LCD Initializing Function
void toggle();				//----Latching function of LCD
void lcd_cmd_hf(char v1);   //----Function to send half byte command to LCD
void lcd_cmd(char v2);		//----Function to send Command to LCD
void lcd_dwr(unsigned char v3);		//----Function to send data to LCD
void lcd_msg(char *c);		//----Function to Send String to LCD
void delay(int ms);			//----Delay function
void lcd_lef_sh();			//----Left Shifting Function
void lcd_rig_sh();			//----Right Shifting Function
//----------------------//

void toggle()
{
	TWDR |= 0x02;					//---PIN En de la LCD en = 1;  -----Latching data in to LCD data register using High to Low signal
	TWCR = (1<<TWINT) | (1<<TWEN);	//---Enable I2C and clear interrupt- Esta linea y la siguiente simepre van despues de querer mandar un coamndo por TDWR
	while  (!(TWCR &(1<<TWINT)));	//---Simepre poner despues de la linea anterior al mandar datos por TWDR
	delay(1);
	TWDR &= ~0x02;					//---PIN del Enable de la LCD en = 0;
	TWCR = (1<<TWINT) | (1<<TWEN);	//---Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
}

void lcd_cmd_hf(char v1)
{
	TWDR &=~0x01;					//PIN RS de la LCD rs = 0; ----Selecting register as Command register
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR &= 0x0F;					//----clearing the Higher 4 bits
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR |= (v1 & 0xF0);			//----Masking higher 4 bits and sending to LCD
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	toggle();
}

void lcd_cmd(char v2)
{
	TWDR&=~0x01;					//rs = 0; ----Selecting register as command register
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR &= 0x0F;                   //----clearing the Higher 4 bits
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR |= (v2 & 0xF0);			//----Masking higher 4 bits and sending to LCD
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	toggle();
	
	TWDR &= 0x0F;                    //----clearing the Higher 4 bits
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR |= ((v2 & 0x0F)<<4);		//----Masking lower 4 bits and sending to LCD
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	toggle();
}

void lcddata(unsigned char data){
	PORTA |= (1<<RS); //RS=1 for data
	PORTA &= ~(1<<RW); //RW=0 for write
	LCD= data & 0xF0;  //Send upper nibble
	PORTD|= (1<<EN); //EN=1 for H to L pulse
	_delay_ms(5);
	PORTD &= ~(1<<EN); // EN=0 for H to L pulse

	LCD = data<<4; // send low nibble;
	PORTD |=(1<<EN); //EN=1 for H to L pulse
	_delay_ms(5);
	PORTD &= ~(1<<EN);
	
}


void lcd_dwr(unsigned char v3)
{
	TWDR|=0x01;						//rs = 1; ----Selecting register as command register
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR &= 0x0F;				    //----clearing the Higher 4 bits
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR |= (v3 & 0xF0);			//----Masking higher 4 bits and sending to LCD
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	toggle();
	
	TWDR &= 0x0F;					//----clearing the Higher 4 bits
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR |= ((v3 & 0x0F)<<4);		//----Masking lower 4 bits and sending to LCD
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	toggle();
}

void lcd_init()
{
	lcd_cmd_hf(0x30);       //-----Sequence for initializing LCD
	lcd_cmd_hf(0x30);       //-----     "            "              "               "
	lcd_cmd_hf(0x20);       //-----     "            "              "               "
	lcd_cmd(0x28);          //-----Selecting 16 x 2 LCD in 4Bit mode
	lcd_cmd(0x0C);          //-----Display ON Cursor OFF
	lcd_cmd(0x01);          //-----Clear display
	lcd_cmd(0x06);          //-----Cursor Auto Increment
	lcd_cmd(0x80);          //-----1st line 1st location of LCD
}
//delay function
void delay(int ms)
{
	int i,j;
	for(i=0;i<=ms;i++)
	for(j=0;j<=120;j++);
}

void lcd_msg(char *c)
{
	int i;
	for(i=0;c[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		lcd_dwr(c[i]);
	}
}

void lcd_rig_sh()
{
	lcd_cmd(0x1C);      //----Command for right Shift
	delay(400);
}

void lcd_lef_sh()
{
	lcd_cmd(0x18);      //----Command for Left Shift
	delay(200);
}

//ultra sonic

static volatile unsigned int tCount = 0;// Variable to hold the counter value from TCNT1.
static volatile int i = 0; //Variable to indicate the state of ultrasonic sensor. i = 1 mean sensor is measuring the distance.
double distance = 0;// A variable to store the measured distance in cm

void HCSR04()
{
	
	UBRRH=0x00;
	UBRRL=51;
	DDRB = DDRB | 1 << DDB4; // Set LED pin as an output.
	DDRD = DDRD | 1 << DDD4; // Set Trigger pin as an output. 
	DDRD = DDRD & ~(1 << DDD3); // Set Echo pin(INT1 - PD3 on atmega32) as an input.
	PORTD = PORTD & ~(1 << PD3); //Disable pull-up resistor for echo pin.
	_delay_ms(50);

	GICR |= 1<<INT1; //Enable interrupt for INT0
	MCUCR |= 1<<ISC10;// Make any logical change on INT1 generate an interrupt.
	TIMSK |= 1 << TOIE1;// Enable Timer/Counter 1, Overflow interrupt.
	
	sei();
	while(1){
		
		//Send a 15us pulse to the trigger pin
		PORTD = PORTD | 1<<PD4;
		_delay_us(15);
		PORTD = PORTD & ~(1<<PD4);
		_delay_us(15);
		//Then echo pin will go hight. That interrupt will cause function ISR(INT1_vect) to get executed for the first time.
		//After detecting reflected sound wave, echo pin will go low. That interrupt will cause function ISR(INT1_vect) to get executed for the second time.
		//If echo pin didn't go low before the counter count to 65 535. Counter will overflow and that interrupt will cause ISR(TIMER1_OVF_vect) function to get executed.
		
		distance = tCount*1000000.0/F_CPU/58;
		//Turn on or off the LED based on the condition
		if (distance < 15)
		{
			PORTB = PORTB | 1 << PB4;
			lcd_cmd(0x01);
			lcd_msg("Check coco water");
			_delay_ms(1000);
			lcd_cmd(0xC0);
			lcd_msg("---THANK YOU---");
			_delay_ms(1200);
	
			
		}
		else
		{
			
		
			PORTB = PORTB & ~ (1 << PB4);
			lcd_cmd(0x01);
			lcd_msg("---THANK YOU---");
			_delay_ms(1200);
			
			
		}
		_delay_ms(200);
		
	}
	
}

ISR(INT1_vect)// This function will get executed whenever the micro controller is interrupted by INT1.
{
	if(i == 0)//This part get executed when interrupted by a rising edge(for the first time, since i = 0)
	{
		TCCR1B |= 1<<CS10;//Start counting(No prescaling).
		i = 1;// So in next interrupt false part will get executed
	}
	else//This part get executed when interrupted by the falling edge(for the second time, since we set i = 1 previously)
	{
		TCCR1B = 0;// Stop counting
		tCount = TCNT1;// Take the value of counter to our variable
		TCNT1 = 0;// Reset counter to 0
		i = 0; // So in next interrupt true part will get executed
	}
}

ISR(TIMER1_OVF_vect){// This function will get executed if Timer/counter 1 get overflowed
	TCCR1B = 0;// Stop counting
	tCount = 200;// This will make the variable ~0 when there is a counter overflow
	TCNT1 = 0;// Reset counter to 0
	i = 0;// So in next interrupt true part will get executed in ISR(INT1_vect) function
}

// servo 1
void servo1(){
	PORTA &= ~ (1<<4);
	//Rotate Motor to 0 degree
	PORTA |= 0x10;
	_delay_us(1000);
	PORTA &= ~ 0x10;
	_delay_ms(2000);
	
	//Rotate Motor to 90 degree
	PORTA |= 0x10;
	_delay_us(1500);
	PORTA &=~  0x10;	
}
// servo 2
void servo2(){
	PORTA &= ~ (1<<4);
	PORTA |= 0x30;
	_delay_us(1000);
	PORTA &= ~0x30;
	_delay_ms(2000);
	//Rotate Motor to 90 degree
	PORTA |= 0x30;
	_delay_us(1500);
		PORTA &=~ 0x30;
}
//one part scrape dc
void DC1()
{
do 
{
		PORTA |= 0x01;
} while (!color);
		
}
//two part scrape dc
void stopDC()
{
	
	PORTA &=~ (1<<0);
	PORTA &=~ (1<<2);
	PORTA &=~ (1<<1);
	PORTA &=~ (1<<3);
	
}
void DC2()
{
	do 
	{
		for (int i=0;i<6;i++)
		{
			PORTA |= (1<<0);
			PORTA |= (1<<2);
		}
	} while (!color);
	
	
}
void DC3time1()
{
	for (int i=0;i<6;i++)
	{
		PORTA |= 0b01000000;
	}
	_delay_ms(1000);
	PORTA &=~ (1<<6);
	PORTA &=~ (1<<7);
	
}

void DC3time2()
{
	for (int i=0;i<12;i++)
	{
	PORTA |= 0b01000000;
	}
	_delay_ms(1000);
	PORTA &=~ (1<<6);
	PORTA &=~ (1<<7);
	
}
