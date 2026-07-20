"""
Fast simulation GIF renderer using PyQtGraph + Qt + FFmpeg.

Key differences from the Matplotlib version:
- Creates the plots once and updates existing graphics items.
- Keeps a violin plot (fast histogram-based Gaussian KDE).
- Keeps a pie chart (custom QGraphicsObject).
- Streams rendered RGB frames to one temporary MP4.
- Converts the MP4 to a palette-optimized GIF with FFmpeg.
- Does not create hundreds of PNG files.
"""

from __future__ import annotations

import json
import os
import subprocess
import time
import warnings
from pathlib import Path

# Must be set before importing PySide6/pyqtgraph.
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import imageio.v2 as imageio
import imageio_ffmpeg
import numpy as np
import pandas as pd
import pyqtgraph as pg
from PySide6 import QtCore, QtGui, QtWidgets
from tqdm import tqdm

# ---------------------------------------------------------------------------
SIM_CSV = os.environ.get("SIM_CSV")
DEFINES_JSON = "Defines.json"
GIF_OUTPUT = "test.gif"

STRIDE = 10
GIF_DURATION = 0.05          # seconds per GIF frame
POLY_DEGREE = 4
TOP_TRIM_PCT = 0.25
MAX_SECONDS = 10

# Smaller dimensions render much faster. Both should be even for H.264.
FRAME_WIDTH = 1200
FRAME_HEIGHT = 700

# Violin quality/speed controls.
VIOLIN_BINS = 72
VIOLIN_WIDTH = 0.42

# Keep the temporary video for debugging if desired.
KEEP_TEMP_VIDEO = False

# ---------------------------------------------------------------------------


def padded_range(values: np.ndarray, pad_fraction: float = 0.05) -> tuple[float, float]:
    """Return a finite non-zero plotting range."""
    values = np.asarray(values, dtype=np.float64)
    values = values[np.isfinite(values)]

    if values.size == 0:
        return 0.0, 1.0

    low = float(values.min())
    high = float(values.max())

    if low == high:
        pad = max(abs(low) * pad_fraction, 1.0)
    else:
        pad = (high - low) * pad_fraction

    return low - pad, high + pad


def histogram_kde(values: np.ndarray, bins: int = VIOLIN_BINS) -> tuple[np.ndarray, np.ndarray, float]:
    """
    Fast one-dimensional KDE for the violin plot.

    It bins the samples and smooths the histogram with a Gaussian kernel. This
    is much cheaper than evaluating one Gaussian per sample at every grid point.
    Returns (vertical_positions, normalized_density, mean).
    """
    values = np.asarray(values, dtype=np.float64)
    values = values[np.isfinite(values)]

    if values.size == 0:
        return np.array([0.0, 1.0]), np.zeros(2), 0.0

    mean = float(values.mean())
    minimum = float(values.min())
    maximum = float(values.max())

    if values.size == 1 or minimum == maximum:
        pad = max(abs(mean) * 0.02, 1.0)
        grid = np.linspace(mean - pad, mean + pad, bins)
        sigma = max(pad * 0.20, np.finfo(float).eps)
        density = np.exp(-0.5 * ((grid - mean) / sigma) ** 2)
        density /= density.max()
        return grid, density, mean

    std = float(values.std(ddof=1))
    q25, q75 = np.percentile(values, [25.0, 75.0])
    robust_sigma = float((q75 - q25) / 1.34)

    scale_candidates = [v for v in (std, robust_sigma) if np.isfinite(v) and v > 0]
    scale = min(scale_candidates) if scale_candidates else (maximum - minimum) / 6.0

    # Silverman-style bandwidth with defensive fallbacks.
    bandwidth = 0.9 * scale * values.size ** (-0.2)
    if not np.isfinite(bandwidth) or bandwidth <= 0:
        bandwidth = max((maximum - minimum) / 20.0, np.finfo(float).eps)

    low = minimum - 3.0 * bandwidth
    high = maximum + 3.0 * bandwidth

    counts, edges = np.histogram(values, bins=bins, range=(low, high))
    centers = 0.5 * (edges[:-1] + edges[1:])
    bin_width = float(edges[1] - edges[0])

    sigma_bins = max(bandwidth / bin_width, 0.5)
    radius = min(max(1, int(np.ceil(3.0 * sigma_bins))), max(1, bins // 2 - 1))
    offsets = np.arange(-radius, radius + 1, dtype=np.float64)
    kernel = np.exp(-0.5 * (offsets / sigma_bins) ** 2)
    kernel /= kernel.sum()

    density = np.convolve(counts.astype(np.float64), kernel, mode="same")
    peak = float(density.max())
    if peak > 0:
        density /= peak

    return centers, density, mean


class ViolinItem:
    """A reusable violin made from two curves and one FillBetweenItem."""

    def __init__(self, plot: pg.PlotItem) -> None:
        outline_pen = pg.mkPen(40, 90, 180, width=1.5)
        fill_brush = pg.mkBrush(70, 130, 230, 120)
        mean_pen = pg.mkPen(20, 20, 20, width=2)

        self.left = pg.PlotCurveItem(pen=outline_pen)
        self.right = pg.PlotCurveItem(pen=outline_pen)
        self.fill = pg.FillBetweenItem(self.left, self.right, brush=fill_brush)
        self.mean_line = pg.PlotCurveItem(pen=mean_pen)

        plot.addItem(self.fill)
        plot.addItem(self.left)
        plot.addItem(self.right)
        plot.addItem(self.mean_line)

    def set_data(self, values: np.ndarray) -> None:
        vertical, density, mean = histogram_kde(values)
        half_width = density * VIOLIN_WIDTH

        self.left.setData(-half_width, vertical)
        self.right.setData(half_width, vertical)
        self.mean_line.setData(
            np.array([-VIOLIN_WIDTH * 0.85, VIOLIN_WIDTH * 0.85]),
            np.array([mean, mean]),
        )


class PieChartItem(pg.GraphicsObject):
    """A reusable two-slice pie chart drawn directly with QPainter."""

    def __init__(self) -> None:
        super().__init__()
        self.without_value = 0.0
        self.with_value = 0.0
        self.without_color = QtGui.QColor(185, 185, 185)
        self.with_color = QtGui.QColor(70, 155, 90)

    def boundingRect(self) -> QtCore.QRectF:  # noqa: N802 (Qt naming)
        return QtCore.QRectF(-1.0, -1.0, 2.0, 2.0)

    def set_values(self, without_value: float, with_value: float) -> None:
        self.without_value = max(0.0, float(without_value))
        self.with_value = max(0.0, float(with_value))
        self.update()

    def paint(self, painter: QtGui.QPainter, option, widget=None) -> None:
        del option, widget
        painter.setRenderHint(QtGui.QPainter.RenderHint.Antialiasing, True)

        rect = QtCore.QRectF(-0.92, -0.92, 1.84, 1.84)
        total = self.without_value + self.with_value

        border_pen = QtGui.QPen(QtGui.QColor(255, 255, 255))
        border_pen.setCosmetic(True)
        border_pen.setWidth(2)
        painter.setPen(border_pen)

        if total <= 0:
            painter.setBrush(QtGui.QBrush(self.without_color))
            painter.drawEllipse(rect)
            return

        start_angle = 90 * 16
        slices = (
            (self.without_value, self.without_color),
            (self.with_value, self.with_color),
        )

        for value, color in slices:
            if value <= 0:
                continue
            # Negative span draws clockwise, matching typical pie-chart layout.
            span_angle = -int(round((value / total) * 360.0 * 16.0))
            painter.setBrush(QtGui.QBrush(color))
            painter.drawPie(rect, start_angle, span_angle)
            start_angle += span_angle


class SimulationRenderer:
    """Owns one persistent Qt canvas and updates it for each frame."""

    def __init__(
            self,
            food: np.ndarray,
            total_population: np.ndarray,
            iterations: np.ndarray,
            population_bid: np.ndarray,
            population_size: np.ndarray,
            cumulative_bought: np.ndarray,
            frames: list[int],
    ) -> None:
        self.food = food
        self.total_population = total_population
        self.iterations = iterations
        self.population_bid = population_bid
        self.population_size = population_size
        self.cumulative_bought = cumulative_bought
        self.frames = frames

        self.app = QtWidgets.QApplication.instance() or QtWidgets.QApplication([])

        pg.setConfigOptions(
            antialias=False,
            background="w",
            foreground="k",
            useOpenGL=False,
        )

        self.canvas = pg.GraphicsLayoutWidget(show=False)
        self.canvas.resize(FRAME_WIDTH, FRAME_HEIGHT)
        self.canvas.setWindowTitle("Simulation")

        self._build_plots()
        self._set_fixed_ranges()

        # Showing an offscreen widget ensures that Qt completes layout geometry.
        self.canvas.show()
        self.app.processEvents()

    def _build_plots(self) -> None:
        # Demand curve: first two columns, first row.
        self.demand_plot = self.canvas.addPlot(row=0, col=0, colspan=2)
        self.demand_plot.setTitle("Demand curve")
        self.demand_plot.setLabel("bottom", "Cumulative quantity")
        self.demand_plot.setLabel("left", "Food value")
        self.demand_plot.showGrid(x=True, y=True, alpha=0.15)

        self.fit_curve = self.demand_plot.plot(
            pen=pg.mkPen(210, 60, 60, width=2),
        )
        self.demand_points = self.demand_plot.plot(
            pen=None,
            symbol="o",
            symbolSize=3,
            symbolPen=None,
            symbolBrush=pg.mkBrush(40, 90, 210),
        )
        self.food_line = pg.InfiniteLine(
            angle=0,
            movable=False,
            pen=pg.mkPen(50, 155, 70, width=2),
        )
        self.quantity_line = pg.InfiniteLine(
            angle=90,
            movable=False,
            pen=pg.mkPen(40, 90, 210, width=2),
        )
        self.demand_plot.addItem(self.food_line)
        self.demand_plot.addItem(self.quantity_line)

        # Population history: first two columns, second row.
        self.population_plot = self.canvas.addPlot(row=1, col=0, colspan=2)
        self.population_plot.setTitle("Population")
        self.population_plot.setLabel("bottom", "Iteration")
        self.population_plot.setLabel("left", "Total population")
        self.population_plot.showGrid(x=True, y=True, alpha=0.15)

        self.population_curve = self.population_plot.plot(
            pen=pg.mkPen(40, 90, 210, width=2),
        )
        self.population_max_line = pg.InfiniteLine(
            angle=0,
            movable=False,
            pen=pg.mkPen(50, 155, 70, width=2),
        )
        self.population_plot.addItem(self.population_max_line)

        # Violin: right column, top two rows.
        self.violin_plot = self.canvas.addPlot(row=0, col=2, rowspan=2)
        self.violin_plot.setTitle("Pop distribution")
        self.violin_plot.setLabel("left", "Population")
        self.violin_plot.hideAxis("bottom")
        self.violin_plot.showGrid(x=False, y=True, alpha=0.15)
        self.violin_plot.setMouseEnabled(x=False, y=False)
        self.violin = ViolinItem(self.violin_plot)

        # Pie: middle column, bottom row.
        self.pie_plot = self.canvas.addPlot(row=2, col=1)
        self.pie_plot.setTitle("Food allocation")
        self.pie_plot.hideAxis("bottom")
        self.pie_plot.hideAxis("left")
        self.pie_plot.setAspectLocked(True)
        self.pie_plot.setMouseEnabled(x=False, y=False)

        self.pie_item = PieChartItem()
        self.pie_plot.addItem(self.pie_item)

        self.without_label = pg.TextItem(
            text="w/o: 0.0%",
            color=(70, 70, 70),
            anchor=(0.5, 0.5),
        )
        self.with_label = pg.TextItem(
            text="with: 0.0%",
            color=(30, 100, 45),
            anchor=(0.5, 0.5),
        )
        self.without_label.setPos(-0.55, -1.13)
        self.with_label.setPos(0.55, -1.13)
        self.pie_plot.addItem(self.without_label)
        self.pie_plot.addItem(self.with_label)

        # Keep the empty bottom-left and bottom-right cells as spacers.
        left_spacer = self.canvas.addLabel("", row=2, col=0)
        right_spacer = self.canvas.addLabel("", row=2, col=2)
        left_spacer.setMaximumHeight(1)
        right_spacer.setMaximumHeight(1)

    def _set_fixed_ranges(self) -> None:
        """Fixed ranges avoid expensive and visually distracting auto-ranging."""
        trim_index = round(self.population_bid.shape[0] * TOP_TRIM_PCT)
        selected = np.asarray(self.frames, dtype=np.int64)

        demand_x = self.cumulative_bought[trim_index:, :][:, selected]
        demand_y = self.population_bid[trim_index:, :][:, selected]
        food_y = self.food[selected]

        x_low, x_high = padded_range(demand_x)
        y_low, y_high = padded_range(np.concatenate((demand_y.ravel(), food_y.ravel())))
        self.demand_plot.setXRange(x_low, x_high, padding=0.0)
        self.demand_plot.setYRange(y_low, y_high, padding=0.0)
        self.demand_plot.disableAutoRange()

        iteration_low, iteration_high = padded_range(self.iterations[selected], 0.02)
        total_low, total_high = padded_range(self.total_population[selected])
        self.population_plot.setXRange(iteration_low, iteration_high, padding=0.0)
        self.population_plot.setYRange(total_low, total_high, padding=0.0)
        self.population_plot.disableAutoRange()

        violin_low, violin_high = padded_range(self.population_size[:, selected])
        self.violin_plot.setXRange(-0.60, 0.60, padding=0.0)
        self.violin_plot.setYRange(violin_low, violin_high, padding=0.0)
        self.violin_plot.disableAutoRange()

        self.pie_plot.setXRange(-1.20, 1.20, padding=0.0)
        self.pie_plot.setYRange(-1.30, 1.10, padding=0.0)
        self.pie_plot.disableAutoRange()

    def update(self, level: int) -> None:
        x = self.cumulative_bought[:, level]
        y = self.population_bid[:, level]

        trim_index = round(x.size * TOP_TRIM_PCT)
        x = x[trim_index:]
        y = y[trim_index:]

        current_food = float(self.food[level])
        below = np.flatnonzero(y < current_food)
        equilibrium_quantity = float(x[below[0]]) if below.size else float(x[-1])

        self.demand_points.setData(x, y)
        self.food_line.setValue(current_food)
        self.quantity_line.setValue(equilibrium_quantity)
        self.demand_plot.setTitle(f"Demand curve — iter {level}")

        # Normalizing x before polyfit improves numerical stability.
        finite = np.isfinite(x) & np.isfinite(y)
        fit_x = x[finite]
        fit_y = y[finite]

        if fit_x.size > POLY_DEGREE and np.unique(fit_x).size > POLY_DEGREE:
            center = float(fit_x.mean())
            scale = float(fit_x.std())
            if scale > 0:
                try:
                    with warnings.catch_warnings():
                        warnings.simplefilter("ignore")
                        coefficients = np.polyfit(
                            (fit_x - center) / scale,
                            fit_y,
                            POLY_DEGREE,
                            )
                    fitted_y = np.polyval(coefficients, (fit_x - center) / scale)
                    self.fit_curve.setData(fit_x, fitted_y)
                except (np.linalg.LinAlgError, ValueError, FloatingPointError):
                    self.fit_curve.setData([], [])
            else:
                self.fit_curve.setData([], [])
        else:
            self.fit_curve.setData([], [])

        population_history = self.total_population[: level + 1]
        self.population_curve.setData(
            self.iterations[: level + 1],
            population_history,
        )
        self.population_max_line.setValue(float(np.nanmax(population_history)))

        current_sizes = self.population_size[:, level]
        self.violin.set_data(current_sizes)

        has_food = self.population_bid[:, level] > current_food
        with_food = float(np.nansum(current_sizes[has_food]))
        without_food = max(0.0, float(self.total_population[level]) - with_food)
        with_food = max(0.0, with_food)

        self.pie_item.set_values(without_food, with_food)
        pie_total = without_food + with_food
        if pie_total > 0:
            without_pct = 100.0 * without_food / pie_total
            with_pct = 100.0 * with_food / pie_total
        else:
            without_pct = 0.0
            with_pct = 0.0

        self.without_label.setText(f"w/o: {without_pct:.1f}%")
        self.with_label.setText(f"with: {with_pct:.1f}%")

        # Flush pending scene/layout changes before rendering to QImage.
        self.app.processEvents()

    def capture_rgb(self) -> np.ndarray:
        """Render the widget directly into an RGB NumPy array."""
        image = QtGui.QImage(
            FRAME_WIDTH,
            FRAME_HEIGHT,
            QtGui.QImage.Format.Format_RGB888,
        )
        image.fill(QtGui.QColor(255, 255, 255))

        painter = QtGui.QPainter(image)
        self.canvas.render(painter)
        painter.end()

        raw = np.frombuffer(
            image.bits(),
            dtype=np.uint8,
            count=image.sizeInBytes(),
        )
        raw = raw.reshape(FRAME_HEIGHT, image.bytesPerLine())
        rgb = raw[:, : FRAME_WIDTH * 3].reshape(FRAME_HEIGHT, FRAME_WIDTH, 3)

        # Copy because QImage owns the original memory.
        return np.ascontiguousarray(rgb.copy())

    def close(self) -> None:
        self.canvas.close()
        self.app.processEvents()


# ---------------------------------------------------------------------------


def convert_video_to_gif(video_path: Path, gif_path: Path, fps: float) -> None:
    """Use a generated palette for better GIF quality and compression."""
    ffmpeg = imageio_ffmpeg.get_ffmpeg_exe()
    palette_path = gif_path.with_name(f".{gif_path.stem}_palette.png")

    palette_filter = f"fps={fps:g},palettegen=stats_mode=diff"
    gif_filter = (
        f"[0:v]fps={fps:g}[frames];"
        "[frames][1:v]paletteuse=dither=sierra2_4a"
    )

    try:
        subprocess.run(
            [
                ffmpeg,
                "-y",
                "-loglevel",
                "error",
                "-i",
                str(video_path),
                "-vf",
                palette_filter,
                "-frames:v",
                "1",
                "-update",
                "1",
                str(palette_path),
            ],
            check=True,
        )

        subprocess.run(
            [
                ffmpeg,
                "-y",
                "-loglevel",
                "error",
                "-i",
                str(video_path),
                "-i",
                str(palette_path),
                "-filter_complex",
                gif_filter,
                "-loop",
                "0",
                str(gif_path),
            ],
            check=True,
        )
    finally:
        palette_path.unlink(missing_ok=True)


def simulation_csv_path() -> Path:
    """Find the newest simulation output unless SIM_CSV is explicit."""
    if SIM_CSV:
        path = Path(SIM_CSV)
        if not path.is_file():
            raise FileNotFoundError(f"SIM_CSV does not exist: {path}")
        return path

    candidates = [
        Path("build/data.csv"),
        Path("cmake-build-debug/data.csv"),
        Path("data.csv"),
    ]
    existing = [path for path in candidates if path.is_file()]
    if not existing:
        searched = ", ".join(str(path) for path in candidates)
        raise FileNotFoundError(f"No simulation CSV found; searched: {searched}")
    return max(existing, key=lambda path: path.stat().st_mtime)


def load_simulation_data():
    with open(DEFINES_JSON, "r", encoding="utf-8") as file:
        defines = json.load(file)

    n_populations = int(defines["n_agents"]) + 1
    configured_levels = int(defines["ITERATIONS"])

    dataframe = pd.read_csv(simulation_csv_path())
    n_levels = min(configured_levels, len(dataframe))
    if n_levels <= 0:
        raise ValueError("The CSV contains no simulation levels.")

    food = dataframe["food value"].to_numpy(dtype=np.float64)[:n_levels]
    total_population = dataframe["total population"].to_numpy(dtype=np.float64)[:n_levels]
    iterations = dataframe["iteration"].to_numpy(dtype=np.float64)[:n_levels]

    population_bid = np.vstack(
        [
            dataframe[f"Pop {index} food value"].to_numpy(dtype=np.float64)[:n_levels]
            for index in range(n_populations)
        ]
    )
    population_bought = np.vstack(
        [
            dataframe[f"Pop {index} Bought"].to_numpy(dtype=np.float64)[:n_levels]
            for index in range(n_populations)
        ]
    )
    population_size = np.vstack(
        [
            dataframe[f"Pop {index} Population"].to_numpy(dtype=np.float64)[:n_levels]
            for index in range(n_populations)
        ]
    )
    bid_order = np.argsort(-population_bid, axis=0)
    population_bid = np.take_along_axis(population_bid, bid_order, axis=0)
    population_bought = np.take_along_axis(population_bought, bid_order, axis=0)
    population_size = np.take_along_axis(population_size, bid_order, axis=0)
    cumulative_bought = np.cumsum(population_bought, axis=0, dtype=np.float64)

    return (
        n_levels,
        food,
        total_population,
        iterations,
        population_bid,
        population_size,
        cumulative_bought,
    )


def main() -> None:
    start_time = time.monotonic()
    gif_path = Path(GIF_OUTPUT).resolve()
    gif_path.parent.mkdir(parents=True, exist_ok=True)

    temp_video = gif_path.with_name(f".{gif_path.stem}_temporary.mp4")
    temp_video.unlink(missing_ok=True)

    (
        n_levels,
        food,
        total_population,
        iterations,
        population_bid,
        population_size,
        cumulative_bought,
    ) = load_simulation_data()

    frames = list(range(0, n_levels, STRIDE))
    if not frames:
        raise ValueError("No frames were selected. Check ITERATIONS and STRIDE.")

    fps = 1.0 / GIF_DURATION
    print(
        f"Rendering {len(frames)} frames at {FRAME_WIDTH}x{FRAME_HEIGHT}, "
        f"{fps:g} fps"
    )

    renderer = SimulationRenderer(
        food=food,
        total_population=total_population,
        iterations=iterations,
        population_bid=population_bid,
        population_size=population_size,
        cumulative_bought=cumulative_bought,
        frames=frames,
    )

    render_start = time.monotonic()
    writer = imageio.get_writer(
        str(temp_video),
        format="FFMPEG",
        mode="I",
        fps=fps,
        codec="libx264",
        pixelformat="yuv420p",
        macro_block_size=None,
        output_params=["-preset", "ultrafast", "-crf", "18"],
        ffmpeg_log_level="error",
    )

    try:
        for level in tqdm(frames, desc="Rendering", unit="frame"):
            renderer.update(level)
            writer.append_data(renderer.capture_rgb())
    finally:
        writer.close()
        renderer.close()

    render_finished = time.monotonic()

    print("Converting temporary video to GIF...")
    convert_video_to_gif(temp_video, gif_path, fps)

    if not KEEP_TEMP_VIDEO:
        temp_video.unlink(missing_ok=True)

    elapsed = time.monotonic() - start_time
    render_elapsed = render_finished - render_start

    print(
        f"Saved {gif_path} "
        f"({len(frames)} frames, render={render_elapsed:.1f}s, total={elapsed:.1f}s)"
    )
    if elapsed > MAX_SECONDS:
        print(f"Warning: {elapsed:.1f}s exceeds the {MAX_SECONDS}s budget")


if __name__ == "__main__":
    main()
