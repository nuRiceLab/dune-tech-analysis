#!/usr/bin/env python3
"""Convert sparse 3D particle tensors into MNIST-like 2D image tensors.

The output is a ``torch.save`` payload with:

    images:  FloatTensor [N, 1, H, W], values normalized to [0, 1]
    labels:  LongTensor  [N]
    energies_gev: FloatTensor [N]
    pdgs: LongTensor [N]
    indices: LongTensor [N]

This is meant to be loaded with ``torch.utils.data.TensorDataset`` as a
drop-in replacement for the MNIST examples:

    payload = torch.load("lartpc_mnist_like.pt")
    dataset = torch.utils.data.TensorDataset(payload["images"], payload["labels"])
"""

from __future__ import annotations

import argparse
from collections import Counter
from pathlib import Path

import torch

try:
    from tqdm.auto import tqdm
except ImportError:  # pragma: no cover - only used on minimal environments.
    tqdm = None


PDG_TO_NAME = {
    11: "electron",
    -11: "electron",
    13: "muon",
    -13: "muon",
    211: "pion",
    -211: "pion",
    2212: "proton",
    -2212: "proton",
    321: "kaon",
    -321: "kaon",
}

CLASS_NAMES = ["electron", "muon", "pion", "proton", "kaon"]
CLASS_TO_LABEL = {name: index for index, name in enumerate(CLASS_NAMES)}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Project sparse 3D particle tensors into MNIST-like images."
    )
    parser.add_argument("inputs", nargs="*", type=Path, help="Input .pt chunk(s).")
    parser.add_argument(
        "--input-list",
        action="append",
        type=Path,
        default=[],
        help="Text file with one input .pt chunk path per line. Can be repeated.",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=Path("MachineLearning/2026/lartpc_mnist_like.pt"),
        help="Output torch dataset path.",
    )
    parser.add_argument(
        "--image-size",
        type=int,
        default=28,
        help="Output image height/width. MNIST uses 28.",
    )
    parser.add_argument(
        "--crop-size",
        type=float,
        default=None,
        help=(
            "Projected crop side length in source coordinate bins. By default this is "
            "computed as image-side-cm / voxel-size-cm."
        ),
    )
    parser.add_argument(
        "--image-side-cm",
        type=float,
        default=100.0,
        help="Physical side length of the square crop. Default is 1 m.",
    )
    parser.add_argument(
        "--voxel-size-cm",
        type=float,
        default=0.5,
        help="Source voxel pitch. Default is 0.5 cm.",
    )
    parser.add_argument(
        "--drop-axis",
        choices=("x", "y", "z"),
        default="x",
        help=(
            "3D axis to drop for fixed-axis projection. Ignored by detector and "
            "PCA projection modes."
        ),
    )
    parser.add_argument(
        "--projection-mode",
        choices=("detector", "global-pca", "per-event-pca", "fixed"),
        default="detector",
        help=(
            "detector uses fixed detector coordinates and does not run PCA. "
            "global-pca uses one PCA basis for the full sample."
        ),
    )
    parser.add_argument(
        "--anchor-mode",
        choices=("start-plane", "event-min", "first", "global-start"),
        default="start-plane",
        help=(
            "Where to put the low edge of the image along the travel-like axis. "
            "start-plane uses a fixed detector-axis start plane; event-min uses each "
            "event's low-PC1 endpoint with the global basis; global-start uses one PCA "
            "start coordinate inferred from the dataset."
        ),
    )
    parser.add_argument(
        "--start-axis",
        choices=("x", "y", "z"),
        default="z",
        help="Detector axis used by --anchor-mode start-plane.",
    )
    parser.add_argument(
        "--start-plane",
        type=float,
        default=20000.0,
        help="Detector-axis coordinate of the fixed longitudinal start plane.",
    )
    parser.add_argument(
        "--travel-sign",
        choices=("positive", "negative"),
        default="negative",
        help=(
            "Direction of travel along --start-axis from --start-plane. For the muon "
            "sample, start is z=20000 and travel is toward lower z, so use negative."
        ),
    )
    parser.add_argument(
        "--transverse-center-mode",
        choices=("local-slice", "event-median", "global"),
        default="local-slice",
        help=(
            "How to center the visible transverse axis. local-slice uses points in "
            "the first crop window along the travel axis."
        ),
    )
    parser.add_argument(
        "--transverse-axis",
        choices=("x", "y", "z"),
        default="x",
        help=(
            "Detector axis drawn vertically in detector projection mode. The "
            "remaining axis is collapsed by summing deposits in the same image bin."
        ),
    )
    parser.add_argument(
        "--transverse-start-plane",
        type=float,
        default=20000.0,
        help=(
            "Detector-axis coordinate centered vertically in detector projection "
            "mode. Default assumes particles also start at x/y=20000."
        ),
    )
    parser.add_argument(
        "--start-padding-cm",
        type=float,
        default=None,
        help=(
            "Physical padding before the start plane. If omitted, use "
            "--start-padding-pixels instead."
        ),
    )
    parser.add_argument(
        "--start-padding-pixels",
        type=float,
        default=1.0,
        help=(
            "Output-pixel padding before the start plane. Default puts the start "
            "plane in the second pixel column from the left."
        ),
    )
    parser.add_argument(
        "--energy-min",
        type=float,
        default=0.0,
        help="Minimum kinetic energy in GeV, read from the 'target' field.",
    )
    parser.add_argument(
        "--energy-max",
        type=float,
        default=1.0,
        help="Maximum kinetic energy in GeV, read from the 'target' field.",
    )
    parser.add_argument(
        "--max-events",
        type=int,
        default=None,
        help="Optional cap for quick previews.",
    )
    parser.add_argument(
        "--max-per-class",
        type=int,
        default=None,
        help="Optional cap per particle class, e.g. 10000 for a balanced sample.",
    )
    parser.add_argument(
        "--shuffle-seed",
        type=int,
        default=12345,
        help="Seed used to shuffle the final image order before writing.",
    )
    parser.add_argument(
        "--no-shuffle",
        dest="shuffle",
        action="store_false",
        help="Keep selected events in input order instead of shuffling before writing.",
    )
    parser.set_defaults(shuffle=True)
    parser.add_argument(
        "--log1p",
        action="store_true",
        help="Apply log1p compression before clipping. Disabled by default.",
    )
    parser.add_argument(
        "--clip-mev",
        type=float,
        default=None,
        help=(
            "Fixed per-pixel clipping scale in MeV after same-bin deposits are summed. "
            "If omitted, use --mip-multiplier times the expected MIP deposit across "
            "one output pixel."
        ),
    )
    parser.add_argument(
        "--mip-dedx-mev-per-cm",
        type=float,
        default=2.1,
        help="Approximate MIP dE/dx in liquid argon.",
    )
    parser.add_argument(
        "--mip-multiplier",
        type=float,
        default=5.0,
        help="Default saturation level in units of MIP-like deposit per output pixel.",
    )
    parser.add_argument(
        "--feature-column",
        type=int,
        default=0,
        help=(
            "Feature column to use as deposited energy when features are 2D. "
            "For current muon chunks, column 0 is charge/energy and column 1 "
            "contains particle/label-like values."
        ),
    )
    parser.add_argument(
        "--feature-mev-scale",
        type=float,
        default=1.0,
        help=(
            "Conversion from stored feature values to MeV before clipping. Default "
            "assumes the feature is already in MeV."
        ),
    )
    parser.add_argument(
        "--feature-min",
        type=float,
        default=0.0,
        help="Drop voxels with raw feature values below this threshold.",
    )
    parser.add_argument(
        "--feature-max",
        type=float,
        default=100.0,
        help=(
            "Drop voxels with raw feature values above this threshold before binning. "
            "This removes sentinel-like values around 1e9 seen in some chunks."
        ),
    )
    parser.add_argument(
        "--preview-dir",
        type=Path,
        default=Path("MachineLearning/2026/previews"),
        help="Directory for preview images. Files are PGM, which most image viewers can open.",
    )
    parser.add_argument(
        "--num-previews",
        type=int,
        default=8,
        help="Number of preview images to write.",
    )
    parser.add_argument(
        "--no-progress",
        dest="progress",
        action="store_false",
        help="Disable tqdm progress bars.",
    )
    parser.set_defaults(progress=True)
    return parser.parse_args()


def input_paths_from_args(args: argparse.Namespace) -> list[Path]:
    paths = list(args.inputs)
    for list_path in args.input_list:
        with list_path.open() as fin:
            for line in fin:
                stripped = line.strip()
                if stripped and not stripped.startswith("#"):
                    paths.append(Path(stripped))

    if not paths:
        raise ValueError("Provide input chunks as positional arguments or with --input-list.")
    return paths


def particle_name(pdg: int) -> str:
    return PDG_TO_NAME.get(int(pdg), f"pdg_{int(pdg)}")


def progress_bar(items: list[Path], description: str, enabled: bool):
    if enabled and tqdm is not None:
        return tqdm(items, desc=description, unit="file")
    return items


def chunk_particle_name(chunk: list[dict], input_path: Path) -> str:
    if not chunk:
        raise ValueError(f"Input chunk is empty: {input_path}")
    name = particle_name(int(chunk[0]["pdg"]))
    if name not in CLASS_TO_LABEL:
        raise ValueError(f"Unsupported particle PDG {int(chunk[0]['pdg'])} ({name}).")
    return name


def caps_reached(selected: Counter[str], target_classes: list[str], max_per_class: int | None) -> bool:
    if max_per_class is None:
        return False
    return all(selected[name] >= max_per_class for name in target_classes)


def projected_axes(drop_axis: str) -> tuple[int, int]:
    axes = {"x": 0, "y": 1, "z": 2}
    drop = axes[drop_axis]
    return tuple(axis for axis in range(3) if axis != drop)


def axis_index(axis: str) -> int:
    return {"x": 0, "y": 1, "z": 2}[axis]


def pca_basis_from_coords(coords: torch.Tensor) -> torch.Tensor:
    """Return 3 PCA basis vectors as columns, sorted by descending variance."""
    points = coords.float()
    center = points.mean(dim=0)
    centered = points - center
    covariance = centered.T @ centered / max(len(points) - 1, 1)
    eigenvalues, eigenvectors = torch.linalg.eigh(covariance)
    order = torch.argsort(eigenvalues, descending=True)
    return eigenvectors[:, order]


def global_pca_geometry(
    input_paths: list[Path],
    energy_min: float,
    energy_max: float,
    max_per_class: int | None,
    progress: bool,
) -> dict:
    """Infer one PCA basis and crop reference values for the whole dataset."""
    sum_xyz = torch.zeros(3, dtype=torch.float64)
    sum_outer = torch.zeros((3, 3), dtype=torch.float64)
    count = 0
    first_xyz = []
    selected_per_class: Counter[str] = Counter()

    for input_path in progress_bar(input_paths, "PCA covariance", progress):
        chunk = torch.load(input_path, map_location="cpu", weights_only=False)
        chunk_name = chunk_particle_name(chunk, input_path)
        if max_per_class is not None and selected_per_class[chunk_name] >= max_per_class:
            continue
        for event in chunk:
            energy = float(event["target"])
            if energy < energy_min or energy > energy_max:
                continue
            name = particle_name(int(event["pdg"]))
            if max_per_class is not None and selected_per_class[name] >= max_per_class:
                break

            coords = event["coords"].double()
            sum_xyz += coords.sum(dim=0)
            sum_outer += coords.T @ coords
            count += len(coords)
            first_xyz.append(coords[0])
            selected_per_class[name] += 1
            if max_per_class is not None and selected_per_class[name] >= max_per_class:
                break
        if caps_reached(selected_per_class, CLASS_NAMES, max_per_class):
            break

    if count == 0:
        raise RuntimeError("No events survived the energy cut for global PCA.")

    mean = sum_xyz / count
    covariance = sum_outer / count - torch.outer(mean, mean)
    eigenvalues, eigenvectors = torch.linalg.eigh(covariance)
    order = torch.argsort(eigenvalues, descending=True)
    basis = eigenvectors[:, order].float()
    eigenvalues = eigenvalues[order].float()

    first_mean = torch.stack(first_xyz).mean(dim=0).float()
    if torch.dot(mean.float() - first_mean, basis[:, 0]) < 0:
        basis[:, 0] *= -1

    u_min, u_first, u_max, v_median = [], [], [], []
    selected_per_class = Counter()
    for input_path in progress_bar(input_paths, "PCA summary", progress):
        chunk = torch.load(input_path, map_location="cpu", weights_only=False)
        chunk_name = chunk_particle_name(chunk, input_path)
        if max_per_class is not None and selected_per_class[chunk_name] >= max_per_class:
            continue
        for event in chunk:
            energy = float(event["target"])
            if energy < energy_min or energy > energy_max:
                continue
            name = particle_name(int(event["pdg"]))
            if max_per_class is not None and selected_per_class[name] >= max_per_class:
                break

            projected = event["coords"].float() @ basis[:, :2]
            u_min.append(projected[:, 0].min())
            u_first.append(projected[0, 0])
            u_max.append(projected[:, 0].max())
            v_median.append(projected[:, 1].median())
            selected_per_class[name] += 1
            if max_per_class is not None and selected_per_class[name] >= max_per_class:
                break
        if caps_reached(selected_per_class, CLASS_NAMES, max_per_class):
            break

    u_min_t = torch.stack(u_min)
    u_first_t = torch.stack(u_first)
    u_max_t = torch.stack(u_max)
    v_median_t = torch.stack(v_median)

    return {
        "basis": basis,
        "eigenvalues": eigenvalues,
        "mean": mean.float(),
        "global_start": u_min_t.median(),
        "global_first": u_first_t.median(),
        "global_end": u_max_t.median(),
        "transverse_center": v_median_t.median(),
        "summary": {
            "u_min": summarize_tensor(u_min_t),
            "u_first": summarize_tensor(u_first_t),
            "u_max": summarize_tensor(u_max_t),
            "v_median": summarize_tensor(v_median_t),
        },
    }


def summarize_tensor(values: torch.Tensor) -> dict:
    values = values.float()
    return {
        "min": float(values.min()),
        "p10": float(torch.quantile(values, 0.10)),
        "median": float(values.median()),
        "p90": float(torch.quantile(values, 0.90)),
        "max": float(values.max()),
    }


def per_event_pca_project(coords: torch.Tensor) -> torch.Tensor:
    """Project 3D coordinates onto per-event PCA axes.

    The first image axis follows the largest-variance direction. The third PCA
    axis is collapsed, which is a closer toy analogue of choosing a useful wire
    view than blindly dropping x/y/z.
    """
    points = coords.float()
    center = points.mean(dim=0)
    basis = pca_basis_from_coords(coords)[:, :2]

    # PCA fixes an axis, not a direction. Orient the main axis so the cloud mean
    # lies downstream of the chosen start point.
    start = points[0]
    if torch.dot(center - start, basis[:, 0]) < 0:
        basis[:, 0] *= -1

    return points @ basis


def fixed_axis_project(coords: torch.Tensor, keep_axes: tuple[int, int]) -> torch.Tensor:
    points = coords[:, keep_axes].float()
    return points - points[0]


def detector_project(
    coords: torch.Tensor,
    start_axis: int,
    transverse_axis: int,
    start_plane: float,
    transverse_start_plane: float,
    travel_sign: str,
    crop_size: float,
    margin: float,
) -> tuple[torch.Tensor, torch.Tensor]:
    """Project with fixed detector axes and fixed start coordinates."""
    sign = 1.0 if travel_sign == "positive" else -1.0
    longitudinal = sign * (coords[:, start_axis].float() - start_plane)
    transverse = coords[:, transverse_axis].float()
    points = torch.stack((longitudinal, transverse), dim=1)

    lower = torch.empty(2, dtype=torch.float32)
    lower[0] = -crop_size * margin
    lower[1] = transverse_start_plane - crop_size / 2
    return points, lower


def crop_lower_corner(points_2d: torch.Tensor, crop_size: float, margin: float) -> torch.Tensor:
    """Place the start point near the low edge of the PCA/fixed image."""
    lower = torch.empty(2, dtype=torch.float32)
    lower[0] = -crop_size * margin
    lower[1] = points_2d[:, 1].median() - crop_size / 2
    return lower


def crop_lower_with_geometry(
    points_2d: torch.Tensor,
    coords: torch.Tensor,
    crop_size: float,
    margin: float,
    anchor_mode: str,
    transverse_center_mode: str,
    geometry: dict | None,
    start_axis: int,
    start_plane: float,
    travel_sign: str,
) -> torch.Tensor:
    lower = torch.empty(2, dtype=torch.float32)
    if anchor_mode == "start-plane":
        sign = 1.0 if travel_sign == "positive" else -1.0
        longitudinal = sign * (coords[:, start_axis].float() - start_plane)
        start_u = torch.tensor(0.0, dtype=torch.float32)
        points_2d[:, 0] = longitudinal
    elif anchor_mode == "global-start":
        if geometry is None:
            raise ValueError("--anchor-mode global-start requires global PCA geometry.")
        start_u = geometry["global_start"]
    elif anchor_mode == "first":
        start_u = points_2d[0, 0]
    else:
        start_u = points_2d[:, 0].min()

    if transverse_center_mode == "global":
        if geometry is None:
            transverse_center = points_2d[:, 1].median()
        else:
            transverse_center = geometry["transverse_center"]
    elif transverse_center_mode == "local-slice":
        in_longitudinal_window = (points_2d[:, 0] >= start_u) & (
            points_2d[:, 0] < start_u + crop_size
        )
        if bool(in_longitudinal_window.any()):
            transverse_center = points_2d[in_longitudinal_window, 1].median()
        else:
            transverse_center = points_2d[:, 1].median()
    else:
        transverse_center = points_2d[:, 1].median()

    lower[0] = start_u - crop_size * margin
    lower[1] = transverse_center - crop_size / 2
    return lower


def make_image(
    coords: torch.Tensor,
    features: torch.Tensor,
    keep_axes: tuple[int, int],
    projection_mode: str,
    anchor_mode: str,
    transverse_center_mode: str,
    geometry: dict | None,
    image_size: int,
    crop_size: float,
    start_margin: float,
    use_log1p: bool,
    clip_mev: float,
    feature_column: int,
    feature_mev_scale: float,
    feature_min: float,
    feature_max: float,
    start_axis: int,
    start_plane: float,
    travel_sign: str,
    transverse_axis: int,
    transverse_start_plane: float,
) -> torch.Tensor | None:
    """Project one sparse 3D tensor into one dense [1, H, W] image."""
    if projection_mode == "detector":
        points, lower = detector_project(
            coords=coords,
            start_axis=start_axis,
            transverse_axis=transverse_axis,
            start_plane=start_plane,
            transverse_start_plane=transverse_start_plane,
            travel_sign=travel_sign,
            crop_size=crop_size,
            margin=start_margin,
        )
    elif projection_mode == "global-pca":
        if geometry is None:
            raise ValueError("--projection-mode global-pca requires global PCA geometry.")
        points = coords.float() @ geometry["basis"][:, :2]
        lower = crop_lower_with_geometry(
            points,
            coords,
            crop_size,
            start_margin,
            anchor_mode,
            transverse_center_mode,
            geometry,
            start_axis,
            start_plane,
            travel_sign,
        )
    elif projection_mode == "per-event-pca":
        points = per_event_pca_project(coords)
        lower = crop_lower_with_geometry(
            points,
            coords,
            crop_size,
            start_margin,
            anchor_mode,
            transverse_center_mode,
            geometry,
            start_axis,
            start_plane,
            travel_sign,
        )
    else:
        points = fixed_axis_project(coords, keep_axes=keep_axes)
        lower = crop_lower_with_geometry(
            points,
            coords,
            crop_size,
            start_margin,
            anchor_mode,
            transverse_center_mode,
            geometry,
            start_axis,
            start_plane,
            travel_sign,
        )
    rel = (points - lower) / crop_size
    pix = torch.floor(rel * image_size).long()

    inside = (pix[:, 0] >= 0) & (pix[:, 0] < image_size)
    inside &= (pix[:, 1] >= 0) & (pix[:, 1] < image_size)
    if not bool(inside.any()):
        return None

    pix = pix[inside]
    selected_features = features
    if selected_features.ndim > 1:
        selected_features = selected_features[:, feature_column]
    raw_charge = selected_features[inside].float().view(-1)
    valid_charge = (raw_charge >= feature_min) & (raw_charge <= feature_max)
    if not bool(valid_charge.any()):
        return None

    pix = pix[valid_charge]
    charge = raw_charge[valid_charge] * feature_mev_scale
    if use_log1p:
        charge = torch.log1p(charge)

    image = torch.zeros((image_size, image_size), dtype=torch.float32)
    # Same-pixel deposits are summed here. Longitudinal coordinate maps to
    # columns so particles travel left-to-right; transverse maps to rows.
    image.index_put_((pix[:, 1], pix[:, 0]), charge, accumulate=True)

    image = (image / clip_mev).clamp(0, 1)
    return image.unsqueeze(0)


def save_pgm(path: Path, image: torch.Tensor) -> None:
    """Write a grayscale preview using only the Python stdlib + torch."""
    arr = (image.squeeze(0).clamp(0, 1) * 255).round().to(torch.uint8).cpu()
    height, width = arr.shape
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as fout:
        fout.write(f"P5\n{width} {height}\n255\n".encode("ascii"))
        fout.write(bytes(arr.flatten().tolist()))


def preview_name(index: int, pdg: int, energy_gev: float, class_name: str) -> str:
    energy_mev = int(round(1000.0 * energy_gev))
    return f"preview_{index:04d}_{class_name}_pdg{pdg}_ekin{energy_mev:04d}MeV.pgm"


def main() -> None:
    args = parse_args()
    input_paths = input_paths_from_args(args)
    keep_axes = projected_axes(args.drop_axis)
    start_axis = axis_index(args.start_axis)
    transverse_axis = axis_index(args.transverse_axis)
    if args.projection_mode == "detector" and transverse_axis == start_axis:
        raise ValueError("--transverse-axis must differ from --start-axis in detector mode.")
    crop_size = (
        args.crop_size
        if args.crop_size is not None
        else args.image_side_cm / args.voxel_size_cm
    )
    if args.start_padding_cm is None:
        start_padding_bins = args.start_padding_pixels * crop_size / args.image_size
    else:
        start_padding_bins = args.start_padding_cm / args.voxel_size_cm
    start_margin = start_padding_bins / crop_size
    pixel_size_cm = crop_size * args.voxel_size_cm / args.image_size
    clip_mev = (
        args.clip_mev
        if args.clip_mev is not None
        else args.mip_multiplier * args.mip_dedx_mev_per_cm * pixel_size_cm
    )
    geometry = None
    if args.projection_mode == "global-pca":
        geometry = global_pca_geometry(
            input_paths,
            args.energy_min,
            args.energy_max,
            args.max_per_class,
            args.progress,
        )

    images: list[torch.Tensor] = []
    labels: list[int] = []
    energies: list[float] = []
    pdgs: list[int] = []
    indices: list[int] = []

    seen = 0
    skipped_energy = 0
    skipped_class_full = 0
    skipped_files_class_full = 0
    skipped_empty = 0
    selected_per_class: Counter[str] = Counter()

    for input_path in progress_bar(input_paths, "Converting", args.progress):
        chunk = torch.load(input_path, map_location="cpu", weights_only=False)
        chunk_name = chunk_particle_name(chunk, input_path)
        if args.max_per_class is not None and selected_per_class[chunk_name] >= args.max_per_class:
            skipped_files_class_full += 1
            continue
        for event in chunk:
            seen += 1
            energy = float(event["target"])
            if energy < args.energy_min or energy > args.energy_max:
                skipped_energy += 1
                continue

            pdg = int(event["pdg"])
            name = particle_name(pdg)
            if name not in CLASS_TO_LABEL:
                raise ValueError(f"Unsupported particle PDG {pdg} ({name}).")
            if (
                args.max_per_class is not None
                and selected_per_class[name] >= args.max_per_class
            ):
                skipped_class_full += 1
                break

            image = make_image(
                coords=event["coords"],
                features=event["features"],
                keep_axes=keep_axes,
                projection_mode=args.projection_mode,
                anchor_mode=args.anchor_mode,
                transverse_center_mode=args.transverse_center_mode,
                geometry=geometry,
                image_size=args.image_size,
                crop_size=crop_size,
                start_margin=start_margin,
                use_log1p=args.log1p,
                clip_mev=clip_mev,
                feature_column=args.feature_column,
                feature_mev_scale=args.feature_mev_scale,
                feature_min=args.feature_min,
                feature_max=args.feature_max,
                start_axis=start_axis,
                start_plane=args.start_plane,
                travel_sign=args.travel_sign,
                transverse_axis=transverse_axis,
                transverse_start_plane=args.transverse_start_plane,
            )
            if image is None:
                skipped_empty += 1
                continue

            images.append(image)
            labels.append(CLASS_TO_LABEL[name])
            energies.append(energy)
            pdgs.append(pdg)
            indices.append(int(event["index"]))
            selected_per_class[name] += 1

            if args.max_events is not None and len(images) >= args.max_events:
                break
            if (
                args.max_per_class is not None
                and selected_per_class[name] >= args.max_per_class
            ):
                break
        if args.max_events is not None and len(images) >= args.max_events:
            break
        if caps_reached(selected_per_class, CLASS_NAMES, args.max_per_class):
            break

    if not images:
        raise RuntimeError("No images survived the requested cuts.")

    image_tensor = torch.stack(images)
    label_tensor = torch.tensor(labels, dtype=torch.long)
    energy_tensor = torch.tensor(energies, dtype=torch.float32)
    pdg_tensor = torch.tensor(pdgs, dtype=torch.long)
    index_tensor = torch.tensor(indices, dtype=torch.long)

    if args.shuffle:
        generator = torch.Generator().manual_seed(args.shuffle_seed)
        permutation = torch.randperm(len(label_tensor), generator=generator)
        image_tensor = image_tensor[permutation].contiguous()
        label_tensor = label_tensor[permutation].contiguous()
        energy_tensor = energy_tensor[permutation].contiguous()
        pdg_tensor = pdg_tensor[permutation].contiguous()
        index_tensor = index_tensor[permutation].contiguous()

    payload = {
        "images": image_tensor,
        "labels": label_tensor,
        "energies_gev": energy_tensor,
        "pdgs": pdg_tensor,
        "indices": index_tensor,
        "class_names": CLASS_NAMES,
        "class_to_label": CLASS_TO_LABEL,
        "selected_per_class": dict(selected_per_class),
        "config": {
            "input_count": len(input_paths),
            "image_size": args.image_size,
            "crop_size": crop_size,
            "image_side_cm": args.image_side_cm,
            "voxel_size_cm": args.voxel_size_cm,
            "drop_axis": args.drop_axis,
            "projected_axes": keep_axes,
            "projection_mode": args.projection_mode,
            "anchor_mode": args.anchor_mode,
            "start_axis": args.start_axis,
            "start_plane": args.start_plane,
            "travel_sign": args.travel_sign,
            "transverse_axis": args.transverse_axis,
            "transverse_start_plane": args.transverse_start_plane,
            "transverse_center_mode": args.transverse_center_mode,
            "start_padding_cm": args.start_padding_cm,
            "start_padding_pixels": args.start_padding_pixels,
            "start_padding_bins": start_padding_bins,
            "start_margin": start_margin,
            "energy_min": args.energy_min,
            "energy_max": args.energy_max,
            "max_events": args.max_events,
            "max_per_class": args.max_per_class,
            "shuffle": args.shuffle,
            "shuffle_seed": args.shuffle_seed,
            "progress": args.progress,
            "log1p": args.log1p,
            "clip_mev": clip_mev,
            "feature_column": args.feature_column,
            "pixel_size_cm": pixel_size_cm,
            "mip_dedx_mev_per_cm": args.mip_dedx_mev_per_cm,
            "mip_multiplier": args.mip_multiplier,
            "feature_mev_scale": args.feature_mev_scale,
            "feature_min": args.feature_min,
            "feature_max": args.feature_max,
            "global_pca": None
            if geometry is None
            else {
                "mean": geometry["mean"].tolist(),
                "basis": geometry["basis"].tolist(),
                "eigenvalues": geometry["eigenvalues"].tolist(),
                "global_start": float(geometry["global_start"]),
                "global_first": float(geometry["global_first"]),
                "global_end": float(geometry["global_end"]),
                "transverse_center": float(geometry["transverse_center"]),
                "summary": geometry["summary"],
            },
        },
    }

    args.output.parent.mkdir(parents=True, exist_ok=True)
    torch.save(payload, args.output)

    for i, image in enumerate(payload["images"][: args.num_previews]):
        label = int(payload["labels"][i])
        pdg = int(payload["pdgs"][i])
        energy = float(payload["energies_gev"][i])
        name = preview_name(i, pdg, energy, CLASS_NAMES[label])
        save_pgm(args.preview_dir / name, image)

    print(f"Read events        : {seen}")
    print(f"Skipped by energy  : {skipped_energy}")
    print(f"Skipped class full : {skipped_class_full}")
    print(f"Skipped full files : {skipped_files_class_full}")
    print(f"Skipped empty crop : {skipped_empty}")
    print(f"Wrote images       : {len(images)}")
    print(f"Selected per class : {dict(selected_per_class)}")
    print(f"Image tensor shape : {tuple(payload['images'].shape)}")
    print(f"Projection mode    : {args.projection_mode}")
    print(f"Crop size          : {crop_size} source bins")
    print(f"Pixel size         : {pixel_size_cm} cm")
    print(f"Clip scale         : {clip_mev} MeV")
    print(f"Shuffle            : {args.shuffle} (seed={args.shuffle_seed})")
    if geometry is not None:
        print("Global PC1 xyz     :", [round(v, 6) for v in geometry["basis"][:, 0].tolist()])
        print("Global PC2 xyz     :", [round(v, 6) for v in geometry["basis"][:, 1].tolist()])
        print("Global PC3 xyz     :", [round(v, 6) for v in geometry["basis"][:, 2].tolist()])
        print("PC1 start summary  :", geometry["summary"]["u_min"])
    print(f"Classes            : {CLASS_NAMES}")
    print(f"Output             : {args.output}")
    print(f"Previews           : {args.preview_dir}")


if __name__ == "__main__":
    main()
