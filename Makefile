
GBA_ARM_CC = ${DEVKITARM}/bin/arm-none-eabi-g++
GBA_ARM_AR = ${DEVKITARM}/bin/arm-none-eabi-ar
FLAGS = -mthumb -mthumb-interwork -mcpu=arm7tdmi -mtune=arm7tdmi -D__GBA__ -fno-rtti -g3 -Wall -pedantic -ffast-math -fno-math-errno -Wdouble-promotion -fomit-frame-pointer -O2 -fno-exceptions -std=gnu++2a



all: gba-static regression


gba-static:
	@echo Build GBA Static Library
	${GBA_ARM_CC} -I ./ ${FLAGS} -c flash_filesystem.cpp platform/gba/gba_platform.cpp platform/gba/bootleg_cart.cpp
	@echo Done!


regression:
	@echo ""
	@echo Build and run regression...
	g++ -std=c++2a flash_filesystem.cpp -I ./ -g3 -D__FAKE_VECTOR__ -D__TEST__ -o fs_regression
	./fs_regression
	rm *.regr_output*


clean:
	rm *.o
