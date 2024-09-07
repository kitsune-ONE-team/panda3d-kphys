"""Based on gltf.converter."""
from panda3d import core as p3d


class MaterialMixin(object):
    def load_material(self, matid, gltf_mat):
        matname = gltf_mat.get('name', 'mat'+str(matid))
        state = self.mat_states.get(matid, p3d.RenderState.make_empty())

        if matid not in self.mat_mesh_map:
            self.mat_mesh_map[matid] = []

        pmat = p3d.Material(matname)
        texinfos = []
        mat_extensions = gltf_mat.get('extensions', {})

        if 'BP_materials_legacy' in mat_extensions:
            matsettings = mat_extensions['BP_materials_legacy']['bpLegacy']
            pmat.set_shininess(matsettings['shininessFactor'])
            pmat.set_ambient(p3d.LColor(*matsettings['ambientFactor']))

            if 'diffuseTexture' in matsettings:
                texinfo = matsettings['diffuseTexture']
                texinfo['mode'] = p3d.TextureStage.M_modulate
                if matsettings['diffuseTextureSrgb'] and texinfo['index'] in self.textures:
                    self.make_texture_srgb(self.textures[texinfo['index']])
                texinfos.append(texinfo)
            else:
                pmat.set_diffuse(p3d.LColor(*matsettings['diffuseFactor']))

            if 'emissionTexture' in matsettings:
                texinfo = matsettings['emissionTexture']
                texinfo['mode'] = p3d.TextureStage.M_emission
                if matsettings['emissionTextureSrgb'] and texinfo['index'] in self.textures:
                    self.make_texture_srgb(self.textures[texinfo['index']])
                texinfos.append(texinfo)
            else:
                pmat.set_emission(p3d.LColor(*matsettings['emissionFactor']))

            if 'specularTexture' in matsettings:
                texinfo = matsettings['specularTexture']
                if matsettings['specularTextureSrgb'] and texinfo['index'] in self.textures:
                    self.make_texture_srgb(self.textures[texinfo['index']])
                texinfos.append(texinfo)
            else:
                pmat.set_specular(p3d.LColor(*matsettings['specularFactor']))

        elif 'pbrMetallicRoughness' in gltf_mat:
            pbrsettings = gltf_mat['pbrMetallicRoughness']

            pmat.set_base_color(p3d.LColor(
                *pbrsettings.get('baseColorFactor', [1.0, 1.0, 1.0, 1.0])))
            texinfo = pbrsettings.get('baseColorTexture')
            if texinfo:
                if texinfo['index'] in self.textures:
                    self.make_texture_srgb(self.textures[texinfo['index']])
                texinfos.append(texinfo)

            pmat.set_metallic(pbrsettings.get('metallicFactor', 1.0))
            pmat.set_roughness(pbrsettings.get('roughnessFactor', 1.0))
            texinfo = pbrsettings.get('metallicRoughnessTexture')
            if texinfo:
                texinfo['mode'] = p3d.TextureStage.M_selector
                texinfos.append(texinfo)

        # Normal map
        texinfo = gltf_mat.get('normalTexture')
        if texinfo:
            texinfo['mode'] = p3d.TextureStage.M_normal
            texinfos.append(texinfo)

        # Emission map
        pmat.set_emission(p3d.LColor(
            *gltf_mat.get('emissiveFactor', [0.0, 0.0, 0.0]), w=0.0))
        texinfo = gltf_mat.get('emissiveTexture')
        if texinfo:
            texinfo['mode'] = p3d.TextureStage.M_emission
            if texinfo['index'] in self.textures:
                self.make_texture_srgb(self.textures[texinfo['index']])
            texinfos.append(texinfo)

        # Index of refraction
        ior_ext = mat_extensions.get('KHR_materials_ior', {})
        pmat.set_refractive_index(ior_ext.get('ior', 1.5))

        double_sided = gltf_mat.get('doubleSided', False)
        pmat.set_twoside(double_sided)

        state = state.set_attrib(p3d.MaterialAttrib.make(pmat))

        if double_sided:
            state = state.set_attrib(p3d.CullFaceAttrib.make(p3d.CullFaceAttrib.MCullNone))

        # Setup textures
        tex_attrib = p3d.TextureAttrib.make()
        tex_mat_attrib = None
        for i, texinfo in enumerate(texinfos):
            texdata = self.textures.get(texinfo['index'], None)
            if texdata is None:
                print("Could not find texture for key: {}".format(texinfo['index']))
                continue

            texstage = p3d.TextureStage(str(i))
            texstage.set_sort(i)
            texstage.set_texcoord_name(p3d.InternalName.get_texcoord_name(
                str(texinfo.get('texCoord', 0))))
            texstage.set_mode(texinfo.get('mode', p3d.TextureStage.M_modulate))
            tex_attrib = tex_attrib.add_on_stage(texstage, texdata)

            transform_ext = texinfo.get('extensions', {}).get('KHR_texture_transform')
            if transform_ext:
                if 'texCoord' in transform_ext:
                    # This overrides, if present.
                    texstage.set_texcoord_name(p3d.InternalName.get_texcoord_name(
                        str(transform_ext['texCoord'])))

                # glTF uses a transform origin of the upper-left corner of the
                # texture, whereas Panda uses the lower-left corner.
                mat = p3d.Mat3()
                scale = transform_ext.get('scale')
                if scale:
                    mat *= (p3d.Mat3.translate_mat(0, -1) *
                            p3d.Mat3.scale_mat(scale[0], scale[1]) *
                            p3d.Mat3.translate_mat(0, 1))

                rot = transform_ext.get('rotation')
                if rot:
                    mat *= (p3d.Mat3.translate_mat(0, -1) *
                            p3d.Mat3.rotate_mat(math.degrees(rot)) *
                            p3d.Mat3.translate_mat(0, 1))

                offset = transform_ext.get('offset', [0, 0])
                if offset:
                    mat *= p3d.Mat3.translate_mat(offset[0], -offset[1])

                transform = p3d.TransformState.make_mat3(mat)
                if not tex_mat_attrib:
                    tex_mat_attrib = p3d.TexMatrixAttrib.make(texstage, transform)
                else:
                    tex_mat_attrib = tex_mat_attrib.add_stage(texstage, transform)

        state = state.set_attrib(tex_attrib)
        if tex_mat_attrib:
            state = state.set_attrib(tex_mat_attrib)

        # Setup Alpha mode
        alpha_mode = gltf_mat.get('alphaMode', 'OPAQUE')
        if alpha_mode == 'MASK':
            alpha_cutoff = gltf_mat.get('alphaCutoff', 0.5)
            alpha_attrib = p3d.AlphaTestAttrib.make(
                p3d.AlphaTestAttrib.M_greater_equal, alpha_cutoff)
            state = state.set_attrib(alpha_attrib)
        elif alpha_mode == 'BLEND':
            transp_attrib = p3d.TransparencyAttrib.make(p3d.TransparencyAttrib.M_alpha)
            state = state.set_attrib(transp_attrib)
        elif alpha_mode != 'OPAQUE':
            print(
                "Warning: material {} has an unsupported alphaMode: {}"
                .format(matid, alpha_mode)
            )

        # Remove stale meshes
        self.mat_mesh_map[matid] = [
            pair for pair in self.mat_mesh_map[matid] if pair[0] in self.meshes
        ]

        # Reload the material
        for meshid, geom_idx in self.mat_mesh_map[matid]:
            self.meshes[meshid].set_geom_state(geom_idx, state)

        self.mat_states[matid] = state
