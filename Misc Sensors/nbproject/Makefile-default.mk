#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=cof
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Misc_Sensors.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Misc_Sensors.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=J1939.C main.c ecocar.c ADC/adcbusy.c ADC/adcclose.c ADC/adcconv.c ADC/adcopen.c ADC/adcread.c ADC/adcselchconv.c ADC/adcsetch.c AnalogHelper.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/J1939.o ${OBJECTDIR}/main.o ${OBJECTDIR}/ecocar.o ${OBJECTDIR}/ADC/adcbusy.o ${OBJECTDIR}/ADC/adcclose.o ${OBJECTDIR}/ADC/adcconv.o ${OBJECTDIR}/ADC/adcopen.o ${OBJECTDIR}/ADC/adcread.o ${OBJECTDIR}/ADC/adcselchconv.o ${OBJECTDIR}/ADC/adcsetch.o ${OBJECTDIR}/AnalogHelper.o
POSSIBLE_DEPFILES=${OBJECTDIR}/J1939.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/ecocar.o.d ${OBJECTDIR}/ADC/adcbusy.o.d ${OBJECTDIR}/ADC/adcclose.o.d ${OBJECTDIR}/ADC/adcconv.o.d ${OBJECTDIR}/ADC/adcopen.o.d ${OBJECTDIR}/ADC/adcread.o.d ${OBJECTDIR}/ADC/adcselchconv.o.d ${OBJECTDIR}/ADC/adcsetch.o.d ${OBJECTDIR}/AnalogHelper.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/J1939.o ${OBJECTDIR}/main.o ${OBJECTDIR}/ecocar.o ${OBJECTDIR}/ADC/adcbusy.o ${OBJECTDIR}/ADC/adcclose.o ${OBJECTDIR}/ADC/adcconv.o ${OBJECTDIR}/ADC/adcopen.o ${OBJECTDIR}/ADC/adcread.o ${OBJECTDIR}/ADC/adcselchconv.o ${OBJECTDIR}/ADC/adcsetch.o ${OBJECTDIR}/AnalogHelper.o

# Source Files
SOURCEFILES=J1939.C main.c ecocar.c ADC/adcbusy.c ADC/adcclose.c ADC/adcconv.c ADC/adcopen.c ADC/adcread.c ADC/adcselchconv.c ADC/adcsetch.c AnalogHelper.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE} ${MAKE_OPTIONS} -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/Misc_Sensors.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=18F2685
MP_PROCESSOR_OPTION_LD=18f2685
MP_LINKER_DEBUG_OPTION= -u_DEBUGCODESTART=0x17d30 -u_DEBUGCODELEN=0x2d0 -u_DEBUGDATASTART=0xcf4 -u_DEBUGDATALEN=0xb
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/J1939.o: J1939.C  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/J1939.o.d 
	@${RM} ${OBJECTDIR}/J1939.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/J1939.o   J1939.C 
	@${DEP_GEN} -d ${OBJECTDIR}/J1939.o 
	@${FIXDEPS} "${OBJECTDIR}/J1939.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/main.o   main.c 
	@${DEP_GEN} -d ${OBJECTDIR}/main.o 
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ecocar.o: ecocar.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/ecocar.o.d 
	@${RM} ${OBJECTDIR}/ecocar.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ecocar.o   ecocar.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ecocar.o 
	@${FIXDEPS} "${OBJECTDIR}/ecocar.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcbusy.o: ADC/adcbusy.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcbusy.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcbusy.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcbusy.o   ADC/adcbusy.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcbusy.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcbusy.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcclose.o: ADC/adcclose.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcclose.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcclose.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcclose.o   ADC/adcclose.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcclose.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcclose.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcconv.o: ADC/adcconv.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcconv.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcconv.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcconv.o   ADC/adcconv.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcconv.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcconv.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcopen.o: ADC/adcopen.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcopen.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcopen.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcopen.o   ADC/adcopen.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcopen.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcopen.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcread.o: ADC/adcread.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcread.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcread.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcread.o   ADC/adcread.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcread.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcread.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcselchconv.o: ADC/adcselchconv.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcselchconv.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcselchconv.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcselchconv.o   ADC/adcselchconv.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcselchconv.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcselchconv.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcsetch.o: ADC/adcsetch.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcsetch.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcsetch.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcsetch.o   ADC/adcsetch.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcsetch.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcsetch.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/AnalogHelper.o: AnalogHelper.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/AnalogHelper.o.d 
	@${RM} ${OBJECTDIR}/AnalogHelper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/AnalogHelper.o   AnalogHelper.c 
	@${DEP_GEN} -d ${OBJECTDIR}/AnalogHelper.o 
	@${FIXDEPS} "${OBJECTDIR}/AnalogHelper.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
else
${OBJECTDIR}/J1939.o: J1939.C  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/J1939.o.d 
	@${RM} ${OBJECTDIR}/J1939.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/J1939.o   J1939.C 
	@${DEP_GEN} -d ${OBJECTDIR}/J1939.o 
	@${FIXDEPS} "${OBJECTDIR}/J1939.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/main.o   main.c 
	@${DEP_GEN} -d ${OBJECTDIR}/main.o 
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ecocar.o: ecocar.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/ecocar.o.d 
	@${RM} ${OBJECTDIR}/ecocar.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ecocar.o   ecocar.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ecocar.o 
	@${FIXDEPS} "${OBJECTDIR}/ecocar.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcbusy.o: ADC/adcbusy.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcbusy.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcbusy.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcbusy.o   ADC/adcbusy.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcbusy.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcbusy.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcclose.o: ADC/adcclose.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcclose.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcclose.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcclose.o   ADC/adcclose.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcclose.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcclose.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcconv.o: ADC/adcconv.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcconv.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcconv.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcconv.o   ADC/adcconv.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcconv.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcconv.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcopen.o: ADC/adcopen.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcopen.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcopen.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcopen.o   ADC/adcopen.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcopen.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcopen.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcread.o: ADC/adcread.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcread.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcread.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcread.o   ADC/adcread.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcread.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcread.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcselchconv.o: ADC/adcselchconv.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcselchconv.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcselchconv.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcselchconv.o   ADC/adcselchconv.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcselchconv.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcselchconv.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/ADC/adcsetch.o: ADC/adcsetch.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/ADC 
	@${RM} ${OBJECTDIR}/ADC/adcsetch.o.d 
	@${RM} ${OBJECTDIR}/ADC/adcsetch.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/ADC/adcsetch.o   ADC/adcsetch.c 
	@${DEP_GEN} -d ${OBJECTDIR}/ADC/adcsetch.o 
	@${FIXDEPS} "${OBJECTDIR}/ADC/adcsetch.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/AnalogHelper.o: AnalogHelper.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/AnalogHelper.o.d 
	@${RM} ${OBJECTDIR}/AnalogHelper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/AnalogHelper.o   AnalogHelper.c 
	@${DEP_GEN} -d ${OBJECTDIR}/AnalogHelper.o 
	@${FIXDEPS} "${OBJECTDIR}/AnalogHelper.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/Misc_Sensors.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE)   -p$(MP_PROCESSOR_OPTION_LD)  -w -x -u_DEBUG -m"${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map"  -z__MPLAB_BUILD=1  -u_CRUNTIME -z__MPLAB_DEBUG=1 -z__MPLAB_DEBUGGER_PK3=1 $(MP_LINKER_DEBUG_OPTION) -l ${MP_CC_DIR}\\..\\lib  -o dist/${CND_CONF}/${IMAGE_TYPE}/Misc_Sensors.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}   
else
dist/${CND_CONF}/${IMAGE_TYPE}/Misc_Sensors.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE)   -p$(MP_PROCESSOR_OPTION_LD)  -w  -m"${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map"  -z__MPLAB_BUILD=1  -u_CRUNTIME -l ${MP_CC_DIR}\\..\\lib  -o dist/${CND_CONF}/${IMAGE_TYPE}/Misc_Sensors.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}   
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
