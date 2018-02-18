# MarketMaker Command-Line Interface

This application is a CLI front-end to `marketmaker` API (barterDEX).
It builds a JSON requests based on given arguments and display the results returned by the `marketmaker` (MM) back-end.

Two ancillary functionalities were also implemented: cached authentication and API refresh.

## Cached Authentication

To avoid the input of user credentials on every invocation, a pseudo-method name `_config` shall be used in order to store this information on a local file.

Example:

```shell
$ ./marketmaker-cli _config http://127.0.0.1:7783 \
1d8b27b21efabcd96571cd56f91a40fb9aa4cc623d273c63bf9223dc6f8cd81f
```

## Duped API

Instead of duplicating more than 50 API calls, this application "__learns__" the MM API based on the response given for the `help` method. To force refreshing this information, one can use the `_refresh` pseudo-method:

```shell
$ ./marketmaker-cli _refresh
```
