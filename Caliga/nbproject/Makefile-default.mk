#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile).
#
_/_=/ShExtension=.shDevice=PIC32MZ2048EFG064
ProjectDir=/home/lauren/agape/Caliga
ProjectName=Caliga
ConfName=default
TOOLCHAIN_XC32=/opt/microchip/xc32/v4.45/bin
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
image: /home/lauren/agape/Caliga/caliga nbproject/Makefile-default.mk 
else
image: /home/lauren/agape/Caliga/caliga nbproject/Makefile-default.mk 
endif

.PHONY: /home/lauren/agape/Caliga/caliga
/home/lauren/agape/Caliga/caliga: 
	cd . && make

.PHONY: /home/lauren/agape/Caliga/caliga
/home/lauren/agape/Caliga/caliga: 
	cd . && make


# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	cd . && make clean

