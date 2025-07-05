#!/bin/bash

# ps 명령어를 사용하여 root 사용자가 실행하는 프로세스 중 CMD가 ssu_sync인 프로세스들을 찾고,
# 해당 프로세스들의 PID를 얻어옵니다.
# 그 후 얻어온 PID를 이용하여 해당 프로세스를 kill합니다.

# ps 명령어를 사용하여 root 사용자가 실행하는 프로세스 중 CMD가 ssu_sync인 프로세스들을 찾고, 해당 PID들을 얻어옵니다.
pids=$(ps -o pid,cmd | grep 'daemon' | grep -v grep | awk '{print $1}')

# 얻어온 PID들을 이용하여 각각의 프로세스를 kill합니다.
for pid in $pids; do
    echo "Killing process with PID: $pid"
    kill 9 $pid
done
