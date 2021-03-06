///
/// @file trnu_cli.h
/// @authors k. headley
/// @date 09 jul 2019

/// TRN update UDP client API

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
  
 Copyright 2002-YYYY MBARI
 Monterey Bay Aquarium Research Institute, all rights reserved.
 
 Terms of Use
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. You can access the GPLv3 license at
 http://www.gnu.org/licenses/gpl-3.0.html
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details
 (http://www.gnu.org/licenses/gpl-3.0.html)
 
 MBARI provides the documentation and software code "as is", with no warranty,
 express or implied, as to the software, title, non-infringement of third party
 rights, merchantability, or fitness for any particular purpose, the accuracy of
 the code, or the performance or results which you may obtain from its use. You
 assume the entire risk associated with use of the code, and you agree to be
 responsible for the entire cost of repair or servicing of the program with
 which you are using the code.
 
 In no event shall MBARI be liable for any damages, whether general, special,
 incidental or consequential damages, arising out of your use of the software,
 including, but not limited to, the loss or corruption of your data or damages
 of any kind resulting from use of the software, any prohibited use, or your
 inability to use the software. You agree to defend, indemnify and hold harmless
 MBARI and its officers, directors, and employees against any claim, loss,
 liability or expense, including attorneys' fees, resulting from loss of or
 damage to property or the injury to or death of any person arising out of the
 use of the software.
 
 The MBARI software is provided without obligation on the part of the
 Monterey Bay Aquarium Research Institute to assist in its use, correction,
 modification, or enhancement.
 
 MBARI assumes no responsibility or liability for any third party and/or
 commercial software required for the database or applications. Licensee agrees
 to obtain and maintain valid licenses for any additional third party software
 required.
 */

// Always do this
#ifndef TRNU_CLI_H
#define TRNU_CLI_H

/////////////////////////
// Includes 
/////////////////////////

#include "trnw.h"
#include "mb1_msg.h"
#include "trn_msg.h"
#include "mframe.h"
#include "msocket.h"
#include "medebug.h"

/////////////////////////
// Macros
/////////////////////////
//#define TRNUC_WITH_STATIC
// return true if block-on-connect flag set
#define TRNUC_BLK_CON(f) ( ((f&TRNUC_BLK_CON)!=0) ? true : false)
// return true if block-on-listen flag set
#define TRNUC_BLK_LISTEN(f) ( ((f&TRNUC_BLK_LISTEN)!=0) ? true : false)
// return true if enable connect message flag set
#define TRNUC_CON_MSG(f) ( ((f&TRNUC_CON_MSG)!=0) ? true : false)
// set flags
// pf: flag pointer
// m: mask (flags to set)
#define TRNUC_MSET(pf,m) do{ if(NULL!=pf)*pf|=m; }while(0)
// clear flags
// pf: flag pointer
// m: mask (flags to set)
#define TRNUC_MCLR(pf,m) do{ if(NULL!=pf)*pf&=~(m); }while(0)
// update string buffer len (holds max string)
#define TRNUC_STR_LEN 1024
// number of fields in CSV record
#define TRNUC_CSV_FIELDS 41
// max CSV record string length
#define TRNUC_CSV_LINE_BYTES 512

/////////////////////////
// Type Definitions
/////////////////////////

// Configuration flags
// TRNUC_BLK_CON    block on connect
// TRNUC_BLK_LISTEN block on listen
// TRNUC_CON_MSG    send connect message
typedef enum{
    TRNUC_BLK_CON   =0x010,
    TRNUC_BLK_LISTEN=0x020,
    TRNUC_CON_MSG   =0x100
}trnuc_flags_t;

// callback function typedef
typedef int (* update_callback_fn)(trnu_pub_t *update);

// trnu_cli instance
typedef struct trnucli_s{
    // struct trnucli_s::trnu
    // server socket connection
    msock_connection_t *trnu;
    // struct trnucli_s::update
    // last update received (NULL if unset)
    trnu_pub_t *update;
    // struct trnucli_s::update_fn
    // callback function (NULL if unset)
    update_callback_fn update_fn;
    // struct trnucli_s::flags
    // configuration flags
    trnuc_flags_t flags;
    // struct trnucli_s::hbto
    // heartbeat timeout (s)
    double hbto;
}trnucli_t;

// update string format flags
// TRNUC_FMT_PRETTY : ASCII, for console
// TRNUC_FMT_CSV    : comma separated value
// TRNUC_FMT_HEX    : hex
// TRNUC_FMT_PRETTY_HEX: formatted hex (w/ offsets)
typedef enum{
    TRNUC_FMT_PRETTY=0X1,
    TRNUC_FMT_CSV=0X2,
    TRNUC_FMT_HEX=0X4,
    TRNUC_FMT_PRETTY_HEX=0X8
}trnuc_fmt_t;

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

    // TODO: add update age threshold/rejection
    // TODO: add UDP buffer configuration
    // TODO: add drain function to empty UDP buffer
    // TODO: add update copy and/or buffering

    // get a new trnu_cli instance
    // caller must release resources using trnucli_destroy
    // fn   : optional update callback (may be NULL)
    // flags: blocking and other flags
    // hbto : heartbeat timeout (s)
    // returns new instance
    trnucli_t *trnucli_new(update_callback_fn update_fn, trnuc_flags_t flags, double hbto);

    // release trnu_cli instance resources
    // pself : pointer to instance pointer
    // return: none, instance pointer set to NULL
    void trnucli_destroy(trnucli_t **pself);

    // connect to trnusvr
    // host: host name or IP address
    // port: port number
    // returns 0 on success, -1 otherwise
    int trnucli_connect(trnucli_t *self, char *host, int port);

    // disconnect from trnusvr
    // returns 0 on success, -1 otherwise
    int trnucli_disconnect(trnucli_t *self);

    // optionally set a callback to be called by listen
    // when an update is received
    // func: optional update callback (may be NULL)
    // returns 0 on success, -1 otherwise
    int trnucli_set_callback(trnucli_t *self, update_callback_fn func);

    // listen for updates, invoke callback if set
    // returns 0 on success (instance update current), -1 otherwise (instance update empty)
    int trnucli_listen(trnucli_t *self);

    // issue TRN reset request
    // must check subsequent update reinit count to confirm
    // returns 0 on request ACK, -1 otherwise
    int trnucli_reset_trn(trnucli_t *self);

    // issue heartbeat request
    // caller must manage timing
    // returns 0 on request ACK, -1 otherwise
    int trnucli_hbeat(trnucli_t *self);

    // convert update to formatted string
    // caller must free destination
    // dest: pointer to buffer (will dynamically allocate if *dest==NULL)
    // len: size of buffer (<=0 if *dest==NULL)
    // fmt: format flags
    // return length of string or -1 on error
    int trnucli_update_str(trnu_pub_t *self, char **dest, int len, trnuc_fmt_t fmt);

#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
