"""Based on gltf.converter."""
import base64
import dataclasses
import itertools
import math
import os

from typing import Optional, Union

from . import spec
from .mixins import MaterialMixin, MeshMixin, NodeMixin, SpringMixin, TextureMixin
from direct.stdpy.file import open as dopen
from panda3d import bullet, core as p3d


@dataclasses.dataclass
class GltfSettings:
    skip_axis_conversion: bool = False
    no_srgb: bool = False
    textures: str = 'ref'
    flatten_nodes: bool = False
    spring_bones: str = 'wigglebone'


class Converter(MaterialMixin, MeshMixin, NodeMixin, SpringMixin, TextureMixin):
    def __init__(
            self,
            filepath: Union[str, p3d.Filename],
            settings: Optional[GltfSettings] = None):
        if not isinstance(filepath, p3d.Filename):
            filepath = p3d.Filename.from_os_specific(filepath)
        if settings is None:
            settings = GltfSettings()
        self.filepath = filepath
        self.filedir = p3d.Filename(filepath.get_dirname())
        self.settings = settings
        self.cameras = {}
        self.buffers = {}
        self.lights = {}
        self.textures = {}
        self.mat_states = {}
        self.mat_mesh_map = {}
        self.meshes = {}
        self.node_paths = {}
        self.scenes = {}
        self.skeletons = {}
        self.joints = {}
        self.character_joints = {}
        self.springid = 0
        self.spring_bones = {}

        # Coordinate system transform matrix
        self.csxform = p3d.LMatrix4.convert_mat(p3d.CS_yup_right, p3d.CS_default)
        self.csxform_inv = p3d.LMatrix4.convert_mat(p3d.CS_default, p3d.CS_yup_right)
        self.compose_cs = p3d.CS_yup_right

        # Scene props
        self.active_scene = p3d.NodePath(p3d.ModelRoot('default'))
        self.background_color = (0, 0, 0)
        self.active_camera = None

    def update(self, gltf_data):
        skip_axis_conversion = (
            'extensionsUsed' in gltf_data and 'BP_zup' in gltf_data['extensionsUsed'] or
            self.settings.skip_axis_conversion
        )

        if skip_axis_conversion:
            self.csxform = p3d.LMatrix4.ident_mat()
            self.csxform_inv = p3d.LMatrix4.ident_mat()
            self.compose_cs = p3d.CS_zup_right

        # Convert data
        for buffid, gltf_buffer in enumerate(gltf_data.get('buffers', [])):
            self.load_buffer(buffid, gltf_buffer)

        for camid, gltf_cam in enumerate(gltf_data.get('cameras', [])):
            self.load_camera(camid, gltf_cam)

        if self.settings.spring_bones == 'wigglebone':
            if 'secondaryAnimation' in gltf_data.get('extensions', {}).get('VRM', {}):
                secondary_animation = gltf_data['extensions']['VRM']['secondaryAnimation']
                for chainid, vrm_chain in enumerate(secondary_animation.get('boneGroups', [])):
                    for nodeid in vrm_chain.get('bones'):
                        self.spring_bones[nodeid] = vrm_chain

        if 'extensions' in gltf_data and 'KHR_lights' in gltf_data['extensions']:
            lights = gltf_data['extensions']['KHR_lights'].get('lights', [])
            for lightid, gltf_light in enumerate(lights):
                self.load_light(lightid, gltf_light)

        if 'extensions' in gltf_data and 'KHR_lights_punctual' in gltf_data['extensions']:
            lights = gltf_data['extensions']['KHR_lights_punctual'].get('lights', [])
            for lightid, gltf_light in enumerate(lights):
                self.load_light(lightid, gltf_light, punctual=True)

        for texid, gltf_tex in enumerate(gltf_data.get('textures', [])):
            self.load_texture(texid, gltf_tex, gltf_data)

        for matid, gltf_mat in enumerate(gltf_data.get('materials', [])):
            self.load_material(matid, gltf_mat)

        for skinid, gltf_skin in enumerate(gltf_data.get('skins', [])):
            self.load_skin(skinid, gltf_skin, gltf_data)

        for sceneid, gltf_scene in enumerate(gltf_data.get('scenes', [])):
            for nodeid in gltf_scene.get('nodes', []):
                gltf_node = gltf_data['nodes'][nodeid]
                self.build_character_joints(nodeid, gltf_node, gltf_data)

        for meshid, gltf_mesh in enumerate(gltf_data.get('meshes', [])):
            self.load_mesh(meshid, gltf_mesh, gltf_data)

        # Build scenegraphs
        for sceneid, gltf_scene in enumerate(gltf_data.get('scenes', [])):
            scene_name = gltf_scene.get('name', 'scene'+str(sceneid))
            scene_root = p3d.NodePath(p3d.ModelRoot(scene_name))

            node_list = gltf_scene['nodes']
            hidden_nodes = self.get_extras(gltf_scene).get('hidden_nodes', [])
            node_list += hidden_nodes

            for nodeid in node_list:
                self.add_node(scene_root, gltf_scene, nodeid, gltf_data)

            if self.settings.flatten_nodes:
                scene_root.flatten_medium()

            self.scenes[sceneid] = scene_root

        for nodeid, gltf_node in enumerate(gltf_data.get('nodes', [])):
            self.move_skinned_nodes(nodeid, gltf_node, gltf_data)

        if self.settings.spring_bones == 'bullet':
            if 'secondaryAnimation' in gltf_data.get('extensions', {}).get('VRM', {}):
                secondary_animation = gltf_data['extensions']['VRM']['secondaryAnimation']
                for chainid, vrm_chain in enumerate(secondary_animation.get('boneGroups', [])):
                    self.load_bullet_spring_chain(chainid, vrm_chain, gltf_data)

        # Set the active scene
        sceneid = gltf_data.get('scene', 0)
        if sceneid in self.scenes:
            self.active_scene = self.scenes[sceneid]
        if 'scenes' in gltf_data:
            gltf_scene = gltf_data['scenes'][sceneid]
            scene_extras = self.get_extras(gltf_scene)
            if 'background_color' in scene_extras:
                self.background_color = scene_extras['background_color']
            if 'active_camera' in scene_extras:
                self.active_camera = scene_extras['active_camera']

    def load_matrix(self, mat):
        lmat = p3d.LMatrix4()

        for i in range(4):
            lmat.set_row(i, p3d.LVecBase4(*mat[i * 4: i * 4 + 4]))
        return lmat

    def load_buffer(self, buffid, gltf_buffer):
        if 'uri' not in gltf_buffer:
            assert self.buffers[buffid]
            return

        uri = gltf_buffer['uri']
        if uri == '_glb_bin' and buffid == 0:
            buff_data = gltf_buffer['_glb_bin']
        elif uri.startswith('data:application/octet-stream;base64') or \
           uri.startswith('data:application/gltf-buffer;base64'):
            buff_data = gltf_buffer['uri'].split(',')[1]
            buff_data = base64.b64decode(buff_data)
        elif uri.endswith('.bin'):
            buff_fname = os.path.join(self.filedir.to_os_specific(), uri)
            with dopen(buff_fname, 'rb') as buff_file:
                buff_data = buff_file.read(gltf_buffer['byteLength'])
        else:
            print(
                "Buffer {} has an unsupported uri ({}), using a zero filled buffer instead"
                .format(buffid, uri)
            )
            buff_data = bytearray(gltf_buffer['byteLength'])
        self.buffers[buffid] = buff_data

    def get_buffer_view(self, gltf_data, view_id):
        buffview = gltf_data['bufferViews'][view_id]
        buff = self.buffers[buffview['buffer']]
        start = buffview.get('byteOffset', 0)
        end = start + buffview['byteLength']
        if 'byteStride' in buffview:
            return memoryview(buff)[start:end:buffview['byteStride']]
        else:
            return memoryview(buff)[start:end]

    def load_skin(self, skinid, gltf_skin, gltf_data):
        # Find a common root node.

        # First gather the parents of each node.
        parents = [None] * len(gltf_data['nodes'])  # child to parent map
        for i, node in enumerate(gltf_data['nodes']):
            for child in node.get('children', ()):
                parents[child] = i

        # Now create a path for each joint node and the skeleton.
        paths = []
        for i, gltf_node in enumerate(gltf_data['nodes']):
            if i in gltf_skin['joints'] or i == gltf_skin.get('skeleton'):
                path = [i]
                while parents[i] is not None:
                    i = parents[i]
                    path.insert(0, i)
                paths.append(tuple(path))

        # Find the longest prefix that is shared by all paths.
        common_path = paths[0]
        for path in paths[1:]:
            path = list(path[:len(common_path)])
            while path:
                if common_path[:len(path)] == tuple(path):
                    common_path = tuple(path)
                    break

                path.pop()

        try:
            root_nodeid = common_path[-2]

        except IndexError:
            nodeid = common_path[-1]
            root_nodeid = len(gltf_data['nodes'])
            gltf_node = {
                'children': [nodeid],
            }
            gltf_data['nodes'].append(gltf_node)
            for gltf_scene in gltf_data['scenes']:
                if nodeid in gltf_scene['nodes']:
                    gltf_scene['nodes'].remove(nodeid)
                    gltf_scene['nodes'].append(root_nodeid)

        self.skeletons[root_nodeid] = skinid
        for jointid in gltf_skin['joints']:
            self.joints[jointid] = None

    def load_camera(self, camid, gltf_camera):
        camname = gltf_camera.get('name', 'cam'+str(camid))
        node = self.cameras.get(camid, p3d.Camera(camname))

        if gltf_camera['type'] == 'perspective':
            gltf_lens = gltf_camera['perspective']
            lens = p3d.PerspectiveLens()
            aspect_ratio = gltf_lens.get(
                'aspectRatio',
                lens.get_aspect_ratio()
            )
            lens.set_fov(math.degrees(gltf_lens['yfov'] * aspect_ratio), math.degrees(gltf_lens['yfov']))
            lens.set_near_far(gltf_lens['znear'], gltf_lens['zfar'])
            lens.set_view_vector((0, 0, -1), (0, 1, 0))
            node.set_lens(lens)

        self.cameras[camid] = node

    def load_light(self, lightid, gltf_light, punctual=False):
        node = self.lights.get(lightid, None)
        lightname = gltf_light.get('name', 'light'+str(lightid))

        ltype = gltf_light['type']
        # Construct a new light if needed
        if node is None:
            if ltype == 'point':
                node = p3d.PointLight(lightname)
            elif ltype == 'directional':
                node = p3d.DirectionalLight(lightname)
            elif ltype == 'spot':
                node = p3d.Spotlight(lightname)
            else:
                print(
                    "Unsupported light type for light with name {}: {}"
                    .format(lightname, gltf_light['type']))
                node = p3d.PandaNode(lightname)

        # Update the light
        if punctual:
            # For PBR, attention should always be (1, 0, 1)
            if hasattr(node, 'attenuation'):
                node.attenuation = p3d.LVector3(1, 0, 1)

            if 'color' in gltf_light:
                node.set_color(
                    p3d.LColor(*gltf_light['color'], w=1) *
                    gltf_light.get('intensity', 1))
            if 'range' in gltf_light:
                node.max_distance = gltf_light['range']
            if ltype == 'spot':
                spot = gltf_light.get('spot', {})
                inner = spot.get('innerConeAngle', 0)
                outer = spot.get('outerConeAngle', math.pi / 4)
                fov = math.degrees(outer) * 2
                node.get_lens().set_fov(fov, fov)

                if inner >= outer:
                    node.exponent = 0
                else:
                    # The value of exp was chosen empirically to give a smooth
                    # cutoff without straying too far from the spec; higher
                    # exponents will have a smoother cutoff but sharper falloff.
                    exp = 8 / 3
                    node.exponent = 2 * (math.pi * 0.5 / outer) ** exp
        else:
            if ltype == 'unsupported':
                lightprops = {}
            else:
                lightprops = gltf_light[ltype]

            if ltype in ('point', 'directional', 'spot'):
                node.set_color(p3d.LColor(*lightprops['color'], w=1))

            if ltype in ('point', 'spot'):
                att = p3d.LPoint3(
                    lightprops['constantAttenuation'],
                    lightprops['linearAttenuation'],
                    lightprops['quadraticAttenuation']
                )
                node.set_attenuation(att)

        self.lights[lightid] = node

    def get_extras(self, gltf_data):
        extras = gltf_data.get('extras', {})
        if not isinstance(extras, dict):
            # weird, but legal; fail silently for now
            return {}
        return extras
