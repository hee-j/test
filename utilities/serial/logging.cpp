#include "logging.h"
#include <stdarg.h>
#include "serial.h"

extern uint8_t  DEBUGGING_ON;


void _ack(const char *fmt, ...)
{
    va_list ap; 
    const char *p, *sval;
    int ival;
    double dval;
    va_start(ap, fmt);

    if ( DEBUGGING_ON )
    {
        for (p = fmt; *p; p++) 
        {
            if (*p != '%') 
            {
                pc_putc(*p);
                continue;
            }
            switch (*++p) 
            {
                case 'd':
                    ival = va_arg(ap, int);
                    pc_printf("%d", ival);
                    break;
                case 'f':
                    dval = va_arg(ap, double);
                    pc_printf("%f", dval);
                    break;
                case 's':
                    for (sval = va_arg(ap, char *); *sval; sval++)
                    pc_putc(*sval);
                    break;
                default:
                    pc_putc(*p);
                    break;
            }
        }
        va_end(ap);

        fflush(stdout);
    }
}

void _debug(const char *fmt, ...)
{

    va_list ap;
    char *p, *sval;
    int ival;
    double dval;
    va_start(ap, fmt);

    if ( DEBUGGING_ON )
    {
        pc_printf("\033[0;34m");
        pc_printf("[i]");
        pc_printf("\033[0m");
        pc_printf(" ");

        for (p = (char *) fmt; *p; p++) 
        {
            if (*p != '%') 
            {
                pc_putc(*p);
                continue;
            }
            switch (*++p) 
            {
                case 'd':
                    ival = va_arg(ap, int);
                    pc_printf("%d", ival);
                    break;
                case 'f':
                    dval = va_arg(ap, double);
                    pc_printf("%f", dval);
                    break;
                case 's':
                    for (sval = va_arg(ap, char *); *sval; sval++)
                    pc_putc(*sval);
                    break;
                default:
                    pc_putc(*p);
                    break;
            }
        }
        va_end(ap);

        fflush(stdout);
    }
}

void _error(const char *fmt, ...)
{
    va_list ap;
    const char *p, *sval;
    int ival;
    double dval;
    va_start(ap, fmt); 

    pc_printf("\033[0;31m");
    pc_printf("[ERR]");
    pc_printf("\033[0m");
    pc_printf(" ");

    for (p = fmt; *p; p++) 
    {
      if (*p != '%') 
      {
         pc_putc(*p);
         continue;
      }
      switch (*++p) 
      {
         case 'd':
            ival = va_arg(ap, int);
            pc_printf("%d", ival);
            break;
        case 'f':
            dval = va_arg(ap, double);
            pc_printf("%f", dval);
            break;
        case 's':
            for (sval = va_arg(ap, char *); *sval; sval++)
            pc_putc(*sval);
            break;
        default:
            pc_putc(*p);
            break;
      }
   }
   va_end(ap);
   fflush(stdout);

}

void _warning(const char *fmt, ...)
{
    va_list ap;
    const char *p, *sval;
    int ival;
    double dval;
    va_start(ap, fmt);

    pc_printf("\033[0;33m");
    pc_printf("[!]");
    pc_printf("\033[0m");
    pc_printf(" ");

    for (p = fmt; *p; p++) 
    {
      if (*p != '%') 
      {
         pc_putc(*p);
         continue;
      }
      switch (*++p) 
      {
         case 'd':
            ival = va_arg(ap, int);
            pc_printf("%d", ival);
            break;
        case 'f':
            dval = va_arg(ap, double);
            pc_printf("%f", dval);
            break;
        case 's':
            for (sval = va_arg(ap, char *); *sval; sval++)
            pc_putc(*sval);
            break;
        default:
            pc_putc(*p);
            break;
      }
   }

   va_end(ap);
   fflush(stdout);
}