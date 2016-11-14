#!/usr/bin/python

import os
import subprocess
import sys

def parse_filename(filename):
    if '_' in filename:
        number, sep, after = filename.partition("_")
        return number
    else:
        number, sep, after = filename.partition(".")
        return number

def main():
    dirname = os.path.normpath(sys.argv[1])
    sourcedir = dirname + "-obfuscated"
    outdir = dirname + "-archived"
    files = os.listdir(sourcedir)
    batch_files = {}

    for filename in files:
        number = parse_filename(filename)

        if number not in batch_files:
            batch_files[number] = []

        batch_files[number].append(filename)

    for first, input_files in batch_files.iteritems():
        archive_name = os.path.join(outdir, first + ".7z")

        # the point in the source filenames precludes 7z from including the
        # directory name into the archive

        full_input_files = map(lambda filename: os.path.join(".", sourcedir, filename),
                               input_files)
        cmd = ["7z", "a", "-t7z", "-m0=lzma2", "-mx=9", archive_name] + full_input_files
        print cmd
        subprocess.check_call(cmd);

if __name__ == "__main__":
    main()
