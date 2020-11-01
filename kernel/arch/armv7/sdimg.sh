#!/bin/bash

dd if=/dev/zero of=sd.img bs=1M count=32
mkfs.ext4 sd.img
mkdir -p sd_tmp
sudo mount -o loop sd.img ./sd_tmp/
sudo cp lambda.ukern sd_tmp/
sudo umount sd_tmp
sleep 1
sudo rm -rf sd_tmp