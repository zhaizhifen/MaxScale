#ifndef _DCB_H
#define _DCB_H
/*
 * Copyright (c) 2016 MariaDB Corporation Ab
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl.
 *
 * Change Date: 2019-07-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */
#include <spinlock.h>
#include <buffer.h>
#include <listmanager.h>
#include <gw_protocol.h>
#include <gw_authenticator.h>
#include <gw_ssl.h>
#include <modinfo.h>
#include <gwbitmask.h>
#include <skygw_utils.h>
#include <netinet/in.h>

#define ERRHANDLE

struct session;
struct server;
struct service;
struct servlistener;

/**
 * @file dcb.h  The Descriptor Control Block
 *
 * The function pointer table used by descriptors to call relevant functions
 * within the protocol specific code.
 *
 * @verbatim
 * Revision History
 *
 * Date         Who                     Description
 * 01/06/2013   Mark Riddoch            Initial implementation
 * 11/06/2013   Mark Riddoch            Updated GWPROTOCOL structure with new
 *                                      entry points
 * 18/06/2013   Mark Riddoch            Addition of the listener entry point
 * 02/07/2013   Massimiliano Pinto      Addition of delayqlock, delayq and authlock
 *                                      for handling backend asynchronous protocol connection
 *                                      and a generic lock for backend authentication
 * 12/07/2013   Massimiliano Pinto      Added auth entry point
 * 15/07/2013   Massimiliano Pinto      Added session entry point
 * 16/07/2013   Massimiliano Pinto      Added command type for dcb
 * 07/02/2014   Massimiliano Pinto      Added ipv4 data struct into for dcb
 * 07/05/2014   Mark Riddoch            Addition of callback mechanism
 * 08/05/2014   Mark Riddoch            Addition of writeq high and low watermarks
 * 27/08/2014   Mark Riddoch            Addition of write event queuing
 * 23/09/2014   Mark Riddoch            New poll processing queue
 * 19/06/2015   Martin Brampton         Provision of persistent connections
 * 20/01/2016   Martin Brampton         Moved GWPROTOCOL to gw_protocol.h
 * 01/02/2016   Martin Brampton         Added fields for SSL and authentication
 * 27/06/2016   Martin Brampton         Changed DCB to conform to list manager
 *
 * @endverbatim
 */

struct dcb;

/**
 * The event queue structure used in the polling loop to maintain a queue
 * of events that need to be processed for the DCB.
 *
 *      next                    The next DCB in the event queue
 *      prev                    The previous DCB in the event queue
 *      pending_events          The events that are pending processing
 *      processing_events       The evets currently being processed
 *      processing              Flag to indicate the processing status of the DCB
 *      eventqlock              Spinlock to protect this structure
 *      inserted                Insertion time for logging purposes
 *      started                 Time that the processign started
 */
typedef struct
{
    struct  dcb     *next;
    struct  dcb     *prev;
    uint32_t        pending_events;
    uint32_t        processing_events;
    int             processing;
    SPINLOCK        eventqlock;
    unsigned long   inserted;
    unsigned long   started;
} DCBEVENTQ;

#define DCBEVENTQ_INIT {NULL, NULL, 0, 0, 0, SPINLOCK_INIT, 0, 0}

#define DCBFD_CLOSED -1

/**
 * The statistics gathered on a descriptor control block
 */
typedef struct dcbstats
{
    int     n_reads;        /*< Number of reads on this descriptor */
    int     n_writes;       /*< Number of writes on this descriptor */
    int     n_accepts;      /*< Number of accepts on this descriptor */
    int     n_buffered;     /*< Number of buffered writes */
    int     n_high_water;   /*< Number of crosses of high water mark */
    int     n_low_water;    /*< Number of crosses of low water mark */
} DCBSTATS;

#define DCBSTATS_INIT {0}

/**
 * The data structure that is embedded witin a DCB and manages the complex memory
 * management issues of a DCB.
 *
 * The DCB structures are used as the user data within the polling loop. This means that
 * polling threads may aschronously wake up and access these structures. It is not possible
 * to simple remove the DCB from the epoll system and then free the data, as every thread
 * that is currently running an epoll call must wake up and re-issue the epoll_wait system
 * call, the is the only way we can be sure that no polling thread is pending a wakeup or
 * processing an event that will access the DCB.
 *
 * We solve this issue by making the dcb_free routine merely mark a DCB as a zombie and
 * place it on a special zombie list. Before placing the DCB on the zombie list we create
 * a bitmask with a bit set in it for each active polling thread. Each thread will call
 * a routine to process the zombie list at the end of the polling loop. This routine
 * will clear the bit value that corresponds to the calling thread. Once the bitmask
 * is completely cleared the DCB can finally be freed and removed from the zombie list.
 */
typedef struct
{
    GWBITMASK       bitmask;        /*< The bitmask of threads */
    struct dcb      *next;          /*< Next pointer for the zombie list */
} DCBMM;

#define DCBMM_INIT {GWBITMASK_INIT}

/* DCB states */
typedef enum
{
    DCB_STATE_UNDEFINED,    /*< State variable with no state */
    DCB_STATE_ALLOC,        /*< Memory allocated but not populated */
    DCB_STATE_POLLING,      /*< Waiting in the poll loop */
    DCB_STATE_WAITING,      /*< Client wanting a connection */
    DCB_STATE_LISTENING,    /*< The DCB is for a listening socket */
    DCB_STATE_DISCONNECTED, /*< The socket is now closed */
    DCB_STATE_NOPOLLING,    /*< Removed from poll mask */
    DCB_STATE_ZOMBIE,       /*< DCB is no longer active, waiting to free it */
} dcb_state_t;

typedef enum
{
    DCB_ROLE_SERVICE_LISTENER,      /*< Receives initial connect requests from clients */
    DCB_ROLE_CLIENT_HANDLER,        /*< Serves dedicated client */
    DCB_ROLE_BACKEND_HANDLER,       /*< Serves back end connection */
    DCB_ROLE_INTERNAL               /*< Internal DCB not connected to the outside */
} dcb_role_t;

/**
 * Callback reasons for the DCB callback mechanism.
 */
typedef enum
{
    DCB_REASON_CLOSE,               /*< The DCB is closing */
    DCB_REASON_DRAINED,             /*< The write delay queue has drained */
    DCB_REASON_HIGH_WATER,          /*< Cross high water mark */
    DCB_REASON_LOW_WATER,           /*< Cross low water mark */
    DCB_REASON_ERROR,               /*< An error was flagged on the connection */
    DCB_REASON_HUP,                 /*< A hangup was detected */
    DCB_REASON_NOT_RESPONDING       /*< Server connection was lost */
} DCB_REASON;

/**
 * Callback structure - used to track callbacks registered on a DCB
 */
typedef struct dcb_callback
{
    DCB_REASON           reason;         /*< The reason for the callback */
    int                 (*cb)(struct dcb *dcb, DCB_REASON reason, void *userdata);
    void                 *userdata;      /*< User data to be sent in the callback */
    struct dcb_callback  *next;          /*< Next callback for this DCB */
} DCB_CALLBACK;

/**
 * State of SSL connection
 */
typedef enum
{
    SSL_HANDSHAKE_UNKNOWN,          /*< The DCB has unknown SSL status */
    SSL_HANDSHAKE_REQUIRED,         /*< SSL handshake is needed */
    SSL_HANDSHAKE_DONE,             /*< The SSL handshake completed OK */
    SSL_ESTABLISHED,                /*< The SSL connection is in use */
    SSL_HANDSHAKE_FAILED            /*< The SSL handshake failed */
} SSL_STATE;

/**
 * Descriptor Control Block
 *
 * A wrapper for a network descriptor within the gateway, it contains all the
 * state information necessary to allow for the implementation of the asynchronous
 * operation of the protocol and gateway functions. It also provides links to the service
 * and session data that is required to route the information within the gateway.
 *
 * It is important to hold the state information here such that any thread within the
 * gateway may be selected to execute the required actions when a network event occurs.
 *
 * Note that the first few fields (up to and including "entry_is_ready") must
 * precisely match the LIST_ENTRY structure defined in the list manager.
 */
typedef struct dcb
{
    LIST_ENTRY_FIELDS
    skygw_chk_t     dcb_chk_top;
    bool            dcb_errhandle_called; /*< this can be called only once */
    bool            dcb_is_zombie;  /**< Whether the DCB is in the zombie list */
    bool            draining_flag;  /**< Set while write queue is drained */
    bool            drain_called_while_busy; /**< Set as described */
    dcb_role_t      dcb_role;
    SPINLOCK        dcb_initlock;
    DCBEVENTQ       evq;            /**< The event queue for this DCB */
    int             fd;             /**< The descriptor */
    dcb_state_t     state;          /**< Current descriptor state */
    SSL_STATE       ssl_state;      /**< Current state of SSL if in use */
    int             flags;          /**< DCB flags */
    char            *remote;        /**< Address of remote end */
    char            *user;          /**< User name for connection */
    struct sockaddr_in ipv4;        /**< remote end IPv4 address */
    char            *protoname;     /**< Name of the protocol */
    void            *protocol;      /**< The protocol specific state */
    size_t           protocol_packet_length; /**< How long the protocol specific packet is */
    size_t           protocol_bytes_processed; /**< How many bytes of a packet have been read */
    struct session  *session;       /**< The owning session */
    struct servlistener *listener;  /**< For a client DCB, the listener data */
    GWPROTOCOL      func;           /**< The protocol functions for this descriptor */
    GWAUTHENTICATOR authfunc;       /**< The authenticator functions for this descriptor */

    int             writeqlen;      /**< Current number of byes in the write queue */
    SPINLOCK        writeqlock;     /**< Write Queue spinlock */
    GWBUF           *writeq;        /**< Write Data Queue */
    SPINLOCK        delayqlock;     /**< Delay Backend Write Queue spinlock */
    GWBUF           *delayq;        /**< Delay Backend Write Data Queue */
    GWBUF           *dcb_readqueue; /**< read queue for storing incomplete reads */
    SPINLOCK        authlock;       /**< Generic Authorization spinlock */

    DCBSTATS        stats;          /**< DCB related statistics */
    unsigned int    dcb_server_status; /*< the server role indicator from SERVER */
    struct dcb      *nextpersistent;   /**< Next DCB in the persistent pool for SERVER */
    time_t          persistentstart;   /**< Time when DCB placed in persistent pool */
    struct service  *service;       /**< The related service */
    void            *data;          /**< Specific client data, shared between DCBs of this session */
    void            *authenticator_data; /**< The authenticator data for this DCB */
    DCBMM           memdata;        /**< The data related to DCB memory management */
    SPINLOCK        cb_lock;        /**< The lock for the callbacks linked list */
    DCB_CALLBACK    *callbacks;     /**< The list of callbacks for the DCB */
    SPINLOCK        pollinlock;
    int             pollinbusy;
    int             readcheck;

    SPINLOCK        polloutlock;
    int             polloutbusy;
    int             writecheck;
    long            last_read;      /*< Last time the DCB received data */
    int             high_water;     /**< High water mark */
    int             low_water;      /**< Low water mark */
    struct server   *server;        /**< The associated backend server */
    SSL*            ssl;            /*< SSL struct for connection */
    bool            ssl_read_want_read;    /*< Flag */
    bool            ssl_read_want_write;    /*< Flag */
    bool            ssl_write_want_read;    /*< Flag */
    bool            ssl_write_want_write;    /*< Flag */
    int             dcb_port;       /**< port of target server */
    skygw_chk_t     dcb_chk_tail;
} DCB;

#define DCB_INIT {.dcb_chk_top = CHK_NUM_DCB, .dcb_initlock = SPINLOCK_INIT, \
    .evq = DCBEVENTQ_INIT, .ipv4 = {0}, .func = {0}, .authfunc = {0}, \
    .writeqlock = SPINLOCK_INIT, .delayqlock = SPINLOCK_INIT, \
    .authlock = SPINLOCK_INIT, .stats = {0}, .memdata = DCBMM_INIT, \
    .cb_lock = SPINLOCK_INIT, .pollinlock = SPINLOCK_INIT, \
    .fd = DCBFD_CLOSED, .stats = DCBSTATS_INIT, .ssl_state = SSL_HANDSHAKE_UNKNOWN, \
    .state = DCB_STATE_ALLOC, .polloutlock = SPINLOCK_INIT, .dcb_chk_tail = CHK_NUM_DCB, \
    .authenticator_data = NULL}

/**
 * The DCB usage filer used for returning DCB's in use for a certain reason
 */
typedef enum
{
    DCB_USAGE_CLIENT,
    DCB_USAGE_LISTENER,
    DCB_USAGE_BACKEND,
    DCB_USAGE_INTERNAL,
    DCB_USAGE_ZOMBIE,
    DCB_USAGE_ALL
} DCB_USAGE;

#if defined(FAKE_CODE)
extern unsigned char dcb_fake_write_errno[10240];
extern __int32_t     dcb_fake_write_ev[10240];
extern bool          fail_next_backend_fd;
extern bool          fail_next_client_fd;
extern int           fail_next_accept;
extern int           fail_accept_errno;
#endif /* FAKE_CODE */

/* A few useful macros */
#define DCB_SESSION(x)                  (x)->session
#define DCB_PROTOCOL(x, type)           (type *)((x)->protocol)
#define DCB_ISZOMBIE(x)                 ((x)->state == DCB_STATE_ZOMBIE)
#define DCB_WRITEQLEN(x)                (x)->writeqlen
#define DCB_SET_LOW_WATER(x, lo)        (x)->low_water = (lo);
#define DCB_SET_HIGH_WATER(x, hi)       (x)->low_water = (hi);
#define DCB_BELOW_LOW_WATER(x)          ((x)->low_water && (x)->writeqlen < (x)->low_water)
#define DCB_ABOVE_HIGH_WATER(x)         ((x)->high_water && (x)->writeqlen > (x)->high_water)

#define DCB_POLL_BUSY(x)                ((x)->evq.next != NULL)

DCB *dcb_get_zombies(void);
int dcb_write(DCB *, GWBUF *);
DCB *dcb_accept(DCB *listener, GWPROTOCOL *protocol_funcs);
bool dcb_pre_alloc(int number);
DCB *dcb_alloc(dcb_role_t, struct servlistener *);
void dcb_free(DCB *);
void dcb_free_all_memory(DCB *dcb);
DCB *dcb_connect(struct server *, struct session *, const char *);
DCB *dcb_clone(DCB *);
int dcb_read(DCB *, GWBUF **, int);
int dcb_drain_writeq(DCB *);
void dcb_close(DCB *);
DCB *dcb_process_zombies(int);              /* Process Zombies except the one behind the pointer */
void printAllDCBs();                         /* Debug to print all DCB in the system */
void printDCB(DCB *);                        /* Debug print routine */
void dprintDCBList(DCB *);                 /* Debug print DCB list statistics */
void dprintAllDCBs(DCB *);                   /* Debug to print all DCB in the system */
void dprintOneDCB(DCB *, DCB *);             /* Debug to print one DCB */
void dprintDCB(DCB *, DCB *);                /* Debug to print a DCB in the system */
void dListDCBs(DCB *);                       /* List all DCBs in the system */
void dListClients(DCB *);                    /* List al the client DCBs */
const char *gw_dcb_state2string(dcb_state_t);              /* DCB state to string */
void dcb_printf(DCB *, const char *, ...) __attribute__((format(printf, 2, 3))); /* DCB version of printf */
void dcb_hashtable_stats(DCB *, void *);     /**< Print statisitics */
int dcb_add_callback(DCB *, DCB_REASON, int (*)(struct dcb *, DCB_REASON, void *), void *);
int dcb_remove_callback(DCB *, DCB_REASON, int (*)(struct dcb *, DCB_REASON, void *), void *);
int dcb_isvalid(DCB *);                     /* Check the DCB is in the linked list */
int dcb_count_by_usage(DCB_USAGE);          /* Return counts of DCBs */
int dcb_persistent_clean_count(DCB *, bool);      /* Clean persistent and return count */

void dcb_call_foreach (struct server* server, DCB_REASON reason);
void dcb_hangup_foreach (struct server* server);
size_t dcb_get_session_id(DCB* dcb);
bool dcb_get_ses_log_info(DCB* dcb, size_t* sesid, int* enabled_logs);
char *dcb_role_name(DCB *);                  /* Return the name of a role */
int dcb_accept_SSL(DCB* dcb);
int dcb_connect_SSL(DCB* dcb);
int dcb_listen(DCB *listener, const char *config, const char *protocol_name);
void dcb_append_readqueue(DCB *dcb, GWBUF *buffer);

/**
 * DCB flags values
 */
#define DCBF_CLONE              0x0001  /*< DCB is a clone */
#define DCBF_HUNG               0x0002  /*< Hangup has been dispatched */
#define DCBF_REPLIED    0x0004  /*< DCB was written to */

#define DCB_IS_CLONE(d) ((d)->flags & DCBF_CLONE)
#define DCB_REPLIED(d) ((d)->flags & DCBF_REPLIED)
#endif /*  _DCB_H */
