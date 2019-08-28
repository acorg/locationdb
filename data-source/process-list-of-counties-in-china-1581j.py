#! /usr/bin/env python3
import sys, os, csv, subprocess

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} list_of_counties_in_china-1581j.csv", file=sys.stderr)
    exit(1)

def generate(short_name, full_name):
    # print (short_name, full_name)
    status = subprocess.run(["locdb", full_name], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if status.returncode != 0 and b"NOT FOUND:" in status.stdout:
        status = subprocess.run(["locdb", short_name], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if status.returncode != 0:
        print(status.stdout.decode("utf-8"))
    # else:
    #     print(status.stdout.decode("utf-8"))
    return status.returncode

for row in csv.DictReader(open(sys.argv[1])):
    generate(short_name=f"""{row["province"]}-{row["county"]}""".upper(), full_name=f"""{row["province"]} {row["prefecture"]} {row["county"]}""".upper())
