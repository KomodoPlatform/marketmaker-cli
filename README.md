# MarketMaker Command-Line Interface

This application is a CLI front-end to `marketmaker` API (barterDEX).
It builds a JSON requests based on given arguments and display the results returned by the `marketmaker` (MM) back-end.

## How to build

The build system is based on CMake 3.2+. Follow these steps to build the application:

```console
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Usage

Two ancillary functionalities were also implemented: cached authentication and API refresh.

### Cached Authentication

To avoid the input of user credentials on every invocation, a pseudo-method name `_config` shall be used in order to 
store this information on a local file (`$HOME/.mmcli.config`).

Example:

```console
$ ./marketmaker-cli _config http://127.0.0.1:7783 \
1d8b27b21efabcd96571cd56f91a40fb9aa4cc623d273c63bf9223dc6f8cd81f
```

### Duped API

Instead of duplicating and hard-coding the definition of more than 50 API calls, this application *learns* the MM API 
from the response given for the `help` method and caches these definitions on an external file (`$HOME/.mmcli.api`).
To force refreshing this information, one can use the `_refresh` pseudo-method explicitly:

```console
$ ./marketmaker-cli _refresh
```

## Unit Tests

The Unit Tests were implemented in C++ using the Google Test Framework (GTF), and their sources are located at 
`$PROJECT_DIR/src/test`. The build system automatically downloads GTF, and build the proper release required.



To run the Unit Tests, just run the following command under the `build` directory:

```console
$ make && ./tests
```