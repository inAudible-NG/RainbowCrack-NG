## FORK README
### NOTE

This is a monkey-patch to get it working on windows with modern (3.x.x) OpenSSL x64.
Beware, it's barely tested (and the original code is 8 years old)
SRC also now has test table and hashes.txt for your pleasure

### INSTALLATION (WINDOWS)
- Install OpenSSL from https://slproweb.com/products/Win32OpenSSL.html
- Launch x64 Native Tools Command Prompt for VS 20** (This opens CMD, continue in it)
- Go to .../RainbowCrack-NG/src (Wherever you cloned it)
- Run nmake -f makefile.win
Shouldn't break the linux, but who the fuck knows, if it did

### Changes
- Edited makefile.win to include the libraries
- #include <unistd.h> only if not _WIN32
- Replaced old libeay32.lib with libcrypto.lib
- Switched to targer x64 ('cause OpenSSL)
- Changed GetTableList for _WIN32 to work (don't know, why old one didn't and don't want to know. Just made 'Chat, j'ai pété' write a new one)
- Added files for testing the modules

## OG README
### RainbowCrack-NG

Free and open-source software to generate and use rainbow tables.

### Why?

FOSS packages to generate rainbow tables do not exist.

rcracki_mt (freerainbowtables), and RainbowCrack (project-rainbowcrack.com),
both went closed source, for the table generation part.

### Upstream

* https://packages.gentoo.org/package/app-crypt/rainbowcrack

* Based on "rainbowcrack-1.2-r2.ebuild" file

### Changes

* CRLF to LF changes

* Fix whitespace, and indentation errors

* Disabled "MD2" for good

* GetAvailPhysMemorySize portability fixes (borrowed from rcracki_mt project)

* Rainbow tables for Audible

### Platforms

Tested on DragonFly BSD 4.0.5, FreeBSD 10.1, Ubuntu 14.04.2 LTS
