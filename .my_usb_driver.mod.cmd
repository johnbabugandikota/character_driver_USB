savedcmd_my_usb_driver.mod := printf '%s\n'   my_usb_driver.o | awk '!x[$$0]++ { print("./"$$0) }' > my_usb_driver.mod
