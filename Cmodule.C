#include <wiringPi.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#define countElements 20000
#define countBuf 3
#define countBufArray 10

enum workType{
Ready,
Write,
Pause
};

int n=0;
char out[countBufArray];
bool State_A, State_B;//каналы gpio
bool Mah=true;//фикс движения обратно
double Time[countElements];//массив с временем
int Coord[countElements];//массив с координатами
long Coordinate = 0;//координата
int count=0;//счетчик в массиве
struct timeval start;//время от запуска всей программы(не нашел другой метод,поэтому просто будем вычитать это значение из времени полученного при срабатывания прерывания)
struct timeval timevals[countElements];//массив структуры миллисекунд
int stopReadFromPipe=100000;//остановка на 1/10 сек для считывания с pipe
char readbuffer[countBuf];//буфер для чтения
char bufe[countBuf];//буфер для чтения
char Channel='o';//напралвение движения
int pendPoint=10;//точка начала отчета
int offsetPointMax=2;//максимально возможное смещение в сторону противоположную перемещению маятника при измерении маха
enum workType typeWork=Ready;//режим сенсора

void timevalToDouble(){//перевод из формата timaval в double
for(int i=0;i<=count;i++)
Time[i]=(double)(timevals[i].tv_usec - start.tv_usec) / 1000000 + (double)(timevals[i].tv_sec - start.tv_sec);//вычисляем разность времени для каждого тика
}

void Clear(){
for(int i=0;i<=count;i++){
Time[i]=0;
Coord[i]=0;
timevals[i]=(struct timeval){0};
}
start = (struct timeval){0};
count=0;
}

void getCurrentCoordinate(int write){ //получить текующую координату
n=sprintf(out, "%ld\n", Coordinate);
write(write, out, n);
}

void getDataFromSensor(int write){
if(count<1){
write(write, "N", 1);
return;
}
timevalToDouble();//преобразование времени

n=sprintf(out, "%ld\n", count);
write(write, out, n);
  
  
for(int i=0;i<=count;i++){
n=sprintf(out, "%f\n", Time[i]);
write(write, out, n);

n=sprintf(out, "%ld\n", Coord[i]);
write(write, out, n);

}
Clear();
}

void ISR_A()//прерывание по каналу A
{
switch(typeWork){//смотрим текущее состояние работы

case Ready ://ожидание смещения для записи
State_A = !State_A;
if (State_B != State_A){
Coordinate++;
if(abs(Coordinate)>pendPoint){//проверка на смещение относительно начала
typeWork=Write;//переход в режим записи
Channel='+';
}//учет направления движения
}
else {
Coordinate--;
if(abs(Coordinate)>pendPoint){//переход в режим записи
typeWork=Write;
Channel='-';
}//учет направления движения
}
break;

case Write ://запись данных в память
State_A = !State_A;
if (State_B != State_A){//движение в одну сторону
if(Channel=='+'){//если текущее направление движения совпадает с начальным-пишем
Coordinate++;
Coord[count]=abs(Coordinate);//коордианта
gettimeofday(&timevals[count], NULL);//время
count++;//ячейка массива
Mah=true;//снимаем фиксацию с offsetPointMax
}
else{//если пошли обратно
if(Mah){//если первое отклонение назад
offsetPointMax=abs(Coordinate)-offsetPointMax;//записываем начальную коордианту откуда пошло движение назад
Mah=false;//фиксируем коордианту откуда пошло движение назад
}
Coordinate++;
if(abs(Coordinate)<=offsetPointMax){//смотрим на сколько сместилось
typeWork=Pause;//если да перестаем писать
}

}
}
else {//движение в противоположную сторону


if(Channel=='-'){//если текущее направление движения совпадает с начальным-пишем
Coordinate--;
Coord[count]=abs(Coordinate);
gettimeofday(&timevals[count], NULL);
count++;
Mah=true;
}
else{//иначе пошли обратно
if(Mah){//если первое или последовательное отклонение назад
offsetPointMax=abs(Coordinate)-offsetPointMax;
Mah=false;
}
Coordinate--;
if(abs(Coordinate)<=offsetPointMax){
typeWork=Pause;
}
}

}
break;
case Pause://просто отслеживаем маятик без записи в память
State_A = !State_A;
if (State_B != State_A)
Coordinate++;
else {
Coordinate--;
}
break;

}
}

void ISR_B()
{
switch(typeWork){

case Ready ://ожидание сдвига маятника на n тиков
State_B = !State_B;
if (State_B == State_A){
Coordinate++;
if(abs(Coordinate)>pendPoint){//переход в режим записи
typeWork=Write;
Channel='+';
}//учет направления движения
}
else {
Coordinate--;
if(abs(Coordinate)>pendPoint){//переход в режим записи
typeWork=Write;
Channel='-';
}//учет направления движения
}
break;

case Write ://запись маха
State_B = !State_B;
if (State_B == State_A){//движение в одну сторону
if(Channel=='+'){//если текущее направление движения совпадает с начальным-пишем
Coordinate++;
Coord[count]=abs(Coordinate);
gettimeofday(&timevals[count], NULL);
count++;
Mah=true;
}
else{//иначе пошли обратно
if(Mah){//если первое или последовательное отклонение назад
offsetPointMax=abs(Coordinate)-offsetPointMax;
Mah=false;
}
Coordinate++;
if(abs(Coordinate)<=offsetPointMax){
typeWork=Pause;
}

}
}
else {//движение в противоположную сторону
if(Channel=='-'){//если текущее направление движения совпадает с начальным-пишем
Coordinate--;
Coord[count]=abs(Coordinate);
gettimeofday(&timevals[count], NULL);
count++;
Mah=true;
}
else{//иначе пошли обратно
if(Mah){//если первое или последовательное отклонение назад
offsetPointMax=abs(Coordinate)-offsetPointMax;
Mah=false;
}
Coordinate--;
if(abs(Coordinate)<=offsetPointMax){
typeWork=Pause;
}
}

}

break;

case Pause ://просто отслеживаем маятик без записи в память
State_B = !State_B;
if (State_B == State_A)
Coordinate++;
else {
Coordinate--;
}
break;

}
}



int main()
{
readbuffer[0]='0';
int fd[2];
pipe(fd);
wiringPiSetupGpio (); //BCM mode
pinMode (23, INPUT);
pinMode(24, INPUT);
State_A = digitalRead(23);
State_B = digitalRead(24);
while(1) {
read(fd[0], readbuffer, sizeof(readbuffer));

if(readbuffer[0]== 'E')exit(0);//Выход
if(readbuffer[0]== 'N'){//получение текущей координаты
getCurrentCoordinate(fd[1]);
}
if(readbuffer[0]== 'W'){//Запись в буфер маха
Clear();
gettimeofday(&start, NULL);
typeWork=Ready;
}
if(readbuffer[0]== 'M'){//Отдать данные в систему
typeWork=Pause;
getDataFromSensor(fd[1]);
typeWork=Ready;
}
if(readbuffer[0]== 'C'){//Сброс
Clear();
Coordinate=0;
Channel='o';
pendPoint=0;
offsetPointMax=0;
}
if(readbuffer[0]== 'S'){//Изменение чувствительнсоти
{
  read(fd[0], readbuffer, sizeof(readbuffer));

for(int i=0;i<sizeof(readbuffer);i++){
    bufe[i]=readbuffer[i];
}
pendPoint=atoi(bufe);
  
read(fd[0], readbuffer, sizeof(readbuffer));

for(int i=0;i<sizeof(readbuffer);i++){
    bufe[i]=readbuffer[i];
}
pendPoint=atoi(bufe);
  
  
}
usleep(stopReadFromPipe);// сон после выполнения операция на пол сек

}
}
}
