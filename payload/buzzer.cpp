#include "buzzer.h"
#include "wd.h"
#define BUZZER_ON 1;


void start_morse_thread();

DigitalOut buzzer(BUZZER_PIN);
//DigitalOut burn(RBF);

void morse_thread();

uint8_t MORSE_SIGNAL_ON = 0;

char morse_str[50];

void buzzer_init(void)
{
    buzzer = 0;
    _debug("Buzzer loaded.\n\r");
}
/*
void burn10s(void)
{
    burn = 1;
    for(int i = 0; i<100; i++)
    {
        feed_the_dog();
        ThisThread::sleep_for(100ms);
    }
    burn = 0;
}
*/
void beep(char times)
{   
    for(int i=0; i<times; i++) {
        buzzer = 1;
        ThisThread::sleep_for(200ms);
        buzzer = 0;
        ThisThread::sleep_for(200ms);
        feed_the_dog();
    }
}


void bz_on() {
    buzzer = BUZZER_ON;
}

void bz_off() {
    buzzer = 0;
}

void quick_beep()
{
    bz_on();
    ThisThread::sleep_for(100ms);
    bz_off();
}

void beep_long()
{
    buzzer = 1; //BUZZER_ON;
    ThisThread::sleep_for(THREE_UNITS);
    buzzer = 0; 
    ThisThread::sleep_for(ONE_UNIT);
}

void beep_short()
{
    buzzer = 1; //BUZZER_ON;
    ThisThread::sleep_for(ONE_UNIT);
    buzzer = 0;
    ThisThread::sleep_for(ONE_UNIT);
}

void letter_space()
{
    ThisThread::sleep_for(TWO_UNITS);
}

void word_space()
{
    ThisThread::sleep_for(SIX_UNITS);
}

void play_morse(char c)
{
    if ( c == '-')
    {
        beep_long();
        return;
    }
    else if ( c == '.')
    {
        beep_short();
        return;
    }
    else {
        word_space();
    }
}

void morse_thread()
{
    //osSignalThisThread::sleep_for(0x01, osThisThread::sleep_forForever);
    if(MORSE_SIGNAL_ON)
    {
        int i = 0, j=0;
        std:string encoded_str = "";
        
        while(i < strlen(morse_str))
        {
            char c = morse_str[i];

            encoded_str = morse_encode(c);
            _debug("%s\n\r", encoded_str.c_str());
            i++;

            j = 0;
            while( j < (encoded_str).length())
            {
                play_morse(encoded_str[j]);
                j++;
            }
            letter_space();
            feed_the_dog();
        }
        MORSE_SIGNAL_ON = 0;
    }
}

void morse(char* mors)
{
    sprintf(morse_str, "%s", mors);
    MORSE_SIGNAL_ON = 1;
    //osSignalSet(threadId, 0x01);
}

// Command interface

void BEEP(char *_params) 
{
    // Beep number of times

    char times = (_params[0] - '0');
    
    times > 10 ? times = 10 : times;
    times < 0 ? times = 1 : times;

    _debug("Beep %d times\n\r", times);
    
    beep(times);
}

void STOP_BEEP(char *_params) 
{
    bz_off();
}

void MORSE(char *_params)
{
    morse(_params);   
}