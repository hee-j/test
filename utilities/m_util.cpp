#include "m_util.h"
//#include "legacy_comm.h"
#include "serial/serial.h"
#include "serial/logging.h"
#include "../loop.h"

extern char transmissionMode; // settings.cpp

int min(int a, int b)
{
  if(a>b)
  {
    return b;  
  }
  return a;
}

int max(int a, int b)
{
  if(a>b)
  {
    return a;   
  }   
  return b;
}

int remove_duplicate_elements(int arr[], int n)
{
    if (n==0 || n==1)
        return n;

    int temp[n];

    int j = 0;
    int i;
    for (i=0; i<n-1; i++)
        if (arr[i] != arr[i+1])
            temp[j++] = arr[i];
    temp[j++] = arr[n-1];

    for (i=0; i<j; i++)
        arr[i] = temp[i];

    return j;
}
void print_memory_info() {
    // allocate enough room for every thread's stack statistics
    int cnt = osThreadGetCount();
    mbed_stats_stack_t *stats = (mbed_stats_stack_t*) malloc(cnt * sizeof(mbed_stats_stack_t));
 
    cnt = mbed_stats_stack_get_each(stats, cnt);
    for (int i = 0; i < cnt; i++) {
        printf("Thread: 0x%X, Stack size: %u / %u\r\n", stats[i].thread_id, stats[i].max_size, stats[i].reserved_size);
    }
    free(stats);
 
    // Grab the heap statistics
    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);
    printf("Heap size: %u / %u bytes\r\n", heap_stats.current_size, heap_stats.reserved_size);
}


int str2int(const char *s)
{
    // returns zero when not succesful
    char *ptr;
    return std::strtol(s, &ptr, 10);
}

void printPacket(char *packet, int packetLength)
{
        for(int i = 0; i<packetLength; i++)
        {
         pc_printf("%02X", packet[i]);
         if(i<packetLength-1)
         {
           pc_printf("-");  
         }   
        }
       pc_printf("\n");
}


long chartobin(char c)
{
    int i, j=0;
    char binarray[8];
    memset(binarray,0,sizeof(binarray));
    
    
    for(i=7; i>=0; i--) {
        binarray[j] = ( ( c & (1 << i)) ? '1' : '0' );
        j++;
    }
    
    return strtol(binarray, NULL, 2);
}

int isDigit(char c)
{
     char digits[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
     for(int i=0; i<10; i++)
     {
        if(c==digits[i])
        {
          return true;
        }  
     } 
     return false;
}

int int2bool(int i){
    if(i){
        return 1;
    }
    return 0;
}

uint32_t ufnv(char *bytes, int str_len)
{
  uint32_t hval = 0x811c9dc5;
  uint32_t fnv_32_prime = 0x01000193;
  uint64_t uint32_max = 4294967296;
  for(int i = 0; i< str_len; i++)
  {
    hval = hval ^ bytes[i];
    hval = (hval * fnv_32_prime) % uint32_max; 
  }
  
  return hval;
}

int checkFNV(char *data, int length)
{
    uint32_t local_fnv = ufnv(data, length-4);
    uint32_t parsed_fnv = *(uint32_t *) &data[length-4];

    return (local_fnv == parsed_fnv);
}

void system_rst() 
{
    _debug("Rebooting.\n\r");
    cleanup();
    NVIC_SystemReset();
}
