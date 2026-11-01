#!/usr/bin/env python3
"""PyQt6 interface for sorting, benchmarking, and known-answer tests."""

import os
import subprocess
import sys
from pathlib import Path

from PyQt6.QtCore import QProcess
from PyQt6.QtWidgets import (
    QApplication,
    QCheckBox,
    QComboBox,
    QFileDialog,
    QFormLayout,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMainWindow,
    QMessageBox,
    QPlainTextEdit,
    QPushButton,
    QSpinBox,
    QTabWidget,
    QVBoxLayout,
    QWidget,
)

ROOT = Path(__file__).resolve().parents[1]
BUILD_DIR = ROOT / "build"
OUTPUT_DIR = ROOT / "Output"
ALGORITHMS = [
    ("Insertion sort", "insertion", "1"),
    ("Selection sort", "selection", "2"),
    ("Heap sort", "heap", "3"),
    ("Merge sort", "merge", "4"),
    ("Quick sort", "quick", "5"),
]


class SortAppWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Sorting Algorithm Analyzer")
        self.resize(1100, 780)
        self.process = QProcess(self)
        self.process.readyReadStandardOutput.connect(self.read_output)
        self.process.readyReadStandardError.connect(self.read_output)
        self.process.started.connect(self.write_stdin)
        self.process.finished.connect(self.process_finished)
        self.pending_stdin = b""
        self.running_label = ""
        self.build_ui()

    def build_ui(self):
        central = QWidget()
        layout = QVBoxLayout(central)

        header = QHBoxLayout()
        title = QLabel("<h2>Sorting Algorithm Analyzer</h2>")
        header.addWidget(title)
        header.addStretch()
        self.status = QLabel("Ready")
        header.addWidget(self.status)
        layout.addLayout(header)

        self.tabs = QTabWidget()
        self.tabs.addTab(self.create_sort_tab(), "Sort")
        self.tabs.addTab(self.create_benchmark_tab(), "Benchmark")
        self.tabs.addTab(self.create_kat_tab(), "Known-answer test")
        layout.addWidget(self.tabs, 1)

        layout.addWidget(QLabel("Process log"))
        self.log = QPlainTextEdit()
        self.log.setReadOnly(True)
        self.log.setMaximumBlockCount(5000)
        layout.addWidget(self.log, 1)

        toolbar = QHBoxLayout()
        self.build_button = QPushButton("Build")
        self.stop_button = QPushButton("Stop")
        clear_button = QPushButton("Clear log")
        open_output_button = QPushButton("Open Output")
        self.build_button.clicked.connect(self.build)
        self.stop_button.clicked.connect(self.stop_process)
        clear_button.clicked.connect(self.log.clear)
        open_output_button.clicked.connect(self.open_output)
        toolbar.addWidget(self.build_button)
        toolbar.addWidget(self.stop_button)
        toolbar.addWidget(clear_button)
        toolbar.addWidget(open_output_button)
        toolbar.addStretch()
        layout.addLayout(toolbar)

        self.setCentralWidget(central)
        self.stop_button.setEnabled(False)

    def create_sort_tab(self):
        tab = QWidget()
        layout = QVBoxLayout(tab)

        algorithm_row = QHBoxLayout()
        algorithm_row.addWidget(QLabel("Algorithm:"))
        self.sort_algorithm = self.algorithm_combo(include_all=False)
        algorithm_row.addWidget(self.sort_algorithm)
        algorithm_row.addStretch()
        layout.addLayout(algorithm_row)

        stdin_group = QGroupBox("stdin / stdout")
        stdin_layout = QVBoxLayout(stdin_group)
        stdin_layout.addWidget(QLabel("Input format: n followed by n integers"))
        self.stdin_input = QPlainTextEdit()
        self.stdin_input.setPlaceholderText("Example:\n5\n4 1 5 2 3")
        stdin_layout.addWidget(self.stdin_input)
        stdin_actions = QHBoxLayout()
        run_stdin = QPushButton("Run stdin/stdout")
        clear_stdin = QPushButton("Clear input")
        run_stdin.clicked.connect(self.run_stdin)
        clear_stdin.clicked.connect(self.stdin_input.clear)
        stdin_actions.addWidget(run_stdin)
        stdin_actions.addWidget(clear_stdin)
        stdin_layout.addLayout(stdin_actions)
        layout.addWidget(stdin_group, 1)

        file_group = QGroupBox("File input / output")
        file_form = QFormLayout(file_group)
        self.input_path = QLineEdit()
        file_form.addRow("Input .inp:", self.path_row(self.input_path, self.choose_input))
        self.output_path = QLineEdit(str(OUTPUT_DIR / "sorted.out"))
        file_form.addRow("Output .out:", self.path_row(self.output_path, self.choose_output))
        run_file = QPushButton("Run file input/output")
        run_file.clicked.connect(self.run_file)
        file_form.addRow("", run_file)
        layout.addWidget(file_group)
        return tab

    def create_benchmark_tab(self):
        tab = QWidget()
        layout = QVBoxLayout(tab)
        form = QFormLayout()
        self.bench_input = QLineEdit("dataset/input")
        self.bench_expected = QLineEdit("dataset/output")
        self.bench_csv = QLineEdit("Output/benchmark.csv")
        form.addRow("Input directory:", self.path_row(self.bench_input, self.choose_bench_input))
        form.addRow("Expected output:", self.path_row(self.bench_expected, self.choose_bench_expected))
        form.addRow("CSV output:", self.path_row(self.bench_csv, self.choose_bench_csv))
        self.bench_repeat = QSpinBox()
        self.bench_repeat.setRange(1, 100)
        self.bench_repeat.setValue(3)
        self.bench_timeout = QSpinBox()
        self.bench_timeout.setRange(1, 3600)
        self.bench_timeout.setValue(180)
        form.addRow("Repeat:", self.bench_repeat)
        form.addRow("Timeout (seconds):", self.bench_timeout)
        self.skip_tle = QCheckBox("Skip sizes after TLE")
        self.skip_tle.setChecked(True)
        form.addRow("", self.skip_tle)
        layout.addLayout(form)

        actions = QHBoxLayout()
        run_benchmark = QPushButton("Run all benchmark")
        plot_benchmark = QPushButton("Generate charts and tables")
        run_benchmark.clicked.connect(self.run_benchmark)
        plot_benchmark.clicked.connect(self.generate_charts)
        actions.addWidget(run_benchmark)
        actions.addWidget(plot_benchmark)
        actions.addStretch()
        layout.addLayout(actions)
        layout.addStretch()
        return tab

    def create_kat_tab(self):
        tab = QWidget()
        layout = QVBoxLayout(tab)
        form = QFormLayout()
        self.kat_vectors = QLineEdit("test-vector/vectors.json")
        form.addRow("Vectors JSON:", self.path_row(self.kat_vectors, self.choose_vectors))
        self.kat_algorithm = self.algorithm_combo(include_all=True)
        form.addRow("Algorithm:", self.kat_algorithm)
        self.kat_category = QComboBox()
        self.kat_category.addItems(["All categories", "best", "worst", "normal", "edge"])
        form.addRow("Category:", self.kat_category)
        self.kat_timeout = QSpinBox()
        self.kat_timeout.setRange(1, 3600)
        self.kat_timeout.setValue(10)
        form.addRow("Timeout (seconds):", self.kat_timeout)
        self.kat_verbose = QCheckBox("Show passing tests")
        form.addRow("", self.kat_verbose)
        layout.addLayout(form)

        run_kat = QPushButton("Run known-answer tests")
        run_kat.clicked.connect(self.run_kat)
        layout.addWidget(run_kat)
        layout.addStretch()
        return tab

    def algorithm_combo(self, include_all):
        combo = QComboBox()
        if include_all:
            combo.addItem("All algorithms", "")
        for label, _name, number in ALGORITHMS:
            combo.addItem(label, number)
        return combo

    def path_row(self, field, callback):
        row = QHBoxLayout()
        row.addWidget(field)
        button = QPushButton("Browse...")
        button.clicked.connect(callback)
        row.addWidget(button)
        return row

    def executable(self):
        return BUILD_DIR / ("sort_app.exe" if os.name == "nt" else "sort_app")

    def start_process(self, program, arguments, stdin_data=b"", label="Process"):
        if self.process.state() != QProcess.ProcessState.NotRunning:
            QMessageBox.warning(self, "Busy", "Another process is already running.")
            return False
        self.pending_stdin = stdin_data
        self.running_label = label
        self.status.setText(f"Running: {label}")
        self.stop_button.setEnabled(True)
        self.build_button.setEnabled(False)
        self.log.appendPlainText(f"> {program} {' '.join(arguments)}")
        self.process.setWorkingDirectory(str(ROOT))
        self.process.start(program, arguments)
        return True

    def start_app(self, arguments, stdin_data=b"", label="sort_app"):
        executable = self.executable()
        if not executable.exists():
            QMessageBox.warning(self, "Executable not found", "Build sort_app first.")
            return False
        return self.start_process(str(executable), arguments, stdin_data, label)

    def run_stdin(self):
        data = self.stdin_input.toPlainText().strip()
        if not data:
            QMessageBox.warning(self, "Invalid input", "Enter n and the array values.")
            return
        algorithm = self.sort_algorithm.currentData()
        self.start_app(
            [],
            f"{algorithm}\n{data}\n".encode(),
            "stdin/stdout",
        )

    def run_file(self):
        input_path = self.resolve_path(self.input_path.text())
        output_path = self.resolve_path(self.output_path.text())
        if not input_path.is_file():
            QMessageBox.warning(self, "Invalid input", "Select an existing .inp file.")
            return
        output_path.parent.mkdir(parents=True, exist_ok=True)
        self.start_app(
            ["--file", str(input_path), str(output_path), self.sort_algorithm.currentData()],
            label="file input/output",
        )

    def run_benchmark(self):
        csv_path = self.resolve_path(self.bench_csv.text())
        arguments = [
            "--benchmark",
            "--in",
            str(self.resolve_path(self.bench_input.text())),
            "--out",
            str(self.resolve_path(self.bench_expected.text())),
            "--csv",
            str(csv_path),
            "--repeat",
            str(self.bench_repeat.value()),
            "--timeout",
            str(self.bench_timeout.value()),
        ]
        if self.skip_tle.isChecked():
            arguments.append("--skip-after-tle")
        self.start_app(arguments, label="benchmark")

    def run_kat(self):
        arguments = [
            "--kat",
            "--vectors",
            str(self.resolve_path(self.kat_vectors.text())),
            "--timeout",
            str(self.kat_timeout.value()),
        ]
        algorithm = self.kat_algorithm.currentData()
        category = self.kat_category.currentText()
        if algorithm:
            arguments.extend(["--algo", algorithm])
        if category != "All categories":
            arguments.extend(["--category", category])
        if self.kat_verbose.isChecked():
            arguments.append("-v")
        self.start_app(arguments, label="known-answer test")

    def generate_charts(self):
        csv_path = self.resolve_path(self.bench_csv.text())
        if not csv_path.is_file():
            QMessageBox.warning(self, "CSV not found", "Run benchmark first.")
            return
        self.start_process(
            sys.executable,
            [
                str(ROOT / "scripts" / "plot_bench.py"),
                "--input",
                str(csv_path),
                "--output-dir",
                str(OUTPUT_DIR),
            ],
            label="plot benchmark results",
        )

    def build(self):
        if self.process.state() != QProcess.ProcessState.NotRunning:
            QMessageBox.warning(self, "Busy", "Stop the current process first.")
            return
        script = ROOT / "scripts" / ("build.bat" if os.name == "nt" else "build.sh")
        program = "cmd.exe" if os.name == "nt" else "bash"
        arguments = ["/c", str(script)] if os.name == "nt" else [str(script)]
        self.start_process(program, arguments, label="build")

    def write_stdin(self):
        if self.pending_stdin:
            self.process.write(self.pending_stdin)
            self.process.closeWriteChannel()
            self.pending_stdin = b""

    def read_output(self):
        stdout = bytes(self.process.readAllStandardOutput()).decode(errors="replace")
        stderr = bytes(self.process.readAllStandardError()).decode(errors="replace")
        if stdout:
            self.log.appendPlainText(stdout.rstrip())
        if stderr:
            self.log.appendPlainText(stderr.rstrip())
        self.log.ensureCursorVisible()

    def process_finished(self, exit_code, _status):
        self.read_output()
        self.status.setText(
            f"{self.running_label}: {'成功' if exit_code == 0 else '失败'} ({exit_code})"
        )
        self.stop_button.setEnabled(False)
        self.build_button.setEnabled(True)
        self.running_label = ""

    def stop_process(self):
        if self.process.state() != QProcess.ProcessState.NotRunning:
            self.process.kill()
            self.status.setText("Stopped")

    def resolve_path(self, value):
        path = Path(value).expanduser()
        return path if path.is_absolute() else ROOT / path

    def choose_input(self):
        selected, _ = QFileDialog.getOpenFileName(
            self, "Select input file", str(ROOT / "dataset" / "input"), "Input (*.inp)"
        )
        if selected:
            self.input_path.setText(selected)

    def choose_output(self):
        selected, _ = QFileDialog.getSaveFileName(
            self, "Select output file", str(OUTPUT_DIR), "Output (*.out)"
        )
        if selected:
            self.output_path.setText(selected)

    def choose_bench_input(self):
        self.choose_directory(self.bench_input, "Select benchmark input directory")

    def choose_bench_expected(self):
        self.choose_directory(self.bench_expected, "Select expected output directory")

    def choose_bench_csv(self):
        selected, _ = QFileDialog.getSaveFileName(
            self, "Select benchmark CSV", str(OUTPUT_DIR / "benchmark.csv"), "CSV (*.csv)"
        )
        if selected:
            self.bench_csv.setText(selected)

    def choose_vectors(self):
        selected, _ = QFileDialog.getOpenFileName(
            self, "Select vectors JSON", str(ROOT / "test-vector"), "JSON (*.json)"
        )
        if selected:
            self.kat_vectors.setText(selected)

    def choose_directory(self, field, title):
        selected = QFileDialog.getExistingDirectory(self, title, str(ROOT))
        if selected:
            field.setText(selected)

    def open_output(self):
        OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
        if os.name == "nt":
            os.startfile(OUTPUT_DIR)
        else:
            subprocess.Popen(["xdg-open", str(OUTPUT_DIR)])


def main():
    app = QApplication(sys.argv)
    window = SortAppWindow()
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
