#include "GPS.h"


UnbufferedSerial *gps_rawSerial;

char lastNMEASentence[84];
uint8_t sentenceIndex = 0;
void sendCommand(const char *str);
bool parseNMEA(char *nmea);

void resetNMEA();

GePeS *gps;

char gps_serial_getc()
{
    char input[1];
    gps_rawSerial->read(input, 1);
    return input[0];
}


void gpsCallback()
{
    while(gps_rawSerial->readable()){
        char c = gps_serial_getc();
        if(c == 0x24) // New line
        {
            resetNMEA();
        }else if(c == 0x0a){ // EOL
            parseNMEA(lastNMEASentence);
            resetNMEA();
        }
        if(sentenceIndex < 84){
            lastNMEASentence[sentenceIndex++] = c;
        }else{ //flush input
            while(gps_rawSerial->readable()){
                gps_serial_getc();
            }
            resetNMEA();
        }
    }
}

void resetNMEA(){
    memset(lastNMEASentence, 0, sizeof(lastNMEASentence));
    sentenceIndex = 0;
}

void GPS_begin(UnbufferedSerial *ser, GePeS *_gps){
    gps_rawSerial = ser;
    gps = _gps;

    
  gps->hour = gps->minute = gps->seconds = gps->year = gps->month = gps->day = 0;
  gps->milliseconds = 0;
  gps->latitude = gps->longitude = gps->geoidheight = gps->altitude = 0;
  gps->speed = gps->angle = gps->magvariation = gps->HDOP = 0;
  gps->lat = gps->lon = gps->mag = 0;
  gps->fix = 0;
  gps->fixquality = gps->satellites = 0;

    gps_rawSerial->baud(9600);    
    gps_rawSerial->attach(gpsCallback);

    sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    sendCommand(PMTK_SET_NMEA_UPDATE_10s);
    sendCommand(PGCMD_NOANTENNA); 
}


void sendCommand(const char *str) {
  char command[strlen(str)+2];
  sprintf(command, "%s\r\n", str);

  gps_rawSerial->write(command, strlen(command));
}


uint8_t parseHex(char c) {
    if (c < '0')
      return 0;
    if (c <= '9')
      return c - '0';
    if (c < 'A')
       return 0;
    if (c <= 'F'){}
       return (c - 'A')+10;
    return 0;
}

bool matchNMEAType(char *nmea, const char *type)
{
    for(int i = 0; i < 3; i++)
    {
        if(nmea[i+3] != type[i])
        {
            return false;
        }
    }
    return true;
}

bool parseNMEA(char *nmea) {
  // do checksum check

  // first look if we even have one
  if (nmea[strlen(nmea)-4] == '*') {
    uint16_t sum = parseHex(nmea[strlen(nmea)-3]) * 16;
    sum += parseHex(nmea[strlen(nmea)-2]);
    
    // check checksum 
    for (uint8_t i=1; i < (strlen(nmea)-4); i++) {
      sum ^= nmea[i];
    }
    if (sum != 0) {
      // bad checksum :(
      //return false;
    }
  }

  // look for a few common sentences
  if (matchNMEAType(nmea, "GGA")) {
    // found GGA
    char *p = nmea;
    // get time
    p = strchr(p, ',')+1;
    float timef = atof(p);
    uint32_t time = timef;
    gps->hour = time / 10000;
    gps->minute = (time % 10000) / 100;
    gps->seconds = (time % 100);

    gps->milliseconds = fmod((double) timef, 1.0) * 1000;

    // parse out latitude
    p = strchr(p, ',')+1;
    gps->latitude = atof(p);

    p = strchr(p, ',')+1;
    if (p[0] == 'N') gps->lat = 'N';
    else if (p[0] == 'S') gps->lat = 'S';
    else if (p[0] == ',') gps->lat = 0;
    else return false;

    // parse out longitude
    p = strchr(p, ',')+1;
    gps->longitude = atof(p);

    p = strchr(p, ',')+1;
    if (p[0] == 'W') gps->lon = 'W';
    else if (p[0] == 'E') gps->lon = 'E';
    else if (p[0] == ',') gps->lon = 0;
    else return false;

    p = strchr(p, ',')+1;
    gps->fixquality = atoi(p);

    p = strchr(p, ',')+1;
    gps->satellites = atoi(p);

    p = strchr(p, ',')+1;
    gps->HDOP = atof(p);

    p = strchr(p, ',')+1;
    gps->altitude = atof(p);
    p = strchr(p, ',')+1;
    p = strchr(p, ',')+1;
    gps->geoidheight = atof(p);
    return true;
  }
  if (matchNMEAType(nmea, "$GPRMC")) {
   // found RMC
    char *p = nmea;

    // get time
    p = strchr(p, ',')+1;
    float timef = atof(p);
    uint32_t time = timef;
    gps->hour = time / 10000;
    gps->minute = (time % 10000) / 100;
    gps->seconds = (time % 100);

    gps->milliseconds = fmod((double) timef, 1.0) * 1000;

    p = strchr(p, ',')+1;
    // Serial.println(p);
    if (p[0] == 'A') 
      gps->fix = 1;
    else if (p[0] == 'V')
      gps->fix = 0;
    else
      return false;

    // parse out latitude
    p = strchr(p, ',')+1;
    gps->latitude = atof(p);

    p = strchr(p, ',')+1;
    if (p[0] == 'N') gps->lat = 'N';
    else if (p[0] == 'S') gps->lat = 'S';
    else if (p[0] == ',') gps->lat = 0;
    else return false;
    if(gps->lat == 'S'){
        gps->latitude = -1 * gps->latitude;
    }

    // parse out longitude
    p = strchr(p, ',')+1;
    gps->longitude = atof(p);

    p = strchr(p, ',')+1;
    if (p[0] == 'W') gps->lon = 'W';
    else if (p[0] == 'E') gps->lon = 'E';
    else if (p[0] == ',') gps->lon = 0;
    else return false;
    
    if(gps->lon == 'W'){
        gps->longitude = -1 * gps->longitude;
    }

    // speed
    p = strchr(p, ',')+1;
    gps->speed = atof(p);

    // angle
    p = strchr(p, ',')+1;
    gps->angle = atof(p);

    p = strchr(p, ',')+1;
    uint32_t fulldate = atof(p);
    gps->day = fulldate / 10000;
    gps->month = (fulldate % 10000) / 100;
    gps->year = (fulldate % 100);

    // we dont parse the remaining, yet!
    return true;
  }

  return false;
}
