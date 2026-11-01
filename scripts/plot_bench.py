#!/usr/bin/env python3
"""Generate charts from benchmark files 01.inp through 19.inp only."""

import argparse
import csv
from collections import defaultdict
from pathlib import Path

SELECTED_SIZES = (10**3, 10**4, 10**5, 10**6)
ALGORITHM_ORDER = ("Insertion", "Selection", "Heap", "Merge", "Quick")
FIRST_DATASET_FILE = 1
LAST_DATASET_FILE = 19


def is_first_dataset_file(file_name):
    """Return whether file_name is one of 01.inp through 19.inp."""
    path = Path(file_name)
    if path.suffix.lower() != ".inp" or not path.stem.isdigit():
        return False
    number = int(path.stem)
    return FIRST_DATASET_FILE <= number <= LAST_DATASET_FILE


def load_rows(path):
    rows = []
    with path.open(newline="", encoding="utf-8-sig") as stream:
        for row in csv.DictReader(stream):
            if not is_first_dataset_file(row.get("file", "")):
                continue
            if row.get("status") != "ok" or not row.get("avg_ms"):
                continue
            rows.append(
                {
                    "file": row["file"],
                    "n": int(row["n"]),
                    "algorithm": row["algorithm"],
                    "runtime_ms": float(row["avg_ms"]),
                }
            )
    if not rows:
        raise ValueError(
            f"No successful benchmark rows found for 01.inp through 19.inp in {path}"
        )
    return sorted(rows, key=lambda row: (row["n"], row["algorithm"]))


def group_rows(rows):
    grouped = defaultdict(dict)
    for row in rows:
        grouped[row["n"]][row["algorithm"]] = row["runtime_ms"]
    algorithms = [
        algorithm
        for algorithm in ALGORITHM_ORDER
        if any(row["algorithm"] == algorithm for row in rows)
    ]
    algorithms.extend(
        sorted(
            {
                row["algorithm"]
                for row in rows
                if row["algorithm"] not in algorithms
            }
        )
    )
    return grouped, algorithms


def write_tables(rows, output_dir, prefix="runtime_table"):
    by_size, algorithms = group_rows(rows)
    table_path = output_dir / f"{prefix}.csv"
    with table_path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(["n", *algorithms])
        for n in sorted(by_size):
            writer.writerow(
                [n, *[by_size[n].get(algorithm, "") for algorithm in algorithms]]
            )

    markdown_path = output_dir / f"{prefix}.md"
    with markdown_path.open("w", encoding="utf-8") as stream:
        stream.write("| Payload size (n) | " + " | ".join(algorithms) + " |\n")
        stream.write("|---:|" + "|".join(["---:"] * len(algorithms)) + "|\n")
        for n in sorted(by_size):
            values = [by_size[n].get(algorithm, "") for algorithm in algorithms]
            stream.write(
                f"| {n} | "
                + " | ".join(f"{value:.6f}" if value != "" else "" for value in values)
                + " |\n"
            )
    return table_path, markdown_path


def matplotlib():
    try:
        import matplotlib.pyplot as plt
    except ImportError as error:
        raise RuntimeError(
            "matplotlib is required; install it with: python -m pip install matplotlib"
        ) from error
    return plt


def plot_all_sizes(rows, output_dir):
    plt = matplotlib()
    grouped, algorithms = group_rows(rows)
    figure, axis = plt.subplots(figsize=(11, 6))
    for algorithm in algorithms:
        points = sorted(
            (n, values[algorithm])
            for n, values in grouped.items()
            if algorithm in values
        )
        axis.plot(
            [point[0] for point in points],
            [point[1] for point in points],
            marker="o",
            linewidth=1.5,
            label=algorithm,
        )
    axis.set_title("Sorting algorithm runtime")
    axis.set_xlabel("Payload size (n)")
    axis.set_ylabel("Average runtime (ms)")
    axis.set_xscale("log")
    axis.set_yscale("log")
    axis.grid(True, which="both", linestyle=":", alpha=0.5)
    axis.legend()
    figure.tight_layout()
    chart_path = output_dir / "runtime_comparison.png"
    figure.savefig(chart_path, dpi=160)
    plt.close(figure)
    return chart_path


def plot_selected_sizes(rows, output_dir):
    plt = matplotlib()
    selected = [row for row in rows if row["n"] in SELECTED_SIZES]
    if not selected:
        return None, []

    grouped, algorithms = group_rows(selected)
    figure, axis = plt.subplots(figsize=(11, 6))
    positions = list(range(len(SELECTED_SIZES)))
    width = 0.8 / max(len(algorithms), 1)
    for index, algorithm in enumerate(algorithms):
        values = [grouped[n].get(algorithm, 0) for n in SELECTED_SIZES]
        offset = (index - (len(algorithms) - 1) / 2) * width
        axis.bar(
            [position + offset for position in positions],
            values,
            width=width,
            label=algorithm,
        )
    axis.set_title("Runtime comparison at selected payload sizes")
    axis.set_xlabel("Payload size (n)")
    axis.set_ylabel("Average runtime (ms)")
    axis.set_xticks(positions, [f"1e{len(str(size)) - 1}" for size in SELECTED_SIZES])
    axis.set_yscale("log")
    axis.grid(True, axis="y", which="both", linestyle=":", alpha=0.5)
    axis.legend()
    figure.tight_layout()
    chart_path = output_dir / "runtime_comparison_selected_sizes.png"
    figure.savefig(chart_path, dpi=160)
    plt.close(figure)
    return chart_path, sorted(selected, key=lambda row: (row["n"], row["algorithm"]))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=Path("Output/benchmark.csv"))
    parser.add_argument("--output-dir", type=Path, default=Path("Output"))
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    rows = load_rows(args.input)
    table_csv, table_md = write_tables(rows, args.output_dir)
    chart = plot_all_sizes(rows, args.output_dir)
    selected_chart, selected_rows = plot_selected_sizes(rows, args.output_dir)
    print(f"Chart: {chart}")
    print(f"CSV table: {table_csv}")
    print(f"Markdown table: {table_md}")
    if selected_chart is None:
        print("Selected-size chart: skipped (no 1e3/1e4/1e5/1e6 rows found)")
    else:
        selected_csv, selected_md = write_tables(
            selected_rows, args.output_dir, "runtime_selected_sizes"
        )
        print(f"Selected-size chart: {selected_chart}")
        print(f"Selected-size CSV table: {selected_csv}")
        print(f"Selected-size Markdown table: {selected_md}")


if __name__ == "__main__":
    main()
