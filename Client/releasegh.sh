#!/bin/bash
./version_tool.py -inc && \
    make rebuild -j && \
    git commit -m "Increment" Version.cpp && \
    git push && \
    gh release create b`./version_tool.py` --title "b`./version_tool.py`" agape.hex
