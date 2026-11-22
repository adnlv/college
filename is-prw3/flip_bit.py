#!/usr/bin/env python3
"""Flip a single bit (default: LSB of the last byte) in a binary file."""

import argparse
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Flip a bit in a binary file to demonstrate lack of integrity protection."
    )
    parser.add_argument("input_file", type=Path, help="Path to source binary file")
    parser.add_argument("output_file", type=Path, help="Path to modified binary file")
    parser.add_argument(
        "--byte-index",
        type=int,
        default=-1,
        help="Target byte offset (supports negative Python indexing; default: -1)",
    )
    parser.add_argument(
        "--mask",
        type=lambda value: int(value, 0),
        default=0x01,
        help="XOR bitmask to apply (default: 0x01)",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    data = bytearray(args.input_file.read_bytes())
    if not data:
        raise ValueError(f"Input file '{args.input_file}' is empty.")

    target_index = args.byte_index if args.byte_index >= 0 else len(data) + args.byte_index
    if not 0 <= target_index < len(data):
        raise IndexError(
            f"Byte index {args.byte_index} out of range for {len(data)}-byte file."
        )

    before = data[target_index]
    after = before ^ args.mask
    data[target_index] = after
    args.output_file.write_bytes(data)

    print(
        f"Flipped byte[{target_index}] (of {len(data)} bytes): "
        f"0x{before:02x} ({before:08b}) -> 0x{after:02x} ({after:08b})"
    )


if __name__ == "__main__":
    main()
