RainbowCrack 1.2 readme

RainbowCrack is a general propose implementation of Philippe Oechslin<http://lasecwww.epfl.ch/philippe.shtml>'s faster time-memory trade-off technique<http://lasecwww.epfl.ch/php_code/publications/search.php?ref=Oech03>.
In short, the RainbowCrack tool is a hash cracker. While a traditional brute force cracker try all possible plaintexts one by one in cracking time, RainbowCrack works in another way. It precompute all possible plaintext - ciphertext pairs in advance and store them in the file so called "rainbow table". It may take a long time to precompute the tables, but once the one time precomputation is finished, you will always be able to crack the ciphertext covered by the rainbow tables in seconds.

System requirements
Operating System: windows, linux
CPU:              the faster the better
Memory:           rtgen:  no special requirement
                  rtdump: no special requirement
                  rtsort: 32MB free physical memory at least
                  rcrack: no special requirement, performance is maximized when free physical memory size is as large as the rainbow table in use, however, it still works well when memory is low

Documentation
First of all, you can always take Philippe Oechslin's paper as your best reference of the algorithm.
If you have never used the tool before, you can take a glance at some sample outputs of RainbowCrack<doc/rcrackdemo.htm> when cracking some alpha only and alpha numeric windows passwords.
The RainbowCrack tutorial<doc/rcracktutorial.htm> is an essential reading if you want to make things working.
"Large charset configurations for RainbowCrack"<doc/configurations.htm> is useful for those want to create rainbow tables for large charset. It is still an interesting reading if you don't have enough processor resource.
Some tips about charset customization is in the document "Custom charset in RainbowCrack"<doc/customcharset.htm>.

Compatibility issue
RainbowCrack 1.2 support all rainbow tables generated/sorted by earlier versions of the tool.

Changes in version 1.2
- Multiple hash algorithms support
  RainbowCrack 1.2 is more than an instant windows password cracker, multiple hash algorithms(md5, sha1...) are supported which means it is also an instant md5 hash cracker, an instant sha1 hash cracker...
  Other hash algorithm can be easily added as required.
- Multiple platforms support
  RainbowCrack now works on windows and linux. The rainbow tables are exchangable, which means you can generate the tables on one platform and use it on the other.
  The code is in c++ and stl, porting to other system will not be very difficult.
- Configurable plaintext length support
  This is useful if you want to generate some application specific rainbow tables, with application specific hash algorithm and charset.
- LanManager password case correction
  It is with the help of NTLM hash in pwdump file. However, this kind of case correction takes very little time.
- Filename convenience change
  Earlier versions of RainbowCrack will generate the table like "lm_alpha_0_2100x8000000_all.rt", while the new version generate the table like "lm_alpha#1-7_0_2100x8000000_all.rt". Here "#1-7" is the plaintext length definition. In earlier versions, it is fixed to "1 to 7". If RainbowCrack 1.2 doesn't find the plaintext length definition in the filename, it will consider it as "1 to 7". Alteratively, you can manually rename the file "lm_alpha_0_2100x8000000_all.rt" to the new style filename "lm_alpha#1-7_0_2100x8000000_all.rt". It makes no difference to RainbowCrack 1.2.

Changes in version 1.1
- Custom charset support

Changes in version 1.01
- Bugfix

Changes in version 1.0
- Initial release

Zhu Shuanglei <shuanglei@hotmail.com>
http://www.antsight.com/zsl/rainbowcrack/
2003/11/21
