include $(TOPDIR)/rules.mk

PKG_NAME:=unittest
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/unittest
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=unittest -- prints a snarky message
	DEPENDS:=+libpthread +librt +libubox +libuci +libstdcpp +libusb-compat
endef

define Package/unittest/description
	Using the package to test some api
endef

define Build/Prepare
	echo "Here is Package/Prepare"
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/unittest/install
	echo "Here is Package/install"
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/unittest $(1)/bin/
endef

$(eval $(call BuildPackage,unittest))
