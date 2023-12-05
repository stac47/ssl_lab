# Technical prelude
SHELL := bash
.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

main: main.o
	gcc -o $@ $< -lssl -lcrypto 

main.o: main.c
	gcc -c -o $@ $<

