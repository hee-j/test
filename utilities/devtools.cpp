#include "devtools.h"

void test_communication_with_responder()
{
    char answer[0];
    sendPacket(12, 3, answer, 0);
}