#ifndef __OBC__H_
#define __OBC__H_

#include "mbed.h"
#include "logging.h"
#include "submodule.h"
#include "m_util.h"
#include "version.h"
#include "sdhc.h"

#include "legacy_comm.h"


// Command interface

// OBC specific
void    _RESET();
void    PING        ();
void    MARK_TO_LOG (char* _params);
void    ANSWER_MODE (char* _params);
void    DEBUG_MESSAGES_ON(char* _params);
void    IS_MOUNTED(void);
void    MOUNT();
void    UNMOUNT();
void    GET_NETWORK_ID(void);
void    SET_NETWORK_ID(char* _params);
void    SET_TEMPORARY_NETWORK_ID(char* _params);
void    GET_VERSION(void);
void    SD_CARD_CONNECTED(void);
void    RBF_STATUS(void);
void    UPTIME(void);

void    GET_NODE_ID();


// General
void    WHOAMI      (uint32_t cmd);

#endif