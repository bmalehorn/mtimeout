#!/bin/bash

mtimeout /tmp/ 5 sleep 1000 &
sleep 1
kill %1
wait
ps aux | grep -v grep | grep 'sleep 1000'
