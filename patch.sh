#!/bin/bash

wget $1.patch
patch -p1 < *.patch
echo ----------------------------------------------------------------
find . -regextype 'posix-egrep' -regex '.*\.rej$'
find . -regextype 'posix-egrep' -regex '.*\.(patch|orig)$' | xargs rm

