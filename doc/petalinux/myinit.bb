#
# This file is the myapp-init recipe.
#
SUMMARY = "Simple myinit application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://myinit \
 "

S = "${WORKDIR}"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
inherit update-rc.d
INITSCRIPT_NAME = "myinit"
INITSCRIPT_PARAMS = "start 99 S ."

do_install() {
 install -d ${D}${sysconfdir}/init.d
 install -m 0755 ${S}/myinit ${D}${sysconfdir}/init.d/myinit
}

FILES_${PN} += "${sysconfdir}/*"

