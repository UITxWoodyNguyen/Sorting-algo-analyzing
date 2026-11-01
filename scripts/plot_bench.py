#!/usr/bin/env python3
"""Generate charts from benchmark files 01.inp through 19.inp only."""

import argparse
import csv
import re
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


def algorithm_slug(algorithm):
    return re.sub(r"[^a-z0-9]+", "_", algorithm.lower()).strip("_")


def write_algorithm_table(rows, output_dir, algorithm):
    algorithm_rows = sorted(
        (row for row in rows if row["algorithm"] == algorithm),
        key=lambda row: row["n"],
    )
    prefix = algorithm_slug(algorithm)
    csv_path = output_dir / f"{prefix}_runtime.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(["file", "n", "runtime_ms"])
        for row in algorithm_rows:
            writer.writerow([row["file"], row["n"], row["runtime_ms"]])

    markdown_path = output_dir / f"{prefix}_runtime.md"
    with markdown_path.open("w", encoding="utf-8") as stream:
        stream.write("| File | Payload size (n) | Runtime (ms) |\n")
        stream.write("|---|---:|---:|\n")
        for row in algorithm_rows:
            stream.write(
                f"| {row['file']} | {row['n']} | {row['runtime_ms']:.6f} |\n"
            )
    return csv_path, markdown_path


def plot_algorithm_details(rows, output_dir, algorithm):
    plt = matplotlib()
    algorithm_rows = sorted(
        (row for row in rows if row["algorithm"] == algorithm),
        key=lambda row: row["n"],
    )
    if not algorithm_rows:
        return []

    algorithm_dir = output_dir / "algorithms" / algorithm_slug(algorithm)
    algorithm_dir.mkdir(parents=True, exist_ok=True)
    sizes = [row["n"] for row in algorithm_rows]
    runtimes = [row["runtime_ms"] for row in algorithm_rows]
    charts = []

    figure, axis = plt.subplots(figsize=(10, 6))
    axis.plot(sizes, runtimes, marker="o", linewidth=1.8, color="#2563eb")
    axis.set_title(f"{algorithm} sort - runtime by payload size")
    axis.set_xlabel("Payload size (n)")
    axis.set_ylabel("Average runtime (ms)")
    axis.set_xscale("log")
    axis.set_yscale("log")
    axis.grid(True, which="both", linestyle=":", alpha=0.5)
    figure.tight_layout()
    path = algorithm_dir / "runtime_by_size.png"
    figure.savefig(path, dpi=160)
    plt.close(figure)
    charts.append(path)

    selected = [row for row in algorithm_rows if row["n"] in SELECTED_SIZES]
    if selected:
        figure, axis = plt.subplots(figsize=(9, 6))
        labels = [f"1e{len(str(row['n'])) - 1}" for row in selected]
        axis.bar(labels, [row["runtime_ms"] for row in selected], color="#16a34a")
        axis.set_title(f"{algorithm} sort - selected payload sizes")
        axis.set_xlabel("Payload size (n)")
        axis.set_ylabel("Average runtime (ms)")
        axis.set_yscale("log")
        axis.grid(True, axis="y", which="both", linestyle=":", alpha=0.5)
        figure.tight_layout()
        path = algorithm_dir / "runtime_selected_sizes.png"
        figure.savefig(path, dpi=160)
        plt.close(figure)
        charts.append(path)

    baseline = runtimes[0]
    if baseline > 0:
        relative = [runtime / baseline for runtime in runtimes]
        figure, axis = plt.subplots(figsize=(10, 6))
        axis.plot(sizes, relative, marker="o", linewidth=1.8, color="#dc2626")
        axis.set_title(f"{algorithm} sort - relative runtime growth")
        axis.set_xlabel("Payload size (n)")
        axis.set_ylabel("Runtime / first measured runtime")
        axis.set_xscale("log")
        axis.set_yscale("log")
        axis.grid(True, which="both", linestyle=":", alpha=0.5)
        figure.tight_layout()
        path = algorithm_dir / "relative_runtime_growth.png"
        figure.savefig(path, dpi=160)
        plt.close(figure)
        charts.append(path)

    table_csv, table_md = write_algorithm_table(algorithm_rows, algorithm_dir, algorithm)
    charts.extend([table_csv, table_md])
    return charts


def plot_algorithm_details_for_all(rows, output_dir):
    _, algorithms = group_rows(rows)
    generated = {}
    for algorithm in algorithms:
        generated[algorithm] = plot_algorithm_details(rows, output_dir, algorithm)
    return generated


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
    algorithm_outputs = plot_algorithm_details_for_all(rows, args.output_dir)
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
    print("Per-algorithm outputs:")
    for algorithm, paths in algorithm_outputs.items():
        print(f"  {algorithm}: {len(paths) - 2} charts, 2 tables")


if __name__ == "__main__":
    main()
