include $(TOPDIR)/rules.mk

PKG_NAME:=gdk-pixbuf
PKG_VERSION:=2.44.8
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNOME/$(PKG_NAME)/$(basename $(PKG_VERSION))
PKG_HASH:=919f529512961a12e81cd4b4b466a48c3933469e7f9a310c6513cd4fb252ba3c

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gnome:gdk-pixbuf

PKG_BUILD_DEPENDS:=glib2/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

MESON_ARGS += \
	-Ddocumentation=false \
	-Dglycin=disabled \
	-Dinstalled_tests=false \
	-Dman=false \
	-Dtests=false

define Package/libgdk-pixbuf
  SECTION:=video-frameworks
  CATEGORY:=Video
  SUBMENU:=Frameworks and Toolkits
  DEPENDS:=+PACKAGE_glib2-gir:glib2-gir +giflib +libpng +libtiff +libjpeg +shared-mime-info
  TITLE:=GdkPixbuf: Image loading library
  URL:=http://www.gtk.org/
endef

define Package/libgdk-pixbuf/description
  GdkPixbuf is a library that loads image data in various formats and stores it as linear buffers in memory.
endef

PKG_CONFIG_DEPENDS += CONFIG_PACKAGE_glib2-gir
# Metadata scans ignore .config, so declare the GIR build deps conditionally.
PKG_BUILD_DEPENDS += PACKAGE_glib2-gir:gobject-introspection/host PACKAGE_glib2-gir:gobject-introspection PACKAGE_glib2-gir:glib2-gir
ifeq ($(CONFIG_PACKAGE_glib2-gir),y)
include $(INCLUDE_DIR)/gobject-introspection.mk
MESON_ARGS += -Dintrospection=enabled -Ddefault_library=both
endif

define Build/InstallDev
	$(if $(CONFIG_PACKAGE_glib2-gir),$(INSTALL_DIR) $(1)/usr/share/gir-1.0 $(1)/usr/lib/girepository-1.0)
	$(if $(CONFIG_PACKAGE_glib2-gir),$(CP) $(PKG_INSTALL_DIR)/usr/share/gir-1.0/*.gir $(1)/usr/share/gir-1.0/)
	$(if $(CONFIG_PACKAGE_glib2-gir),$(CP) $(PKG_INSTALL_DIR)/usr/lib/girepository-1.0/*.typelib $(1)/usr/lib/girepository-1.0/)
	$(INSTALL_DIR) $(1)/usr/include/gdk-pixbuf-2.0/gdk-pixbuf
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/gdk-pixbuf-2.0/gdk-pixbuf/*.h \
		$(1)/usr/include/gdk-pixbuf-2.0/gdk-pixbuf/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.{so*,a} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig
endef

define Package/libgdk-pixbuf/install
	$(if $(CONFIG_PACKAGE_glib2-gir),$(INSTALL_DIR) $(1)/usr/lib/girepository-1.0)
	$(if $(CONFIG_PACKAGE_glib2-gir),$(CP) $(PKG_INSTALL_DIR)/usr/lib/girepository-1.0/*.typelib $(1)/usr/lib/girepository-1.0/)
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libgdk-pixbuf))
