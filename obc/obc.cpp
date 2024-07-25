#include "obc.h"
#include "settings.h"
#include "errors.h"
#include "../utilities/version.h"
#include "timers.h"

DigitalIn sd_card_connected(SD_CARD_DET);
DigitalIn rbf_connected(RBF);

// Unique ID
unsigned long   *uid = (unsigned long *)0x1FFF7A10;

// General and OBC commands

void WHOAMI(uint32_t cmd)
{
    char whoami_response[64];
    memset(whoami_response,0,sizeof(whoami_response));
    
    sprintf(whoami_response, "Kitsat FW Version: %s, Kitsat ID: %08lX %08lX %08lX",
                                                        FW_VERSION,
                                                        uid[0], uid[1], uid[2]);

    _debug("%s\n\r", whoami_response);
    _debug("Whoami response: %s\n\r", whoami_response);
    
    sendPacket(GENERAL, 1, (char*)whoami_response, 63);
}

void _RESET()
{
    system_rst();
}

void GET_NODE_ID()
{
    int _node_id = read_node_id_from_file();
    char id[2];

    sprintf(id, "%c", _node_id);
    if( read_node_id_from_file() >= 0 )
        sendPacket(GENERAL, 9, id, strlen(id));
    else
        _message("fail", GENERAL, 9);
    return;
}

void GET_NETWORK_ID()
{
    GET_NODE_ID();
}

void SET_NETWORK_ID(char *_params)
{
    int node_id = 0;

    node_id = str2int(_params);

    write_node_id_to_file(node_id);
    char answer[32];
    sprintf(answer, "Set satellite id to %d", node_id);
    sendPacket(OBC, 10, answer,strlen(answer));
}


void SET_TEMPORARY_NETWORK_ID(char *_params)
{
    int node_id = 0;

    node_id = str2int(_params);
    setNetworkID(node_id);
    _message("Network id set\r\n");
}


void SD_CARD_CONNECTED()
{
    uint8_t status = 0;
    status = sd_card_connected; // read the GPIO in
    status = !status; // reverse to coincide with the protocol
    char answer[4];
    sprintf(answer, "%d", status);

    sendPacket(OBC,12,answer,strlen(answer));
}

void RBF_STATUS()
{
    uint8_t status = 0;
    status = rbf_connected; // read the GPIO in
    status = !status; // reverse to coincide with the protocol
    char answer[4];
    sprintf(answer, "%d", status);

    sendPacket(GENERAL,5,answer,strlen(answer));
}

void GET_VERSION()
{
    char answer[32];
    sprintf(answer, "%s", FW_VERSION);
    sendPacket(OBC, 11, answer,strlen(answer));
}

void IS_MOUNTED()
{
    uint8_t mounted = is_mounted();
    if( mounted )
    {
        _debug("Filesystem is mounted\n\r");
    }
    else {
        _debug("Filesystem is not mounted\n\r");
    }

    // sendPacket....
}

void UNMOUNT()
{
    uint8_t mounted = is_mounted();
    if( mounted )
    {
        if ( sd_cleanup() == NO_ERR )
            _debug("Filesystem unmounted.\n\r");
    } else {
        _warning("Filesystem already unmounted!\n\r");
    }

    // sendPacket....
}

void MOUNT()
{
    uint8_t mounted = is_mounted();
    if( !mounted )
    {
        if ( filesystem_mount() == NO_ERR )
            _debug("Filesystem mounted.\n\r");
    } else {
        _warning("Filesystem already mounted!\n\r");
    }

    // sendPacket....
}

void ANSWER_MODE(char* _params)
{
    _error("Not yet implemented.\n\r");
}

void MARK_TO_LOG(char* _params)
{
    _error("Not yet implemented.\n\r");
}

void DEBUG_MESSAGES_ON(char* _params)
{
    _error("Not yet implemented.\n\r");
}

void PING()
{
    _debug("Pong!\n\r");
    _message("1", OBC, 5);
}

void UPTIME()
{
    char msg[16];
    memset(msg, 0, 16);
    sprintf(msg, "%d", ms_since_boot());
    sendPacket(GENERAL,6,msg,strlen(msg));
}