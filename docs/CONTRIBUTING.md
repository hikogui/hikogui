Contributing to the TTauri project
==================================

Code of Conduct
---------------
This project and everyone participating in it is coverned by the
(TTauri Code of Conduct)[https://github.com/ttauri-project/ttauri/blob/main/docs/CODE_OF_CONDUCT.md]

Project management
------------------
The management of the ttauri-project is done through at <https://github.com/ttauri-project/>

When you want to suggest a new feature, an improvement or file a bug
report you can do so through the "issues".

If you want to work on an issue you probably want to assign yourself to the issue so
that we don't accidently have two people working on the same thing. If you have
questions about the issue you can add additional comments to the issue and
discuss it on discord (see below).


Communication channels
----------------------
For communication we use the following Discord server: <https://discord.gg/7e8pFTsujw>

Some of the channel that we use:
 - #development: Here we can talk about developing for ttauri.
 - #ideas-and-suggestion: Here we can talk about the proposal of new
   ideas and suggestions before we turn them into issues on github.

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

