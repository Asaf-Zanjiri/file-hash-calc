# File hash calculator
This is my first ever program written in C (besides a simple hello program 😅).
I decided to make this program when I was downloading a file from the internet and the site showed what the supposed hash of the downloaded file should be. So I started looking into how I can easily calculate the hash of the file I downloaded. Unfortunately I haven't found a simple solution that doesnt require CMD/Powershell. Hence I decided to make an easy to use software myself.

**Important Note:**

This application was only tested on a Windows 10 (19043). It may not work on other versions.


**Features**

- Customizable installation paths via the code macros.
- Calculates MD5 & SHA256 Hashes.
- Ability to install itself into %ProgramFiles%/HashCalc.
- Ability to calculate a file's hash via Right-Clicking and choosing "Calculate file hash".
- Ability to uninstall the program and its registry keys via windows "Add or remove programs" option.



**Installation/Usage Guidelines:**

1. Compile the code.
2. Run `[filename].exe --install` in the command line as administrator to install the program. Note: [filename] is the name of the compiled executable of the code.
3. Right click a file and choose "Calculate file hash" - a window message should show up with the file's MD5 and SHA256 hashes.
Note: Sometimes there's an error during installation, I'm still yet to solve it, but it should work after multiple install attempts.



**Credits:**

To calculate the hashes from the file's data I used @kazuho 's picohash header.
You can take a look at his project at https://github.com/kazuho/picohash

