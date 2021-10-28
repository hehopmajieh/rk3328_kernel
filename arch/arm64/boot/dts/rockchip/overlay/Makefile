# SPDX-License-Identifier: GPL-2.0
dtbo-$(CONFIG_ARCH_ROCKCHIP) += \
	rk3328-i2c1.dtbo \
	rk3328-uart1.dtbo \
	rk3328-uart2.dtbo \
	rk3328-console-on-uart1.dtbo \
	rk3328-console-on-uart2.dtbo \
	rk3328-spi-spidev.dtbo \
	rk3328-w1-gpio.dtbo \
	rk3328-usb-otg.dtbo \
	rk3328-rock-pi-e-model-v11.dtbo

scr-$(CONFIG_ARCH_ROCKCHIP) += \
	rockchip-fixup.scr

dtbotxt-$(CONFIG_ARCH_ROCKCHIP) += \
	README.rockchip-overlays

targets += $(dtbo-y) $(scr-y) $(dtbotxt-y)

always			:= $(dtbo-y) $(scr-y) $(dtbotxt-y)
clean-files		:= *.dtbo *.scr

