#include<lpc17xx.h>						// including LPC1768 Header file.
#include<stdio.h>						// including Standard input, output file.
#include<stdint.h>						// including Standard integer header file.
#include<string.h>						// including String header file.
// #include "uart.h"					// including UART header file.
#include "lcd.c"						// including lcd implementations file.
#include "easyPasswordDetection.c"		// including easy password detection file.

// Keypad setup
#define ROW_PINS (0x0F << 4)
#define COL_PINS (0x0F << 0)

// Global variables.
// Variable to store the original password during registration.
char ownerPassword[12];
// Variable to store the user input for comparision with originalPassword.
char userPassword[12];
// Simple character array to store the content to be displayed in LCD.
char t[20];
// An integer array to store the status of appliances.
int appliances[4] = {0};
// Variables for maintaining the user status.
int isLoggedIn = 0, incorrectAttempts = 0;
int x = 0;		// Variable used for countdown in incorrect login attempts.

void welcome_message(void);				// Function to display welcome message.
void set_owner_password(void);			// Function for registering owner password.
void int_config(void);					// Function to configure interrupt.
void timer_config(void);				// Function to configure timer.
char getKeyInput(void);					// Function which waits for user input and gets it.
void update_led_status(void);			// Function to update LED statuses wrt. array values.
void panic_alert(void);					// Function to ring buzzer for 1 sec.
int main(void)
{
	// temp is used to get the keypad into it and do the actions necessary.
	char temp;
	// i is used as a iterator variable.
	int i;
	// ownerPassword character array initialised as empty string.
	ownerPassword[0] = '\0';
	// userPassword character array initialised as empty string.
	userPassword[0] = '\0';

    LPC_GPIO2->FIODIR = ROW_PINS;			//Keypad setup
    LPC_GPIO2->FIODIR &= ~COL_PINS;			// Keypad setup
	LPC_GPIO1 -> FIODIR |= (0XFF << 19);	// All LED's from 19 to 26 configured as output.
    LPC_GPIO1 -> FIODIR |= (0x01 << 28);	// LED to show keypad is waiting for input.
	LPC_GPIO1 -> FIODIR |= (0x01 << 27);	// Buzzer to indicate many incorrect attempts.

	// Initial configuration function calls.
	lcd_init();
	timer_config();
	int_config();
	
	// Printing welcome message.
	welcome_message();
	// Owner password registration.
	set_owner_password();

	while(1)	//Software loop
	{
		temp = '\0';		// Initially temp is stored with null char.
		if(!isLoggedIn)		// If user is not logged in this will work.
		{
			// Checking whether he made many incorrect attempts or not.
			// If incorrectAttempts made are greater than or equal to 3 making him wait.
			if(incorrectAttempts >= 3)
			{	
				lcd_cmd_write(0x01); // command to clear display.
				lcd_str_write("Many attempts.");
				lcd_cmd_write(0xC0); // command to go to line 2.
				lcd_str_write("Wait: ");
				x = 10;
				while(x--)			// displaying countdown from 9 to 0
				{
					sprintf(t, "%d", x);
					lcd_str_write(t);
					panic_alert();	// Make alert for 1 sec.
					lcd_cmd_write(0x10); // Moving cursor left by one position.
				}
				LPC_GPIO1 -> FIOCLR |= (0x01 << 27);
				incorrectAttempts--;	// Decrementing incorrect attempts to allow user to try logging in.
			}
			lcd_cmd_write(0x01); // command to clear display.
			lcd_str_write("Enter Pin:");
			lcd_cmd_write(0xC0); // command to go to line 2.
			temp = getKeyInput();
			i=0;
			// 'F' is considered as enter button and size is max upto 10.
			while(temp != 'F' && (i+1) < 10)
			{
				if(temp == 'E')		// 'E' button is used to clear the current entry.
				{
					lcd_cmd_write(0x01); // command to clear display.
					lcd_str_write("Enter Pin:");
					lcd_cmd_write(0xC0); // command to go to line 2.
					i=0;
					userPassword[i] = '\0';	
				}
				else
				{
					userPassword[i] = temp;		// Getting keypad input and adding it to string.
					userPassword[i+1] = '\0';	// Limiting the inputs end.
					lcd_data_write(temp);
					delay(20);
					lcd_cmd_write(0x10); // Moving cursor left by one position.
					lcd_data_write('*'); // Replacing displayed char with * after delay.
					i++;
				}
				temp = getKeyInput();	// Getting the keypad input.
			}
			// If password matches, then login status set to 1.
			if(strcmp(userPassword, ownerPassword) == 0)
			{
				incorrectAttempts = 0;
				isLoggedIn = 1;
			}
			// If password not matched then incrementing incorrectAttempts.
			else
			{
				lcd_cmd_write(0x01); // command to clear display.
				lcd_str_write("Incorrect PIN");
				incorrectAttempts++;
				delay(100);
			}
		}
		// If login status is set to 1 then user gets access.
		if(isLoggedIn)
		{
			// Simple configuration to show realtime status of four appliances.
			// User can controll them using 0, 1, 2, 3 buttons in keypad.
			// Their status is used to controll LED's by toggling them.
			lcd_cmd_write(0x01); // command to clear display.
			lcd_str_write("CB1 CB2 MT1 MT2");
			lcd_cmd_write(0xC0); // command to go to line 2.
			sprintf(t, " %d   %d   %d   %d",appliances[0], appliances[1], appliances[2], appliances[3]);
			lcd_str_write(t);		// Displaying appliances status.
			
			// Conditions for updating status.	
			update_led_status();

			temp = getKeyInput(); 	// Getting keypad input.
			// Conditional controll.
			if(temp == '0')
				appliances[0] ^= 1;
				
			else if(temp == '1')
				appliances[1] ^= 1;
				
			else if(temp == '2')
				appliances[2] ^= 1;
				
			else if(temp == '3')
				appliances[3] ^= 1;

			update_led_status();
		}
	}
}

void welcome_message()
{
	lcd_cmd_write(0x01); // command to clear display.
	lcd_cmd_write(0x0C); // display on cursor off command.
	lcd_str_write("Authentication");
	lcd_cmd_write(0xC0); // command to go to line 2.
	lcd_str_write(" -For Industries");
	delay(200);
}

void set_owner_password()
{
	char temp;
	int i;
	lcd_cmd_write(0x01); // command to clear display.
	lcd_str_write("Set Owner's Pin");
	lcd_cmd_write(0xC0); // command to go to line 2.
	lcd_str_write(" (Max. 10 nums) ");
	delay(400);
	lcd_cmd_write(0x01); // command to clear display.

	lcd_str_write("Enter Pin:");
	lcd_cmd_write(0xC0); // command to go to line 2.
	temp = getKeyInput();
	i=0;
	// In this state both 'E' and 'F' are used as enter button.
	while(temp != 'F' && temp != 'E' && (i+1) < 10)
	{
		ownerPassword[i] = temp;
		ownerPassword[i+1] = '\0';
		lcd_data_write(temp);
		delay(20);
		lcd_cmd_write(0x10); // Moving cursor left by one position.
		lcd_data_write('*'); // Replacing displayed char with * after delay.
		i++;
		temp = getKeyInput();
	}
	if(!(is_easy_password(ownerPassword)))
	{
		lcd_cmd_write(0x01); // command to clear display.
		lcd_str_write("Setup Successful");
		lcd_cmd_write(0xC0); // command to go to 2nd line
		lcd_str_write(ownerPassword); // function call to display setup password.
		delay(100);
	}
	else
	{
		ownerPassword[0] = '\0';
		lcd_cmd_write(0x01); // command to clear display.
		lcd_str_write("Your Password");
		lcd_cmd_write(0xC0);
		lcd_str_write(" is not Strong!");
		delay(150);
		set_owner_password();
	}
}

// Function used to wait and get keypad input
char getKeyInput()
{
	uint32_t  i,j,val;
    uint8_t scan[4]= {0xE,0xD,0xB,0x7};
    char key[4][4] = { {'0','1','2','3'},
                       {'4','5','6','7'},
                       {'8','9','A','B'},
                       {'C','D','E','F'}
                     };
	LPC_GPIO1 -> FIOSET |= (0x01 << 28);
	while(1)
    {
        for(i=0;i<4;i++)
        {
            LPC_GPIO2->FIOCLR |= ROW_PINS; //clear row lines
            LPC_GPIO2->FIOSET |= scan[i] << 4; //activate single row at a time
            val = LPC_GPIO2->FIOPIN & COL_PINS;//read column lines
            for(j=0;j<4;j++)
            {
                if(val == scan[j]) break; //if any key is pressed stop scanning
            }
            if(val != 0x0F) // if key pressed in the scanned row, print key using lookup table
            {
				LPC_GPIO1 -> FIOCLR |= (0x01 << 28);
				return key[i][j];
            }
        }
    }
}

void timer_config(void) {

    LPC_SC->PCONP |= (1 << 1);    // Power up Timer0
    LPC_TIM0->PR = 3;            // Set the prescaler value
    LPC_TIM0->MR0 = 15000000;      // Set match register for 15 seconds delay
    LPC_TIM0->MCR = (1 << 0) | (1 << 1);  // Interrupt and Reset on Match0
    LPC_TIM0->TCR = 1 << 1;        // Reset Timer0
    LPC_TIM0->TCR = 1;              // Enable Timer0
}


void int_config(void) 
{
    NVIC_ClearPendingIRQ(TIMER0_IRQn);     // Clear pending interrupts for Timer0
    NVIC_SetPriority(TIMER0_IRQn, 1);      // Set priority for Timer0 interrupt
    NVIC_EnableIRQ(TIMER0_IRQn);           // Enable Timer0 interrupt 
}


void TIMER0_IRQHandler (void) {
	isLoggedIn = 0;
	LPC_TIM0->IR = 1 << 0;                 // Clear interrupt flag
    NVIC_ClearPendingIRQ(TIMER0_IRQn);     // Clear pending interrupt for Timer0
}

void update_led_status(void)
{
	if(appliances[0])
	LPC_GPIO1->FIOSET |= (0x03<<19);
	else
	LPC_GPIO1->FIOCLR |= (0x03<<19);
	if(appliances[1])
	LPC_GPIO1->FIOSET |= (0x03<<21);
	else
	LPC_GPIO1->FIOCLR |= (0x03<<21);
	if(appliances[2])
	LPC_GPIO1->FIOSET |= (0x03<<23);
	else
	LPC_GPIO1->FIOCLR |= (0x03<<23);
	if(appliances[3])
	LPC_GPIO1->FIOSET |= (0x03<<25);
	else
	LPC_GPIO1->FIOCLR |= (0x03<<25);
}

void panic_alert(void)
{
	int i;
	for(i = 0; i < 14; i++)
	{
		LPC_GPIO1 -> FIOCLR |= (0xFF << 19);
		LPC_GPIO1 -> FIOSET |= (0x01 << 27);
		delay(5);
		LPC_GPIO1 -> FIOSET |= (0xFF << 19);
		LPC_GPIO1 -> FIOCLR |= (0x01 << 27);
		delay(3);
	}
	LPC_GPIO1 -> FIOSET |= (0x01 << 27);
	delay(20);
	LPC_GPIO1 -> FIOCLR |= (0x01 << 27);
	LPC_GPIO1 -> FIOCLR |= (0xFF << 19);
	delay(5);	
}
