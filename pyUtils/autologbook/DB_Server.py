#!/usr/bin/python3
# should work in either python2 or 3

import xmlrpc.client

if __name__ == "__main__":
    s = xmlrpc.client.ServerProxy('http://localhost:8000', allow_none=True)
    print(s.update())
