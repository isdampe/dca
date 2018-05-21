#!/usr/bin/env python3
import os, time, datetime, shutil

def tryDeploy():

	fl =  os.listdir(os.path.expanduser('~/Downloads'))
	firmware = False
	firmwareName = False

	for file in fl:
		if file[-3:] == "bin":
			firmware = file
			break

	if not firmware:
		return
	
	firmwareName = firmware
	firmware = os.path.expanduser('~/Downloads/' + firmware)
	fl =  os.listdir('/Volumes/MBED')
	for file in fl:
		if not file[1:] == "." and file[-3:] == "bin":
			os.remove('/Volumes/MBED/' + file)

	time.sleep(0.5)
	shutil.copyfile(firmware, '/Volumes/MBED/' + firmwareName)
	os.remove(firmware)
	time.sleep(0.5)
	print("Deploy success at " + datetime.datetime.now().strftime("%I:%M:%S%p"))

if __name__ == "__main__":
	while True:
		tryDeploy()
		time.sleep(5.0)