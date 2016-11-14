#!/usr/bin/python

import subprocess
import os
import sys
import shutil
import platform
import argparse

def is_windows():
    return platform.system() == "Windows"

def is_linux():
    return platform.system() == "Linux"

def check_call(args):
    print args
    subprocess.check_call(args)

def improve():
    # do not use check_call
    # geomagic returns not-zero exit status
    if is_windows():
        subprocess.call([geomagic_path, "geomagic.py", "-ExitOnMacroEnd"])
    else:
        print "Copying files..."
        shutil.copyfile("geomagic-tmp-input.ply", "geomagic-tmp-output.ply")
        print "Done..."


def decimate(infile, outfile):
    check_call(
            [meshlab_path, "-i", infile, "-o", outfile, "-s", "decimate.mlx"])

def decimate_and_improve_in_geomagic():
    subprocess.call([geomagic_path, "geomagic-decimate-doctor.py", "-ExitOnMacroEnd"])

def uv(infile, outfile):
    if is_windows():
        infile_abs = os.path.abspath(infile)
        outfile_abs = os.path.abspath(outfile)
        current_directory = os.getcwd()
        head, tail = os.path.split(make_uv_path)
        os.chdir(head)
        check_call([tail, infile_abs, outfile_abs])
        os.chdir(current_directory)
    else:
        check_call([make_uv_path, infile, outfile])


def transfer_color_to_texture(file_color, file_uv, outfile):
    check_call(
            [meshlab_path, "-i", file_uv, file_color, "-o", outfile,
             "-om", "wt", "-s", "transfer-color-to-texture.mlx"])

def convert(infile, outfile):
    if is_windows():
        in_dir, in_short_file = os.path.split(infile)
        out_dir, out_short_file = os.path.split(outfile)
        if in_dir != out_dir:
            raise Exception("Output and input dirs should be the same")
        current_directory = os.getcwd()
        os.chdir(in_dir)
        check_call([meshlab_path, "-i", in_short_file, "-o", out_short_file, "-om", "wt"])
        os.chdir(current_directory)
    else:
        check_call([meshlab_path, "-i", infile, "-o", outfile, "-om", "wt"])

def make_textured_mesh(infile, outfile):
    geomagic_input_file="geomagic-tmp-input.ply"
    geomagic_output_file="geomagic-tmp-output.ply"

    if is_windows():
        shutil.copyfile(infile, geomagic_input_file)
        decimate_and_improve_in_geomagic()
    else:
        decimate(infile, geomagic_input_file)
        improve()

    root, extension = os.path.splitext(outfile)
    file_uv = root + "-uv.obj"
    file_texture_ply = root + ".ply"

    uv(geomagic_output_file, file_uv)
    transfer_color_to_texture(infile, file_uv, file_texture_ply)

    if extension != ".ply":
        convert(file_texture_ply, outfile)

    os.remove(geomagic_input_file)
    os.remove(geomagic_output_file)
    os.remove(file_uv)

def make_directory(path):
    if not os.path.exists(path):
        os.makedirs(path)

def texture_mesh_batch(input_dir, output_dir):
    for infile_short in os.listdir(input_dir):
        dirname = os.path.join(output_dir, infile_short)
        make_directory(dirname)
        root_file_name, extension = os.path.splitext(infile_short)

        outfile = os.path.join(dirname, root_file_name + ".obj")
        infile = os.path.join(input_dir, infile_short)

        make_textured_mesh(infile, outfile)

def configure():
    global geomagic_path
    global meshlab_path
    global make_uv_path
    system_name = platform.system()
    if is_linux():
        geomagic_path="C:\Program Files\Geomagic\Foundation 2013\StudioCORE.exe"
        meshlab_path="meshlabserver"
        make_uv_path="/home/alex/src/GraphiteTwo/build/Linux-Release/binaries/bin/make-uv"
    elif is_windows():
        geomagic_path=r"C:\Program Files\Geomagic\Foundation 2013\StudioCORE.exe"
        meshlab_path=r"C:\Program Files\VCG\MeshLab\meshlabserver.exe"
        make_uv_path=r"D:\Downloads\Software\GraphiteTwo\build\binaries\bin\Release\make-uv.exe"
        print make_uv_path
    else:
        raise Exception("Unsupported platform " + system_name)

def main():
    configure()
    parser = argparse.ArgumentParser()
    parser.add_argument('--batch', action='store_true')
    parser.add_argument('pathes', nargs=2)
    args = parser.parse_args()
    if args.batch:
        print "batch mode"
        texture_mesh_batch(args.pathes[0], args.pathes[1])
    else:
        print "single mode"
        make_textured_mesh(args.pathes[0], args.pathes[1])

    return


if __name__ == "__main__":
    main()
