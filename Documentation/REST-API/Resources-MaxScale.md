# MaxScale Resource

The MaxScale resource represents a MaxScale instance and it is the core on top
of which the modules build upon.

## Resource Operations

## Get global information

Retrieve global information about a MaxScale instance. This includes various
file locations, configuration options and version information.

```
GET /v1/maxscale
```

#### Response

```
Status: 200 OK

{
    "config": "/etc/maxscale.cnf",
    "logdir": "/var/log/maxscale/",
    "cachedir": "/var/cache/maxscale/",
    "datadir": "/var/lib/maxscale/"
    "libdir": "/usr/lib64/maxscale/",
    "piddir": "/var/run/maxscale/",
    "execdir": "/usr/bin/",
    "languagedir": "/var/lib/maxscale/",
    "user": "maxscale",
    "maxlog": true,
    "syslog": false,
    "log_levels": [
        "error",
        "warning",
        "notice",
        "info",
        "debug"
    ],
    "log_augmentation": [
        "function"
    ],
    "log_throttling": {
        "limit": 8,
        "window": 2000,
        "suppression": 10000
    }
    "threads": 4,
    "version": "2.1.0",
    "commit": "12e7f17eb361e353f7ac413b8b4274badb41b559"
    "started": "Wed, 31 Aug 2016 23:29:26 +0300"
}
```

## Get polling information

Get detailed information and statistics about the threads.

```
GET /v1/maxscale/threads
```

#### Response

```
Status: 200 OK

{
    "load_average": {
        "historic": 1.05,
        "current": 1.00,
        "1min": 0.00,
        "5min": 0.00,
        "15min": 0.00
    },
    "threads": [
        {
            "id": 0,
            "state": "processing",
            "file_descriptors": 1,
            "event": [
                "in",
                "out"
            ],
            "run_time": 300
        },
        {
            "id": 1,
            "state": "polling",
            "file_descriptors": 0,
            "event": [],
            "run_time": 0
        }        
    ]
}
```

TODO: Add epoll statistics and rest of the supported methods.
