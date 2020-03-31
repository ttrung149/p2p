# p2p

Napster-like peer-to-peer file transferring services

## Installing

Clone the repository, change directory to the root project folder, and run the following commands:
```sh
make peer
make index
```

## Docs
#### Peer node

To start a peer node, run the following command:
```sh
./peer <port number>
```

There are few options to operate a peer node. While peer node is running, type `Ctrl + C` to pause server loop.

```sh
======================================================
Starting peer server at port 9065
^C
======================================================
Peer server loop paused. Type 'help' for more details.
Enter options: help

Available options:
-----------------
> q/quit        Quit peer node
> req/request   Request file from index server
> reg/register  Register file to index server
> i/index       Update index server information
```

To register a file to index server, select `register` option, and enter file to be registered. The registered file will be confirmed by index server before being registered to the system, which can be further requested by other peers.

To request file, select `request` option, and enter file to be requested. Succesful download will appear in `files/` folder.

#### Index node
To start an index node, run the following command:
```sh
./index <port number>
```

There are few options to operate an index node. While index node is running, type `Ctrl + C` to pause server loop.

```sh
======================================================
Starting index server at port 9065
^C
======================================================
Index server loop paused. Type 'help' for more details.
Enter options: help

Available options:
-----------------
> q/quit        Quit index node
> query         Query file from entry table
```

## Report
[Proposal](assets/proposal.pdf) and final report can be found in `assets/` folder

## Authors

* [Trung Truong](https://github.com/ttrung149)

## License

This project is licensed under the MIT License - see the [LICENSE.txt](LICENSE.txt) file for details
