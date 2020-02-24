#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pigpio.h>
#include "rotary_encoder.h"
#define countElements 20000
#define countBuf 3
#define countBufArray 10

enum workType
{
  Ready,
  Write,
  Pause
};

int n = 0;
char out[countBufArray];
bool State_A, State_B;	
bool Mah = true;	
double Time[countElements];	
int Coord[countElements];
static short Coordinate = 0;	
static short saveWay = 0;	
int count = 0;			
struct timeval start;	
struct timeval timevals[countElements];	
int stopReadFromPipe = 100000;	
char readbuffer[countBuf];	
char bufe[countBuf];	
char Channel = 'o';		
short pendPoint = 10;		
short pendOffsetNow = 0;	
short pendOffset = 10;	
enum workType typeWork = Ready;	

void
timevalToDouble ()
{			
  for (int i = 0; i <= count; i++)
    Time[i] = (double) (timevals[i].tv_usec - start.tv_usec) / 1000000 + (double) (timevals[i].tv_sec - start.tv_sec);	
}

void
Clear ()
{
  for (int i = 0; i <= count; i++)
    {
      Time[i] = 0;
      Coord[i] = 0;
      timevals[i] = (struct timeval)
      {
      0};
    }
  start = (struct timeval)
  {
  0};
  count = 0;
}

void
getCurrentCoordinate ()
{		
  sprintf (out, "%d\n", Coordinate);
  fputs (out, stdout);
  fflush(stdout);
}

void
getDataFromSensor ()
{
  if (count < 1)
    {
      fputs ("N\n", stdout);
      fflush(stdout);
      return;
    }
  timevalToDouble ();		

  n = sprintf (out, "%d\n", count);
  fputs (out, stdout);
  fflush(stdout);


  for (int i = 0; i <= count-1; i++)
    {
      n = sprintf (out, "%f\n", Time[i]);
      fputs (out, stdout);
      fflush(stdout);

      n = sprintf (out, "%d\n", Coord[i]);
      fputs (out, stdout);
      fflush(stdout);

    }
  Clear ();
}




void callback(int way)
{
   saveWay=way;
   Coordinate += way;
   switch(typeWork){
	    
   case Ready:
   if (abs(Coordinate) > pendPoint){

	      if(Coordinate>0){
	         Channel = '+';
	         typeWork = Write;

		 }
	      else{
	      Channel = '-';
	      typeWork = Write;
	    }			

}

   break;

   
   
   
   
   
   case Write:
	      Coord[count] = abs (Coordinate);
	      gettimeofday (&timevals[count], NULL);
	      count++;
	      switch (Channel){
	          case '+':
	          if(saveWay<0){
	              if(Mah==true)
	              pendOffsetNow = Coordinate - pendOffset;
	              if(pendOffsetNow>=Coordinate){
	              typeWork=Pause;
	              Mah=true;
	              }
	               Mah=false;
	          }
	          else
	          {
	              Mah=true;
	          }
	          
	          case '-':
	          if(saveWay>0){
	              if(Mah==true)
	              pendOffsetNow = Coordinate + pendOffset;
	              if(pendOffsetNow<=Coordinate){
	              typeWork=Pause;
	              Mah=true;
	              }
	                    Mah=false;
	          }
	          else
	          {
	              Mah=true;
	          }
	      }
	      	  
   
   
   break;
   
   case Pause:
   break;
   
   
   default:
   break;

}
}



int main ()
{
  typeWork=Pause;
  Pi_Renc_t * renc;
  if (gpioInitialise() < 0) return 1;
  renc = Pi_Renc(17, 27, callback);
  readbuffer[0] = '0';
  while (1)
    {
      fgets (readbuffer, countBuf, stdin);

      if (readbuffer[0] == 'E')
	exit (0);	
      if (readbuffer[0] == 'N')
	{		
	  getCurrentCoordinate ();
	}
      if (readbuffer[0] == 'W')
	{		
	  Clear ();
	  gettimeofday (&start, NULL);
	  Channel='o';
	  saveWay=0;
	  typeWork = Ready;
	  
	}
      if (readbuffer[0] == 'M')
	{		
	  typeWork = Pause;
	  getDataFromSensor ();
	  typeWork = Ready;
	}
      if (readbuffer[0] == 'C')
	{		
	  Clear ();
	  Coordinate = 0;
	  Channel = 'o';
	  pendOffset = 0;
	  pendOffsetNow = 0;
	}
      if (readbuffer[0] == 'S')
	{		
	  {
	    fgets (readbuffer, countBuf, stdin);

	    for (int i = 0; i < sizeof (readbuffer); i++)
	      {
		bufe[i] = readbuffer[i];
	      }
	    pendPoint = atoi (bufe);

	    fgets (readbuffer, countBuf, stdin);

	    for (int i = 0; i < sizeof (readbuffer); i++)
	      {
		bufe[i] = readbuffer[i];
	      }
	    pendOffset = atoi (bufe);


	  }
	  usleep (stopReadFromPipe);	

	}
    }
}
