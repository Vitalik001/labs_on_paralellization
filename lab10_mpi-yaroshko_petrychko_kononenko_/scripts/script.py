import argparse
import subprocess
import os

def compile_mpi_program(compile_script_path):
    subprocess.run(f"chmod +x {compile_script_path}", shell=True)
    devnull = open(os.devnull, 'w')
    subprocess.run(f"./{compile_script_path}", shell=True, stdout=devnull, stderr=subprocess.STDOUT)
    devnull.close()

def run_mpi_program(program_path, num_processes):
    mpi_command = f"mpirun -np {num_processes} {program_path}"
    subprocess.run(mpi_command, shell=True)

# Parse command-line arguments
parser = argparse.ArgumentParser(description="MPI Program Runner")
parser.add_argument("num_processes", type=int, help="Number of processes")
args = parser.parse_args()

num_processes = args.num_processes

if __name__ == "__main__":
    compile_script_path = "compile.sh"
    program_path = "./bin/template"
    compile_mpi_program(compile_script_path)
    run_mpi_program(program_path, num_processes)
    subprocess.run(f"python3 scripts/create_animation.py", shell=True)
