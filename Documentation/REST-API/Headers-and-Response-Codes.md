# Headers and Response Codes

## Request Headers

REST makes use of the HTTP protocols in its aim to provide a natural way to
understand the workings of an API. The following request headers are understood
by this API.

### Date

This header is required and should be in the RFC 1123 standard form, e.g. Mon,
18 Nov 2013 08:14:29 -0600. Please note that the date must be in English. It
will be checked by the API for being close to the current date and time.

### Content-Type

All PUT, POST and PATCH requests must provide the `Content-Type:
application/json` header. Only JSON input is accepted for all API input.

### X-HTTP-Method-Override

Some clients only support GET and PUT requests. By providing the string value of
the intended method in the `X-HTTP-Method-Override` header, a client can perform
a POST, PATCH or DELETE request with the PUT method
(e.g. `X-HTTP-Method-Override: PATCH`).

TODO: Add the rest

## Response Headers

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
body. The _error_ field contains the short description and the _description_
field contains the detailed information about the error.

```
{
    "error": "Method not supported",
    "description": "The `/service/` resource does not support PUT."
}
```
