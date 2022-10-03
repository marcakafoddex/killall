# Killall
POSIX inspired killall utility for Windows.
Searches for processes ignoring their extension, so if e.g. `cl` is specified, any process whose main module is called `cl.exe`, `cl.com` or `cl.dll` is killed.

# Building instructions
- Requires Visual Studio 2022 (can probably easily be backported to an older version)
- Just open the .sln file and hit Build in visual studio

# Usage
- To kill all `cl.exe` processes, just write `killall cl`
- To list all `cl.exe` processes rather than kill them, write `killall -l cl`
- To see all available options, write `killall -h`

# License
Do with this source code whatever you want, but a mention of this repo would be niced if you copy/fork it.
