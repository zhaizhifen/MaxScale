# REST API design document

This document contains the design for the MaxScale REST API.
Resources

The main objects in MaxScale are service, server, monitor and filter. These
objects are represented as individual sections in the configuration file and can
exist alone. Additionally, the current maxadmin commands that produce diagnostic
output can be thought of as read-only resources.

Even though the listeners are defined as separate entities in the configuration
file, they are actually a part of the service. This can be seen from the fact
that a listener always requires a service. Internally, they are just DCBs of a
different type.

# Resources that support GET

The response to a GET request will be a JSON representation of the requested
resource. The structure of the returned JSON object is dependant on the resource
being requested and the current configuration of MaxScale (e.g. servers can have
multiple weighting parameters).

## `/monitor`

Returns information about all monitors. Returns an array of JSON objects similar
to the `/monitor/:id` resource.

## `/monitor/:id`

Returns information about a specific monitor.

### Common fields

All monitors will return these fields in the response. In addition to these
fields, it is also possible that the monitor returns extra fields.

|Field              |Type        |Value                                         |
|-------------------|------------|----------------------------------------------|
|name               |string      |Monitor name                                  |
|module             |string      |Monitor module                                |
|state              |string      |Monitor state                                 |
|servers            |string array|The servers used by this monitor              |
|monitor_interval   |number      |The monitoring interval in milliseconds       |
|connect_timeout    |number      |Connect timeout for backend connections       |
|read_timeout       |number      |Read timeout for backend connections          |
|write_timeout      |number      |Write timeout for backend connections         |

## `/filter`

Returns information about all filters. Returns an array of JSON objects similar
to the `/filter/:id` resource.

## `/filter/:id`

All filters will return these fields in the response. In addition to these
fields, each filter can return extra fields specific to that filter module.

|Field              |Type        |Value                                         |
|-------------------|------------|----------------------------------------------|
|name               |string      |Filter name                                   |
|module             |string      |Filter module                                 |

## `/session`

Returns information about all sessions. Returns an array of JSON objects similar
to the `/session/:id` resource.

## `/session/:id`

Returns information about a session. All session will return these fields in
the response. Filters and routers can add module specific extra fields to the
response.

|Field    |Type  |Value |
|---------|------|------|
|id       |number|Unique session identified|
|state    |string|Session state, one of `Session Allocated`, `Dummy Session`,`Session Ready`, `Session ready for routing`, `Listener Session`, `Stopped Listener Session`, `Stopping session`, `Session to be freed` or `Freed session`|
|service  |string|The service where this session was create|
|client   |string|The client user and hostname in `user@hostname` format|
|connected|string|Time when client connection was made|
|idle     |number|Number of seconds the client has been idle

## `/session/:id/dcb`

Returns information about all DCB. Returns an array of JSON objects similar
to the `/session/:id/dcb/:id` resource.

## `/session/:id/dcb/:id`

The DCB is MaxScale's representation of a network connection. A _dcb_ resource
contains the following fields. The _:id_ is the unique address of this DCB.

|Field     |Type  |Value |
|----------|------|------|
|address   |string|Unique address of the DCB. The address is unique only among the active DCBs and the addresses will be reused.|
|state     |string|DCB state, one of `DCB Allocated`, `DCB in the polling loop`, `DCB not in polling loop`, `DCB for listening socket`, `DCB socket closed` or `DCB Zombie`|
|service   |string|The service where this DCB connects to or from which it originates|
|role      |string|DCB role, one of `Service Listener` or `Client Request Handler`|
|statistics|object|DCB statistics, see next section for details|

### DCB statistics

The _statistics_ object of a _dcb_ resource contains the following fields.

|Field          |Type    |Value                                 |
|---------------|--------|--------------------------------------|
|reads          |number  |Number of reads                       |
|writes         |number  |Number of writes                      |
|buffered_writes|number  |Number of buffered writes             |
|accepts        |number  |Number of accepts, only for listeners |
|high_water     |number  |Number of high water events           |
|low_water      |number  |Number of low water events            |

## `/task`

Returns information about all currently running housekeeping tasks. Comparable
to the output of maxadmin show tasks.

## `/thread`

Returns information about threads. Comparable to the combined output of maxadmin
show [threads|epoll|eventq|eventstats].

## `/module`

Returns information about currently loaded modules. Comparable to the output of
maxadmin show modules.

# Resources that support PUT

All PUT requests that fail due to errors in the provided data receive a response
with the HTTP response code 400. If the failure is due to an internal error, the
HTTP response code 500 is returned.

## `/server/:id`

Modifies the server whose name matches the :id component of the URL. The body of
the PUT statement must represent the new server configuration in JSON and should
have all the required fields. If optional fields (weighting parameter, server
priority etc.) are missing, they are removed from the server configuration.
Changes to the server configuration take effect only for new connections created
after the configuration change. Old connections need to be manually closed.

### Required fields

|Field              |Type        |Description                                   |
|-------------------|------------|----------------------------------------------|
|name               |string      |Server name                                   |
|address            |string      |Server network address                        |
|port               |number      |Server port                                   |
|protocol           |string      |Backend server protocol e.g. MySQLBackend     |

## `/monitor/:id`

Modifies the monitor whose name matches the :id component of the URL. The body
of the PUT statement must represent the new monitor configuration in JSON and
should have all the required fields. If monitor specific extra fields are
missing, they are removed from the configuration. Changes to the configuration
take effect on the next monitoring interval.

### Required fields

|Field              |Type        |Value                                         |
|-------------------|------------|----------------------------------------------|
|name               |string      |Monitor name                                  |
|module             |string      |Monitor module                                |
|state              |string      |Monitor state                                 |
|servers            |string array|The servers used by this monitor              |
|monitor_interval   |number      |The monitoring interval in milliseconds       |
|connect_timeout    |number      |Connect timeout for backend connections       |
|read_timeout       |number      |Read timeout for backend connections          |
|write_timeout      |number      |Write timeout for backend connections         |

## `/filter/:id`

Modifies the filter whose name matches the :id component of the URL. The body of
the PUT statement must represent the new filter configuration in JSON and should
have all the required fields. If filter specific extra fields are missing, they
are removed from the configuration. Depending on the filter module, changes to
the configuration can take effect immediately or only for new filter sessions.

### Required fields

|Field              |Type        |Value                                         |
|-------------------|------------|----------------------------------------------|
|name               |string      |Filter name                                   |
|module             |string      |Filter module                                 |
