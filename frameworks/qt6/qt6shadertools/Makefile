#
# Copyright (C) 2026 OpenWrt
# Author: Mirko Vogt <mirko-openwrt@nanl.de>
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=qt6shadertools
PKG_RELEASE:=1
PKG_HASH:=d86079425ec71eecbb05f4d20623b0a403a7d21a5cfa7efb9734ea3d538d801b

PKG_MAINTAINER:=Mirko Vogt <mirko-openwrt@nanl.de>
PKG_LICENSE:=LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
# Qt's own licenses, followed by those of the bundled third-party code
# (glslang and SPIRV-Cross, linked statically into QtShaderTools)
PKG_LICENSE_FILES:= \
  LICENSES/LicenseRef-Qt-Commercial.txt \
  LICENSES/LGPL-3.0-only.txt \
  LICENSES/GPL-2.0-only.txt \
  LICENSES/GPL-3.0-only.txt \
  LICENSES/AML-glslang.txt \
  LICENSES/Apache-2.0.txt \
  LICENSES/BSD-3-Clause.txt \
  LICENSES/Bison-exception-2.2.txt \
  LICENSES/GPL-3.0-or-later.txt \
  LICENSES/LicenseRef-MIT-Khronos-old.txt \
  LICENSES/MIT.txt

PKG_BUILD_DEPENDS:=qt6base qt6shadertools/host
HOST_BUILD_DEPENDS:=qt6base/host

QT_MODULE:=qtshadertools

include ../common.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/nls.mk
include ../cmake.mk

define Package/qt6shadertools
  SECTION:=video-frameworks
  CATEGORY:=Video
  SUBMENU:=Frameworks and Toolkits
  TITLE:=Qt6ShaderTools
  DEPENDS:=+qt6base-core +qt6base-gui +libstdcpp
  URL:=https://www.qt.io
endef

Build/InstallDev = $(Build/InstallDev/Qt)

define Package/qt6shadertools/install
	$(call Build/Install/Libs,$(1),libQt6ShaderTools)
endef

$(eval $(call BuildPackage,qt6shadertools))
$(eval $(call HostBuild))
