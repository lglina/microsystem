#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile).
#
_/_=/ShExtension=.shDevice=PIC32MM0256GPM064
ProjectDir=/home/lauren/microsystem/Client/Estelle
ProjectName=agape-estelle
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
image: /home/lauren/microsystem/Client/Estelle/estelle nbproject/Makefile-default.mk 
else
image: /home/lauren/microsystem/Client/Estelle/estelle nbproject/Makefile-default.mk 
endif

.PHONY: /home/lauren/microsystem/Client/Estelle/estelle
/home/lauren/microsystem/Client/Estelle/estelle: 
	cd . && make

.PHONY: /home/lauren/microsystem/Client/Estelle/estelle
/home/lauren/microsystem/Client/Estelle/estelle: 
	cd . && make


# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	cd . && make clean

