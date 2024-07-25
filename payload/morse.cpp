#include "morse.h"

// Morse encode
std::string morse_encode(wchar_t x)
{ 
  switch (x) { 
  case 'a': 
  case 'A':
    return ".-"; 
  case 'b':
  case 'B': 
    return "-..."; 
  case 'c':
  case 'C': 
    return "-.-."; 
  case 'd':
  case 'D': 
    return "-.."; 
  case 'e': 
  case 'E':
    return "."; 
  case 'f':
  case 'F': 
    return "..-."; 
  case 'g':
  case 'G': 
    return "--."; 
  case 'h':
  case 'H': 
    return "...."; 
  case 'i':
  case 'I': 
    return ".."; 
  case 'j':
  case 'J': 
    return ".---"; 
  case 'k':
  case 'K':
    return "-.-"; 
  case 'l': 
  case 'L':
    return ".-.."; 
  case 'm':
  case 'M': 
    return "--"; 
  case 'n':
  case 'N': 
    return "-."; 
  case 'o': 
  case 'O':
    return "---"; 
  case 'p':
  case 'P': 
    return ".--."; 
  case 'q':
  case 'Q': 
    return "--.-"; 
  case 'r':
  case 'R': 
    return ".-."; 
  case 's':
  case 'S': 
    return "..."; 
  case 't':
  case 'T': 
    return "-"; 
  case 'u':
  case 'U': 
    return "..-"; 
  case 'v':
  case 'V': 
    return "...-"; 
  case 'w':
  case 'W': 
    return ".--"; 
  case 'x':
  case 'X': 
    return "-..-"; 
  case 'y':
  case 'Y': 
    return "-.--"; 
  // for space 
  case 'z':
  case 'Z': 
    return "--.."; 
  // Finland stronk
  case '\xC5': // Å
  case '\xE5': // å
    return ".--.-";
  case '\xC4': // Ä
  case '\xE4': // ä
    return ".-.-";
  case '\xD6': // Ö
  case '\xF4': // ö
    return "---.";
  case '\xDC': // Ü
  case '\xFC': // ü
    return "---.";
  case '1':
    return ".----";
  case '2':
    return "..---";
  case '3':
    return "...--";
  case '4':
    return "....-";
  case '5':
    return ".....";
  case '6':
    return "-....";
  case '7':
    return "--...";
  case '8':
    return "---..";
  case '9':
    return "----.";
  case '0':
    return "-----";
  case ' ':
  default:
    return " ";
  } 
}

