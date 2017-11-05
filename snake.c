// 16MHz clock
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h> // for rand() function
#include "led_control.c"// 8x16 LED Matrix header

//user defined variables for simplicity
typedef unsigned char   uint8_t;
typedef unsigned int    uint16_t;

//variable to hold the game state
static uint8_t gameover = 0;

//variable to hold the highscore of the game
static uint16_t highscore;


/* This function initializes the ADC, Buzzer and the two led matrices*/
void init_game()
{

	//initialise the ADC
	DDRA = 0; //port a as input
	ADCSRA=(1<<ADEN) | (1<<ADPS2) | (1<<ADPS1); // enable ADC , sampling freq = clk/64

	// TODO : make some tune with the buzzer

	//initialize first LED Matrix
	init_led_matrix(2);
	clear_led_matrix(0);

	//initialize second LED Matrix
	clear_led_matrix(1);

	// TODO : load highscore from eeprom

}


//This function retreives the ADC value from the given channel
uint16_t get_adc_value(uint8_t channel)
{
	//select channel
	ADMUX = channel;

    //start conversion on channel
    ADCSRA |= (1<<ADSC); 

    //wait for conversion to finish
   	while(ADCSRA&(1<<ADIF) == 0); 

   	return(ADCW); // return the converted value which is in ADCW
}

/* This function reads the joystick analog sensors input and returns the direction
0 - Stationary  1 - Up  2 - Right 3 - Down 4- Left*/
uint8_t snake_direction()
{
    uint16_t x_pos, y_pos;
    static uint8_t direction; //static to retain direction during subsequent calls

    //Read the value of ADC0 into x_pos
    x_pos = get_adc_value(0);

    //Read the value of ADC1 into y_pos 
    y_pos = get_adc_value(1);

    //set direction according to x and y values with some margins
    if((x_pos>=0)&&(x_pos < 256))
    	direction = 3;

    if((x_pos >= 768)&&(x_pos < 1024))
     	direction = 1;

    if((y_pos >=0 )&&(y_pos<256))
    	direction = 4;

    if((y_pos >= 768)&&(y_pos<1024))
        direction =2;

   	return direction;
  
}

// This function gets random values for food generation
uint8_t* get_food(uint8_t vlen)
{   
    static uint8_t v_pos[2]; // food position to retain value
    srand(vlen); // generate seed for random number generation
    v_pos[0] = rand()%15; // generate random numbers
    v_pos[1] = rand()%7;
    return v_pos;  
}

//custom delay function
void delay_ms(int d)
{
  int i;
  for (i = 0; i < d/10; ++i)
  {
    _delay_ms(10);
  }
}

/* This function contains the main logic of the game. It takes the direction as input
and draws the modified snake on the display. This function also contains the code for 
displaying the food and growing the snake if it eats the food. It also checks for self
collision */
void snake_main(uint8_t v_dir)
{

    //A 2D array hold the snake position with rows and column matrices(pixels)
    //old snake
    static uint8_t snake[][2] = {
                                  {1,1},{2,1},{3,1},{4,1},{5,1},{6,1},{7,1},{8,1},{9,1},{10,1},
                                  {11,1},{12,1},{13,1},{14,1},{15,1}
                                 }; 
    //new snake
    static uint8_t n_snake[][2] = {
                                    {1,1},{2,1},{3,1},{4,1},{5,1},{6,1},{7,1},{8,1},{9,1},{10,1},
                                    {11,1},{12,1},{13,1},{14,1},{15,1}
                                   };
    //game variables -->
    //v_len stores the current length of the snake                             
    static uint8_t v_len = 2;

    //This variable holds the current direction of the snake
    static uint8_t curr_dir = 0; 

    //loop variables
    static uint8_t i, j = 0;

    //variable to indicate which led matrix to use while displaying
    static uint8_t v_matrix = 0; 

    //variable to indicate the delay of the game (which directly affects the speed of the snake)
    static uint16_t snake_speed = 150; // in milli seconds

    // food variables
    static uint8_t *food_pos; // pointer to hold snake food position   
    static uint8_t food_draw = 1; // variable to indicate if food is present on board.
    
/*==============================================================================================
==========================CHANGE THE SNAKE AND DISPLAY THE MODIFIED SNAKE=======================
================================================================================================*/

    // shift the old snake until the last point(i.e the 'head')
    for(i =0; i<v_len-1; i++)
    {
        for(j=0; j<2; j++)
        n_snake[i][j] = snake[i+1][j];          
    }

    // finding if direction change is possible (up to down, left to right, etc not feasible)
    // else no direction change
    if(curr_dir - v_dir == 2 || curr_dir - v_dir == -2)
    	v_dir = curr_dir;
    
    // finding the next 'head' according to the analog input (% rotates through the matrix)
    switch(v_dir)
    {
      // take lite
        case 0: break;

      // up is pressed, column of old snake head should be incremented.
        case 1: 
        	n_snake[v_len-1][0] = (snake[v_len-1][0]%16);
            n_snake[v_len-1][1] = (snake[v_len-1][1]+1)%8; break;

      // right is pressed, row of old snake head should be incremented
        case 2: 
            n_snake[v_len-1][0] = (snake[v_len-1][0]+1)%16;
            n_snake[v_len-1][1] = snake[v_len-1][1]%8; break;
      
      // down is pressed, column of old snake head should be decremented
        case 3: 
            n_snake[v_len-1][0] = snake[v_len-1][0]%16;
            
            if((snake[v_len-1][1]-1) < 0)
            {
               // rotate to the top row
               n_snake[v_len-1][1] = 7;
            }
            else
            {
              n_snake[v_len-1][1] = (snake[v_len-1][1]-1)%8; 
            }
            break;

      // left is pressed, row of old snake head should be decremented
        case 4: 
            if((snake[v_len-1][0]-1)<0)
            {
              // rotate through left to rightmost column 
              n_snake[v_len-1][0] = 15;
            }
            else
            {
              n_snake[v_len-1][0] = (snake[v_len-1][0]-1)%16;
            }
            
            n_snake[v_len-1][1] = snake[v_len-1][1]%8;
            break;
    }

    // clear old Snake
    for(i =0; i < v_len; i++)
    {
        if(snake[i][0]<8)
        v_matrix = 0;        
        if(snake[i][0] >= 8)
        v_matrix = 1;
        set_led_matrix(v_matrix,(snake[i][0])%8, (snake[i][1]),0); // draw new snake
    }

    // Display the new snake
	for(i =0; i < v_len; i++)
    {
        if(n_snake[i][0]<8)
        v_matrix = 0;        
        if(n_snake[i][0] >= 8)
        v_matrix = 1;
        set_led_matrix(v_matrix,(n_snake[i][0])%8, (n_snake[i][1]),1); // draw new snake
    }


    // copy the snake for next time
    for(i =0; i<v_len; i++)
    {
        for(j=0; j<2; j++)
        snake[i][j] = n_snake[i][j];            
    }
/*==================================================================================================
=================================FOOD AND COLLISION DETECTION==============================
===================================================================================================*/
    
    // if food is not drawn draw it and make the flag zero
    if(food_draw)
    {
      	// get a random position for food in range(15,8)
      	food_pos = get_food(v_len);
      	food_draw = 0;
      	if(food_pos[0]>7)
      	{
        	// draw on matrix 2
        	set_led_matrix(1, food_pos[0]%8, food_pos[1], 1 );
      	}
      	else
      	{
        	// draw on matrix 1
        	set_led_matrix(0, food_pos[0], food_pos[1], 1 );
      	}
    } 
  
    
    
    // check if snakes eats the food and grow the snake
    if((n_snake[v_len-1][0] == food_pos[0])&&(n_snake[v_len-1][1] == food_pos[1]))
    {
       	
       	// TODO : beep the buzzer with some tune

    	// TODO : Display some simple animation on LEDs


       	food_draw = 1; // new food needs to be drawn next time
      
      	if(food_pos[0]>7)
      	{
        	// remove the food from LED 2
        	set_led_matrix(1, food_pos[0]%8, food_pos[1], 0 );
      	}
      	else
      	{
        	// remove the food from LED 1
        	set_led_matrix(0, food_pos[0], food_pos[1], 0 );
      	}

      	// increment the length variable
 	  	v_len++;  

 	  	// grow the snake according to the direction
      	switch(v_dir)
      	{
        	case 1: 
            	n_snake[v_len-1][0] = food_pos[0];
            	n_snake[v_len-1][1] = food_pos[1]+1;
            	snake[v_len-1][0] = food_pos[0];
            	snake[v_len-1][1] = food_pos[1]+1;
            	break;
        	case 2:
	            n_snake[v_len-1][0] = food_pos[0]+1;
	            n_snake[v_len-1][1] = food_pos[1];
	            snake[v_len-1][0] = food_pos[0]+1;
	            snake[v_len-1][1] = food_pos[1];
	            break;
        	case 3:
	            n_snake[v_len-1][0] = food_pos[0];
	            n_snake[v_len-1][1] = food_pos[1]-1;
	            snake[v_len-1][0] = food_pos[0];
	            snake[v_len-1][1] = food_pos[1]-1;
	            break;
        	case 4:
	            n_snake[v_len-1][0] = food_pos[0]-1;
	            n_snake[v_len-1][1] = food_pos[1];
	            snake[v_len-1][0] = food_pos[0]-1;
	            snake[v_len-1][1] = food_pos[1];
	            break;                   
        }          
       snake_speed -= 7; // increase the speed by decreasing the delay
    }

   	// check for snake collision  
   	// if head hits any part on snake body the game is over
    for(i = 1; i < v_len-1; i++)
    {
        if((n_snake[i][0] == n_snake[0][0])&&(n_snake[i][1] == n_snake[0][1]))
        {
			
            // TODO : Scroll "GAMEOVER" on the display

            // check if player's score beats the highscore
            if(v_len - 3 > highscore)
			{
		        // TODO : some winner music on buzzer
		        
		        // TODO : Scroll "YOU BEAT THE HIGHSCORE" on display

		        highscore = v_len - 3; //initial length is 3

		        // TODO : store the highscore in the eeprom

		   	}  

            // TODO : display player's Score and high score

		   	  v_len = 2; // reset the snake length
		   	  gameover = 1; // set gameover variable
          snake_speed = 150; // reset the snake speed
          break; // break from the for loop
        }

    }  
    delay_ms(snake_speed); // delay of the snake (which ultimately affects the game speed)      
} 

int main(void)
{
	init_game(); // initialise the snake & led
	while(1) // game loop
	{
		if(!gameover)
	        snake_main(snake_direction());
	    else
	        gameover = 0;
	}               
}
