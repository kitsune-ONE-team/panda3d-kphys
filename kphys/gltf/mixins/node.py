"""Based on gltf.converter."""
from typing import Optional

from ...core import ArmatureNode, BoneNode, WiggleBoneNode

from panda3d import bullet, core as p3d


class NodeMixin(object):
    def build_character_joints(
            self, nodeid: int, gltf_node: dict, gltf_data: dict,
            charid: Optional[int] = None):
        """Build joint index maps."""
        if nodeid in self.joints:
            if charid is not None:
                # we are tracking some skeleton, keep tracking it's bone index
                if charid not in self.character_joints:
                    self.character_joints[charid] = 0
                self.joints[nodeid] = self.character_joints[charid]
                self.character_joints[charid] += 1

        elif nodeid in self.skeletons:
            # start tracking current skeleton and remember node ID
            charid = nodeid

        for child_nodeid in gltf_node.get('children', []):
            self.build_character_joints(
                child_nodeid, gltf_data['nodes'][child_nodeid],
                gltf_data, charid)

    def create_character(self, nodeid: int, gltf_node: dict, gltf_data: dict):
        """Crate character object."""
        node_name = gltf_node.get('name', 'node'+str(nodeid))
        return ArmatureNode(node_name)

    def create_joint(
            self, nodeid: int, gltf_node: dict, gltf_data: dict,
            is_spring: bool = False):
        """Create character joint."""
        node_name = gltf_node.get('name', 'node'+str(nodeid))
        if is_spring:
            return WiggleBoneNode(node_name, self.joints[nodeid])
        else:
            return BoneNode(node_name, self.joints[nodeid])

    def add_node(
            self, parent_np: p3d.NodePath, gltf_scene: dict,
            nodeid: int, gltf_data: dict, is_spring: bool = False):
        try:
            gltf_node = gltf_data['nodes'][nodeid]
        except IndexError:
            print("Could not find node with index: {}".format(nodeid))
            return

        scene_extras = self.get_extras(gltf_scene)
        node_name = gltf_node.get('name', 'node'+str(nodeid))

        if nodeid in self.joints:
            panda_node = self.create_joint(nodeid, gltf_node, gltf_data, is_spring)

        elif nodeid in self.skeletons:
            panda_node = self.create_character(nodeid, gltf_node, gltf_data)

        else:
            panda_node = p3d.PandaNode(node_name)

        if nodeid in self.spring_bones:
            is_spring = True

        # Determine the transformation.
        if 'matrix' in gltf_node:
            gltf_mat = p3d.LMatrix4(*gltf_node.get('matrix'))
        else:
            gltf_mat = p3d.LMatrix4(p3d.LMatrix4.ident_mat())
            if 'scale' in gltf_node:
                gltf_mat.set_scale_mat(tuple(gltf_node['scale']))

            if 'rotation' in gltf_node:
                rot_mat = p3d.LMatrix4()
                rot = gltf_node['rotation']
                quat = p3d.LQuaternion(rot[3], rot[0], rot[1], rot[2])
                quat.extract_to_matrix(rot_mat)
                gltf_mat *= rot_mat

            if 'translation' in gltf_node:
                gltf_mat *= p3d.LMatrix4.translate_mat(*gltf_node['translation'])

        panda_node.set_transform(p3d.TransformState.make_mat(
            self.csxform_inv * gltf_mat * self.csxform))

        np = self.node_paths.get(nodeid, parent_np.attach_new_node(panda_node))
        self.node_paths[nodeid] = np

        if 'hidden_nodes' in scene_extras:
            if nodeid in scene_extras['hidden_nodes']:
                panda_node = panda_node.make_copy()

        if 'mesh' in gltf_node:
            meshid = gltf_node['mesh']
            gltf_mesh = gltf_data['meshes'][meshid]
            mesh = self.meshes[meshid]
            np.attach_new_node(mesh)

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
                lnp = np.attach_new_node(light)
                if self.compose_cs == p3d.CS_zup_right:
                    lnp.set_p(lnp.get_p() - 90)
                lnp.set_r(lnp.get_r() - 90)
                # if isinstance(light, p3d.Light):
                #     root.set_light(lnp)

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
                if phynode is not None:
                    phynp = np.attach_new_node(phynode)
                    for geomnode in np.find_all_matches('+GeomNode'):
                        # geomnode.reparent_to(phynp)
                        geomnode.remove_node()

        for key, value in self.get_extras(gltf_node).items():
            np.set_tag(key, str(value))

        for child_nodeid in gltf_node.get('children', []):
            self.add_node(np, gltf_scene, child_nodeid, gltf_data, is_spring)

        # Handle visibility after children are loaded
        def visible_recursive(node, visible):
            if visible:
                node.show()
            else:
                node.hide()
            for child in node.get_children():
                visible_recursive(child, visible)

        hidden_nodes = scene_extras.get('hidden_nodes', [])
        if nodeid in hidden_nodes:
            visible_recursive(np, False)
        else:
            visible_recursive(np, True)

        # Check if we need to deal with negative scale values
        scale = panda_node.get_transform().get_scale()
        negscale = scale.x * scale.y * scale.z < 0
        if negscale:
            for geomnode in np.find_all_matches('**/+GeomNode'):
                tmp = geomnode.get_parent().attach_new_node(p3d.PandaNode('ReverseCulling'))
                tmp.set_attrib(p3d.CullFaceAttrib.make_reverse())
                geomnode.reparent_to(tmp)

    def move_skinned_nodes(self, nodeid: int, gltf_node: dict, gltf_data: dict):
        """
        Move skinned nodes inside the skeleton tree when they out of tree,
        but have the 'skeleton' option specified.
        """
        if 'skin' not in gltf_node:
            return

        skinid = gltf_node['skin']
        gltf_skin = gltf_data['skins'][skinid]
        if 'skeleton' not in gltf_skin:
            return

        np = self.node_paths[nodeid]
        mat = np.get_mat(self.active_scene)

        skeletonid = gltf_skin['skeleton']
        skeleton_np = self.node_paths[skeletonid]
        while skeleton_np:
            if skeleton_np and isinstance(skeleton_np.node(), ArmatureNode):
                break
            skeleton_np = skeleton_np.get_parent()

        np.reparent_to(skeleton_np or self.node_paths[skeletonid])
        np.set_mat(self.active_scene, mat)

    def load_physics_bullet(
            self,
            node_name: str, geomnode: Optional[p3d.GeomNode],
            shape_type: str, bounding_box: Optional[list],
            radius: float, height: float, intangible: bool,
            gltf_rigidbody: Optional[dict]):
        shape = None
        static = gltf_rigidbody is not None and 'static' in gltf_rigidbody and gltf_rigidbody['static']

        if shape_type == 'BOX':
            shape = bullet.BulletBoxShape(p3d.LVector3(*bounding_box) / 2.0)
        elif shape_type == 'SPHERE':
            shape = bullet.BulletSphereShape(max(bounding_box) / 2.0)
        elif shape_type == 'CAPSULE':
            shape = bullet.BulletCapsuleShape(radius, height - 2.0 * radius, bullet.ZUp)
        elif shape_type == 'CYLINDER':
            shape = bullet.BulletCylinderShape(radius, height, bullet.ZUp)
        elif shape_type == 'CONE':
            shape = bullet.BulletConeShape(radius, height, bullet.ZUp)
        elif shape_type == 'CONVEX_HULL':
            if geomnode:
                shape = bullet.BulletConvexHullShape()

                for geom in geomnode.get_geoms():
                    shape.add_geom(geom)
        elif shape_type == 'MESH':
            if geomnode:
                mesh = bullet.BulletTriangleMesh()
                for geom in geomnode.get_geoms():
                    mesh.add_geom(geom)
                shape = bullet.BulletTriangleMeshShape(mesh, dynamic=not static)
        else:
            print("Unknown collision shape ({}) for object ({})".format(shape_type, node_name))

        if shape is not None:
            if intangible:
                phynode = bullet.BulletGhostNode(node_name)
            else:
                phynode = bullet.BulletRigidBodyNode(node_name)
            phynode.add_shape(shape)
            if not static:
                mass = 1.0 if gltf_rigidbody is None else gltf_rigidbody.get('mass', 1.0)
                phynode.set_mass(mass)
            return phynode
        else:
            print("Could not create collision shape for object ({})".format(node_name))
