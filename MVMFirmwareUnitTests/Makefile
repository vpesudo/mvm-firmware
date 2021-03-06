#
# File: Makefile
#
# Author: Francesco Prelz Francesco.Prelz@mi.infn.it
#
# Revision history:
#
#  23-Apr-2020 Initial creation.
#
# Description:
# Unix makefile for building the MVM Firmware Unit test framework.
#

MVM_FW_ROOT := ../MVMFirmwareCpp/MVMFirmwareCore
MVM_FW_INCLUDES := -I . -I $(MVM_FW_ROOT) -I rapidjson/include -I exprtk

DEBUG = -g 

GCOV_OPTS = -fprofile-arcs  -ftest-coverage -g 

CPPFLAGS := $(MVM_FW_INCLUDES) $(CPPFLAGS)
CXXFLAGS := --std=c++11 $(GCOV_OPTS) $(DEBUG) $(CXXFLAGS)

                       
MVM_FIRMWARE_OBJECTS = HAL.o \
                       Alarms.o \
                       CircularBuffer.o \
                       ConfigManager.o \
                       DebugIface.o \
                       driver_5525DSO.o \
                       driver_ADS1115.o \
                       driver_OxygenSensor.o \
                       driver_SFM3019.o \
                       driver_Supervisor.o \
                       driver_VenturiFlowMeter.o \
                       hw_ard_esp32.o \
                       MVMCore.o \
                       MVM_StateMachine.o \
                       PressureLoop.o \
                       TidalVolume.o \
                       hw.o

MWM_FW_TEST_OBJECTS =  SerialImpl.o \
                       stdlib_noniso.o WString.o \
                       simulated_fw_board_v4.o \
                       i2c_device_library.o \
                       simulate_i2c_devices.o \
                       mvm_fw_unit_test_config.o \
                       mvm_fw_unit_test_run.o

MVM_FW_TEST_EXECUTABLES = mvm_fw_unit_test_run

all: fw_board_ni_v4.h $(MVM_FIRMWARE_OBJECTS) $(MWM_FW_TEST_OBJECTS) \
     $(MVM_FW_TEST_EXECUTABLES) ;

mvm_fw_unit_test_run.o: mvm_fw_unit_test_run.cpp quantity_timelines.hpp ;\
   $(COMPILE.C) -D WITH_POSIX_PTS mvm_fw_unit_test_run.cpp

fw_board_ni_v4.h: simulated_fw_board_v4.h ;\
   /bin/ln simulated_fw_board_v4.h fw_board_ni_v4.h; \
   /bin/mv $(MVM_FW_ROOT)/fw_board_ni_v4.h \
           $(MVM_FW_ROOT)/fw_board_ni_v4.h.STASH_OUT_OF_THE_WAY


mvm_fw_unit_test_run: $(MWM_FW_TEST_OBJECTS) $(MVM_FIRMWARE_OBJECTS); \
    $(LINK.C) -o $@ $(MWM_FW_TEST_OBJECTS) $(MVM_FIRMWARE_OBJECTS)

%.o: $(MVM_FW_ROOT)/%.cpp ;\
   $(COMPILE.C) $<

test_qtl: test_qtl.o; \
    $(LINK.C) -o $@ test_qtl.o

test_pflow: test_pflow.o mvm_fw_unit_test_config.o; \
    $(LINK.C) -o $@ test_pflow.o mvm_fw_unit_test_config.o

gcov: mvm_fw_unit_test_run.gcda mvm_fw_unit_test_run.gcno ; \
    gcov $(MVM_FIRMWARE_OBJECTS)

clean_gcov: ; /bin/rm -f *.gcov *.gcda 

clean: clean_gcov ; /bin/rm -f *.o $(MVM_FW_TEST_EXECUTABLES);
