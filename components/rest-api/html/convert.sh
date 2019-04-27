#!/bin/bash

xxd -i -a index.html > ../include/index.html.h
sed -i -e 's/unsigned char/const char/g' ../include/index.html.h
sed -i -e 's/unsigned int/const unsigned int/g' ../include/index.html.h