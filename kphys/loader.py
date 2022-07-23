import json
import os
import struct

from panda3d.core import (
    get_model_path, Filename, LMatrix4, LQuaternion,
    NodePath, ModelRoot, PandaNode, TransformState)

from gltf.converter import read_glb_chunk, Converter, GltfSettings

from .core import ArmatureNode, BoneNode


class KPhysConverter(Converter):
    def __init__(self, *args, **kw):
        super().__init__(*args, **kw)
        self._joint_ids = {}  # node id to joint index mapping

    def build_character_sliders(self, *args, **kw):
        return {}

    def update(self, gltf_data, writing_bam=False):
        #pprint.pprint(gltf_data)

        skip_axis_conversion = (
            'extensionsUsed' in gltf_data and 'BP_zup' in gltf_data['extensionsUsed'] or
            self.settings.skip_axis_conversion
        )

        if skip_axis_conversion:
            self.csxform = LMatrix4.ident_mat()
            self.csxform_inv = LMatrix4.ident_mat()
            self.compose_cs = CS_zup_right

        # Convert data
        for buffid, gltf_buffer in enumerate(gltf_data.get('buffers', [])):
            self.load_buffer(buffid, gltf_buffer)

        for camid, gltf_cam in enumerate(gltf_data.get('cameras', [])):
            self.load_camera(camid, gltf_cam)

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
        self.load_fallback_texture()

        for matid, gltf_mat in enumerate(gltf_data.get('materials', [])):
            self.load_material(matid, gltf_mat)

        for skinid, gltf_skin in enumerate(gltf_data.get('skins', [])):
            self.load_skin(skinid, gltf_skin, gltf_data)

        for meshid, gltf_mesh in enumerate(gltf_data.get('meshes', [])):
            self.load_mesh(meshid, gltf_mesh, gltf_data)

        # If we support writing bam 6.40, we can safely write out
        # instanced lights.  If not, we have to copy it.
        copy_lights = writing_bam and not hasattr(BamWriter, 'root_node')

        # Build scenegraphs
        def add_node(root, gltf_scene, nodeid, jvtmap, cvsmap):
            try:
                gltf_node = gltf_data['nodes'][nodeid]
            except IndexError:
                print("Could not find node with index: {}".format(nodeid))
                return

            node_name = gltf_node.get('name', 'node'+str(nodeid))
            if nodeid in self._joint_nodes:
                # Handle non-joint children of joints, but don't add joints themselves
                for child_nodeid in gltf_node.get('children', []):
                    add_node(root, gltf_scene, child_nodeid, jvtmap, cvsmap)
                return

            jvtmap = dict(jvtmap)
            cvsmap = dict(cvsmap)

            if nodeid in self.skeletons:
                # This node is the root of an animated character.
                # panda_node = CharacterNode(node_name)
                panda_node = ArmatureNode(node_name)
            elif nodeid in self._joint_ids:
                panda_node = BoneNode(node_name, self._joint_ids[nodeid])
            else:
                panda_node = PandaNode(node_name)

            # Determine the transformation.
            if 'matrix' in gltf_node:
                gltf_mat = LMatrix4(*gltf_node.get('matrix'))
            else:
                gltf_mat = LMatrix4(LMatrix4.ident_mat())
                if 'scale' in gltf_node:
                    gltf_mat.set_scale_mat(tuple(gltf_node['scale']))

                if 'rotation' in gltf_node:
                    rot_mat = LMatrix4()
                    rot = gltf_node['rotation']
                    quat = LQuaternion(rot[3], rot[0], rot[1], rot[2])
                    quat.extract_to_matrix(rot_mat)
                    gltf_mat *= rot_mat

                if 'translation' in gltf_node:
                    gltf_mat *= LMatrix4.translate_mat(*gltf_node['translation'])

            panda_node.set_transform(TransformState.make_mat(self.csxform_inv * gltf_mat * self.csxform))

            np = self.node_paths.get(nodeid, root.attach_new_node(panda_node))
            self.node_paths[nodeid] = np

            if nodeid in self.skeletons:
                self.build_character(panda_node, nodeid, jvtmap, cvsmap, gltf_data)

            if 'extras' in gltf_scene and 'hidden_nodes' in gltf_scene['extras']:
                if nodeid in gltf_scene['extras']['hidden_nodes']:
                    panda_node = panda_node.make_copy()

            if 'mesh' in gltf_node:
                meshid = gltf_node['mesh']
                gltf_mesh = gltf_data['meshes'][meshid]
                mesh = self.meshes[meshid]

                # Does this mesh have weights, but are we not under a character?
                # If so, create a character just for this mesh.
                if gltf_mesh.get('weights') and not jvtmap and not cvsmap:
                    mesh_name = gltf_mesh.get('name', 'mesh'+str(meshid))
                    char = Character(mesh_name)
                    cvsmap2 = {}
                    self.build_character(char, nodeid, {}, cvsmap2, gltf_data, recurse=False)
                    self.combine_mesh_morphs(mesh, meshid, cvsmap2)

                    np.attach_new_node(char).attach_new_node(mesh)
                else:
                    np.attach_new_node(mesh)
                    if jvtmap:
                        self.combine_mesh_skin(mesh, jvtmap)
                    if cvsmap:
                        self.combine_mesh_morphs(mesh, meshid, cvsmap)

            if 'camera' in gltf_node:
                camid = gltf_node['camera']
                cam = self.cameras[camid]
                np.attach_new_node(cam)
            if 'extensions' in gltf_node:
                light_ext = None
                has_light_ext = False
                if 'KHR_lights_punctual' in gltf_node['extensions']:
                    light_ext = 'KHR_lights_punctual'
                    has_light_ext = True
                elif 'KHR_lights' in gltf_node['extensions']:
                    light_ext = 'KHR_lights'
                    has_light_ext = True
                if has_light_ext:
                    lightid = gltf_node['extensions'][light_ext]['light']
                    light = self.lights[lightid]
                    if copy_lights:
                        light = light.make_copy()
                    lnp = np.attach_new_node(light)
                    if self.compose_cs == CS_zup_right:
                        lnp.set_p(lnp.get_p() - 90)
                    lnp.set_r(lnp.get_r() - 90)
                    if isinstance(light, Light):
                        root.set_light(lnp)

                has_physics = (
                    'BLENDER_physics' in gltf_node['extensions'] or
                    'PANDA3D_physics_collision_shapes' in gltf_node['extensions']
                )
                if has_physics:
                    gltf_collisions = gltf_node['extensions'].get(
                        'PANDA3D_physics_collision_shapes',
                        gltf_node['extensions']['BLENDER_physics']
                    )
                    gltf_rigidbody = gltf_node['extensions'].get('BLENDER_physics', None)
                    if 'PANDA3D_physics_collision_shapes' in gltf_node['extensions']:
                        collision_shape = gltf_collisions['shapes'][0]
                        shape_type = collision_shape['type']
                    else:
                        collision_shape = gltf_collisions['collisionShapes'][0]
                        shape_type = collision_shape['shapeType']
                    bounding_box = collision_shape['boundingBox']
                    radius = max(bounding_box[0], bounding_box[1]) / 2.0
                    height = bounding_box[2]
                    geomnode = None
                    intangible = gltf_collisions.get('intangible', False)
                    if 'mesh' in collision_shape:
                        try:
                            geomnode = self.meshes[collision_shape['mesh']]
                        except KeyError:
                            print(
                                "Could not find physics mesh ({}) for object ({})"
                                .format(collision_shape['mesh'], nodeid)
                            )
                    if 'extensions' in gltf_data and 'BP_physics_engine' in gltf_data['extensions']:
                        use_bullet = (
                            gltf_data['extensions']['BP_physics_engine']['engine'] == 'bullet'
                        )
                    else:
                        use_bullet = self.settings.physics_engine == 'bullet'
                    if use_bullet and not HAVE_BULLET:
                        print(
                            'Warning: attempted to export for Bullet, which is unavailable, falling back to builtin'
                        )
                        use_bullet = False

                    if use_bullet:
                        phynode = self.load_physics_bullet(
                            node_name,
                            geomnode,
                            shape_type,
                            bounding_box,
                            radius,
                            height,
                            intangible,
                            gltf_rigidbody
                        )
                    else:
                        phynode = self.load_physics_builtin(
                            node_name,
                            geomnode,
                            shape_type,
                            bounding_box,
                            radius,
                            height,
                            intangible
                        )
                    if phynode is not None:
                        phynp = np.attach_new_node(phynode)
                        for geomnode in np.find_all_matches('+GeomNode'):
                            geomnode.reparent_to(phynp)
            if 'extras' in gltf_node:
                for key, value in gltf_node['extras'].items():
                    np.set_tag(key, str(value))


            for child_nodeid in gltf_node.get('children', []):
                add_node(np, gltf_scene, child_nodeid, jvtmap, cvsmap)

            # Handle visibility after children are loaded
            def visible_recursive(node, visible):
                if visible:
                    node.show()
                else:
                    node.hide()
                for child in node.get_children():
                    visible_recursive(child, visible)
            if 'extras' in gltf_scene and 'hidden_nodes' in gltf_scene['extras']:
                if nodeid in gltf_scene['extras']['hidden_nodes']:
                    #print('Hiding', np)
                    visible_recursive(np, False)
                else:
                    #print('Showing', np)
                    visible_recursive(np, True)

            # Check if we need to deal with negative scale values
            scale = panda_node.get_transform().get_scale()
            negscale = scale.x * scale.y * scale.z < 0
            if negscale:
                for geomnode in np.find_all_matches('**/+GeomNode'):
                    tmp = geomnode.get_parent().attach_new_node(PandaNode('ReverseCulling'))
                    tmp.set_attrib(CullFaceAttrib.make_reverse())
                    geomnode.reparent_to(tmp)

            # Handle parenting to joints
            joint = self.joint_parents.get(nodeid)
            if joint:
                xformnp = root.attach_new_node(PandaNode('{}-parent'.format(node_name)))
                np.reparent_to(xformnp)
                joint.add_net_transform(xformnp.node())

        for sceneid, gltf_scene in enumerate(gltf_data.get('scenes', [])):
            scene_name = gltf_scene.get('name', 'scene'+str(sceneid))
            scene_root = NodePath(ModelRoot(scene_name))

            node_list = gltf_scene['nodes']
            if 'extras' in gltf_scene and 'hidden_nodes' in gltf_scene['extras']:
                node_list += gltf_scene['extras']['hidden_nodes']

            for nodeid in node_list:
                add_node(scene_root, gltf_scene, nodeid, {}, {})

            self.scenes[sceneid] = scene_root

        # Set the active scene
        sceneid = gltf_data.get('scene', 0)
        if sceneid in self.scenes:
            self.active_scene = self.scenes[sceneid]
        if 'scenes' in gltf_data:
            gltf_scene = gltf_data['scenes'][sceneid]
            if 'extras' in gltf_scene:
                if 'background_color' in gltf_scene['extras']:
                    self.background_color = gltf_scene['extras']['background_color']
                if 'active_camera' in gltf_scene['extras']:
                    self.active_camera = gltf_scene['extras']['active_camera']

    def build_character_joints(self, char, root_nodeids, affected_nodeids, skinid, gltf_data):
        gltf_skin = gltf_data['skins'][skinid]

        def create_joint(nodeid):
            node = gltf_data['nodes'][nodeid]
            node_name = node.get('name', 'bone'+str(nodeid))

            if nodeid in gltf_skin['joints']:
                joint_index = gltf_skin['joints'].index(nodeid)
                self._joint_ids[nodeid] = joint_index

            for child in node.get('children', []):
                create_joint(child)

        for root_nodeid in root_nodeids:
            create_joint(root_nodeid)

        return {}


def load_actor(src, gltf_settings=None, converter_class=None):
    if gltf_settings is None:
        gltf_settings = GltfSettings()

    if converter_class is None:
        converter_class = KPhysConverter

    if not isinstance(src, Filename):
        src = Filename.from_os_specific(src)

    indir = Filename(src.get_dirname())
    get_model_path().prepend_directory(indir)

    converter = converter_class(indir=indir, outdir=os.getcwd(), settings=gltf_settings)

    with open(src, 'rb') as glb_file:
        if glb_file.read(4) == b'glTF':
            version, = struct.unpack('<I', glb_file.read(4))
            if version != 2:
                raise RuntimeError("Only GLB version 2 is supported, file is version {0}".format(version))

            length, = struct.unpack('<I', glb_file.read(4))

            chunk_type, chunk_data = read_glb_chunk(glb_file)
            assert chunk_type == b'JSON'
            gltf_data = json.loads(chunk_data.decode('utf-8'))

            if glb_file.tell() < length:
                #if read_bytes % 4 != 0:
                #    glb_file.read((4 - read_bytes) % 4)
                chunk_type, chunk_data = read_glb_chunk(glb_file)
                assert chunk_type == b'BIN\000'
                converter.buffers[0] = chunk_data

            converter.update(gltf_data, writing_bam=False)

        else:
            # Re-open as a text file.
            glb_file.close()

            with open(src) as gltf_file:
                gltf_data = json.load(gltf_file)
                converter.update(gltf_data, writing_bam=False)

    if gltf_settings.print_scene:
        converter.active_scene.ls()

    return converter.active_scene
