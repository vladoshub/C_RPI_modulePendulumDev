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
static short Coordinate2 = 0;	
int count = 0;			
struct timeval start;	
struct timeval timevals[countElements];	
int stopReadFromPipe = 100000;	
char readbuffer[countBuf];	
char bufe[countBuf];	
char Channel = 'o';		
int pendPoint = 10;		
int offsetPointMax = 2;	
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
  fputs ("\n", stdout);
}

void
getDataFromSensor ()
{
  if (count < 1)
    {
      fputs ("N", stdout);
      fputs ("\n", stdout);
      return;
    }
  timevalToDouble ();		

  n = sprintf (out, "%d\n", count);
  fputs (out, stdout);
  fputs ("\n", stdout);


  for (int i = 0; i <= count; i++)
    {
      n = sprintf (out, "%f\n", Time[i]);
      fputs (out, stdout);
      fputs ("\n", stdout);

      n = sprintf (out, "%l\n", Coord[i]);
      fputs (out, stdout);
      fputs ("\n", stdout);

    }
  Clear ();
}

void
ISR_A ()		
{
  switch (typeWork)
    {			

    case Ready:		
      State_A = !State_A;
      if (State_B != State_A)
	{
	  Coordinate++;
	  if (abs (Coordinate) > pendPoint)
	    {			
	      typeWork = Write;	
	      Channel = '+';
	    }		
	}
      else
	{
	  Coordinate--;
	  if (abs (Coordinate) > pendPoint)
	    {		
	      typeWork = Write;
	      Channel = '-';
	    }			
	}
      break;

    case Write:		
      State_A = !State_A;
      if (State_B != State_A)
	{			
	  if (Channel == '+')
	    {		
	      Coordinate++;
	      Coord[count] = abs (Coordinate);
	      gettimeofday (&timevals[count], NULL);
	      count++;		
	      Mah = true;	
	    }
	  else
	    {		
	      if (Mah)
		{	
		  Mah = false;	
		}
	      Coordinate++;
	      if (abs (Coordinate) <= offsetPointMax)
		{		
		  typeWork = Pause;	
		}

	    }
	}
      else
	{			


	  if (Channel == '-')
	    {		
	      Coordinate--;
	      Coord[count] = abs (Coordinate);
	      gettimeofday (&timevals[count], NULL);
	      count++;
	      Mah = true;
	    }
	  else
	    {			
	      if (Mah)
		{	
		  offsetPointMax = abs (Coordinate) - offsetPointMax;
		  Mah = false;
		}
	      Coordinate--;
	      if (abs (Coordinate) <= offsetPointMax)
		{
		  typeWork = Pause;
		}
	    }

	}
      break;
    case Pause:		
      State_A = !State_A;
      if (State_B != State_A)
	Coordinate++;
      else
	{
	  Coordinate--;
	}
      break;

    }
}

void
ISR_B ()
{
  switch (typeWork)
    {

    case Ready:		
      State_B = !State_B;
      if (State_B == State_A)
	{
	  Coordinate++;
	  if (abs (Coordinate) > pendPoint)
	    {		
	      typeWork = Write;
	      Channel = '+';
	    }			
	}
      else
	{
	  Coordinate--;
	  if (abs (Coordinate) > pendPoint)
	    {		
	      typeWork = Write;
	      Channel = '-';
	    }		
	}
      break;

    case Write:	
      State_B = !State_B;
      if (State_B == State_A)
	{		
	  if (Channel == '+')
	    {		
	      Coordinate++;
	      Coord[count] = abs (Coordinate);
	      gettimeofday (&timevals[count], NULL);
	      count++;
	      Mah = true;
	    }
	  else
	    {		
	      if (Mah)
		{	
		  offsetPointMax = abs (Coordinate) - offsetPointMax;
		  Mah = false;
		}
	      Coordinate++;
	      if (abs (Coordinate) <= offsetPointMax)
		{
		  typeWork = Pause;
		}

	    }
	}
      else
	{			
	  if (Channel == '-')
	    {		
	      Coordinate--;
	      Coord[count] = abs (Coordinate);
	      gettimeofday (&timevals[count], NULL);
	      count++;
	      Mah = true;
	    }
	  else
	    {			
	      if (Mah)
		{	
		  offsetPointMax = abs (Coordinate) - offsetPointMax;
		  Mah = false;
		}
	      Coordinate--;
	      if (abs (Coordinate) <= offsetPointMax)
		{
		  typeWork = Pause;
		}
	    }

	}

      break;

    case Pause:	
      State_B = !State_B;
      if (State_B == State_A)
	Coordinate++;
      else
	{
	  Coordinate--;
	}
      break;

    }
}


void callback(int way)
{
   Coordinate2=Coordinate;
   Coordinate += way;
   switch(typeWork){
	    
   case Ready:
   if (abs(Coordinate) > pendPoint){
	    {		
	      typeWork = Write;
	      if(Coordinate>0)
	      Channel = '+';
	      else
	      Channel = '-';
	    }			
	}
   break;
   
   
   
   
   
   case Write:
	      Coord[count] = abs (Coordinate);
	      gettimeofday (&timevals[count], NULL);
	      count++;
	      switch (Channel){
	          case '+':
	          if(Coordinate2>=Coordinate){
	              if(Mah==true)
	              offsetPointMax = Coordinate - offsetPointMax;
	              if(offsetPointMax=>Coordinate)
	              typeWork=Pause;
	              Mah=false;
	          }
	          else
	          {
	              Mah=true;
	              offsetPointMax=10;
	          }
	          
	          case '-':
	          if(Coordinate2<=Coordinate){
	              if(Mah==true)
	              offsetPointMax = Coordinate + offsetPointMax;
	              if(offsetPointMax<=Coordinate)
	              typeWork=Pause;
	              Mah=false;
	          }
	          else
	          {
	              Mah=true;
	              offsetPointMax=10;
	          }
	      }
	      	  
   
   
   break;
   
   
   default:
   break;

}
}



int main ()
{
  _Renc_t * renc;
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
	  pendPoint = 0;
	  offsetPointMax = 0;
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
	    offsetPointMax = atoi (bufe);


	  }
	  usleep (stopReadFromPipe);	

	}
    }
}
