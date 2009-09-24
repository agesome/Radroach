#!/bin/sh

cat .git/`cat .git/HEAD | awk '{print $2}'`