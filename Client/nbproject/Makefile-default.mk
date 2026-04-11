#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile).
#
_/_=/ShExtension=.shDevice=PIC32MZ2048EFG064
ProjectDir=/home/lauren/microsystem/Client
ProjectName=agape-boopie
ConfName=default
TOOLCHAIN_XC32=/opt/microchip/xc32/v4.60/bin
#
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
ifeq (,$(findstring nbproject/Makefile-local-default.mk,$(MAKEFILES)))
MAKEFILES+=nbproject/Makefile-local-default.mk
endif
endif
endif
.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-default.mk image

# ------------------------------------------------------------------------------------
# Rules for buildStep: build and debug
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
image: /home/lauren/microsystem/Client/agape nbproject/Makefile-default.mk 
else
image: /home/lauren/microsystem/Client/agape nbproject/Makefile-default.mk 
endif

.PHONY: /home/lauren/microsystem/Client/agape
/home/lauren/microsystem/Client/agape: 
	cd . && make

.PHONY: /home/lauren/microsystem/Client/agape
/home/lauren/microsystem/Client/agape: 
	cd . && make


# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	cd . && make clean

