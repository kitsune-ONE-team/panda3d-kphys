"""Based on gltf.converter."""
import itertools

from panda3d import core as p3d

from .. import spec


class MeshMixin(object):
    def load_mesh(self, meshid: int, gltf_mesh: dict, gltf_data: dict):
        mesh_name = gltf_mesh.get('name', 'mesh'+str(meshid))
        node = self.meshes.get(meshid, p3d.GeomNode(mesh_name))

        # Clear any existing mesh data
        node.remove_all_geoms()

        # Load primitives
        for gltf_primitive in gltf_mesh['primitives']:
            self.load_primitive(node, gltf_primitive, meshid, gltf_mesh, gltf_data)

        # Save mesh
        self.meshes[meshid] = node

    def load_primitive(
            self, geom_node: p3d.GeomNode, gltf_primitive: dict,
            meshid: int, gltf_mesh: dict, gltf_data: dict):
        # Build Vertex Format
        vformat = p3d.GeomVertexFormat()
        mesh_attribs = gltf_primitive['attributes']
        accessors = [
            {**gltf_data['accessors'][acc_idx], '_attrib': attrib_name}
            for attrib_name, acc_idx in mesh_attribs.items()
        ]

        # Check for morph target columns.
        targets = gltf_primitive.get('targets')
        if targets:
            target_names = self.get_extras(gltf_mesh).get('targetNames', [])

            for i, target in enumerate(targets):
                if i < len(target_names):
                    target_name = target_names[i]
                else:
                    target_name = str(i)

                accessors += [
                    {**gltf_data['accessors'][acc_idx], '_attrib': attrib_name, '_target': target_name}
                    for attrib_name, acc_idx in target.items()
                ]

        accessors = sorted(accessors, key=lambda x: x['bufferView'])
        data_copies = []
        is_skinned = 'JOINTS_0' in mesh_attribs
        calc_normals = 'NORMAL' not in mesh_attribs
        calc_tangents = 'TANGENT' not in mesh_attribs
        normalize_weights = False

        for buffview, accs in itertools.groupby(accessors, key=lambda x: x['bufferView']):
            buffview = gltf_data['bufferViews'][buffview]
            accs = sorted(accs, key=lambda x: x.get('byteOffset', 0))
            is_interleaved = len(accs) > 1 and accs[1].get('byteOffset', 0) < buffview['byteStride']

            varray = p3d.GeomVertexArrayFormat()
            for acc in accs:
                # Gather column information
                attrib_parts = acc['_attrib'].lower().split('_')
                attrib_name = spec.ATTRIB_NAME_MAP.get(attrib_parts[0], attrib_parts[0])
                if attrib_name == 'texcoord' and len(attrib_parts) > 1:
                    internal_name = p3d.InternalName.make(attrib_name+'.', int(attrib_parts[1]))
                else:
                    internal_name = p3d.InternalName.make(attrib_name)
                num_components = spec.COMPONENT_NUM_MAP[acc['type']]
                numeric_type = spec.COMPONENT_TYPE_MAP[acc['componentType']]
                numeric_size = spec.COMPONENT_SIZE_MAP[acc['componentType']]
                content = spec.ATTRIB_CONTENT_MAP.get(attrib_name, p3d.GeomEnums.C_other)
                size = numeric_size * num_components

                if '_target' in acc:
                    internal_name = p3d.InternalName.get_morph(attrib_name, acc['_target'])
                    content = p3d.GeomEnums.C_morph_delta

                # Add this accessor as a column to the current vertex array format
                varray.add_column(internal_name, num_components, numeric_type, content)

                # Check if the weights table is using float or integer component
                # Weights normalization will only be performed on float weights.
                if attrib_parts[0] == 'weights':
                    normalize_weights = numeric_type == p3d.GeomEnums.NT_float32

                if not is_interleaved:
                    # Start a new vertex array format
                    vformat.add_array(varray)
                    varray = p3d.GeomVertexArrayFormat()
                    data_copies.append((
                        buffview['buffer'],
                        acc.get('byteOffset', 0) + buffview.get('byteOffset', 0),
                        acc['count'],
                        size,
                        buffview.get('byteStride', size)
                    ))

            if is_interleaved:
                vformat.add_array(varray)
                stride = buffview.get('byteStride', varray.get_stride())
                data_copies.append((
                    buffview['buffer'],
                    buffview.get('byteOffset', 0),
                    accs[0]['count'],
                    stride,
                    stride,
                ))

        # Copy data from buffers
        reg_format = p3d.GeomVertexFormat.register_format(vformat)
        # print(vformat)
        # print(reg_format)
        vdata = p3d.GeomVertexData(geom_node.name, reg_format, p3d.GeomEnums.UH_stream)

        for array_idx, data_info in enumerate(data_copies):
            buffid, start, count, size, stride = data_info

            handle = vdata.modify_array(array_idx).modify_handle()
            handle.unclean_set_num_rows(count)

            buff = self.buffers[buffid]
            end = start + count * stride
            if stride == size:
                handle.copy_data_from(buff[start:end])
            else:
                src = start
                dest = 0
                while src < end:
                    handle.copy_subdata_from(dest, size, buff[src:src+size])
                    dest += size
                    src += stride
            handle = None

        # Flip UVs
        num_uvs = len({i for i in gltf_primitive['attributes'] if i.startswith('TEXCOORD')})
        for i in range(num_uvs):
            uv_data = p3d.GeomVertexRewriter(vdata, p3d.InternalName.get_texcoord_name(str(i)))

            while not uv_data.is_at_end():
                uvs = uv_data.get_data2f()
                uv_data.set_data2f(uvs[0], 1 - uvs[1])

        if self.compose_cs == p3d.CS_yup_right:
            # Flip morph deltas from Y-up to Z-up.  This is apparently not done by
            # transform_vertices(), below, so we do it ourselves.
            for morph_i in range(reg_format.get_num_morphs()):
                delta_data = p3d.GeomVertexRewriter(vdata, reg_format.get_morph_delta(morph_i))

                while not delta_data.is_at_end():
                    data = delta_data.get_data3f()
                    delta_data.set_data3f(data[0], -data[2], data[1])
            # Flip tangents from Y-up to Z-up.
            if 'TANGENT' in mesh_attribs:
                tangent = p3d.GeomVertexRewriter(vdata, p3d.InternalName.make('tangent'))
                while not tangent.is_at_end():
                    data = tangent.get_data4f()
                    tangent.set_data4f(data[0], -data[2], data[1], data[3])

        # get joint indices
        joints = []
        gltf_skin = {}
        for gltf_node in gltf_data['nodes']:
            if gltf_node.get('mesh') == meshid and 'skin' in gltf_node:
                skinid = gltf_node['skin']
                gltf_skin = gltf_data['skins'][skinid]
                joints = gltf_skin['joints']

        # reindex joints
        if joints:
            indices_data = p3d.GeomVertexRewriter(
                vdata, p3d.InternalName.get_transform_index())
            weights_data = p3d.GeomVertexRewriter(
                vdata, p3d.InternalName.get_transform_weight())

            while not (indices_data.is_at_end() and weights_data.is_at_end()):
                indices_old = indices_data.get_data4f()
                weights_old = weights_data.get_data4f()

                indices_new = [0] * 4
                weights_new = list(weights_old)

                for i in range(4):
                    jointid = int(indices_old[i])
                    nodeid = joints[jointid]
                    indices_new[i] = self.joints[nodeid]

                indices_data.set_data4f(*indices_new)
                weights_data.set_data4f(*weights_new)

        if normalize_weights:
            # The linear sum of all the joint weights must be as close as possible to 1, if the weights are
            # stored as float.
            # Some malformed assets do not respect this, hence we are normalizing them here.
            weights_data = p3d.GeomVertexRewriter(
                vdata, p3d.InternalName.get_transform_weight())
            while not weights_data.is_at_end():
                weights = weights_data.get_data4f()
                max_weight = sum(map(abs, list(weights)))
                if max_weight != 0.0:
                    weights = weights / max_weight
                weights_data.set_data4f(weights)

        # Repack mesh data
        vformat = p3d.GeomVertexFormat()
        varray_vert = p3d.GeomVertexArrayFormat()
        varray_skin = p3d.GeomVertexArrayFormat()
        varray_vertex_morph = p3d.GeomVertexArrayFormat()
        varray_normal_morph = p3d.GeomVertexArrayFormat()

        skin_columns = (
            p3d.InternalName.get_transform_index(),
            p3d.InternalName.get_transform_weight(),
            p3d.InternalName.get_transform_blend()
        )

        for arr in reg_format.get_arrays():
            for column in arr.get_columns():
                iname: p3d.InternalName = column.get_name()
                if iname in skin_columns:
                    varray = varray_skin
                elif iname.get_name().startswith('vertex.morph.'):
                    varray = varray_vertex_morph
                elif iname.get_name().startswith('normal.morph.'):
                    varray = varray_normal_morph
                else:
                    varray = varray_vert

                varray.add_column(
                    column.get_name(),
                    column.get_num_components(),
                    column.get_numeric_type(),
                    column.get_contents()
                )
        vformat.add_array(varray_vert)

        if is_skinned or targets:
            aspec = p3d.GeomVertexAnimationSpec()
            aspec.set_panda()
            vformat.set_animation(aspec)

        if is_skinned:
            varray_blends = p3d.GeomVertexArrayFormat()
            varray_blends.add_column(p3d.InternalName.get_transform_blend(), 1, p3d.GeomEnums.NT_uint16, p3d.GeomEnums.C_index)

            vformat.add_array(varray_blends)
            vformat.add_array(varray_skin)

        reg_format = p3d.GeomVertexFormat.register_format(vformat)
        vdata = vdata.convert_to(reg_format)

        # Construct primitive
        primitiveid = geom_node.get_num_geoms()
        primitivemode = gltf_primitive.get('mode', 4)
        try:
            prim = spec.PRIMITIVE_MODE_MAP[primitivemode](p3d.GeomEnums.UH_static)
        except KeyError:
            print(
                "Warning: primitive {} on mesh {} has an unsupported mode: {}"
                .format(primitiveid, geom_node.name, primitivemode)
            )
            return

        if 'indices' in gltf_primitive:
            index_acc = gltf_data['accessors'][gltf_primitive['indices']]
            prim.set_index_type(spec.COMPONENT_TYPE_MAP[index_acc['componentType']])

            handle = prim.modify_vertices(index_acc['count']).modify_handle()
            handle.unclean_set_num_rows(index_acc['count'])

            buffview = gltf_data['bufferViews'][index_acc['bufferView']]
            buff = self.buffers[buffview['buffer']]
            start = buffview.get('byteOffset', 0) + index_acc.get('byteOffset', 0)
            end = start + index_acc['count'] * buffview.get('byteStride', 1) * prim.index_stride
            handle.copy_data_from(buff[start:end])
            handle = None
        else:
            index_acc = gltf_data['accessors'][gltf_primitive['attributes']["POSITION"]]
            start = index_acc.get('byteOffset', 0)
            prim.setNonindexedVertices(start, index_acc['count'])

        # Assign a material
        mat = None
        matid = gltf_primitive.get('material', None)
        if matid is not None and matid in self.mat_states:
            mat = self.mat_states[gltf_primitive['material']]
            self.mat_mesh_map[gltf_primitive['material']].append((
                geom_node.name, primitiveid))

        geom = p3d.Geom(vdata)
        geom.add_primitive(prim)
        if calc_normals:
            self.calculate_normals(geom)
        if calc_tangents:
            self.calculate_tangents(geom)
        geom.transform_vertices(self.csxform)
        if mat:
            geom_node.add_geom(geom, mat)
        else:
            geom_node.add_geom(geom)

    def read_vert_data(self, gvd, column_name):
        gvr = p3d.GeomVertexReader(gvd, column_name)
        data = []
        while not gvr.is_at_end():
            data.append(p3d.LVecBase4(gvr.get_data4()))
        return data

    def calculate_normals(self, geom: p3d.Geom):
        # Generate flat normals, as required by the glTF spec.
        if geom.get_primitive_type() != p3d.GeomEnums.PT_polygons:
            return

        # We need to deindex the primitive since each occurrence of a vertex on
        # a triangle could have a different normal vector.
        geom.decompose_in_place()
        geom.make_nonindexed(False)

        gvd = geom.get_vertex_data()
        gvd = gvd.replace_column(
            p3d.InternalName.get_normal(), 3,
            p3d.GeomEnums.NT_float32, p3d.GeomEnums.C_normal)
        vertex_reader = p3d.GeomVertexReader(gvd, 'vertex')
        normal_writer = p3d.GeomVertexWriter(gvd, 'normal')

        read_vertex = vertex_reader.get_data3
        write_normal = normal_writer.set_data3

        while not vertex_reader.is_at_end():
            vtx1 = read_vertex()
            vtx2 = read_vertex()
            vtx3 = read_vertex()
            normal = (vtx2 - vtx1).cross(vtx3 - vtx1)
            normal.normalize()
            write_normal(normal)
            write_normal(normal)
            write_normal(normal)

        geom.set_vertex_data(gvd)

    def calculate_tangents(self, geom: p3d.Geom):
        # Adapted from https://www.marti.works/calculating-tangents-for-your-mesh/
        prim = geom.get_primitive(0)
        gvd = geom.get_vertex_data()
        gvd = gvd.replace_column(
            p3d.InternalName.get_tangent(), 4,
            p3d.GeomEnums.NT_float32, p3d.GeomEnums.C_other)
        tangent_writer = p3d.GeomVertexWriter(gvd, p3d.InternalName.get_tangent())

        primverts = prim.get_vertex_list()
        tris = [primverts[i:i+3] for i in range(0, len(primverts), 3)]
        posdata = self.read_vert_data(gvd, p3d.InternalName.get_vertex())
        normaldata = [
            p3d.LVector3(i[0], i[1], i[2])
            for i in self.read_vert_data(gvd, p3d.InternalName.get_normal())
        ]
        uvdata = [
            p3d.LVector2(i[0], i[1])
            for i in self.read_vert_data(gvd, p3d.InternalName.get_texcoord_name('0'))
        ]
        tana = [p3d.LVector3(0, 0, 0) for i in range(len(posdata))]
        tanb = [p3d.LVector3(0, 0, 0) for i in range(len(posdata))]

        if not uvdata:
            # No point generating tangents without UVs.
            return

        # Gather tangent data from triangles
        for tri in tris:
            idx0, idx1, idx2 = tri
            edge1 = posdata[idx1] - posdata[idx0]
            edge2 = posdata[idx2] - posdata[idx0]
            duv1 = uvdata[idx1] - uvdata[idx0]
            duv2 = uvdata[idx2] - uvdata[idx0]

            denom = duv1.x * duv2.y - duv2.x * duv1.y
            if denom != 0.0:
                fconst = 1.0 / denom
                tangent = (edge1.xyz * duv2.y - edge2.xyz * duv1.y) * fconst
                bitangent = (edge2.xyz * duv1.x - edge1.xyz * duv2.x) * fconst
            else:
                tangent = p3d.LVector3(0)
                bitangent = p3d.LVector3(0)

            for idx in tri:
                tana[idx] += tangent
                tanb[idx] += bitangent

        # Calculate per-vertex tangent values
        for normal, tan0, tan1 in zip(normaldata, tana, tanb):
            tangent = tan0 - (normal * normal.dot(tan0))
            tangent.normalize()

            tangent4 = p3d.LVector4(
                tangent.x,
                tangent.y,
                tangent.z,
                -1.0 if normal.cross(tan0).dot(tan1) < 0 else 1.0
            )
            if self.compose_cs == p3d.CS_yup_right:
                tangent_writer.set_data4(tangent4[0], -tangent4[2], tangent4[1], tangent4[3])
            else:
                tangent_writer.set_data4(tangent4)

        geom.set_vertex_data(gvd)
