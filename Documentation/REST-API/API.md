# REST API design document

This document describes the version 1 of the MaxScale REST API.

## HTTP Headers

### Request Headers

REST makes use of the HTTP protocols in its aim to provide a natural way to
understand the workings of an API. The following request headers are understood
by this API.

#### Date

This header is required and should be in the RFC 1123 standard form, e.g. Mon,
18 Nov 2013 08:14:29 -0600. Please note that the date must be in English. It
will be checked by the API for being close to the current date and time.

#### Content-Type

All PUT, POST and PATCH requests must provide the `Content-Type:
application/json` header. Only JSON input is accepted for all API input.

#### X-HTTP-Method-Override

Some clients only support GET and PUT requests. By providing the string value of
the intended method in the `X-HTTP-Method-Override` header, a client can perform
a POST, PATCH or DELETE request with the PUT method
(e.g. `X-HTTP-Method-Override: PATCH`).

TODO: Add the rest

### Response Headers

TODO: Figure these out

## Response Codes

Every HTTP response starts with a line with a return code which indicates the
outcome of the request. The API uses some of the standard HTTP values:

* 200 OK
* 204 No Content
* 400 Bad Request – this includes validation failures
* 401 Unauthorized
* 404 Not Found
* 405 Method Not Allowed
* 409 Conflict – the request is inconsistent with known constraints
* 429 Too Mary Requests - Too many requests were performed within the allowed time interval
* 500 Internal Server Error

All responses with a code of 400 or higher return the error as the response
body. The _error_ field contains a short error description and the _description_
field contains a more detailed version of the error message.

```
{
    "error": "Method not supported",
    "description": "The `/service/` resource does not support PUT."
}
```

## Resources

- [/services](Resources-Service.md)
- [/servers](Resources-Server.md)
- [/filters](Resources-Filter.md)
- [/monitors](Resources-Monitor.md)
- [/sessions](Resources-Session.md)
- [/users](Resources-User.md)

-----

TODO: Move these to a separate document

# Resources that support GET

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
