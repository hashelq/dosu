The dosu.c program

DATE: 2023.03.12
Written by: HASHELQ
LICENSE: MIT

SECURITY:
	I am not sure that this program is secure.
	Use at your own risk.
	Also I recommend to take a look at the code before using it.
	If a vulnerability found, please make an issue.

SUID:
	This program is intended to be run with +s permission (setuid).
	Before using it, add +s: "chmod +s ./dosu"

This is a short program that
1. Checks if the user has group "dosu"
2. Checks if the entered password is correct for the user
3. Runs a program as so if all requirements above are meet.

In other cases, it returns an uppercased error code.

Here is a list of error codes:
* SETUID: *error message*
* NO_CMD
* NO_SUID
* NO_PASSWORD
* WRONG_PASSWORD
* SYS_NO_DOSU_GROUP
* USR_NO_DOSU_GROUP
* CANNOT_EXECUTE: *error message*
