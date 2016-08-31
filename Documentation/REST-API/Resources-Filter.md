# Filter Resource

A filter resource represents an instance of a filter inside MaxScale. Multiple
services can use the same filter and a single service can use multiple filters.

## Resource Operations

### Get a filter

Get a single filter. The _:name_ in the URI must be a valid filter name in
lowercase with all whitespace replaced with underscores.

```
GET /filters/:name
```

#### Response

```
Status: 200 OK

[
    {
        "name": "Query Logging Filter",
        "module": "qlafilter",
        "parameters": {
            "filebase": "/var/log/maxscale/qla/log.",
            "match": "select.*from.*t1"
        }
        "services": [
            "My Service",
            "My Second Service"
        ]
    }
]
```

### Get all filters

Get all filters.

```
GET /filters
```

#### Response

```
Status: 200 OK

[
    {
        "name": "Query Logging Filter",
        "module": "qlafilter",
        "parameters": {
            "filebase": "/var/log/maxscale/qla/log.",
            "match": "select.*from.*t1"
        }
        "services": [
            "My Service",
            "My Second Service"
        ]
    },
    {
        "name": "DBFW Filter",
        "module": "dbfwfilter",
        "parameters": {
            "rules": "/etc/maxscale-rules"
        }
        "services": [
            "My Second Service"
        ]
    }
]
```

### Update a filter

Partially update a filter. The _:name_ in the URI must be a valid filter name
in lowercase with all whitespace replaced with underscores.

```
PATCH /filter/:name
```

### Input

The _parameters_ field must always be defined. If the filter requires no
parameters, the value of _parameters_ should be `null`.

|Field       |Type        |Description      |
|------------|------------|-----------------|
|parameters  |string array|Module specific filter parameters|

```
{
    "parameters": {
        "rules": "/etc/maxscale-rules",
        "action": "allow"
    }

}
```

#### Response

```
Status: 200 OK

[
    {
        "name": "DBFW Filter",
        "module": "dbfwfilter",
        "parameters": {
            "rules": "/etc/maxscale-rules",
            "action": "allow"
        }
        "services": [
            "My Second Service"
        ]
    }
]
```
