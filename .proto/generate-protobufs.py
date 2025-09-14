import argparse
import os
import subprocess
from typing import List

try:
    from termcolor import colored
except ModuleNotFoundError:

    def colored(text, color=None):
        return text

DEBUG_PRINT = True


class InputValues:
    PROTOC_FILE = []
    PROTO_DIRECTORIES = []
    STUB_DIRECTORIES = []
    BUILD_DIRECTORIES = []

    @staticmethod
    def set_build_directories(new_val: str):
        InputValues.BUILD_DIRECTORIES = new_val

    @staticmethod
    def get_build_directories() -> str:
        return InputValues.BUILD_DIRECTORIES

    @staticmethod
    def set_protoc(new_val: str):
        InputValues.PROTOC_FILE = new_val

    @staticmethod
    def get_protoc() -> str:
        return InputValues.PROTOC_FILE

    @staticmethod
    def get_proto_directories() -> str:
        return InputValues.PROTO_DIRECTORIES

    @staticmethod
    def set_proto_directories(new_val: str):
        InputValues.PROTO_DIRECTORIES = new_val

    @staticmethod
    def get_stub_directories() -> str:
        return InputValues.STUB_DIRECTORIES

    @staticmethod
    def set_stub_directories(new_val: str):
        InputValues.STUB_DIRECTORIES = new_val


def info_print(*args, **kwargs):
    if DEBUG_PRINT:
        print("[Python]", *args, **kwargs)


def collect_files_with_extension(directory: str, extension: str) -> List[str]:
    check_directory_existence_and_accessibility(directory)
    proto_files = []
    for current_dir, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(os.path.extsep + extension):
                proto_files.append(os.path.join(current_dir, file))
    return proto_files


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--protoc_path",
        type=str,
        default=[],
        help="",
    )
    parser.add_argument(
        "--proto_path",
        type=str,
        default=[],
        help="",
    )
    parser.add_argument(
        "--stub_path",
        type=str,
        default=[],
        help="",
        required=False,
    )
    parser.add_argument(
        "--build_path",
        type=str,
        default=[],
        help="",
        required=False,
    )
    return parser.parse_args()


def check_directory_existence_and_accessibility(directory: str):
    "Raises RuntimeError if directory not found or not accessible"
    if not os.path.exists(directory):
        raise RuntimeError(f"Directory not found: {directory}")
    if not os.path.isdir(directory):
        raise RuntimeError(f"Directory is not a directory: {directory}")
    if not os.access(directory, os.R_OK):
        raise RuntimeError(f"Directory is not readable: {directory}")


def create_folders_if_not_exists(folder: str):
    if not os.path.exists(folder):
        os.makedirs(folder)
        info_print(f"Created folder: {folder}")


def generate_proto_files_with_protoc_compiler(
        proto_file: str,
        protofile_directory_abs: str,
):
    protoc_result = subprocess.run(
        f"{InputValues.get_protoc()} "
        f"--proto_path {InputValues.get_proto_directories()} "
        f"--cpp_out={InputValues.get_stub_directories()} "
        f"{os.path.join(protofile_directory_abs, proto_file)} "
        f"--experimental_allow_proto3_optional ",
        shell=True,
        capture_output=True,
    )

    if protoc_result.returncode != 0:
        raise RuntimeError(
            f"Failed to generate proto file: {proto_file}. Error code: {protoc_result.returncode}. \nError: {protoc_result.stderr.decode('utf-8')} "
            f"\nCommand: {InputValues.get_protoc()} "
            f"--proto_path {InputValues.get_proto_directories()} "
            f"--cpp_out={InputValues.get_stub_directories()} "
            f"{os.path.join(protofile_directory_abs, proto_file)} "
            f"--experimental_allow_proto3_optional "
        )
    else:
        info_print(f"Generate proto file: from \"{proto_file}\" finished.")


def generate_proto_files(current_protofile_directory_abs: str, protofile: str):
    generate_proto_files_with_protoc_compiler(protofile, current_protofile_directory_abs, )


def main():
    info_print(colored("Generating proto files started", "green"))
    args = parse_args()

    os.environ["LD_LIBRARY_PATH"] = args.build_path + os.path.sep + "protoc"

    InputValues.set_proto_directories(args.proto_path)
    InputValues.set_stub_directories(args.stub_path)
    InputValues.set_build_directories(args.build_path)
    InputValues.set_protoc(args.protoc_path)

    create_folders_if_not_exists(InputValues.get_stub_directories())

    try:
        proto_files = collect_files_with_extension(InputValues.get_proto_directories(), "proto")
    except RuntimeError:
        info_print(
            colored(
                f"[ERROR] Failed to collect proto files from directory {InputValues.get_proto_directories()}", "red"
            )
        )
        raise

    info_print(f"Generate in folder: {InputValues.get_stub_directories()}.")
    try:
        check_directory_existence_and_accessibility(InputValues.get_stub_directories())

        for proto_file in proto_files:
            current_protofile_directory_abs = os.path.dirname(proto_file)
            generate_proto_files(
                current_protofile_directory_abs, os.path.split(proto_file)[1]
            )

    except RuntimeError as e:
        info_print(colored(f"Failed to generate proto files", "red"))
        raise
    info_print(colored("Generating proto files finished", "green"))


if __name__ == "__main__":
    main()
