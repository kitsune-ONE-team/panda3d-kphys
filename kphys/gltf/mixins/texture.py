"""Based on gltf.converter."""
from panda3d import core as p3d

from .. import spec


class TextureMixin(object):
    def load_texture(self, texid: int, gltf_tex: dict, gltf_data: dict):
        if 'source' not in gltf_tex:
            print("Texture '{}' has no source, skipping".format(texid))
            return

        def load_embedded_image(name, ext, data):
            if not name:
                name = f'gltf-embedded-{texid}'
            img_type_registry = p3d.PNMFileTypeRegistry.get_global_ptr()
            img_type = img_type_registry.get_type_from_extension(ext)

            img = p3d.PNMImage()
            img.read(p3d.StringStream(data), type=img_type)

            texture = p3d.Texture(name)
            texture.load(img)

            return texture

        source = gltf_data['images'][gltf_tex['source']]
        if 'uri' in source:
            uri = source['uri']
            if uri.startswith('data:'):
                info, b64data = uri.split(',')

                if not (info.startswith('data:image/') and info.endswith(';base64')):
                    raise RuntimeError(
                        f'Unknown data URI: {info}'
                    )

                name = source.get('name', '')
                ext = info.replace('data:image/', '').replace(';base64', '')
                data = base64.b64decode(b64data)

                texture = load_embedded_image(name, ext, data)
            else:
                uri = p3d.Filename.from_os_specific(uri)
                fulluri = p3d.Filename(self.filedir, uri)
                fulluri.standardize()
                texture = p3d.TexturePool.load_texture(fulluri, 0, False, p3d.LoaderOptions())
                texture.filename = texture.fullpath = fulluri
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
