#Makefile for program 4
#Author : Prakhar Keshari (pkeshari)
all : script
script : script.o
	gcc script.o -o script
script.o : script.c 
	gcc -std=c99 -pedantic -Wall -c script.c
clean :
	rm script.o script