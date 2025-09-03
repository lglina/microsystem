#!/usr/bin/python3

wordlist = open('wordlist.txt', 'r')

lines = wordlist.readlines()

print("[")
firstline = True
for line in lines:
    line = line.strip()
    if(firstline == False):
        print(",")
    firstline = False
    print(f"    \"{line}\"", end="")

print("\n]")
