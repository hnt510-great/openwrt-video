include $(TOPDIR)/rules.mk

PKG_NAME:=libxkbcommon
PKG_VERSION:=1.13.2
PKG_RELEASE:=5

PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=xkbcommon-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/xkbcommon/$(PKG_NAME).git
PKG_MIRROR_HASH:=84d08ef1896ca5eb84d29d6134c95ce0076eac2a1d91b87cc24ccfa5e70cbe25

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=BSD-3-Clause

PKG_INSTALL:=1

PKG_BUILD_DEPENDS:=wayland libxcb

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

MESON_ARGS += \
	-Denable-x11=true \
	-Denable-docs=false \
	-Denable-wayland=true \
	-Dxkb-config-unversioned-extensions-path=/usr/share/xkeyboard-config.d \
	-Dxkb-config-versioned-extensions-path=/usr/share/X11/xkb.d

define Package/libxkbcommon
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libxkbcommon
  URL:=https://xkbcommon.org/
  DEPENDS:=+libwayland +libxml2 +wayland-protocols
endef

define Package/libxkbcommon/description
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libxkbcommon-x11
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libxkbcommon X11 support
  URL:=https://xkbcommon.org/
  DEPENDS:=+libxkbcommon +libxcb
endef

define Package/libxkbcommon/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxkbcommon.so.* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxkbregistry.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/libexec/xkbcommon
	$(CP) $(PKG_INSTALL_DIR)/usr/libexec/xkbcommon/* $(1)/usr/libexec/xkbcommon/
	rm -f $(1)/usr/libexec/xkbcommon/*-x11
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/xkbcli $(1)/usr/bin/
endef

define Package/libxkbcommon-x11/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxkbcommon-x11.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/libexec/xkbcommon
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/libexec/xkbcommon/xkbcli-dump-keymap-x11 \
		$(PKG_INSTALL_DIR)/usr/libexec/xkbcommon/xkbcli-interactive-x11 \
		$(1)/usr/libexec/xkbcommon/
endef

$(eval $(call BuildPackage,libxkbcommon))
$(eval $(call BuildPackage,libxkbcommon-x11))
