Contributing to the TTauri project
==================================

Code of Conduct
---------------
This project and everyone participating in it is foverned by the
(TTauri Code of Conduct)[https://github.com/ttauri-project/ttauri/blob/main/docs/CODE_OF_CONDUCT.md]

Communication channels
----------------------
For communication we use the following Discord server: https://discord.gg/7e8pFTsujw


Testing the demo application
----------------------------
There is a demo application included with the releases of ttauri.

It would be nice if people could test if this application will work on their computers.
And when there is a crash to create a mini-dump and send the mini-dump to the discord channel
 "#demo-mini-dumps" together with the version of the ttauri release.

To make crash mini-dump when the ttauri demo application crashes create the following registry key

`Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps`

 Name         | Type                    | Value
 ------------ | ----------------------- | ------------
 DumpFolder   | Expandable String Value | "%LOCALAPPDATA%\CrashDumps"
 DumpCount    | DWORD (32-bit) Value    | 10
 DumpType     | DWORD (32-bit) Value    | 2

