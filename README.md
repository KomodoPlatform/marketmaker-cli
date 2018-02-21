# MarketMaker Command-Line Interface

[![license](https://img.shields.io/github/license/eliezio/marketmaker-cli.svg)]()
[![CI Build](https://travis-ci.org/eliezio/marketmaker-cli.svg?branch=master)]()
[![codecov](https://codecov.io/gh/eliezio/marketmaker-cli/branch/master/graph/badge.svg)](https://codecov.io/gh/eliezio/marketmaker-cli)

This application is a CLI front-end to `marketmaker` API (barterDEX).
It builds a JSON requests based on given arguments and display the results returned by the `marketmaker` (MM) back-end.

## How to build

The build system is based on CMake 3.2+. Follow these steps to build the application:

```console
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

## Usage

### Authentication

To avoid the input of user credentials on every invocation, a pseudo-method name `_config` shall be used in order to
cache this information on a local file (`$HOME/.mmcli.config`).

Example:

```console
$ ./marketmaker-cli _config http://127.0.0.1:7783 \
1d8b27b21efabcd96571cd56f91a40fb9aa4cc623d273c63bf9223dc6f8cd81f
```

### Calling the API

To see the list of available methods and their respective parameters you can use:
```console
$ ./marketmaker-cli -h   # or './marketmaker-cli --help'
Syntax: ./marketmaker-cli [-h | --help | _config URL USERPASS | _refresh | method [-h | --help | params*]

Method                Parameters
====================  =============================================
autoprice ........... base,rel,fixed,minprice,maxprice,margin,refbase,refrel,factor,offset
balance ............. coin,address
balances ............ address
bot_buy ............. base,rel,maxprice,relvolume
bot_list ............
bot_pause ........... botid
bot_sell ............ base,rel,minprice,basevolume
bot_settings ........ botid,newprice,newvolume
bot_status .......... botid
bot_statuslist ......
bot_stop ............ botid
buy ................. base,rel,price,relvolume,timeout,duration,nonce
calcaddress ......... passphrase
disable ............. coin
dividends ........... coin,height
electrum ............ coin,ipaddr,port
enable .............. coin
fundvalue ........... address,holdings,divisor
getcoin ............. coin
getcoins ............
getmyprice .......... base,rel
getpeers ............
getprice ............ base,rel
getprices ...........
getrawtransaction ... coin,txid
goal ................ coin,val
instantdex_claim ....
instantdex_deposit .. weeks,amount,broadcast
inventory ........... coin,reset,passphrase
jpg ................. srcfile,destfile,power2,password,data,required,ind
lastnonce ...........
listunspent ......... coin,address
myprice ............. base,rel
notarizations ....... coin
notarizations ....... coin
orderbook ........... base,rel,duration
passphrase .......... passphrase,gui,netid,seednode
portfolio ...........
pricearray .......... base,rel,starttime,endtime,timescale
recentswaps ......... limit
secretaddresses ..... prefix,passphrase,num,pubtype,taddr
sell ................ base,rel,price,basevolume,timeout,duration,nonce
sendrawtransaction .. coin,signedtx
setconfirms ......... coin,numconfirms,maxconfirms
setprice ............ base,rel,price,broadcast
snapshot ............ coin,height
snapshot_balance .... coin,height,addresses
statsdisp ........... starttime,endtime,gui,pubkey,base,rel
stop ................
swapstatus .......... base,rel,limit
swapstatus .......... coin,limit
swapstatus .......... pending
swapstatus .......... requestid,quoteid,pending
ticker .............. base,rel
tradesarray ......... base,rel,starttime,endtime,timescale
trust ............... pubkey,trust
withdraw ............ coin,outputs
```

It's also possible to get help for a specific method by using the special parameter `-h` or `--help`. Example:

```console
$ ./marketmaker-cli passphrase -h
Parameters for method 'passphrase': passphrase,gui,netid,seednode
```

All method parameters are treated as if they were optional. Thus you can suppress one or more ending parameters.
See the examples of accepted invocations for the method `passphrase`:

```console
$ ./marketmaker-cli passphrase testtest beerDEX
$ # or
$ ./marketmaker-cli passphrase testtest beerDEX 0
$ # or
$ ./marketmaker-cli passphrase testtest beerDEX 0 xyz
```

## Unit Tests

The Unit Tests were implemented in C++ using the Google Test Framework (GTF), and their sources are located at
`$PROJECT_DIR/src/test`. The build system automatically downloads and builds the required GTF.

To run the Unit Tests, just run the following command under the `build` directory:

```console
$ cmake --build . && ./tests
```

### Troubleshooting

In your first run the program automatically fetches and caches the API methods definitions
extracted from the output of the 'help' method.
In case the `marketmaker` API changes after this first run, or the cache files becomes corrupted,
you can force recreating it using the command:

```console
$ ./marketmaker-cli _refresh
```
