# HiveGamez AION Launcher for Linux/WINE

Legacy AION clients (mostly used on private servers) tend to not work properly
with WINE. This launchers provides fix for 2 of the 3 issues I've identified to
make the client run flawlessly. The 3rd fix is a kernel parameter and requires
manual user intervention to make it work. See below.

## Dependencies

- wine
- winetricks
- GCC (or a working C compiler)

## Usage

```
# Copy hg_aion_linux.sh and `aionfix.c` to the HiveGamez folder
cp hg_aion_linux.sh aionfix.c AION_FOLDER

cd AION_FOLDER

# Before starting the client you need to initialize the WINE environment
./hg_aion_linux.sh setup

# Launch AION client
./hg_aion_linux.sh start
```

# Addressed Issues

List of issues that are addressed by the launcher.

## version.dll Override

Most AION private servers use a hacker `version.dll` file to trick the client
to allow it to connect to their servers. WINE by default uses the built-in
`version.dll` file, which obviously doesn't provide this functionality.

To use the client's `version.dll` file, the WINEDLLOVERRIDES variable must be
set as follows:

```
WINEDLLOVERRIDES="version=n,b"
```

## NtReadVirtualMemory()

Some AION clients issue an excessive amount of calls to `NtReadVirtualMemory()`.
This is most apparent during tab-targetting where it causes massive lag spikes.

`NtReadVirtualMemory() is Windows kernel system call that allows a process to
read the memory of another process. However, in this case, it seems to be used
as some sort of anti-cheat mechanism where the client reads its own memory
(probably for the purpose of scanning for well-known hacks).

The issue with WINE is that the system call is implemented using ptrace(), which
is a rather expensive operation. So for each call to `NtReadVirtualMemory()`, a
ptrace attach/read/detach is performed which is extremely slow.

The workaround is to actually use ptrace() on the AION executable, so when WINE
tries to ptrace() it, it actually fails. This causes `NtReadVirtualMemory()` to
fail as well, but the anti-cheat code doesn't seem to be bothered by it.

This is what `aionfix.c` is for.

I've also implemented experimental WINE patches that bypass the ptrace()
codepath when a process calls `NtReadVirtualMemory()` to read its own memory
(instead it reads the memory directly). This gives the best overall performance,
but requires a custom WINE build.

# Unadressed Issues

## UMIP

If you're still experiencing FPS drop after using the launcher, check your
kernel log messages that look like the ones below:

```
[ 9920.972119] umip: XXXXXXX[25225] ip:14d481e87 sp:327358: SGDT instruction cannot be used by applications.
```

In that case, you need to add the `clearcpuid=514` to the kernel parameters.

