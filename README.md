Have you ever got stressed by a LaTex project with a lot of unmet dependencies? Discovering and installing them one-by-one is a long and tedious process.
But now I have a solution for you. Compile and run this executable `./builddir/latex-depcheck <file.tex>` to instantly find which dependencies you need at once.

It scans for directories `/usr/share/texlive` and `.` in order to find and index all `*.sty` files. Just as LaTex would do, but better.
