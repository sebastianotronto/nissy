# Towards a nissy webapp

## Architecture

* Split in client / server.
* Server can load and keep in memory all the tables, client(s) send messages to
  the server to run commands.
* Use UNIX sockets only first, maybe later try WinSock.

## Simple webapp

* Investigate how to use fastcgi, try simple program first.
* Decide what limits to put in terms of resources and write a "filter" script
  to block big requests (maybe use a timeout).

## Advanced webapp

* Use cubing.js for nice graphics.
* Port it to a graphical desktop version too.
