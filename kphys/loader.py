"""Based on gltf.loader."""
from typing import Optional, Union

import panda3d.core as p3d

from .gltf.converter import Converter, GltfSettings
from .gltf.parseutils import parse_gltf_file


def load_model(
        file_path: Optional[Union[str, p3d.Filename]] = None,
        gltf_data: Optional[dict] = None,
        gltf_settings: Optional[GltfSettings] = None,
        converter_class: Optional[Converter] = None) -> p3d.ModelRoot:
    """
    Load a glTF file from file_path and return a ModelRoot.
    Based on gltf.loader.load_model() function.

    :param file_path: path to file
    :type file_path: str

    :param gltf_data: optional altered glTF data
    :type gltf_data: dict

    :param gltf_settings: optional converter settings
    :type gltf_settings: :class: `GltfSettings`

    :param converter_class: optional converter class override
    :type converter_class: :class: `Converter`
    """
    if converter_class is None:
        converter_class = Converter
    converter = converter_class(file_path, settings=gltf_settings)

    if gltf_data is None:
        gltf_data = parse_gltf_file(file_path)
    converter.update(gltf_data)

    return converter.active_scene.node()
