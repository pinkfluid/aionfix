#!/bin/sh -e

# HiveGamez AION Client launcher for Linux
#
# Requirements:
#   - GCC (or a working C compiler)
#   - wine
#   - winetricks

WINE=${WINE:=wine}

die()
{
    echo ERROR: "$@"
    exit 1
}

help()
{
    echo "aion_linux.sh -- AION launcher for legacy clients"
    echo "Usage:"
    echo
    echo "  start -- launch AION client"
    echo "  setup -- setup the WINE environment (requires winetricks)"
    echo
    echo "Note: Please run setup before starting the client or AION might not work properly."
    exit 1
}

aionfix_init()
{
    [ -e aionfix ] && continue
    cc --version > /dev/null 2>&1 || die "A C compiler is required to run this script."
    cc -O aionfix.c -o aionfix
}

aionfix_setup()
{
    winetricks --help > /dev/null 2>&1 || die "winetricks not installed. Cannot initialize WINE environment."
    echo "Downloading DirectX ..."
    curl -C - 'https://lutris.net/files/tools/directx-2010.tar.gz' -o /tmp/directx-2010.tar.gz || die "Error downloading DirectX"
    echo "Executing winetricks ..."
    winetricks vcrun2008 vcrun2013 corefonts win7 || die "winetricks installation failed"
    echo "Installing DirectX ..."
    mkdir -p "/tmp/directx"
    tar -C /tmp/directx -xzvf /tmp/directx-2010.tar.gz
    (cd /tmp/directx && "$WINE" DXSETUP.exe)
    rm -rf /tmp/directx
}

aionfix_init

case "$1" in
    start)
        echo 'Please run "aion_linux.sh setup" to setup the WINE environment.'
        sleep 2
        # The DLL override below is required to for the custom version.dll
        # override to work (mouse fix).
        # Among other things, the "mouse fix" provides the ability to connect
        # to private servers, so yeah, it is important.
        WINEDLLOVERRIDES="version=n,b" ./aionfix wine Aion_start_HIVEGAMEZ_cc1_ENU.bat
        ;;

    setup)
        aionfix_setup
        ;;

    *)
        help
        ;;

esac
