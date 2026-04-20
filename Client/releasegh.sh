#!/bin/bash
./version.py -inc && make rebuild -j && gh release create b`./version.py` agape.hex
