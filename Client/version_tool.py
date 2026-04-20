#!/usr/bin/python3

import sys
import re

file = open("Version.cpp", "r")
contents = file.read()

match = re.search(r"= (\d);", contents)
if(match):
    current_version = match.group(1)

    if(len(sys.argv) == 1):
        print(f"{current_version}")
    elif(len(sys.argv) == 2 and sys.argv[1] == "-inc"):
        new_contents = re.sub(r"\d;", f"{int(current_version) + 1};", contents)
        file.close()
        file = open("Version.cpp", "w")
        print(new_contents, file=file, end="")
    else:
        print(f"Usage: {sys.argv[0]} [-inc]")

file.close()
