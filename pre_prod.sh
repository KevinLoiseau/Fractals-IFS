#/!bin/bash
cd /opt/Fractals-IFS;
#git reset --hard;git pull;
make build
ssh ompi@192.168.10.252 "cd /opt/Fractals-IFS;git reset --hard;git pull;make build"
ssh ompi@192.168.10.234 "cd /opt/Fractals-IFS;git reset --hard;git pull;make build"
ssh ompi@192.168.10.233 "cd /opt/Fractals-IFS;git reset --hard;git pull;make build"
ssh ompi@192.168.10.250 "cd /opt/Fractals-IFS;git reset --hard;git pull;make build"

