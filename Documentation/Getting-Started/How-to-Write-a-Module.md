# How to Write a Module

Modules are a core concept in MaxScale. This document describes the process of
writing a new module and what the requirements are.

# Basic Module Requirements

Each module must include the _modinfo.h_ header. This header contains the
following macro.

```
MXS_DECLARE_MODULE(type, maturity, description, version, moduleinit, object)
```

This macro is used to declare a module. It takes the following arguments.

- `type`
  - Module API type, and it must be one of:
    - ROUTER
    - FILTER
    - MONITOR
    - AUTHENTICATOR
    - QUERY_CLASSIFIER
    - PROTOCOL

- `maturity`
  - Module maturity, one of:
    - MODULE_IN_DEVELOPMENT
    - MODULE_ALPHA_RELEASE
    - MODULE_BETA_RELEASE
    - MODULE_GA
    - MODULE_EXPERIMENTAL

- `description`
  - Module description

- `version`
  - Module version

- `moduleinit`
  - Optional module specific global initialization function. If `moduleinit` is
    NULL, the initiation function is not called

- `object`
  - Pointer to the module specific entry points (for example a FILTER_OBJECT
    structure as described in filter.h)

# Module Specific Requirements

In addition to the basic module requirements, a module type specific structure
needs to be defined. This structure contains the main interface for the module
and each module type has its own.

## Router Requirements

A router module is declared by calling the MXS_DECLARE_MODULE macro with the
value ROUTER. The _router.h_ header contains router module definitions and the
router module object. All router modules must provide an implementation of the
ROUTER_OBJECT and must pass it as the _object_ parameter of the MODULE_INFO
structure. All entry points in the interface must be defined. This structure
contains the following entry points, in order:

- `ROUTER* (*createInstance)(SERVICE *service, char **options)`

  - Create a new instance of this router module. This is called for each service
    in the configuration file. The _service_ parameter is the service whose
    router is being created and the _options_ parameter is the values of the
    router_options parameter split into an array with the last element set to
    NULL. The return value will be stored and given as a parameter to all other
    entry points.

- `void* (*newSession)(ROUTER *instance, SESSION *session)`

  - Create a new router session for a router instance. The _instance_ parameter
    is the return value of the createInstance entry point. The _session_
    parameter is the client session for which this router session is
    created. The return value will be stored and given as a parameter to the
    closeSession, freeSession, routerQuery, clientReply and handleError entry
    points.

- `void (*closeSession)(ROUTER *instance, void *router_session)`

  - Close a router session. This entry point is called when the client session
    is closed. When this entry point is called, the session can still have
    active threads processing requests and any acquired resources must not be
    released.

- `void (*freeSession)(ROUTER *instance, void *router_session)`

  - Free a router session. This entry point is called after closeSession has
    been called and all threads have stopped processing requests for this
    session.  All resources acquired in newSession should be released at this
    point.

- `int (*routeQuery)(ROUTER *instance, void *router_session, GWBUF *queue)`

  - Route a request from the client. This entry point is the main work function
    for the router. The _queue_ contains the raw data read from the network
    stored in one or more buffers and should be freed by this function by
    calling the `gwbuf_free` once it is no longer used. The return value should
    be 1 if routing of the request was successful. If an error has occurred and
    the session should be closed, the return value should be 0.

- `void (*diagnostics)(ROUTER *instance, DCB *dcb)`

  - Diagnostics function. The _dcb_ is a client DCB where the diagnostic
    information should be written using the function `dcb_printf`.

- `void (*clientReply)(ROUTER* instance, void* router_session, GWBUF* queue, DCB *backend_dcb)`

  - Route a reply from a backend server to the client. The _queue_ contains the
    raw data read from the network and the _backend_dcb_ is the backend which
    sent this reply. The macro SESSION_ROUTE_REPLY, defined in _session.h_,
    should be called when the reply should be sent. The parameters for this
    macro are the SESSION object of the client and the GWBUF containing the
    response. The SESSION object can be retrieved from `backend_dcb->session`.

- `void (*handleError)(ROUTER* instance, void* router_session, GWBUF* errmsgbuf, DCB* backend_dcb, error_action_t action, bool* succp)`

  - Handle a backend error. This function is called when an error has occurred
    in one of the backend DCBs associated with this session. The _errmsgbuf_
    parameter contains the raw error message, _backend_dcb_ is the backend DCB
    where the error occurred, _action_ is the action that should be taken and
    _succp_ is a pointer where the result of error handling should be stored. If
    the router module can recover from the backend error, the value of _succp_
    should be set to true. If the router can't recover from the failure and the
    session should be closed, the value of _succp_ should be set to false.

- `int (*getCapabilities)()`

  - Return router capabilities. See _router.h_ for more details.

## Filter Requirements

A filter module is declared by calling the MXS_DECLARE_MODULE macro with the
value FILTER. The _filter.h_ header contains filter module definitions and the
filter module object. All filter modules must provide an implementation of the
FILTER_OBJECT and must pass it as the _object_ parameter of the MODULE_INFO
structure. All entry points in the interface must be defined. This structure
contains the following entry points, in order:

- `FILTER *(*createInstance)(char **options, FILTER_PARAMETER **parameters)`

  - Create a new filter instance. The _options_ parameter is the values of the
    options parameter split into an array with the last element set to NULL. The
    _parameters_ parameter contains the rest of the filter parameters stored as
    an array of FILTER_PARAMETER objects with the last element set to NULL. The
    return value of this function is passed as a parameter to all the other
    entry points.

- `void   *(*newSession)(FILTER *instance, SESSION *session)`

  - Create a new filter session. The _session_ parameter is the new client
    session being created. The return value of this function is passed as a
    parameter to all entry points except createInstance.

- `void   (*closeSession)(FILTER *instance, void *fsession)`

  - Close a filter session. This entry point is called when the client session
    is closed. When this entry point is called, the session can still have
    active threads processing requests and any acquired resources must not be
    released.

- `void   (*freeSession)(FILTER *instance, void *fsession)`

  - Free a filter session. This entry point is called after closeSession has
    been called and all threads have stopped processing requests for this
    session.  All resources acquired in newSession should be released at this
    point.

- `void   (*setDownstream)(FILTER *instance, void *fsession, DOWNSTREAM *downstream)`

  - Set the downstream component in the filter pipeline. The _downstream_
    parameter contains the next module in the chain of filters and routers. This
    value should be copied for each session inside the _fsession_ parameter as a
    `DOWNSTREAM` object.  (TODO: Make this somehow automatic). Copying the
    downstream can be done with the following:

    ```
    MY_FILTER_SESSION *my_session = (MY_FILTER_SESSION*)fsession;
    my_session->down = *downstream;
    ```
    Here it is assumed that `my_session->down` is of type `DOWNSTREAM`.

- `void   (*setUpstream)(FILTER *instance, void *fsession, UPSTREAM *upstream)`

  - Set the upstream component in the filter pipeline. This entry point is
    optional. If the filter implements the clientReply entry points, it must
    also implement the setUpstream entry point. The _upstream_ parameter
    contains the upstream component in the filter pipeline and it can be either
    a filter or the client protocol module. It should be stored in a similar
    manner to the DOWNSTREAM component.

- `int    (*routeQuery)(FILTER *instance, void *fsession, GWBUF *queue)`

  - Route a query forward in the filter pipeline. The _queue_ parameter contains
    the raw network data read by the client protocol module. The filter must
    pass a `GWBUF` object to the next filter in chain by calling the DOWNSTREAM
    component's routeQuery function:

    ```
    DOWNSTREM down = *my_session->down;
    down.routeQuery(down.instance, down.session, queue);
    ```

    Here _down_ is the DOWNSTREAM object passed to the setDownstream entry
    point. If routing the query was successful, the function should return
    1 and if an error occurred and the session should be closed, the function
    should return 0.

- `int    (*clientReply)(FILTER *instance, void *fsession, GWBUF *queue)`

  - Forward the reply in the filter pipeline. The _queue_ parameter is the reply
    from a backend server and it contains the raw network data read by the
    backend protocol module and processed by the router module. The filter must
    pass a reply to the client by calling the clientReply function of the
    UPSTREAM component:

    ```
    UPSTREAM up = *my_session->up;
    up.clientReply(up.instance, up.session, queue);
    ```

    Here _up_ is the UPSTREAM object passed to the setUpstream entry point. If
    routing the query was successful, the function should return 1 and if an
    error occurred, the function should return 0. Currently the failure to route
    a reply does not close the session.

- `void   (*diagnostics)(FILTER *instance, void *fsession, DCB *dcb)`

  - Diagnostics function. The _dcb_ is a client DCB where the diagnostic
    information should be written using the function `dcb_printf`. If fsession
    is not NULL, an individual filter session is being inspected.

## Monitor Requirements

## Protocol Requirements

## Authenticator Requirements

## Query Classifier Requirements
