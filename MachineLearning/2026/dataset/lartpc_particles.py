"""MNIST-like dataset wrapper for tiny LArTPC particle images.

Expected raw file format
------------------------
The converter in ``convert_lartpc_images.py`` writes a ``torch.save`` payload
with at least:

    images: FloatTensor [N, 1, H, W]
    labels: LongTensor [N]

Optional metadata fields such as ``class_names``, ``pdgs`` and ``energies_gev``
are preserved but not required for the MNIST-like interface.
"""

from __future__ import annotations

import shutil
import urllib.request
from pathlib import Path
from typing import Callable

import torch
from torch.utils.data import Dataset


class LArTPCParticles(Dataset):
    """Dataset with the same basic access pattern as ``torchvision.datasets.MNIST``.

    Each item is returned as ``(image, label)`` where ``image`` is a
    ``torch.FloatTensor`` with shape ``[1, H, W]`` and ``label`` is an ``int``.

    Label convention:
        0: electron
        1: muon
        2: pion
        3: proton
        4: kaon
    """

    class_names = ["electron", "muon", "pion", "proton", "kaon"]
    class_to_idx = {name: index for index, name in enumerate(class_names)}
    idx_to_class = {index: name for name, index in class_to_idx.items()}

    raw_filename = "lartpc_particles.pt"
    training_file = "training.pt"
    test_file = "test.pt"

    def __init__(
        self,
        root: str | Path,
        train: bool = True,
        transform: Callable | None = None,
        target_transform: Callable | None = None,
        download: bool = False,
        url: str | None = None,
        filename: str | None = None,
        split_fraction: float = 0.8,
        split_seed: int = 12345,
    ) -> None:
        self.root = Path(root)
        self.train = train
        self.transform = transform
        self.target_transform = target_transform
        self.url = url
        self.raw_filename = filename or self.raw_filename
        self.split_fraction = split_fraction
        self.split_seed = split_seed

        if download:
            self.download()

        if not self._check_exists() and self.raw_path.exists():
            self.process()

        if not self._check_exists():
            raise RuntimeError(
                "Dataset not found. Use download=True with a URL, or place "
                f"{self.raw_filename!r} in {self.raw_folder} and call process()."
            )

        payload = torch.load(self.processed_paths[0 if train else 1], map_location="cpu")
        self.data = payload["images"].float()
        self.targets = payload["labels"].long()
        self.metadata = {key: value for key, value in payload.items() if key not in {"images", "labels"}}

    @property
    def base_folder(self) -> Path:
        return self.root

    @property
    def raw_folder(self) -> Path:
        return self.base_folder / "raw"

    @property
    def processed_folder(self) -> Path:
        return self.base_folder / "processed"

    @property
    def raw_path(self) -> Path:
        return self.raw_folder / self.raw_filename

    @property
    def processed_paths(self) -> tuple[Path, Path]:
        return (
            self.processed_folder / self.training_file,
            self.processed_folder / self.test_file,
        )

    def __len__(self) -> int:
        return len(self.targets)

    def __getitem__(self, index: int) -> tuple[torch.Tensor, int]:
        image = self.data[index]
        target = int(self.targets[index])

        if self.transform is not None:
            image = self.transform(image)
        if self.target_transform is not None:
            target = self.target_transform(target)

        return image, target

    def _check_exists(self) -> bool:
        return all(path.exists() for path in self.processed_paths)

    def download(self) -> None:
        """Fetch raw data if needed, then create processed train/test files."""
        if self._check_exists():
            return

        self.raw_folder.mkdir(parents=True, exist_ok=True)
        self.processed_folder.mkdir(parents=True, exist_ok=True)

        if not self.raw_path.exists():
            if self.url is None:
                raise RuntimeError(
                    f"Raw dataset file {self.raw_path} is missing and no URL was provided."
                )
            self._fetch(self.url, self.raw_path)

        self.process()

    def process(self) -> None:
        """Create MNIST-like processed train/test payloads from one raw payload."""
        self.processed_folder.mkdir(parents=True, exist_ok=True)
        payload = torch.load(self.raw_path, map_location="cpu")

        if "train" in payload and "test" in payload:
            train_payload = self._normalize_payload(payload["train"], payload)
            test_payload = self._normalize_payload(payload["test"], payload)
        else:
            images = payload["images"].float()
            labels = payload["labels"].long()
            permutation = torch.randperm(len(labels), generator=torch.Generator().manual_seed(self.split_seed))
            split_index = int(round(self.split_fraction * len(labels)))
            train_indices = permutation[:split_index]
            test_indices = permutation[split_index:]
            train_payload = self._subset_payload(payload, images, labels, train_indices)
            test_payload = self._subset_payload(payload, images, labels, test_indices)

        torch.save(train_payload, self.processed_paths[0])
        torch.save(test_payload, self.processed_paths[1])

    @staticmethod
    def _fetch(url: str, destination: Path) -> None:
        if url.startswith("file://"):
            shutil.copyfile(Path(url[7:]), destination)
        elif "://" not in url:
            shutil.copyfile(Path(url), destination)
        else:
            urllib.request.urlretrieve(url, destination)

    @classmethod
    def _normalize_payload(cls, split_payload: dict, parent_payload: dict) -> dict:
        output = {
            "images": split_payload["images"].float(),
            "labels": split_payload["labels"].long(),
            "class_names": parent_payload.get("class_names", cls.class_names),
            "class_to_label": parent_payload.get("class_to_label", cls.class_to_idx),
        }
        for key, value in split_payload.items():
            if key not in output:
                output[key] = value
        return output

    @classmethod
    def _subset_payload(
        cls,
        payload: dict,
        images: torch.Tensor,
        labels: torch.Tensor,
        indices: torch.Tensor,
    ) -> dict:
        output = {
            "images": images[indices].contiguous(),
            "labels": labels[indices].contiguous(),
            "class_names": payload.get("class_names", cls.class_names),
            "class_to_label": payload.get("class_to_label", cls.class_to_idx),
        }
        for key in ("energies_gev", "pdgs", "indices"):
            if key in payload:
                output[key] = payload[key][indices].contiguous()
        if "config" in payload:
            output["config"] = payload["config"]
        return output
