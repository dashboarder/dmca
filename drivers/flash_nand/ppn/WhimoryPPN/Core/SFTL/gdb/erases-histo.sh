#!/bin/sh

sed 's/.*://g' | awk '{ print int($1/10)*10 }' | sort -n | uniq -c
