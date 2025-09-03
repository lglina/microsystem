#!/usr/bin/python3

wordlist = open('wordlist.txt', 'r')

lines = wordlist.readlines()

lastthree = ""
for line in lines:
    line = line.strip()
    if(lastthree and lastthree in line):
        print(f"{lastthree} -> {line}")
    if(len(line) == 3):
        print(line)
        lastthree = line
