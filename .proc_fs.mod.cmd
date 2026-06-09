savedcmd_proc_fs.mod := printf '%s\n'   proc_fs.o | awk '!x[$$0]++ { print("./"$$0) }' > proc_fs.mod
