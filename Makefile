ifdef BRANCH_NAME
BNAME=${BRANCH_NAME}
else
BNAME=$(shell basename ${PWD})
endif

DEST=repo.sheepshed.tk
PLATFORM=esp32dev
PROJ=SENG

all: .test.sf mkdir push

clean:
	pio run --target clean

mkdir:
	@echo "ssh ${DEST} \"mkdir -p /var/www/html/firmware/$(BNAME)/${PLATFORM}/${PROJ}\""
	@ssh ${DEST} "mkdir -p /var/www/html/firmware/$(BNAME)/${PLATFORM}/${PROJ}"

push:
	@echo "PROJ: ${PROJ}"
	@echo "PLATFORM: ${PLATFORM}"
	@pio run
	scp .pio/build/${PLATFORM}/firmware.bin ${DEST}:/var/www/html/firmware/$(BNAME)/${PLATFORM}/${PROJ}/firmware.bin

publish:
	@pio pkg publish

.test.sf: StateFlow/test.sf
	scp StateFlow/test.sf ${DEST}:/var/www/html/StateFlow/test.sf
	touch .test.sf

.upload:
	@pio  run --target upload

mu: .test.sf .upload .monitor

.monitor:
	@pio device monitor



