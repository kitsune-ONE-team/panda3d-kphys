"""Based on gltf.converter."""
from panda3d import core as p3d

from .. import spec


class TexturePool(object):
    textures: dict[str, p3d.Texture] = {}

    @classmethod
    def add_texture(cls, texture: p3d.Texture):
        cls.textures[texture.get_name()] = texture

    @classmethod
    def get_texture(cls, name: str | p3d.Filename) -> p3d.Texture | None:
        if isinstance(name, p3d.Filename):
            name = name.to_os_specific()
        return cls.textures.get(name)


class TextureMixin(object):
    def load_texture(self, texid: int, gltf_tex: dict, gltf_data: dict):
        if 'source' not in gltf_tex:
            print("Texture '{}' has no source, skipping".format(texid))
            return

        def rescale_image(img: p3d.PNMImage) -> p3d.PNMImage:
            if self.settings.texture_scaling == 1:
                return img

            scaled_img = p3d.PNMImage(
                int(img.get_x_size() / self.settings.texture_scaling),
                int(img.get_y_size() / self.settings.texture_scaling),
                img.get_num_channels())
            scaled_img.unfiltered_stretch_from(img)
            return scaled_img

        def load_embedded_image(name, ext, data):
            if not name:
                name = f'gltf-embedded-{texid}'
            img_type_registry = p3d.PNMFileTypeRegistry.get_global_ptr()
            img_type = img_type_registry.get_type_from_extension(ext)

            img = p3d.PNMImage()
            img.read(p3d.StringStream(data), type=img_type)

            texture = p3d.Texture(name)
            texture.load(rescale_image(img))

            return texture

        source = gltf_data['images'][gltf_tex['source']]
        if 'uri' in source:
            uri = source['uri']
            name = source.get('name', '')
            if uri.startswith('data:'):
                info, b64data = uri.split(',')

                if not (info.startswith('data:image/') and info.endswith(';base64')):
                    raise RuntimeError(
                        f'Unknown data URI: {info}'
                    )

                ext = info.replace('data:image/', '').replace(';base64', '')
                data = base64.b64decode(b64data)

                texture = load_embedded_image(name, ext, data)
            else:
                uri = p3d.Filename.from_os_specific(uri)
                fulluri = p3d.Filename(self.filedir, uri)
                fulluri.standardize()

                # texture = p3d.TexturePool.get_texture(fulluri)
                texture = TexturePool.get_texture(fulluri)
                if not texture:
                    if self.settings.texture_scaling == 1:
                        texture = p3d.TexturePool.load_texture(fulluri, 0, False, p3d.LoaderOptions())
                        texture.name = fulluri.to_os_specific()

                    else:
                        img_type_registry = p3d.PNMFileTypeRegistry.get_global_ptr()
                        img_type = img_type_registry.get_type_from_extension(fulluri.get_basename())

                        vfs = p3d.VirtualFileSystem.get_global_ptr()
                        img = p3d.PNMImage(fulluri)
                        img.read(vfs.open_read_file(fulluri, False), type=img_type)

                        texture = p3d.Texture(fulluri.to_os_specific())
                        texture.load(rescale_image(img))

                    texture.filename = texture.fullpath = fulluri

                    # p3d.TexturePool.add_texture(texture)
                    TexturePool.add_texture(texture)

        else:
            name = source.get('name', '')
            ext = source['mimeType'].split('/')[1]
            data = self.get_buffer_view(gltf_data, source['bufferView'])
            texture = load_embedded_image(name, ext, data)

        if 'sampler' in gltf_tex:
            gltf_sampler = gltf_data['samplers'][gltf_tex['sampler']]

            if 'magFilter' in gltf_sampler:
                magfilter = spec.SAMPLER_STATE_FT_MAP.get(gltf_sampler['magFilter'])
                if magfilter:
                    texture.set_magfilter(magfilter)
                else:
                    print(
                        "Sampler {} has unsupported magFilter type {}"
                        .format(gltf_tex['sampler'], gltf_sampler['magFilter'])
                    )

            if 'minFilter' in gltf_sampler:
                minfilter = spec.SAMPLER_STATE_FT_MAP.get(gltf_sampler['minFilter'])
                if minfilter:
                    texture.set_minfilter(minfilter)
                else:
                    print(
                        "Sampler {} has unsupported minFilter type {}"
                        .format(gltf_tex['sampler'], gltf_sampler['minFilter'])
                    )

            wraps = spec.SAMPLER_STATE_WM_MAP.get(gltf_sampler.get('wrapS', 10497))
            if wraps:
                texture.set_wrap_u(wraps)
            else:
                print(
                    "Sampler {} has unsupported wrapS type {}"
                    .format(gltf_tex['sampler'], gltf_sampler['wrapS'])
                )

            wrapt = spec.SAMPLER_STATE_WM_MAP.get(gltf_sampler.get('wrapT', 10497))
            if wrapt:
                texture.set_wrap_v(wrapt)
            else:
                print(
                    "Sampler {} has unsupported wrapT type {}"
                    .format(gltf_tex['sampler'], gltf_sampler['wrapT'])
                )

        self.textures[texid] = texture

    def make_texture_srgb(self, texture: p3d.Texture):
        if self.settings.no_srgb:
            return

        if texture is None:
            return

        if texture.get_num_components() == 3:
            texture.set_format(p3d.Texture.F_srgb)
        elif texture.get_num_components() == 4:
            texture.set_format(p3d.Texture.F_srgb_alpha)
