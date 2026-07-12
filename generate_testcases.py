#!/usr/bin/env python3
"""Generate random Belady caching testcase files.

File format:
    cache_size request_count
    r1 r2 r3 ...

By default this creates testcases for cache sizes 3, 4, and 5 with request
counts from 1000 to 100000. Page numbers are generated from 0 to 9.
"""

from __future__ import annotations

import argparse
import random
from pathlib import Path


DEFAULT_CACHE_SIZES = [3, 5, 10, 50, 100]
DEFAULT_REQUEST_COUNTS = [50, 100, 1_000, 5_000, 10_000, 50_000, 100_000]


def write_testcase(
    output_dir: Path,
    cache_size: int,
    request_count: int,
    min_page: int,
    max_page: int,
    rng: random.Random,
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    path = output_dir / f"cache{cache_size}_{request_count}.txt"

    requests = [str(rng.randint(min_page, max_page)) for _ in range(request_count)]

    with path.open("w", encoding="utf-8") as file:
        file.write(f"{cache_size} {request_count}\n")
        file.write(" ".join(requests))
        file.write("\n")

    print(f"Generated {path}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate random .txt testcases for Belady optimal caching."
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("testcases"),
        help="Directory where testcase files will be written.",
    )
    parser.add_argument(
        "--cache-sizes",
        type=int,
        nargs="+",
        default=DEFAULT_CACHE_SIZES,
        help="Cache sizes to generate.",
    )
    parser.add_argument(
        "--request-counts",
        type=int,
        nargs="+",
        default=DEFAULT_REQUEST_COUNTS,
        help="Request counts to generate.",
    )
    parser.add_argument(
        "--min-page",
        type=int,
        default=0,
        help="Smallest page number.",
    )
    parser.add_argument(
        "--max-page",
        type=int,
        default=1000,
        help="Largest page number.",
    )
    parser.add_argument(
        "--seed",
        type=int,
        default=42,
        help="Random seed for reproducible output.",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()

    if args.min_page > args.max_page:
        raise ValueError("--min-page cannot be greater than --max-page")
    if any(cache_size <= 0 for cache_size in args.cache_sizes):
        raise ValueError("cache sizes must be positive")
    if any(request_count <= 0 for request_count in args.request_counts):
        raise ValueError("request counts must be positive")

    rng = random.Random(args.seed)

    for cache_size in args.cache_sizes:
        for request_count in args.request_counts:
            write_testcase(
                output_dir=args.output_dir,
                cache_size=cache_size,
                request_count=request_count,
                min_page=args.min_page,
                max_page=args.max_page,
                rng=rng,
            )


if __name__ == "__main__":
    main()
