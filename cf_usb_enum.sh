#!/bin/bash

#bash script to list Crystalfontz USB LCD device paths

#uncomment, and set VID/PID to find specific type of module
#see crystalfontz-usb-id-assignments-2019.pdf for device IDs
#VENDORID=223b
#MODELID=0005

###################################

for sysdevpath in $(find /sys/bus/usb/devices/usb*/ -name dev); do
    (
        syspath="${sysdevpath%/dev}"
        devname="$(udevadm info -q name -p $syspath)"
        [[ "$devname" == "bus/"* ]] && continue
        eval "$(udevadm info -q property --export -p $syspath)"
        [[ -z "$ID_SERIAL" ]] && continue
		if [ -z "$VENDORID" ]
		then
			#no VID specified, look for any CF device
			if [ "$ID_VENDOR_ID" = "223b" ]
			then
				#is a CF usb device
				echo $DEVNAME
			fi
			if [ "$ID_VENDOR_ID" = "0403" ]
			then
				#if a FTDI VID, check the PID
				if [ "$ID_MODEL_ID" = "fc08" -o "$ID_MODEL_ID" = "fc09" \
					-o "$ID_MODEL_ID" = "fc0a" -o "$ID_MODEL_ID" = "fc0b" \
					-o "$ID_MODEL_ID" = "fc0c" -o "$ID_MODEL_ID" = "fc0d" \
					-o "$ID_MODEL_ID" = "fc0e" ]
				then
					#is a CF usb device
					echo $DEVNAME
				fi
			fi
		else
			#VID/PID specified, match that device only
			if [ "$ID_VENDOR_ID" = "$VENDORID" -a "$ID_MODEL_ID" = "$MODELID" ]
			then
				#matched vid/pid
				echo $DEVNAME
			fi
		fi
    )
done
