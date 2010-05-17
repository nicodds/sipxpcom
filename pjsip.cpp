// pjsip.cpp : Defines the exported functions for the DLL application.
//

#include <pjsua-lib/pjsua.h>
#include "stdio.h"
#include "string.h"
#include "pjsip.h"

#define THIS_FILE	"APP"
#define current_acc	pjsua_acc_get_default()


/* Callback called by the library upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id, pjsip_rx_data *rdata){
  pjsua_call_info ci;
  PJ_UNUSED_ARG(acc_id);
  PJ_UNUSED_ARG(rdata);
  pjsua_call_get_info(call_id, &ci);
  PJ_LOG(3,(THIS_FILE, "Incoming call from %.*s!!",
        (int)ci.remote_info.slen,
        ci.remote_info.ptr));
  /* Automatically answer incoming calls with 200/OK */
  pjsua_call_answer(call_id, 200, NULL, NULL);
}

/* Callback called by the library when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e) {
  pjsua_call_info ci;
  PJ_UNUSED_ARG(e);
  pjsua_call_get_info(call_id, &ci);
  PJ_LOG(3,(THIS_FILE, "Call %d state=%.*s", call_id,
        (int)ci.state_text.slen,
        ci.state_text.ptr));
}

/* Callback called by the library when call's media state has changed */
static void on_call_media_state(pjsua_call_id call_id){
  pjsua_call_info ci;
  pjsua_call_get_info(call_id, &ci);
  if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
    // When media is active, connect call to sound device.
    pjsua_conf_connect(ci.conf_slot, 0);
    pjsua_conf_connect(0, ci.conf_slot);
  }
}

/* Display error and exit application */
static void error_exit(const char *title, pj_status_t status) {
  pjsua_perror(THIS_FILE, title, status);
  pjsua_destroy();
  exit(1);
}



PJSIP_API int sipregister(long sipPort) {

  static pj_thread_desc desc;
  static pj_thread_t *  thread;
  pj_status_t status;
  pjsua_acc_id acc_id;

#define REGISTER_THREAD()	\
  if(!pj_thread_is_registered()) {\
    pj_thread_register(NULL,desc,&thread);\
  }

  REGISTER_THREAD();
  

  /* Create pjsua first! */
  status = pjsua_create();

  /* if (status != PJ_SUCCESS) error_exit("Error in pjsua_create()", status); */
  if (status != PJ_SUCCESS){ pjsua_destroy();}
  /* Init pjsua */
  {
    pjsua_config cfg;
	pjsua_logging_config log_cfg;

    cfg.user_agent = pj_str((char*)"pjsip");
    /* Bind all multithread calls from an API to a single thread or worker thread */
    cfg.thread_cnt = 0;
    
    pjsua_config_default(&cfg);
    cfg.cb.on_incoming_call = &on_incoming_call;
    cfg.cb.on_call_media_state = &on_call_media_state;
    cfg.cb.on_call_state = &on_call_state;
    pjsua_logging_config_default(&log_cfg);
    log_cfg.console_level = 4;
    status = pjsua_init(&cfg, &log_cfg, NULL);
    /* if (status != PJ_SUCCESS) error_exit("Error in pjsua_init()", status); */
    if (status != PJ_SUCCESS){ pjsua_destroy();}
  }

  /* Add UDP transport. */
  {
    pjsua_transport_config cfg;
    pjsua_transport_config_default(&cfg);
    /*	cfg.port = 5060; Default Port Number */
    cfg.port = sipPort;
    status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);
    /*	if (status != PJ_SUCCESS) error_exit("Error creating transport", status); */
    if (status != PJ_SUCCESS){ pjsua_destroy();}
  }


  /* Initialization is done, now start pjsua */
  status = pjsua_start();
  /*    if (status != PJ_SUCCESS) error_exit("Error starting pjsua", status); */
  if (status != PJ_SUCCESS){ pjsua_destroy();}

  /* Register to SIP server by creating SIP account. */
  {
    pjsua_acc_config cfg;
    pjsua_acc_config_default(&cfg);
    cfg.id = pj_str((char*)"sip:tc@localhost");
    cfg.cred_count = 1;
    status = pjsua_acc_add(&cfg, PJ_TRUE, &acc_id);
    if (status != PJ_SUCCESS){ pjsua_destroy();}
  }
  return 0;
}


PJSIP_API int sipderegister(){
  static pj_thread_desc desc;
  static pj_thread_t *  thread;
  #define REGISTER_THREAD()	\
  if(!pj_thread_is_registered()) {\
    pj_thread_register(NULL,desc,&thread);\
  }
  REGISTER_THREAD();
  /* de-register client */
  pjsua_acc_del(current_acc);
  pjsua_destroy();
  printf("ESCO..\n");
  return 0;
}


PJSIP_API int sipmakecall(char *sipToAddr){
  /* Registering thread (MOZ.) with pj_register_thread before calling any pjlibs function */
  pj_status_t status;
  pj_str_t uri = pj_str(sipToAddr);

  static pj_thread_desc desc;
  static pj_thread_t *  thread;
#define REGISTER_THREAD()	\
  if(!pj_thread_is_registered()) {\
    pj_thread_register(NULL,desc,&thread);\
  }
  REGISTER_THREAD();
  
  status = pjsua_call_make_call(current_acc, &uri, 0, NULL, NULL, NULL);
  return 0;
}


PJSIP_API int siphangup(){
  static pj_thread_desc desc;
  static pj_thread_t *  thread;
#define REGISTER_THREAD()	\
  if(!pj_thread_is_registered()) {\
    pj_thread_register(NULL,desc,&thread);\
  }
  REGISTER_THREAD();
	pjsua_call_hangup_all();
  return 0;
}