#!/usr/bin/python3

wordlist = open('wordlist.txt', 'r')

lines = wordlist.readlines()

lastword = ""
for line in lines:
    line = line.strip()

    if(lastword):
        if(len(lastword) == 3):
            if(lastword == line[0:3]):
                print(f"Dup prefix: {lastword} {line}")
        elif(len(line) == 3):
            if(lastword[0:3] == line):
                print(f"Dup prefix: {lastword} {line}")
        else:
            if(lastword[0:4] == line[0:4]):
                print(f"Dup prefix: {lastword} {line}")
    lastword = line
