
GBA_ARM_CC = ${DEVKITARM}/bin/arm-none-eabi-g++
GBA_ARM_AR = ${DEVKITARM}/bin/arm-none-eabi-ar
FLAGS = -mthumb -mthumb-interwork -mcpu=arm7tdmi -mtune=arm7tdmi -D__GBA__ -fno-rtti -g3 -Wall -pedantic -ffast-math -fno-math-errno -Wdouble-promotion -fomit-frame-pointer -O2 -fno-exceptions -std=gnu++2a



all: gba-static gba-example regression


gba-static:
	@echo Build GBA Static Library
	${GBA_ARM_CC} -I ./ ${FLAGS} -c flash_filesystem.cpp platform/gba/gba_platform.cpp platform/gba/bootleg_cart.cpp
	rm -f libGbaFlash.a
	${GBA_ARM_AR} qc libGbaFlash.a flash_filesystem.o gba_platform.o bootleg_cart.o
	@echo Done!


gba-example: gba-static
	${GBA_ARM_CC} -I ./ ${FLAGS} gba_example.cpp libGbaFlash.a ${DEVKITPRO}/libtonc/lib/libtonc.a -specs=gba.specs -o GbaExample
	${DEVKITARM}/bin/arm-none-eabi-objcopy -O binary GbaExample GbaFlash.gba
	${DEVKITPRO}/tools/bin/gbafix GbaFlash.gba


regression:
	@echo ""
	@echo Build and run regression...
	g++ -std=c++2a flash_filesystem.cpp -I ./ -g3 -D__FAKE_VECTOR__ -D__TEST__ -o fs_regression
	./fs_regression
	rm *.regr_output*


clean:
	rm -f *.o *.a *.regr_output *.sav
