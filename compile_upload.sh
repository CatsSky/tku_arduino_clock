#!/bin/bash

arduino-cli compile --fqbn arduino:avr:mega $1 --upload --port /dev/ttyACM0
# sudo arduino-cli upload --port /dev/ttyACM0 --fqbn arduino:avr:mega $1
