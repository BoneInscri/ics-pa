deps_config := \
	src/device/Kconfig \
	src/memory/Kconfig \
	/media/boneinscri/Expand_1/HDU/ics-pa/ics2022/nemu/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
