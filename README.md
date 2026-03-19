# Discord CDN Dropper
 
A two-component proof-of-concept payload delivery tool that abuses Discord's
CDN as a staging server for file reassembly and execution. Written in C++.
 
## How It Works
 
Traditional payload delivery often involves dropping a single suspicious
executable to disk or hosting it on an attacker-controlled server — both of
which are well-monitored vectors. This PoC demonstrates an alternative
approach: splitting a binary into multiple innocuous-looking chunks and staging
them on a legitimate CDN (Discord's), where each individual fragment carries
no recognizable signature.
 
The toolchain has three components:
 
### 1. Chunker (`chunker/`)
 
Takes an input binary and splits it into 11 equal-sized fraction files. Each
fraction is prefixed with a 32-byte ASCII ordinal header (`<N>`, space-padded
to 32 bytes) that identifies its reassembly position. These fraction files are
then uploaded manually to Discord, and the resulting CDN URLs are hardcoded
into the dropper source.
 
**Usage:**
```
chunker.exe <input_file> <output_directory>
```
 
**Example:**
```
chunker.exe payload.exe C:\Fractions
```
 
This produces `Fraction0` through `Fraction10` in the output directory.
 
---
 
### 2. Dropper (`dropper/`)
 
CDN URLs are hardcoded directly in the dropper source as a `WCHAR` array.
For each URL the dropper:
 
1. Downloads the fraction to a randomly-named `.br` file in `%LOCALAPPDATA%`
2. After all fractions are downloaded, allocates a heap buffer sized to the
   total download
3. Iterates over the downloaded fractions, skipping the 32-byte header in each
   and copying the raw payload data sequentially into the buffer
4. Writes the fully reassembled binary to a randomly-named `.exe` in
   `%LOCALAPPDATA%`
5. Spawns the reassembled binary via `CreateProcessW` and waits for it to
   finish
6. Fraction files are automatically cleaned up via `FILE_FLAG_DELETE_ON_CLOSE`
 
> **Note:** The CDN links hardcoded in this repository are intentionally
> expired. Reproducing this technique requires uploading your own fraction
> files to Discord and updating the source with the new URLs before
> recompiling. This is by design — the goal is to demonstrate the technique,
> not to provide a ready-to-use delivery tool.
 
---
 
### 3. Dummy Payload (`payload/`)
 
A benign MessageBox executable included to make the full delivery pipeline
immediately reproducible without needing to supply your own binary. Compile
`payload.cpp`, run it through the chunker, upload the resulting fractions to
Discord, and update the hardcoded URL array in the dropper source to verify
the entire chain end to end.
 
```cpp
MessageBoxW(NULL, L"Delivered by DiscordCDN", L"Oooo Scary", MB_OK);
```
 
---
 
## Why Discord CDN?
 
Discord CDN links (`cdn.discordapp.com`) originate from a widely trusted,
high-reputation domain used by millions of legitimate applications. Network
monitoring tools and web proxies rarely block or flag traffic to Discord's CDN,
making it an effective staging location for this technique.
 
**Note:** Discord has been progressively expiring CDN attachment links — links
now carry expiry timestamps in their query parameters and die after a few days.
Reproducing this PoC requires generating fresh links by uploading fraction
files to a Discord channel you control.
 
This technique has been documented in the wild by threat intelligence vendors
including CrowdStrike and Trellix and is a known CDN abuse vector. This
implementation is intended to demonstrate the technique for defensive research
purposes.
 
---
 
## Building
 
All three components are standard Win32 C++ projects with no external
dependencies. Build with Visual Studio (any recent version) or MSVC from the
command line:
 
```
cl /EHsc /W3 chunker\chunker.cpp /link /SUBSYSTEM:WINDOWS /OUT:chunker.exe
cl /EHsc /W3 dropper\dropper.cpp /link /SUBSYSTEM:WINDOWS urlmon.lib /OUT:dropper.exe
cl /EHsc /W3 payload\payload.cpp /link /SUBSYSTEM:CONSOLE /OUT:payload.exe
```
 
---
 
## Disclaimer
 
This tool was developed for authorized security research and educational
purposes only. The author is not responsible for any misuse or damage caused
by this software. Only use against systems you own or have explicit written
permission to test. By using this software, you agree to take full
responsibility for your actions.
